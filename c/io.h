/* io.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:15 */

#ifndef IO_H
#define IO_H

#define IO_RD 0
#define IO_WR 1

typedef struct {
  int address;
  int value;
  unsigned int error:8;
  unsigned int op:1;
  unsigned int pad:7;
} io_t;

int read_memory(int address);
int write_memory(int address, int value);

#endif	/* IO_H */
