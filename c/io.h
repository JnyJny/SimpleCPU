/* io.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:15 */

#ifndef IO_H
#define IO_H

#include "constants.h"		/* word_t definition */

#define IO_RD 0
#define IO_WR 1

typedef struct {
  word_t address;
  word_t value;
  unsigned int error:8;
  unsigned int op:1;
  unsigned int pad:7;
} io_t;

word_t read_memory(word_t address);
word_t write_memory(word_t address, word_t value);

#endif	/* IO_H */
