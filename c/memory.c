/* memory.c (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  14:26 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "memory.h"

int memory[NWORDS];

int load(int);
int store(int, int);

int main(int argc, char *argv[]) {
  io_t io;
  int  err;

  
  while (1) {
    
    memset(&io, 0, sizeof(io));
    
    if ((err = read(STDIN_FILENO, &io, sizeof(io))) < 0)
      if (errno == EPIPE)
	break;

    switch(io.op) {
      case IO_RD:
	if ((io.value = load(io.address)) < 0)
	  io.error = errno;
	break;
	
      case IO_WR:
	if ((err = store(io.address, io.value)) < 0)
	  io.error = errno;
	break;

      default:
	io.error = ENOMSG;
	break;
    }

    if ((err = write(STDOUT_FILENO, &io, sizeof(io))) < 0)
      if (errno == EPIPE)
	break;
  }

  return EXIT_SUCCESS;
}


int load(int address)
{
  if ((address < 0) || (address > NWORDS)) {
    errno = EFAULT;
    return -1;
  }
  
  return memory[address];
}

int store(int address, int value)
{
  if ((address < 0) || (address > NWORDS)) {
    errno = EFAULT;
    return -1;
  }

  memory[address] = value;
  
  return 0;
}



