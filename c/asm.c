/* asm.c (ejo@ufo) 13 Sep 23  Modified: 13 Sep 23  10:47 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <assert.h>

#include "constants.h"
#include "instruction.h"
#include "object.h"

#define OPTSTR "df:o:"
extern char *optarg;
extern int   opterr;

#define SEP " \t"



int assemble_program(FILE *ifp, word_t *memory, size_t memsz);
int assemble_program_oldstyle(FILE *fp, word_t *memory, size_t memsz);

instruction_t *dispatch_table;

int main(int argc, char *argv[])
{
  int     opt;
  FILE   *src;
  FILE   *dst;
  word_t  memory[NWORDS];
  int     debug = 0;


  memset(memory, 0, sizeof(memory));

  while((opt=getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 'd':
	debug++;
	break;
	
      case 'f':
	src = fopen(optarg, "r");
	break;
      case 'o':
	dst = fopen(optarg, "w");
	break;
      case '?':
      case 'h':
	fprintf(stderr, "usage: %s -f source -o object\n",
		basename(argv[0]));
	return EXIT_FAILURE;
	/* NOTREACHED */
	break;
    }

  dispatch_table = alloc_dispatch_table();

  if (debug) {
    for (int i=INVALID_OPCODE; i <= END_OPCODE; i++) {
      iprintf(stdout, dispatch_table +i);
    }
    return EXIT_SUCCESS;
  }

  if (assemble_program_oldstyle(src, memory, sizeof(memory)) < 0) {
    perror("assemble_program");
    return EXIT_FAILURE;
  }

  if (write_object_file(fileno(dst), memory, sizeof(memory)) < 0) {
    perror("write_object_file");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


instruction_t encode(char *mnemonic)
{
  for(int i=0; i<END_OPCODE; i++) {
    if (strncasecmp(mnemonic, dispatch_table[i].mnemonic, 16) == 0) {
      return dispatch_table[i];
    }
  }
  return dispatch_table[INVALID_OPCODE];
}


int assemble_program(FILE *ifp, word_t *memory, size_t memsz)
{
  char buf[BUFSIZ];
  int  address = USER_PROGRAM_LOAD;
  int  line;

  if ((!ifp) || !memory || (memsz<=0)) {
    errno = EINVAL;
    return -1;
  }

  line = 0;

  while (fgets(buf, sizeof(buf)-1, ifp)) {
    char *nl;
    int   value;
    char  mnemonic[16];
    instruction_t  instruction;
    
    line++;

    if (address > memsz) {
      errno = ENOMEM;
      return -1;
    }
    
    if ((nl = strchr(buf, '\n')))
      *nl = '\0';

    if (strlen(buf) == 0) {	/* Empty lines are ignored */
      fprintf(stderr, "[DEBUG] [%4d] empty line\n", line);
      continue;
    }

    if (sscanf(buf, ". %d", &value)) {
      fprintf(stderr, "[DEBUG] A [%4d] change address %08d\n", line, value);
      address = value;
      continue;
    }

    if (sscanf(buf, ".DATA %d", &value)) {
      fprintf(stderr, "[DEBUG] B [%4d] data %08d %08d\n", line, address, value);
      memory[address] = value;
      address++;
      continue;
    }

    if (sscanf(buf, "%d", &value)) {

      instruction = dispatch_table[value];
      fprintf(stderr, "[DEBUG] E [Line %4d] [Addr %4d] %8d\n",
	      line,
	      address,
	      value);
      
      memory[address] = value;
      address++;
      continue;
    }    

    if (sscanf(buf, "%s %d", mnemonic, &value)) {
      
      instruction = encode(mnemonic);

      fprintf(stderr, "[DEBUG] C [%4d] %30s [%16s %d] -> %4d:%4d %4d:%4d\n",
	      line,
	      buf,
	      mnemonic,
	      value,
	      address,
	      instruction.opcode,
	      address+1,
	      value);
      
      memory[address] = instruction.opcode;
      memory[address+1] = value;
      address += 2;
      continue;
    }

    if (sscanf(buf, "%s", mnemonic)) {
      instruction = encode(mnemonic);
      fprintf(stderr, "[DEBUG] D [%4d] [%16s] -> %4d:%4d\n",
	      line,
	      mnemonic,
	      address,
	      instruction.opcode);
      
      memory[address] = instruction.opcode;
      assert(instruction.has_operand == 0);

      address ++;
      continue;
    }
    /* unrecognized lines are ignored */
  }
  

  return 0;
}


int assemble_program_oldstyle(FILE *fp, word_t *memory, size_t memsz)
{
  char    buf[BUFSIZ];
  int     lineno;
  int     address;
  int     value;
  char   *nl;
  size_t  nwords = memsz / sizeof(word_t);
  
  lineno = 0;

  address = USER_PROGRAM_LOAD;
  
  while (fgets(buf, 80, fp)) {
    lineno ++;
    
    if ((nl = strchr(buf, '\n')))
      *nl = '\0';

    if (strlen(buf) == 0)
      continue;

    if (sscanf(buf, ". %d", &value)) {
      address = value;
      continue;
    }

    if (sscanf(buf, "%d", &value)) {
      memory[address] = value;
      address++;
      continue;
    }
    // unrecognized lines ignored
  }

  return 0;
}
		     


