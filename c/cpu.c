/* cpu.c (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  16:10 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#include "cpu.h"
#include "memory.h"

#define OPTSTR "dt:"
extern char *optarg;
extern int opterr;

FILE *console;


typedef void (*instr_f)(state_t *);

void load_value(state_t *state);
void load_addr(state_t *state);
void load_indirect(state_t *state);
void load_index_x(state_t *state);
void load_index_y(state_t *state);
void load_sp_x(state_t *state);
void store_addr(state_t *state);
void get(state_t *state);
void put_port(state_t *state);
void add_x(state_t *state);
void add_y(state_t *state);
void sub_x(state_t *state);
void sub_y(state_t *state);
void copy_to_x(state_t *state);
void copy_from_x(state_t *state);
void copy_to_y(state_t *state);
void copy_from_y(state_t *state);
void copy_to_sp(state_t *state);
void copy_from_sp(state_t *state);
void jump_addr(state_t *state);
void jump_eq_addr(state_t *state);
void jump_ne_addr(state_t *state);
void call_addr(state_t *state);
void ret(state_t *state);
void inc_x(state_t *state);
void dec_x(state_t *state);
void push(state_t *state);
void pop(state_t *state);
void interrupt(state_t *state);
void iret(state_t *state);

void timer_interrupt(state_t *state);

int  read_memory(int address, int mode);
void write_memory(int address, int value, int mode);

void dump_state(FILE *fp, state_t *state);

instr_f dispatch_table[30] = {
  load_value,
  load_addr,
  load_indirect,
  load_index_x,
  load_index_y,
  load_sp_x,
  store_addr,
  get,
  put_port,
  add_x,
  add_y,
  sub_x,
  sub_y,
  copy_to_x,
  copy_from_x,
  copy_to_y,
  copy_from_y,
  copy_to_sp,
  copy_from_sp,
  jump_addr,
  jump_eq_addr,
  jump_ne_addr,
  call_addr,
  ret,
  inc_x,
  dec_x,
  push,
  pop,
  interrupt,
  iret
};


int main(int argc, char *argv[])
{
  int      opt;
  state_t  state = STATE_INITIALIZER;
  char    *filename = NULL;
  int      result;

  console = stderr;

  while((opt = getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 't':
	state.timer = atoi(optarg);
	break;
      case 'd':
	state.debug = 1;
	break;
      case '?':
      case 'h':
	fprintf(stderr,"Usage: %s\n", basename(argv[0]));
	exit(EXIT_FAILURE);
	/* NOTREACHED */
	break;
    }


  while (1) {

    /* fetch */

    if (state.debug)
      dump_state(stderr, &state);

    if ((state.cycles % state.timer) == 0)
      timer_interrupt(&state);
    
    state.ir = read_memory(state.pc++, state.mode);
    state.cycles ++;

    /* decode, execute */
    
    if (state.ir == END) {
      // make an error noise here.
      break;
    }

    if ((state.ir < 0) || (state.ir > 30)) {
      // make an error noise here.
      break;
    }

    dispatch_table[state.ir](&state);    
  }
  
  return EXIT_SUCCESS;
}
 
int read_memory(int address, int mode)
{
  io_t io  = {address, 0, 0, IO_RD, 0};

  if ((mode == USER) && (address >= INTERRUPT_PROGRAM_LOAD)) {
    // make an error noise here
    exit(EXIT_FAILURE);
  }
  
  if (write(STDOUT_FILENO, &io, sizeof(io)) < 0) {
    // make an error noise here
    exit(EXIT_FAILURE);
  }
  if (read(STDIN_FILENO, &io, sizeof(io)) < 0) {
    // make an error noise here
    exit(EXIT_FAILURE);    
  }

  return io.value;
}

void write_memory(int address, int value, int mode)
{
  io_t io = {address, value, 0, IO_WR, 0};

  if ((mode == USER) && (address >= INTERRUPT_PROGRAM_LOAD)) {
    // make an error noise here
    exit(EXIT_FAILURE);
  }

  if (write(STDOUT_FILENO, &io, sizeof(io)) < 0) {
    // make an error noise here
    exit(EXIT_FAILURE);
  }

  if (read(STDIN_FILENO, &io, sizeof(io)) < 0) {
    // make an error noise here
    exit(EXIT_FAILURE);
  }
}

void dump_state(FILE *fp, state_t *state)
{

  fprintf(fp, "[mode] %c [debug]: %d [ienable] %d\n",
	  state->mode?'K':'U',
	  state->debug,
	  state->ienable);  
  
  fprintf(fp, "[ir] %08d [pc] %08d [sp] %08d\n",
	  state->ir, state->pc, state->sp);

  fprintf(fp, "[ac] %08d [ x] %08d [ y] %08d\n",
	  state->ac, state->x, state->y);
  
  fprintf(fp, "[timer] %08d [cycles] %08d [tintr] %d\n",
	  state->timer,
	  state->cycles,
	  (state->cycles % state->timer) == 0);
}


void load_value(state_t *state)
{
  /* Load value: Load the value into AC */
  state->ac = read_memory(state->pc++, state->mode);
}

void load_addr(state_t *state)
{
  /* Load addr: Load the value at address into AC */
  int addr;
  
  addr = read_memory(state->pc++, state->mode);
  state->ac = read_memory(addr, state->mode);
}

void load_indirect(state_t *state)
{
  /* LoadInd addr: Load value from address at address */
  int addr;
  
  addr = read_memory(state->pc++, state->mode);
  addr = read_memory(addr, state->mode);
  state->ac = read_memory(addr, state->mode);
}

void load_index_x(state_t *state)
{
  /* LoadIdxX addr: Load value (addr+X) into AC */
  int addr;
  

  addr = read_memory(state->pc++, state->mode);
  state->ac = read_memory(addr + state->x, state->mode);
}

void load_index_y(state_t *state)
{
  /* LoadIdxY addr: Load value (addr+Y) into AC */
  int addr;
  
  addr = read_memory(state->pc++, state->mode);
  state->ac = read_memory(addr + state->y, state->mode);
}

void load_sp_x(state_t *state)
{
  /* LoadSpX: Load from (Sp+X) info AC */
  state->ac = read_memory(state->sp + state->x, state->mode);
}

void store_addr(state_t *state)
{
  /* Store addr: Store AC into address */
  int addr;

  addr = read_memory(state->pc++, state->mode);
  write_memory(addr, state->ac, state->mode);  
}

void get(state_t *state)
{
  /* Get: random into from 1 to 100 into AC */
  state->ac = 1 + (rand() % 100);
}

void put_port(state_t *state)
{
  /* Put port: port1, write AC as int, port2 write AC as char */
  int port;
  
  port = read_memory(state->pc++, state->mode);
  switch(port) {
    case 1:
      fprintf(console, "%d", state->ac);
      break;
    case 2:
      fprintf(console, "%c", state->ac);
      break;
    default:
      break;
  }
}

void add_x(state_t *state)
{
  /* AddX: add X to AC */
  state->ac += state->x;  
}

void add_y(state_t *state)
{
  /* AddY: add Y to AC */
  state->ac += state->y;
}

void sub_x(state_t *state)
{
  /* SubX: subtract X from AC */
  state->ac -= state->x;
}

void sub_y(state_t *state)
{
  /* SubY: subtract Y from AC */
  state->ac -= state->y;
}

void copy_to_x(state_t *state)
{
  /* CopyToX: Copy AC to X */
  state->x = state->ac;
}

void copy_from_x(state_t *state)
{
  /* CopyFromX: Copy X to AC */
  state->ac = state->x;
}

void copy_to_y(state_t *state)
{
  /* CopyToY: Copy AC to Y */
  state->y = state->ac;
}

void copy_from_y(state_t *state)
{
  /* CopyFromY: Copy Y to AC */
  state->ac = state->y;
}

void copy_to_sp(state_t *state)
{
  /* CopyToSP: Copy AC to SP */
  state->sp = state->ac;
}

void copy_from_sp(state_t *state)
{
  /* CopyFromSP: Copy SP to AC */
  state->ac = state->sp;
}

void jump_addr(state_t *state)
{
  /* Jump Addr: Jump to the address */

  state->pc = read_memory(state->pc++, state->mode);
}

void jump_eq_addr(state_t *state)
{
  /* JumpIfEqual addr: jump to addr if AC is zero */
  int addr;
  
  addr = read_memory(state->pc++, state->mode);
  if (state->ac == 0)
    state->pc = addr;
}

void jump_ne_addr(state_t *state)
{
  /* JumpIfNotEqual addr: Jump to addr if AC is not zero */
  int addr;
  
  addr = read_memory(state->pc++, state->mode);
  if (state->ac != 0)
    state->pc = addr;
}

void call_addr(state_t *state)
{
  /* Call addr: Push return address onto stack, jump to addr */
  write_memory(state->sp--, state->pc+2, state->mode);
  state->pc = read_memory(state->pc+1, state->mode);
}

void ret(state_t *state)
{
  /* Ret: Pop return address from stack, jump to address */
  state->pc = read_memory(state->sp++, state->mode);
}

void inc_x(state_t *state)
{
  /* IncX: increment the value in X */
  state->x++;
}

void dec_x(state_t *state)
{
  /* DecX: decrement the value in X */
  state->x++;
}

void push(state_t *state)
{
  /* Push: push AC onto Stack */
  write_memory(state->sp--, state->ac, state->mode);
}

void pop(state_t *state)
{
  /* Pop: pop stack into AC */
  state->ac = read_memory(state->sp++, state->mode);
}

void interrupt(state_t *state)
{
  /* Int: perform system call */
  int sp;
  
  if (!state->ienable)
    return;
  
  state->mode = SYSTEM;
  state->ienable = 0;

  sp = state->sp;
  state->sp = SSTACK_BASE;
  write_memory(state->sp--, sp, state->mode);
  write_memory(state->sp--, state->pc, state->mode);
  state->pc = INTERRUPT_PROGRAM_LOAD;
}

void timer_interrupt(state_t *state)
{
  int sp;
  
  if (!state->ienable)
    return ;
  
  state->mode = SYSTEM;
  state->ienable = 0;

  sp = state->sp;
  state->sp = SSTACK_BASE;
  write_memory(state->sp--, state->sp, state->mode);
  write_memory(state->sp--, state->pc, state->mode);
  state->pc = TIMER_PROGRAM_LOAD;
}

void iret(state_t *state)
{
  /* IRet: return from system call */
  
  state->ienable = 1;
  state->mode = USER;
  /* pop PC and SP from system stack */
  state->pc = read_memory(state->sp++, state->mode);
  state->sp = read_memory(state->sp++, state->mode);
}


