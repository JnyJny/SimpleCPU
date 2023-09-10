/* memory.h (ejo@ufo)  8 Sep 23  Modified:  8 Sep 23  10:47 */

#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY "memory"

#define NWORDS 2000

#define IO_RD 0
#define IO_WR 1


typedef struct {
  int address;
  int value;
  unsigned int error:8;
  unsigned int op:1;
  unsigned int pad:7;
} io_t;


#endif	/* MEMORY_H */
