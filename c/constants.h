/* constants.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:22 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <unistd.h>

typedef int word_t;

typedef struct {
  word_t base;
  word_t count;
  word_t stack;
} segment_t;

#define USER_MODE   0
#define SYSTEM_MODE 1

#define USER_PROGRAM_LOAD         0
#define TIMER_PROGRAM_LOAD     1000
#define INTERRUPT_PROGRAM_LOAD 1500
#define END_OF_MEMORY          2000

#define USER_SEG      0
#define TIMER_SEG     1
#define INTERRUPT_SEG 2
#define NSEGMENTS     3

#define NWORDS                 END_OF_MEMORY

#define USTACK_BASE  999
#define SSTACK_BASE 1999

#define ADDRESS_FAULT(ADDR, MODE) (((MODE)==USER_MODE) && ((ADDR) > USTACK_BASE))
#define UADDRESS(ADDR) (((ADDR)>=USER_PROGRAM_LOAD) && ((ADDR)<=USTACK_BASE))
#define SADDRESS(ADDR) (((ADDR)>=TIMER_PROGRAM_LOAD) && ((ADDR)<=SSTACK_BASE))

#define MEM_WR_CHANNEL STDOUT_FILENO
#define MEM_RD_CHANNEL STDIN_FILENO

#define CONSOLE stderr

#define MAGIC 0x25656a6f

#define SYMSZ 16		/* length of a symbol */

#endif	/* CONSTANTS_H */
