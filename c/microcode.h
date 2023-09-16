/* microcode.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:26 */

#ifndef MICROCODE_H
#define MICROCODE_H

#include "cpu.h"
#include "instruction.h"

void invalid_instruction(void *state);
void load_value(void *state);
void load_addr(void *state);
void load_indirect(void *state);
void load_index_x(void *state);
void load_index_y(void *state);
void load_sp_x(void *state);
void store_addr(void *state);
void get(void *state);
void put_port(void *state);
void add_x(void *state);
void add_y(void *state);
void sub_x(void *state);
void sub_y(void *state);
void copy_to_x(void *state);
void copy_from_x(void *state);
void copy_to_y(void *state);
void copy_from_y(void *state);
void copy_to_sp(void *state);
void copy_from_sp(void *state);
void jump_addr(void *state);
void jump_eq_addr(void *state);
void jump_ne_addr(void *state);
void call_addr(void *state);
void ret(void *state);
void inc_x(void *state);
void dec_x(void *state);
void push(void *state);
void pop(void *state);
void interrupt(void *state);
void iret(void *state);
void end(void *state);

void timer_interrupt(state_t *state);

#endif	/* MICROCODE_H */
