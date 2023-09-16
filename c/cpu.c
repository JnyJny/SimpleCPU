/* cpu.c (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  16:10 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#include "constants.h"
#include "cpu.h"
#include "microcode.h"

#define OPTSTR "dt:"
extern char *optarg;
extern int opterr;

extern microcode_f microcode[NUM_OPCODES];

instruction_t *dispatch_table;

state_t state;

void fetch(state_t *state);
int  execute(state_t *state);

void dump_state(FILE *fp, state_t *state);

  
int read_memory_check(int address)
{
  if (CHECK_ADDRESS(address, state.mode)) {
    fprintf(CONSOLE, "SEGFAULT: [ir] %08d [pc] %08d [addr] %08d\n",
	    state.ir,
	    state.pc,
	    address);
    exit(EXIT_FAILURE);
  }

  return read_memory(address);
}

int write_memory_check(int address, int value)
{
  if (CHECK_ADDRESS(address, state.mode)) {
    fprintf(CONSOLE, "SEGFAULT: [ir] %08d [pc] %08d [addr] %08d\n",
	    state.ir,
	    state.pc,
	    address);    
    exit(EXIT_FAILURE);
  }
  
  return write_memory(address, value);
}


int main(int argc, char *argv[])
{
  int      opt;
  int      timer_fired;
  int      running;

  state.ir = 0;
  state.pc = USER_PROGRAM_LOAD;
  state.sp = USTACK_BASE;
  state.ac = 0;
  state.x = 0;
  state.y = 0;
  state.timer_interval = 0;
  state.mode = USER_MODE;
  state.interrupts = 1;
  state.debug = 0;
  state.cycles = 0;
  state.read_memory = read_memory_check;
  state.write_memory = write_memory_check;
    

  while((opt = getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 't':
	state.timer_interval = atoi(optarg);
	break;
      case 'd':
	state.debug = 1;
	break;
      case '?':
      case 'h':
	fprintf(stderr,"Usage: %s [-d] [-t timer_interval]\n", basename(argv[0]));
	exit(EXIT_FAILURE);
	/* NOTREACHED */
	break;
    }

  if (state.debug)
    fprintf(CONSOLE, "[ CPU] Initializing microcode.\n");

  dispatch_table = alloc_dispatch_table();

  for(int i=0; i<NUM_OPCODES; i++) 
    dispatch_table[i].microcode = microcode[i];  

  if (state.debug)
    fprintf(CONSOLE, "[ CPU] Start. Timer %d Debug %d\n",
	    state.timer_interval, state.debug);

  do {

    if (state.timer_interval &&
	state.cycles &&
	((state.cycles % state.timer_interval) == 0))
	timer_interrupt(&state);

    fetch(&state);

    if ((running = execute(&state)) < 0) {
      fprintf(CONSOLE, "[ CPU] ABEND\n");
      state.debug = 1;
      exit(EXIT_FAILURE);
    }
    
  } while(running);

  if (state.debug)
    fprintf(CONSOLE, "[ CPU] End. Executed %d instructions.\n",
	    state.cycles);
  
  return EXIT_SUCCESS;
}




#define HR "--------------------------------------------------\n"

void dump_state(FILE *fp, state_t *state)
{
  int stack_base;
  int value;
  
  if (!state->debug)
    return;
  
  fputs(HR, fp);
  fprintf(fp, "[ CPU] [ir] %08d [pc] %08d [sp] %08d [mode] %s\n",
	  state->ir, state->pc, state->sp, state->mode?"KERN":"USER");
  fprintf(fp, "[ CPU] [ac] %08d [ x] %08d [ y] %08d [debg] %s\n",
	  state->ac, state->x, state->y, state->debug?"ON":"OFF");
  fprintf(fp, "[ CPU] [cy] %08d [ti] %08d [t?] %8d [intr] %s\n",
	  state->cycles,
	  state->timer_interval,
	  (state->cycles % state->timer_interval) == 0,
	  state->interrupts?"ON":"OFF");

  iprintf(fp, &state->instruction);

  stack_base = (state->mode)?SSTACK_BASE:USTACK_BASE;

  for (int i=state->sp+1; i <= stack_base; i++) {
    fputs("[ CPU] ", fp);
    fprintf(CONSOLE, "[%cstk] %04d: %04d\n",
	    state->mode?'k':'u',
	    i,
	    read_memory(i));
  }  
}


void fetch(state_t *state)
{
  state->ir = state->read_memory(state->pc);

  memset(&state->instruction, 0, sizeof(instruction_t));

  if (!(VALID_OPCODE(state->ir)) )
    state->ir = INVALID_OPCODE;
  else
    state->instruction = dispatch_table[state->ir];
  
  state->instruction.address = state->pc;

  if (state->instruction.has_operand)
    state->instruction.operand = state->read_memory(++state->pc);
    
  if (state->debug)
    dump_state(CONSOLE, state);
  
  return;
}



int execute(state_t *state)
{
  if (!VALID_OPCODE(state->ir)) 
    return -1;
  
  if (state->ir == END_OPCODE) {
    state->cycles++;
    return 0;
  }

  state->instruction.microcode(state);

  if (NOT_CTI(state->ir))
    state->pc ++;
  
  state->cycles++;
  
  return 1;
}



