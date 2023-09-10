/* io.c (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:17 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io.h"

int read_memory(int address)
{
  io_t io = {address, 0, 0, IO_RD, 0};
  
  if (write(STDOUT_FILENO, &io, sizeof(io)) < 0)
    return -1;

  if (read(STDIN_FILENO, &io, sizeof(io)) < 0)
    return -1;

  return io.value;
}

int write_memory(int address, int value)
{
  io_t io = {address, value, 0, IO_WR, 0};

  if (write(STDOUT_FILENO, &io, sizeof(io)) < 0)
    return -1;

  if (read(STDIN_FILENO, &io, sizeof(io)) < 0) 
    return -1;

  return 0;
}
