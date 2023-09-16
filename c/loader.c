/* loader.c (ejo@ufo)  8 Sep 23  Modified:  8 Sep 23  10:40 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

#include "constants.h"
#include "cpu.h"
#include "memory.h"
#include "io.h"
#include "loader.h"

#define OPTSTR "df:t:"
extern char *optarg;
extern int   opterr;

#define USAGE_MSG \
  "usage: %s -f program_file [-d] [-t timer_interval]\n"

#define OPTIONS_MSG \
  "\t-d\tEnable debugging output.\n"\
  "\t-t int\tSpecifying a timer interrupt interval.\n"

int redirect(int ifd, int ofd);

int load_program(char *filename, int debug);

int dump_memory(FILE *fp);

int main(int argc, char *argv[])
{
  int opt;
  int pid;
  int debug = 0;
  int dryrun = 0;
  int disassemble = 0;
  char *program_file = NULL;
  char *memory_dump = NULL;

  int cpu_to_memory[2];
  int memory_to_cpu[2];
  
  char *timer = DEFAULT_TIMER_INTERVAL;

  opterr = 0;

  setlinebuf(CONSOLE);
  
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
	fprintf(stderr, USAGE_MSG, basename(argv[0]));
	fputs(OPTIONS_MSG, stderr);
	return EXIT_FAILURE;
	/* NOTREACHED */
	break;
    }

  if (!program_file) {
    fprintf(stderr, USAGE_MSG,basename(argv[0]));
    fputs(OPTIONS_MSG, stderr);
    fputs("\tProgram file name required.\n", stderr);
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
      if (debug)
	fprintf(CONSOLE, "[LOAD] Launching memory subsystem.\n");
      redirect(memory_to_cpu[IO_RD], cpu_to_memory[IO_WR]);
      execl("./"MEMORY, MEMORY, (debug)?"-d":"", (char *)NULL);
      break;

      
    default:			/* parent */

      if (debug)
	fprintf(CONSOLE, "[LOAD] Launching cpu subsystem.\n");
      
      redirect(cpu_to_memory[IO_RD], memory_to_cpu[IO_WR]);
      
      if (load_program(program_file, debug) < 0) {
	perror("loader:load_memory");
	return EXIT_FAILURE;
	/* NOTREACHED */
      }

      if (debug) {
	dump_memory(CONSOLE);
      }

      execl("./"CPU, CPU, (debug)?"-d":"", "-t", timer, (char *)NULL);
      
      break;
  }

  return EXIT_SUCCESS;
}


int redirect(int ifd, int ofd)
{
  if (ifd != MEM_RD_CHANNEL) {
    if (dup2(ifd, MEM_RD_CHANNEL) != MEM_RD_CHANNEL)
      return -1;
    close(ifd);
  }

  if (ofd != MEM_WR_CHANNEL) {
    if (dup2(ofd, MEM_WR_CHANNEL) != MEM_WR_CHANNEL)
      return -1;
    close(ofd);
  }
  
  return 0;
}



int load_program(char *filename, int debug)
{
  int fd;
  int address;
  
  if (!filename) {
    errno = EINVAL;
    return -1;
  }

  if ((fd = open(filename, O_RDONLY)) < 0)
    return -1;

  if (read(fd, &address, sizeof(address)) < 0){
    perror("read:magic");
    return -1;
  }

  if (address != MAGIC) {
    errno = EBADF;
    close(fd);
    return -1;
  }

  for (address = USER_PROGRAM_LOAD; address < NWORDS; address++) {
    int value;
    if (read(fd, &value, sizeof(value)) < 0) {
      perror("read");
      return -1;
    }
    write_memory(address, value);
  }

  if (debug)
    fprintf(CONSOLE, "[LOAD] %s\n", filename);

  close(fd);
  
  return 0;
}

#define WORDS_PER_LINE 10

int dump_memory(FILE *fp)
{
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
    
    cp += sprintf(cp, "[DUMP] [%04d] ", line * WORDS_PER_LINE);

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
