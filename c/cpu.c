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

int main(int argc, char *argv[])
{
  int      opt;
  state_t  state;
  int      timer_fired;
  int      running;

  state.ir = 0;
  state.pc = USER_PROGRAM_LOAD;
  state.sp = USTACK_BASE;
  state.ac = 0;
  state.x = 0;
  state.y = 0;
  state.timer_interval = DEFAULT_TIMER_INTERVAL;
  state.mode = USER;
  state.interrupts = 1;
  state.debug = 0;
  state.cycles = 0;

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
	fprintf(stderr,"Usage: %s\n", basename(argv[0]));
	exit(EXIT_FAILURE);
	/* NOTREACHED */
	break;
    }

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
      dump_state(CONSOLE, &state);
      exit(EXIT_FAILURE);
    }
    
    dump_state(CONSOLE, &state);

  } while(running);

  fprintf(CONSOLE, "[ CPU] End. Executed %d instructions.\n",
	  state.cycles);
  
  return EXIT_SUCCESS;
}

