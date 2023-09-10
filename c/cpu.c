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

void dump_state(FILE *fp, state_t *state);

int main(int argc, char *argv[])
{
  int      opt;
  state_t  state;
  int      running;

  state.ir = 0;
  state.pc = USER_PROGRAM_LOAD;
  state.sp = USTACK_BASE;
  state.ac = 0;
  state.x = 0;
  state.y = 0;
  state.timer = DEFAULT_TIMER;
  state.mode = USER;
  state.interrupts = 1;
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

  fprintf(CONSOLE, "[ CPU] Start. Timer %d Debug %d\n", state.timer, state.debug);

  do {

    if ( state.cycles && ((state.cycles % state.timer) == 0))
      timer_interrupt(&state);

    fetch(&state);

    dump_state(CONSOLE, &state);    

    if ((running = execute(&state)) < 0) {
      fprintf(CONSOLE, "[ CPU] ABEND %04d @ %04d\n", state.ir, state.pc);
      exit(EXIT_FAILURE);
    }

  } while(running);

  fprintf(CONSOLE, "[ CPU] End. Executed %d instructions.\n",
	  state.cycles);  
  
  return EXIT_SUCCESS;
}
 




