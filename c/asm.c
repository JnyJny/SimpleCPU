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

#define OPTSTR "do:O"
extern char *optarg;
extern int   opterr;
extern int   opterr;

#define SEP " \t"

int parse_source(FILE *ifp, program_t *program);
int assemble_program(FILE *ifp, word_t *memory, size_t memsz);
int assemble_program_oldstyle(FILE *fp, word_t *memory, size_t memsz);

int main(int argc, char *argv[])
{
  int     opt;
  word_t  memory[NWORDS];
  FILE   *src = NULL;
  FILE   *dst = NULL;
  int     debug = 0;
  int     oldstyle = 0;

  program_t program;

  memset(memory, 0, sizeof(memory));

  while((opt=getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 'd':
	debug++;
	break;

      case 'O':
	oldstyle = 1;
	break;
	
      case 'o':
	dst = fopen(optarg, "w");
	break;
	
      case '?':
      case 'h':
	fprintf(stderr, "usage: %s -o object source_file\n",
		basename(argv[0]));
	return EXIT_FAILURE;
	/* NOTREACHED */
	break;
    }

  for (; optind<argc; optind++) {
    if (!(src = fopen(argv[optind], "r"))) {
      perror("fopen:source");
      return EXIT_FAILURE;
      /* NOTREACHED */
    }
  }

  if (!src) {
    fprintf(stderr, "usage: %s -o object source_file\n",
	    basename(argv[0]));
    fprintf(stderr, "Source file path required.\n");
    return EXIT_FAILURE;
    /* NOTREACHED */
  }

  if (!dst) {
    if (!(dst = fopen("a.out", "w"))) {
      perror("open:a.out");
      return EXIT_FAILURE;
      /* NOTREACHED */
    }
  }
	      

  if (oldstyle) {
    if (assemble_program_oldstyle(src, memory, sizeof(memory)) < 0) {
      perror("assemble_program_oldstyle");
      return EXIT_FAILURE;
      /* NOTREACHED */
    }
  }
  else {
    if (parse_source(src, &program) < 0) {
      perror("parse_program");
      return EXIT_FAILURE;
      /* NOTREACHED */
    }

  }

  if (write_object_file(fileno(dst), memory, sizeof(memory)) < 0) {
    perror("write_object_file");
    return EXIT_FAILURE;
    /* NOTREACHED */
  }

  return EXIT_SUCCESS;
}


instruction_t encode(char *mnemonic, int opcode)
{
  static instruction_t *dispatch_table = (instruction_t *)NULL;

  if (!dispatch_table) {
    dispatch_table = alloc_dispatch_table();
  }

  if (mnemonic) {
    for(int i=0; i<END_OPCODE; i++) {
      if (strncasecmp(mnemonic, dispatch_table[i].mnemonic, 16) == 0) {
	return dispatch_table[i];
      }
    }
    return dispatch_table[INVALID_OPCODE];
  }

  if (!VALID_OPCODE(opcode))
    return dispatch_table[INVALID_OPCODE];

  return dispatch_table[opcode];
}



int parse_source(FILE *fp, program_t *program)
{
  char  buf[80];
  char *cp;
  char *word;
  char *symbol;
  char *label;
  char *mnemonic;
  int   line;
  
  word_t value;
  word_t address;
  
  if (!fp || !program ) {
    errno = EINVAL;
    return -1;
  }

  line = -1;

  address = USER_PROGRAM_LOAD - 1;
  
  while(fgets(buf, sizeof(buf)-1, fp)) {
    address++;
    line++;
    
    if ((cp = strchr(buf, '\n')))
      *cp = '\0';

    if ((cp=strchr(buf, '/')) && (*(cp+1) == '/'))
      *(cp+1) = '\0';

    if (strlen(buf) == 0)
      continue;

    printf("DEBUG [%3d] buf = '%s'\n", line, buf);

    if (sscanf(buf, " %d\n", &value) == 1) {
      printf("DEBUG [%03d] %08x bare value %d\n", line, address, value);
      continue;      
    }

    if (sscanf(buf, " . %d", &value) == 1) {
      printf("DEBUG [%3d] %08d change address to %d\n", line, address, value);
      continue;      
    }    

    if (sscanf(buf, " %s %d", mnemonic, &value) == 2) {
      printf("DEBUG [%3d] %08d mnemonic %s value %d\n", line, address, mnemonic, value);
      continue;
    }
    
    if (sscanf(buf, " %s %s", mnemonic, label) == 2) {
      printf("DEBUG [%3d] %08d  %s %s [Unresolved]\n", line, address, mnemonic, label);
      continue;
    }

    if (sscanf(buf, " .DATA %d", &value) == 1) {
      printf("DEBUG [%3d] %08x data value %d\n", line, address, value);
      address ++;
      continue;      
    }

    if (sscanf(buf, " %s", mnemonic) == 1) {
      printf("DEBUG [%03d] %08x mnemonic %s\n", line, address, mnemonic);
      address ++;
      continue;      
    }
    /* ignore anything unrecognized */
  }

  return 0;
}

int resolve_program(instruction_t *program)
{

  return 0;
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
    char *cp;
    int   value;
    char  mnemonic[16];
    instruction_t  instruction;
    
    line++;

    if (address > memsz) {
      errno = ENOMEM;
      return -1;
    }

    if ((cp = strchr(buf, '\n')))
      *cp = '\0';

    if ((cp = strstr(buf, "//")))
      *cp = '\0';

    if (strlen(buf) == 0)	/* Empty lines are ignored */
      continue;

    if (sscanf(buf, " . %d", &value) == 1) {
      address = value;
      continue;
    }

    if (sscanf(buf, " .DATA %d", &value) == 1) {
      memory[address] = value;
      address++;
      continue;
    }

    if (sscanf(buf, " %d", &value) == 1) {
      instruction = encode(NULL, value);
      memory[address] = value;
      address++;
      continue;
    }    

    if (sscanf(buf, " %s %d", mnemonic, &value) == 2) {
      
      instruction = encode(mnemonic, 0);
      memory[address] = instruction.opcode;
      memory[address+1] = value;
      address += 2;
      continue;
    }

    if (sscanf(buf, " %s", mnemonic) == 1) {
      instruction = encode(mnemonic, 0);
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
		     


