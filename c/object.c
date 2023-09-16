/* object.c (ejo@ufo) 14 Sep 23  Modified: 14 Sep 23  10:58 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "instruction.h"
#include "object.h"


int read_object_file(int fd, word_t *dst, size_t dstsz)
{
  struct stat  stbuf;
  size_t       objsz;
  unsigned int magic;
  
  
  if ((fd<0) || !dst || (dstsz<=0)) {
    errno = EINVAL;
    return -1;
  }

  if (fstat(fd, &stbuf) < 0)
    return -1;

  objsz = stbuf.st_size - sizeof(MAGIC);

  if (objsz > dstsz) {
    errno = ENOMEM;
    return -1;
  }

  if (read(fd, &magic, sizeof(magic)) < 0)
    return -1;
  
  if (magic != MAGIC) {
    errno = EBADF;
    return -1;
  }

  if (read(fd, dst, objsz) < 0)
    return -1;
  
  return 0;
}


int write_object_file(int fd, word_t *src, size_t srcsz)
{
  int magic = MAGIC;
  size_t nobjs;
  
  if ((fd<0) || !src || (srcsz <= 0)) {
    errno = EINVAL;
    return -1;
  }

  if (write(fd, &magic, sizeof(magic)) != sizeof(magic))
    return -1;

  if (write(fd, src, srcsz) != srcsz)
    return -1;
  

  return 0;
}


  
