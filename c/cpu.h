/* cpu.h (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  17:29 */

#ifndef USER_H
#define USER_H

#define CPU "cpu"

#define DEFAULT_TIMER_INTERVAL 100

typedef struct {
  int ir;			/* instruction register */
  int pc;			/* program counter */
  int sp;			/* stack pointer */
  int ac;			/* accumulator */
  int x;			/* operand X */
  int y;			/* operand Y */
  int timer_interval;	        /* # cycles to fire timer interrupts */
  unsigned int mode:1;		/* USER or SYSTEM */
  unsigned int interrupts:1;	/* 0=off 1=enable */
  unsigned int debug:1;		/* 0=off 1=enable */
  unsigned int cycles:29;	/* number of instructions executed */
} state_t;




#endif	/* USER_H */
