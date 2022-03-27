#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#if defined(_MSC_VER)
#include <io.h>

#define _EXPAT_read _read
#define _EXPAT_read_count_t int
#define _EXPAT_read_req_t unsigned int
#else
#define _EXPAT_read read
#define _EXPAT_read_count_t ssize_t
#define _EXPAT_read_req_t size_t
#endif
#ifndef S_ISREG
#ifndef S_IFREG
#define S_IFREG _S_IFREG
#endif
#ifndef S_IFMT
#define S_IFMT _S_IFMT
#endif
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#endif
#ifndef O_BINARY
#ifdef _O_BINARY
#define O_BINARY _O_BINARY
#else
#define O_BINARY 0
#endif
#endif

#include "xmltchar.h"
#include "filemap.h"

int filemap(const tchar *name, void (*processor)(const void *, size_t, const tchar *, void *arg), void *arg) {
  size_t nbytes;
  int fd;
  _EXPAT_read_count_t n;
  struct stat sb;
  void *p;
  fd = topen(name, O_RDONLY | O_BINARY);
  if (fd < 0) {
      tperror(name);
      return 0;
  }
  if (fstat(fd, &sb) < 0) {
      tperror(name);
      close(fd);
      return 0;
  }
  if (!S_ISREG(sb.st_mode)) {
      ftprintf(stderr, T("%s: not a regular file\n"), name);
      close(fd);
      return 0;
  }
  if (sb.st_size > XML_MAX_CHUNK_LEN) {
      close(fd);
      return 2;
  }
  nbytes = sb.st_size;
  if (nbytes == 0) {
      static const char c = '\0';
      processor(&c, 0, name, arg);
      close(fd);
      return 1;
  }
  p = malloc(nbytes);
  if (!p) {
      ftprintf(stderr, T("%s: out of memory\n"), name);
      close(fd);
      return 0;
  }
  n = _EXPAT_read(fd, p, (_EXPAT_read_req_t)nbytes);
  if (n < 0) {
      tperror(name);
      free(p);
      close(fd);
      return 0;
  }
  if (n != (_EXPAT_read_count_t)nbytes) {
      ftprintf(stderr, T("%s: read unexpected number of bytes\n"), name);
      free(p);
      close(fd);
      return 0;
  }
  processor(p, nbytes, name, arg);
  free(p);
  close(fd);
  return 1;
}
