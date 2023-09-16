/* memory.c (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  14:26 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

#define OPTSTR "d"
extern char *optarg;
extern int   opterr;

#include "constants.h"
#include "memory.h"
#include "io.h"

int memory[NWORDS];


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


int main(int argc, char *argv[]) {
  io_t request;
  io_t response;
  int  opt;
  int  debug = 0;  
  int  err;

  opterr = 0;

  while ((opt=getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 'd':
	debug = 1;
	break;
      case '?':
      case 'h':
	fprintf(stderr, "usage: %s [-d]\n", basename(argv[0]));
	return EXIT_FAILURE;
	/* NOTREACHED */
	break;
    }
  
  if (debug)
    fprintf(CONSOLE, "[ MEM] Starting.\n");

  memset(memory, 0, sizeof(memory));
    
  while (1) {
    
    memset(&request, 0, sizeof(request));
    
    if ((err = read(MEM_RD_CHANNEL, &request, sizeof(request))) < 0)
      if (errno == EPIPE)
	break;

    response = request;
    response.error = 0;
    
    switch(request.op) {
      
      case IO_RD:
	if ((response.value = load(request.address)) < 0)
	  response.error = errno;
	break;
	
      case IO_WR:
	response.value = request.value;
	if ((err = store(request.address, request.value)) < 0)
	  response.error = errno;
	break;

      default:
	response.address = request.address;
	response.op = request.op;
	response.error = ENOMSG;
	break;
    }

    if ((err = write(MEM_WR_CHANNEL, &response, sizeof(response))) < 0)
      if (errno == EPIPE)
	break;
  }

  return EXIT_SUCCESS;
}




