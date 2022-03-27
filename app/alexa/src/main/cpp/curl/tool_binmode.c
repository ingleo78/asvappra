#include "tool_setup.h"

#ifdef HAVE_SETMODE
#ifdef HAVE_IO_H
//#include <io.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "tool_binmode.h"
#include "memdebug.h"

void set_binmode(FILE *stream) {
#ifdef O_BINARY
#ifdef __HIGHC__
    _setmode(stream, O_BINARY);
#else
    (void)setmode(fileno(stream), O_BINARY);
#endif
#else
    (void)stream;
#endif
}
#endif