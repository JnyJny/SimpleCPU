/* io.h (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:15 */

#ifndef IO_H
#define IO_H

#define IO_RD 0
#define IO_WR 1

typedef struct {
  int address;
  int value;
  unsigned int error:8;
  unsigned int op:1;
  unsigned int pad:7;
} io_t;

int read_memory(int address);
int write_memory(int address, int value);

#define DUMP_IO(PREFIX, FP, IOP) \
  fprintf((FP), "%s %c %c A:V:E:O:P=%08d:%08d:%02d:%1d:%05d\n", \
	  (PREFIX)?(PREFIX):"", \
	  (IOP)->op?'W':'R', \
	 ((IOP)->error)?'E':'G',\
	  (IOP)->address,\
	  (IOP)->value,\
	  (IOP)->error,\
	  (IOP)->op,\
	  (IOP)->pad);


#endif	/* IO_H */
