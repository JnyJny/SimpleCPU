/* object.h (ejo@ufo) 14 Sep 23  Modified: 14 Sep 23  10:59 */


#ifndef OBJECT_H
#define OBJECT_H

#include "constants.h"


typedef struct {
  word_t  address;
  char    symbol[SYMSZ];
} symbol_t;

typedef struct {
  symbol_t      symtab[NWORDS];
  instruction_t instructions[NWORDS];
} program_t;

int read_object_file(int fd, word_t *dst, size_t dstsz);
int write_object_file(int fd, word_t *src, size_t srcsz);


#endif	/* OBJECT */
