/* instructions.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:26 */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "cpu.h"

typedef enum {
  LDV  = 1,			/* Load the value into AC */
  LDA  = 2,			/* Load the value at address into AC */
  LDI  = 3,			/* Load indirect from value at address into AC */
  LDX  = 4, 			/* Load value at (address+X) into AC */
  LDY  = 5,			/* Load the value at (address+Y) into AC */
  LDS  = 6,			/* Load from (SP+X) into AC */
  STA  = 7,			/* Store the value in AC to address */
  GET  = 8,			/* Random int from 1 to 100 into AC */
  PUT  = 9,			/* port 1: print int AC, port 2: print char AC */
  ADDX = 10,			/* Add X to AC store AC */
  ADDY = 11,			/* Add Y to AC store AC */
  SUBX = 12,			/* Subtract X from AC store AC */
  SUBY = 13, 			/* Subtract Y from AC store AC */
  CACX = 14,			/* Copy AC to X */
  CXAC = 15,			/* Copy X to AC */
  CACY = 16,			/* Copy AC to Y */
  CYAC = 17,			/* Copy Y to AC */  
  CACSP= 18,			/* Copy AC to SP */
  CSPAC= 19,			/* Copy SP to AC */
  JUMP = 20,			/* Jump to address */
  JMPEQ= 21,			/* Jump to address if AC is zero */
  JMPNE= 22,			/* Jump to address if AC is not zero */
  CALL = 23,			/* Push return address on stack, jump to address */
  RET  = 24,			/* Pop return address from stack, jump to address */
  INC  = 25,			/* Increment value in X */
  DEC  = 26,			/* Decrement value in X */
  PUSH = 27,			/* Push AC on stack */
  POP  = 28,			/* Pop AC from stack */
  INTR = 29,			/* Perform system call */
  IRET = 30,			/* Return from system call */
  END  = 50,			/* End execution */
} instr_t;

#define VALID_INSTRUCTION(I) (((I)==END) || (((I)>=LDV) &&((I)<=IRET)))

void timer_interrupt(state_t *state);

void fetch(state_t *state);

int  execute(state_t *state);

void dump_state(FILE *fp, state_t *state);

#endif	/* INSTRUCTIONS_H */
