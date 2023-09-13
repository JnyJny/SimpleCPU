/* microcode.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:26 */

#ifndef MICROCODE_H
#define MICROCODE_H

#include "cpu.h"
#include "instruction.h"

void timer_interrupt(state_t *state);

void fetch(state_t *state);

int  execute(state_t *state);

void dump_state(FILE *fp, state_t *state);

#endif	/* MICROCODE_H */
