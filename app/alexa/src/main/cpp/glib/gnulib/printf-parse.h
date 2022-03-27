#ifndef _PRINTF_PARSE_H
#define _PRINTF_PARSE_H

#include "printf-args.h"

#define printf_parse _g_gnulib_printf_parse
#define FLAG_GROUP	 1
#define FLAG_LEFT	 2
#define FLAG_SHOWSIGN	 4
#define FLAG_SPACE	 8
#define FLAG_ALT	16
#define FLAG_ZERO	32
typedef struct {
  const char* dir_start;
  const char* dir_end;
  int flags;
  const char* width_start;
  const char* width_end;
  int width_arg_index;
  const char* precision_start;
  const char* precision_end;
  int precision_arg_index;
  char conversion;
  int arg_index;
} char_directive;
typedef struct {
  unsigned int count;
  char_directive *dir;
  unsigned int max_width_length;
  unsigned int max_precision_length;
} char_directives;
#ifdef STATIC
STATIC
#else
extern
#endif
int printf_parse(const char *format, char_directives *d, arguments *a);

#endif