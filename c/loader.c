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

#define OPTSTR "df:o:"
extern char *optarg;
extern int   opterr;


int load_memory(char *filename, int ofd);
int dump_memory(FILE *fp, int ifd, int ofd);

int redirect(int ifd, int ofd);

int main(int argc, char *argv[])
{
  int opt;
  int pid;
  int debug = 0;
  char *program_file = NULL;
  char *memory_dump = NULL;

  int fd1[2];
  int fd2[2];  

  opterr = 0;
  
  while((opt = getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 'd':
	debug = 1;
	break;
      case 'f':
	program_file = optarg;
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

  if ((pipe(fd1) < 0) || (pipe(fd2) < 0)) {
    perror("pipe");
    return EXIT_FAILURE;
    /* NOTREACHED */
  }


  switch(pid = fork()) {
    case -1:			/* error */
      perror("fork");
      return EXIT_FAILURE;
      break;
      
    case 0:			/* child */

      redirect(fd2[IO_RD], fd1[IO_WR]);
      
      execl("./"MEMORY, MEMORY);
      break;

      
    default:			/* parent */

      redirect(fd1[IO_RD], fd2[IO_WR]);
      
      if (load_memory(program_file, STDOUT_FILENO) < 0) {
	perror("load_memory");
	return EXIT_FAILURE;
	/* NOTREACHED */
      }

      if (debug) {
	dump_memory(stderr, STDIN_FILENO, STDOUT_FILENO);
	return EXIT_SUCCESS;
	/* NOTREACHED */
      }

      execl("./"CPU, CPU);
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

int load_memory(char *filename, int ofd)
{
  FILE *fp;
  char  buf[BUFSIZ];
  int   lineno;
  int   value;
  char *nl;
  io_t io;
  
  if (!filename) {
    errno = EINVAL;
    return -1;
  }

  if (!(fp = fopen(filename, "r"))) {
    return -1;
  }

  lineno = 0;

  io.op = IO_WR;
  io.address = USER_PROGRAM_LOAD;
  
  while (fgets(buf, 80, fp)) {
    lineno ++;
    
    if ((nl = strchr(buf, '\n')))
      *nl = '\0';
    
    if (sscanf(buf, ". %d", &io.value)) {
      io.address = value;
      continue;
    }

    if (sscanf(buf, "%d", &io.value)) {
      if (write(ofd, &io, sizeof(io)) < 0) {
	fprintf(stderr, "Line %4d Failed to write [%d] %d\n",
		lineno, io.address, io.value);
	return -1;
      }
      
      io.address++;
      continue;
    }
    // unrecognized lines ignored
  }

  fclose(fp);
  
  return 0;
}

#define WORDS_PER_LINE 10
#define DUMP_FILE "dump.memory"

int dump_memory(FILE *fp, int ifd, int ofd) {
  io_t io;
  int  err;
  
  if (!fp) {
    if (!(fp = fopen(DUMP_FILE, "w"))) {
      perror("dump_memory:fopen:"DUMP_FILE);
      return -1;
    }
    fprintf(stderr,"DEBUG: memory dumped to %s\n", DUMP_FILE);
  }
  
  for(int l=0; l < NWORDS/WORDS_PER_LINE; l++) {
    
    fprintf(fp, "[%04d] ", l * WORDS_PER_LINE);
    for(int i=0; i < 10; i++) {

      io.op = IO_RD;
      io.address = (l * WORDS_PER_LINE) + i;
      io.value = 0;
      
      if ( write(ofd, &io, sizeof(io)) < 0) {
	perror("dump_memory:write");
	return -1;
      }

      memset(&io, 0, sizeof(io));
      
      if (read(ifd, &io, sizeof(io)) < 0) {
	perror("dump_memory:read");
	return -1;
      }
      fprintf(fp, "%04d ", io.value);
    }
    fputs("\n", fp);
  }
  
  return 0;
}


