/* constants.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:22 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define USER 0
#define SYSTEM 1

#define USER_PROGRAM_LOAD         0
#define TIMER_PROGRAM_LOAD     1000
#define INTERRUPT_PROGRAM_LOAD 1500

#define USTACK_BASE  999
#define SSTACK_BASE 1999

#define CHECK_ADDRESS(ADDR, MODE) (((MODE) == USER) && ((ADDR) > USTACK_BASE))

#define CONSOLE stderr

#endif	/* CONSTANTS_H */
