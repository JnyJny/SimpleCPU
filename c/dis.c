/* dis.c (ejo@ufo) 11 Sep 23  Modified: 11 Sep 23  17:32 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>
#include <errno.h>

#include "constants.h"
#include "instruction.h"
#include "object.h"

#define OPTSTR "f:o:"
extern char *optarg;
extern int   opterr;


void disassemble_memory(FILE *fp, word_t *memory, size_t memsz);

instruction_t *dispatch_table;

int main(int argc, char *argv[])
{
  int     opt;
  FILE   *src = NULL;
  FILE   *dst = stdout;
  word_t  memory[NWORDS];
  
  opterr = 0;

  while((opt = getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 'f':
	if ( !(src = fopen(optarg, "r"))) {
	  perror("fopen:source");
	  return EXIT_FAILURE;
	  /* NOTREACHED */
	}
	break;
	
      case 'o':
	if ( !(dst = fopen(optarg, "w"))) {
	  perror("fopen:destination");
	  return EXIT_FAILURE;
	  /* NOTREACED */
	}
	break;
	
      case '?':
      case 'h':
	fprintf(stderr,"Usage: %s\n",
		basename(argv[0]));
	return EXIT_FAILURE;
	/* NOTREACHED */
	break;
    }

  dispatch_table = alloc_dispatch_table();

  memset(memory, 0, sizeof(memory));

  if (read_object_file(fileno(src), memory, sizeof(memory)) < 0) {
    perror("read_object_file");
    return EXIT_FAILURE;
    /* NOTREACHED */
  }

  disassemble_memory(dst, memory, sizeof(memory));
  
  return EXIT_SUCCESS;
}

instruction_t decode(int opcode)
{
  if (!VALID_OPCODE(opcode)) {
    return dispatch_table[INVALID_OPCODE];
  }

  return dispatch_table[opcode];
}
  

void disassemble_memory(FILE *fp, word_t *memory, size_t memsz)
{
  instruction_t instruction;
  int           address;
  int           opcode;
  int           nwords;

  nwords = memsz / sizeof(word_t);
  
  for(address = USER_PROGRAM_LOAD; address < nwords; address++) {

    opcode = memory[address];

    instruction = decode(opcode);

    instruction.address = address;
    
    if (instruction.opcode == INVALID_OPCODE) 
      continue;

    if (instruction.has_operand)
      instruction.operand = memory[++address];
    
    iprintf(fp, &instruction);
  }
}
