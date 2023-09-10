/* cpu.c (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  16:10 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#include "constants.h"
#include "cpu.h"
#include "instructions.h"

#define OPTSTR "dt:"
extern char *optarg;
extern int opterr;

FILE *console;


void dump_state(FILE *fp, state_t *state);





int main(int argc, char *argv[])
{
  int      opt;
  state_t  state;
  char    *filename = NULL;
  int      result;

  console = stderr;

  state.ir = 0;
  state.pc = USER_PROGRAM_LOAD;
  state.sp = USTACK_BASE;
  state.ac = 0;
  state.x = 0;
  state.y = 0;
  state.timer = DEFAULT_TIMER;
  state.mode = USER;
  state.ienable = 1;
  state.debug = 0;
  state.cycles = 0;

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

    if (state.debug)
      dump_state(stderr, &state);

    if ((state.cycles % state.timer) == 0)
      timer_interrupt(&state);

    fetch(&state);

    if (!execute(&state))
      break;
  }
  
  return EXIT_SUCCESS;
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
  
  fprintf(fp, "[timer] %08d [cycles] %08d [timer?] %d [ienable] %d\n",
	  state->timer,
	  state->cycles,
	  (state->cycles % state->timer) == 0,
	  state->ienable);
}


