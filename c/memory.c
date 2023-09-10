/* memory.c (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  14:26 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "constants.h"
#include "memory.h"
#include "io.h"

int memory[NWORDS];

int load(int);
int store(int, int);

int main(int argc, char *argv[]) {
  io_t request;
  io_t response;
  int  err;

#ifdef DEBUG  
  FILE *trace = fopen("trace.memory", "w");
#endif

  fprintf(CONSOLE, "[ MEM] Starting.\n");

  memset(memory, 0, sizeof(memory));
    
  while (1) {
    
    memset(&request, 0, sizeof(request));
    
    if ((err = read(STDIN_FILENO, &request, sizeof(request))) < 0)
      if (errno == EPIPE)
	break;

    response = request;
    response.error = 0;
    
    switch(request.op) {
      
      case IO_RD:
	if ((response.value = load(request.address)) < 0)
	  response.error = errno;
	
#ifdef DEBUG	
	DUMP_IO(" IN", trace, &request);
	DUMP_IO("OUT", trace, &response);
#endif	
	break;
	
      case IO_WR:
	response.value = request.value;
	if ((err = store(request.address, request.value)) < 0)
	  response.error = errno;
#ifdef DEBUG
	DUMP_IO(" IN", trace, &request);
	DUMP_IO("OUT", trace, &response);
#endif	
	break;

      default:
	response.address = request.address;
	response.op = request.op;
	response.error = ENOMSG;
#ifdef DEBUG
	DUMP_IO(" IN", trace, &request);
	DUMP_IO("OUT", trace, &response);	
#endif		
	break;
    }

    if ((err = write(STDOUT_FILENO, &response, sizeof(response))) < 0)
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



