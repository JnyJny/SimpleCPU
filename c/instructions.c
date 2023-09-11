/* instructions.c (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:34 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "constants.h"
#include "instructions.h"
#include "io.h"

typedef void (*instr_f)(state_t *);


void invalid_instruction(state_t *state);

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
void end(state_t *state);

instr_f instruction_dispatch_table[51] = {
  invalid_instruction,
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
  iret,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  end,
};


/* Bounds checked read and write
 */

int read_memory_check(int address, state_t *state)
{

  if (CHECK_ADDRESS(address, state->mode)) {
    fprintf(CONSOLE,"SEGFAULT: [ir] %08d [pc] %08d [addr] %08d\n",
	    state->pc,
	    state->ir,
	    address);
    exit(EXIT_FAILURE);
  }

  return read_memory(address);
}

int write_memory_check(int address, int value, state_t *state)
{
  if (CHECK_ADDRESS(address, state->mode)) {
    fprintf(CONSOLE,"Memory Fault: [pc] %08d [ir] %08d [A] %08d\n",
	    state->pc,
	    state->ir,
	    address);    
    exit(EXIT_FAILURE);
  }  

  return write_memory(address, value);
}

#define HR "--------------------------------------------------\n"

void dump_state(FILE *fp, state_t *state)
{
  int stack_base;
  int value;
  
  if (!state->debug)
    return;
  
  fputs(HR, fp);
  fprintf(fp, "[ir] %08d [pc] %08d [sp] %08d [mode] %s\n",
	  state->ir, state->pc, state->sp, state->mode?"KERN":"USER");
  fprintf(fp, "[ac] %08d [ x] %08d [ y] %08d [debg] %s\n",
	  state->ac, state->x, state->y, state->debug?"ON":"OFF");
  fprintf(fp, "[cy] %08d [ti] %08d [t?] %8d [intr] %s\n",
	  state->cycles,
	  state->timer_interval,
	  (state->cycles % state->timer_interval) == 0,
	  state->interrupts?"ON":"OFF");

  stack_base = (state->mode)?SSTACK_BASE:USTACK_BASE;

  for (int i=state->sp+1; i <= stack_base; i++) {
    fprintf(CONSOLE, "[%cstk] %04d: %04d\n",
	    state->mode?'k':'u',
	    i,
	    read_memory(i));
  }  
  
}



/* Fetch and Execute 
 */

void fetch(state_t *state)
{
  state->ir = read_memory_check(state->pc, state);
}

int execute(state_t *state)
{

  if (!VALID_INSTRUCTION(state->ir))
    return -1;
  
  if (state->ir == END) {
    state->cycles++;
    return 0;
  }
    
  instruction_dispatch_table[state->ir](state);

  if (NOT_CTI(state->ir)) {
    state->pc++;
  }
  
  state->cycles++;
  
  return 1;
}

/* Instruction Implementation
 */

void invalid_instruction(state_t *state)
{
  fprintf(CONSOLE,"Invalid Instruction: [pc] %08d [ir] %08d\n",
	  state->pc, state->ir);
}


void load_value(state_t *state)
{
  /* Load value: Load the value into AC */
  state->ac = read_memory_check(++state->pc, state);
}

void load_addr(state_t *state)
{
  /* Load addr: Load the value at address into AC */
  int addr;

  addr = read_memory_check(++state->pc, state);
  state->ac = read_memory_check(addr, state);
}

void load_indirect(state_t *state)
{
  /* LoadInd addr: Load value from address at address */
  int addr;
  
  addr = read_memory_check(++state->pc, state);
  addr = read_memory_check(addr, state);
  state->ac = read_memory_check(addr, state);
}

void load_index_x(state_t *state)
{
  /* LoadIdxX addr: Load value (addr+X) into AC */
  int addr;

  addr = read_memory_check(++state->pc, state);
  state->ac = read_memory_check(addr + state->x, state);
}

void load_index_y(state_t *state)
{
  /* LoadIdxY addr: Load value (addr+Y) into AC */
  int addr;
  
  addr = read_memory_check(++state->pc, state);
  state->ac = read_memory_check(addr + state->y, state);
}

void load_sp_x(state_t *state)
{
  /* LoadSpX: Load from (Sp+X) info AC */
  state->ac = read_memory_check(state->sp + state->x, state);
}

void store_addr(state_t *state)
{
  /* Store addr: Store AC into address */
  int addr;

  addr = read_memory_check(++state->pc, state);
  write_memory_check(addr, state->ac, state);
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
  
  port = read_memory_check(++state->pc, state);

  if (state->debug) {
    fprintf(CONSOLE, "[COUT] port=%d AC=%d\n",
	    port,
	    state->ac);
    return;
  }
  
  switch(port) {
    case 1:
      fprintf(CONSOLE, "%d", state->ac);
      break;
    case 2:
      fprintf(CONSOLE, "%c", state->ac);
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

  state->pc = read_memory_check(++state->pc, state);
}

void jump_eq_addr(state_t *state)
{
  /* JumpIfEqual addr: jump to addr if AC is zero */
  int addr;
  
  addr = read_memory_check(++state->pc, state);
  
  if (state->ac == 0)
    state->pc = addr;
  else
    state->pc++;
}

void jump_ne_addr(state_t *state)
{
  /* JumpIfNotEqual addr: Jump to addr if AC is not zero */
  int addr;
  
  addr = read_memory_check(++state->pc, state);
  if (state->ac != 0)
    state->pc = addr;
  else
    state->pc++;
}

void call_addr(state_t *state)
{
  /* Call addr: Push return address onto stack, jump to addr */
  int addr;

  addr = read_memory_check(++state->pc, state);
  write_memory_check(state->sp--, state->pc+1, state);
  state->pc = addr;
}

void ret(state_t *state)
{
  /* Ret: Pop return address from stack, jump to address */
  int addr;

  addr = read_memory_check(++state->sp, state);
  state->pc = addr;
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
  write_memory_check(state->sp--, state->ac, state);
}

void pop(state_t *state)
{
  /* Pop: pop stack into AC */
  state->ac = read_memory_check(state->sp++, state);
}

void interrupt(state_t *state)
{
  /* Int: perform system call */
  int sp;
  
  if (!state->interrupts)
    return;
  
  state->mode = SYSTEM;
  state->interrupts = 0;

  sp = state->sp;
  state->sp = SSTACK_BASE;
  write_memory_check(state->sp--, sp, state);
  write_memory_check(state->sp--, state->pc, state);
  state->pc = INTERRUPT_PROGRAM_LOAD;
}

void timer_interrupt(state_t *state)
{
  int usp;
  int upc;
  
  if (!state->interrupts)
    return ;
  
  state->mode = SYSTEM;
  state->interrupts = 0;

  usp = state->sp;
  upc = state->pc;
    
  state->sp = SSTACK_BASE;
  write_memory_check(state->sp--, usp, state);
  write_memory_check(state->sp--, upc, state);
  state->pc = TIMER_PROGRAM_LOAD;
}

void iret(state_t *state)
{
  /* IRet: return from system call */
  
  /* pop PC and SP from system stack */
  state->pc = read_memory_check(++state->sp, state);
  state->sp = read_memory_check(++state->sp, state);
  /* enable interrupts and return to user mode */
  state->interrupts = 1;
  state->mode = USER;
}

void end(state_t *state)
{
  /* End executation */
}


