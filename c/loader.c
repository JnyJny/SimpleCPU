/* loader.c (ejo@ufo)  8 Sep 23  Modified:  8 Sep 23  10:40 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <string.h>

#include "constants.h"
#include "cpu.h"
#include "memory.h"
#include "io.h"

#define OPTSTR "df:o:t:"
extern char *optarg;
extern int   opterr;


int load_memory(char *filename);
int dump_memory(FILE *fp);

int redirect(int ifd, int ofd);

int main(int argc, char *argv[])
{
  int opt;
  int pid;
  int debug = 0;
  char *program_file = NULL;
  char *memory_dump = NULL;

  int cpu_to_memory[2];
  int memory_to_cpu[2];
  char *timer = "100";

  opterr = 0;

  setlinebuf(stderr);
  
  while((opt = getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 'd':
	debug = 1;
	break;
      case 'f':
	program_file = optarg;
	break;
      case 't':
	timer = optarg;
	break;
      case '?':
      case 'h':
	fprintf(stderr,"usage: %s -f program_file [-d]\n",
		basename(argv[0]));
	return EXIT_FAILURE;
	/* NOTREACHED */
	break;
    }

  if (!program_file) {
    fprintf(stderr,"usage: %s -f program_file [-d]\n",
	    basename(argv[0]));
    fprintf(stderr,"\tProgram file name required.\n");
    return EXIT_FAILURE;
  }

  if ((pipe(cpu_to_memory) < 0) || (pipe(memory_to_cpu) < 0)) {
    perror("pipe");
    return EXIT_FAILURE;
    /* NOTREACHED */
  }

  switch(pid = fork()) {
    case -1:			/* error */
      perror("loader:fork");
      return EXIT_FAILURE;
      break;
      
    case 0:			/* child */

      redirect(memory_to_cpu[IO_RD], cpu_to_memory[IO_WR]);
      
      execl("./"MEMORY, MEMORY, (char *)NULL);
      break;

      
    default:			/* parent */

      redirect(cpu_to_memory[IO_RD], memory_to_cpu[IO_WR]);
      
      if (load_memory(program_file) < 0) {
	perror("loader:load_memory");
	return EXIT_FAILURE;
	/* NOTREACHED */
      }

      if (debug)
	dump_memory(stderr);


      execl("./"CPU, CPU, debug?"-d":"", "-t", timer, (char *)NULL);
      /* NOTREACHED */
      break;
  }

  return EXIT_SUCCESS;
}


int redirect(int ifd, int ofd)
{
  if (ifd != STDIN_FILENO) {
    if (dup2(ifd, STDIN_FILENO) != STDIN_FILENO)
      return -1;
    close(ifd);
  }

  if (ofd != STDOUT_FILENO) {
    if (dup2(ofd, STDOUT_FILENO) != STDOUT_FILENO)
      return -1;
    close(ofd);
  }
  return 0;
}


#define SEP " \t"

int load_memory(char *filename)
{
  FILE *fp;
  char  buf[BUFSIZ];
  int   lineno;
  int   address;
  int   value;
  char *nl;

  fprintf(stderr, "[LOAD] %s\n", filename);
  
  if (!filename) {
    errno = EINVAL;
    return -1;
  }

  if (!(fp = fopen(filename, "r"))) {
    return -1;
  }

  lineno = 0;

  address = USER_PROGRAM_LOAD;

  while (fgets(buf, 80, fp)) {
    lineno ++;
    
    if ((nl = strchr(buf, '\n')))
      *nl = '\0';
    
    if (sscanf(buf, ". %d", &value)) {
      address = value;
      continue;
    }

    if (sscanf(buf, "%d", &value)) {
      write_memory(address, value);
      address++;
      continue;
    }
    // unrecognized lines ignored
  }

  fclose(fp);
  
  return 0;
}

#define WORDS_PER_LINE 10

int dump_memory(FILE *fp) {
  int   address;
  int   value;
  char  buf[BUFSIZ];
  char *cp;
  int   sum;
  
  if (!fp) {
    errno = EINVAL;
    return -1;
  }
  
  for(int line=0; line < (NWORDS/WORDS_PER_LINE); line++) {
    
    memset(buf, '\0', sizeof(buf));

    cp = buf;
    
    cp += sprintf(cp, "[%04d] ", line * WORDS_PER_LINE);

    sum = 0;
    
    for(int i=0; i < WORDS_PER_LINE; i++) {
      address = (line * WORDS_PER_LINE) + i;
      value = read_memory(address);
      sum += value;
      cp += sprintf(cp, "%04d ", value);
    }

    if (sum) {
      sprintf(cp, "\n");
      fputs(buf, fp);
      fflush(fp);
    }
  }
  
  return 0;
}


