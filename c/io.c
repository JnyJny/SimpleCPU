/* io.c (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:17 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "constants.h"
#include "io.h"

word_t read_memory(word_t address)
{
  io_t io = {address, 0, 0, IO_RD, 0};
  
  if (write(MEM_WR_CHANNEL, &io, sizeof(io)) < 0)
    return -1;

  if (read(MEM_RD_CHANNEL, &io, sizeof(io)) < 0)
    return -1;

  return io.value;
}

word_t write_memory(word_t address, word_t value)
{
  io_t io = {address, value, 0, IO_WR, 0};

  if (write(MEM_WR_CHANNEL, &io, sizeof(io)) < 0)
    return -1;

  if (read(MEM_RD_CHANNEL, &io, sizeof(io)) < 0) 
    return -1;

  return 0;
}



