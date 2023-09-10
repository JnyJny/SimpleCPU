/* cpu.h (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  17:29 */

#ifndef USER_H
#define USER_H

#define CPU "cpu"

enum {
  USER,
  SYSTEM,
};

#define USER_PROGRAM_LOAD         0
#define TIMER_PROGRAM_LOAD     1000
#define INTERRUPT_PROGRAM_LOAD 1500

#define USTACK_BASE  999
#define SSTACK_BASE 1999

#define DEFAULT_TIMER 0

typedef struct {
  int ir;			/* instruction register */
  int pc;			/* program counter */
  int sp;			/* stack pointer */
  int ac;			/* accumulator */
  int x;			/* operand X */
  int y;			/* operand Y */
  int timer;			/* interrupt after # instructions, 0=off */
  unsigned int mode:1;
  unsigned int ienable:1;
  unsigned int debug:1;
  unsigned int cycles:29;
} state_t;

#define STATE_INITIALIZER \
  { 0, USER_PROGRAM_LOAD, USTACK_BASE, 0, 0, 0, DEFAULT_TIMER, USER, 0, 0, 0 }


typedef enum {
  LDV = 1,			/* Load the value into AC */
  LDA = 2,			/* Load the value at address into AC */
  LDI = 3,			/* Load indirect from value at address into AC */
  LDX = 4, 			/* Load value at (address+X) into AC */
  LDY = 5,			/* Load the value at (address+Y) into AC */
  LDS = 6,			/* Load from (SP+X) into AC */
  STA = 7,			/* Store the value in AC to address */
  GET = 8,			/* Random int from 1 to 100 into AC */
  PUT = 9,			/* port 1: print AC as int, port 2: print AC as char */
  ADDX = 10,			/* Add X to AC store AC */
  ADDY = 11,			/* Add Y to AC store AC */
  SUBX = 12,			/* Subtract X from AC store AC */
  SUBY = 13, 			/* Subtract Y from AC store AC */
  MVAX = 14,			/* Copy AC to X */
  MVXA = 15,			/* Copy X to AC */
  MVAY = 16,			/* Copy AC to Y */
  MVYA = 17,			/* Copy Y to AC */  
  MVAS = 18,			/* Copy AC to SP */
  MVSA = 19,			/* Copy SP to AC */
  JUMP = 20,			/* Jump to address */
  JMPE = 21,			/* Jump to address if AC is zero */
  JMPN = 22,			/* Jump to address if AC is not zero */
  CALL = 23,			/* Push return address on stack, jump to address */
  RET  = 24,			/* Pop return address from stack, jump to address */
  INC  = 25,			/* Increment value in X */
  DEC  = 26,			/* Decrement value in X */
  PUSH = 27,			/* Push AC on stack */
  POP  = 28,			/* Pop AC from stack */
  INTR = 29,			/* Perform system call */
  IRET = 30,			/* Return from system call */
  END = 50,			/* End execution */
} instr_t;


#endif	/* USER_H */
