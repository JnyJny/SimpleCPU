/* microcode.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:26 */

#ifndef MICROCODE_H
#define MICROCODE_H

#include "cpu.h"

typedef void (*microcode_f)(cpu_t *cpu);

void invalid_instruction(cpu_t *cpu);
void load_value(cpu_t *cpu);
void load_addr(cpu_t *cpu);
void load_indirect(cpu_t *cpu);
void load_index_x(cpu_t *cpu);
void load_index_y(cpu_t *cpu);
void load_sp_x(cpu_t *cpu);
void store_addr(cpu_t *cpu);
void get(cpu_t *cpu);
void put_port(cpu_t *cpu);
void add_x(cpu_t *cpu);
void add_y(cpu_t *cpu);
void sub_x(cpu_t *cpu);
void sub_y(cpu_t *cpu);
void copy_to_x(cpu_t *cpu);
void copy_from_x(cpu_t *cpu);
void copy_to_y(cpu_t *cpu);
void copy_from_y(cpu_t *cpu);
void copy_to_sp(cpu_t *cpu);
void copy_from_sp(cpu_t *cpu);
void jump_addr(cpu_t *cpu);
void jump_eq_addr(cpu_t *cpu);
void jump_ne_addr(cpu_t *cpu);
void call_addr(cpu_t *cpu);
void ret(cpu_t *cpu);
void inc_x(cpu_t *cpu);
void dec_x(cpu_t *cpu);
void push(cpu_t *cpu);
void pop(cpu_t *cpu);
void interrupt(cpu_t *cpu);
void iret(cpu_t *cpu);
void end(cpu_t *cpu);

void timer_interrupt(cpu_t *cpu);

#ifndef MICROCODE_C
extern microcode_f microcode_table[NUM_OPCODES];
#endif

#endif	/* MICROCODE_H */
