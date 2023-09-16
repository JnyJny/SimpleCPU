/* cpu.h (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  17:29 */

#ifndef USER_H
#define USER_H

#include "instruction.h"

#define CPU "cpu"

typedef int (*read_f)(int address);
typedef int (*write_f)(int address, int value);

typedef struct {
  int           ir;			/* instruction register */
  int           pc;			/* program counter */
  int           sp;			/* stack pointer */
  int           ac;			/* accumulator */
  int           x;			/* operand X */
  int           y;			/* operand Y */
  int           timer_interval;	        /* # cycles to fire timer interrupts */
  unsigned int  mode:2;			/* USER, TIMER, INTERRUPT */
  unsigned int  interrupts:1;		/* 0=off 1=enable */
  unsigned int  debug:1;		/* 0=off 1=enable */
  unsigned int  cycles:28;	        /* number of instructions executed */
  instruction_t instruction;		/* last instruction decoded */
  read_f        read_memory;
  write_f       write_memory;
} state_t;


#endif	/* USER_H */
