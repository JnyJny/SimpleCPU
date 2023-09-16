#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


#define RD 0
#define WR 1

int main(int argc, char *argv[])
{
  int fd1[2];
  int fd2[2];

  int pin, pout;
  int cin, cout;
  int pid;

  char payload = 'E';

  pipe(fd1);
  pipe(fd2);

  int t = 100;
  for(int n=0; n<2000; n++) {
    int m = n % t;
    printf("t = %d n = %4d m = %d\n", t,n,m);
  }

  return EXIT_SUCCESS;

  

  switch(pid = fork()) {
    case -1:			/* errror */
      perror("fork");
      return EXIT_FAILURE;
      break;
      
    case 0:			/* child */

      cin = fd1[RD];
      cout = fd2[WR];
      
      if (read(cin, &payload, sizeof(payload)) < 0)
	perror("child:read:fd1[1]");
      
      fprintf(stdout,"Child: received %c\n", payload);
      
      payload = (payload == 'p')?'Y':'N';
      fprintf(stdout,"Child: sending %c\n", payload);
      if (write(cout, &payload, sizeof(payload)) < 0)
	perror("child:write:fd2[0]");
      break;
      
    default:			/* parent */

      pin = fd2[RD];
      pout = fd1[WR];
	
      payload = 'p';
      fprintf(stdout,"Parent: sending %c\n", payload);
      if (write(pout, &payload, sizeof(payload)) < 0)
	perror("parent:write:fd1[0]");

      payload = 'E';
      if (read(pin, &payload, sizeof(payload)) < 0)
	perror("parent:read:fd2[1]");
      
      fprintf(stdout,"Parent: received %c\n", payload);
      break;
  }
  



  return EXIT_SUCCESS;
}


