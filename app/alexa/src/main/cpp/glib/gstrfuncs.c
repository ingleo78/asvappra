#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <ctype.h>
#if !defined (HAVE_STRSIGNAL) || !defined(NO_SYS_SIGLIST_DECL)
#include <signal.h>
#endif
#include "gstrfuncs.h"
#include "gtypes.h"
#include "gprintf.h"
#include "gprintfint.h"
#include "glibintl.h"
#include "gwin32.h"

const guint16 ascii_table_data[256] = {
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x004, 0x104, 0x104, 0x004, 0x104, 0x104, 0x004, 0x004,
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x140, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459,
  0x459, 0x459, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x653, 0x653, 0x653, 0x653, 0x653, 0x653, 0x253,
  0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
  0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
  0x253, 0x253, 0x253, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x473, 0x473, 0x473, 0x473, 0x473, 0x473, 0x073,
  0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
  0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
  0x073, 0x073, 0x073, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x004
};
#define gchar char
#define gboolean gint
const guint16* g_ascii_table = ascii_table_data;
gchar* g_strdup(const gchar *str) {
  gchar *new_str;
  gsize length;
  if (str) {
      length = strlen(str) + 1;
      new_str = g_new(char, length);
      memcpy(new_str, str, length);
  } else new_str = NULL;
  return new_str;
}
gpointer g_memdup(gconstpointer mem, guint byte_size) {
  gpointer new_mem;
  if (mem) {
      new_mem = g_malloc(byte_size);
      memcpy(new_mem, mem, byte_size);
  } else new_mem = NULL;
  return new_mem;
}
gchar* g_strndup(const gchar *str, gsize n) {
  gchar *new_str;
  if (str) {
      new_str = g_new(gchar, n + 1);
      strncpy(new_str, str, n);
      new_str[n] = '\0';
  } else new_str = NULL;
  return new_str;
}
gchar* g_strnfill(gsize length, gchar fill_char) {
  gchar *str;
  str = g_new(gchar, length + 1);
  memset(str, (guchar)fill_char, length);
  str[length] = '\0';
  return str;
}
gchar* g_stpcpy(gchar *dest, const gchar *src) {
#ifdef HAVE_STPCPY
  g_return_val_if_fail(dest != NULL, NULL);
  g_return_val_if_fail(src != NULL, NULL);
  return stpcpy(dest, src);
#else
  register gchar *d = dest;
  register const gchar *s = src;
  g_return_val_if_fail (dest != NULL, NULL);
  g_return_val_if_fail (src != NULL, NULL);
  do {
    *d++ = *s;
  } while (*s++ != '\0');
  return d - 1;
#endif
}
gchar* g_strdup_vprintf(const gchar *format, va_list args) {
  gchar *string = NULL;
  g_vasprintf(&string, format, args);
  return string;
}
gchar* g_strdup_printf(const gchar *format, ...) {
  gchar *buffer;
  va_list args;
  va_start(args, format);
  buffer = g_strdup_vprintf(format, args);
  va_end(args);
  return buffer;
}
gchar* g_strconcat(const gchar *string1, ...) {
  gsize l;
  va_list args;
  gchar *s;
  gchar *concat;
  gchar *ptr;
  if (!string1) return NULL;
  l = 1 + strlen(string1);
  va_start(args, string1);
  s = va_arg(args, gchar*);
  while(s) {
      l += strlen(s);
      s = va_arg(args, gchar*);
  }
  va_end(args);
  concat = g_new(gchar, l);
  ptr = concat;
  ptr = g_stpcpy(ptr, string1);
  va_start(args, string1);
  s = va_arg(args, gchar*);
  while(s) {
      ptr = g_stpcpy(ptr, s);
      s = va_arg(args, gchar*);
  }
  va_end(args);
  return concat;
}
gdouble g_strtod(const gchar *nptr, gchar **endptr) {
  gchar *fail_pos_1;
  gchar *fail_pos_2;
  gdouble val_1;
  gdouble val_2 = 0;
  g_return_val_if_fail(nptr != NULL, 0);
  fail_pos_1 = NULL;
  fail_pos_2 = NULL;
  val_1 = strtod(nptr, &fail_pos_1);
  if (fail_pos_1 && fail_pos_1[0] != 0) val_2 = g_ascii_strtod(nptr, &fail_pos_2);
  if (!fail_pos_1 || fail_pos_1[0] == 0 || fail_pos_1 >= fail_pos_2) {
      if (endptr) *endptr = fail_pos_1;
      return val_1;
  } else {
      if (endptr) *endptr = fail_pos_2;
      return val_2;
  }
}
gdouble g_ascii_strtod(const gchar *nptr, gchar **endptr) {
  gchar *fail_pos;
  gdouble val;
  struct lconv *locale_data;
  const char *decimal_point;
  int decimal_point_len;
  const char *p, *decimal_point_pos;
  const char *end = NULL;
  int strtod_errno;
  g_return_val_if_fail(nptr != NULL, 0);
  fail_pos = NULL;
  locale_data = localeconv();
  decimal_point = locale_data->decimal_point;
  decimal_point_len = strlen(decimal_point);
  g_assert(decimal_point_len != 0);
  decimal_point_pos = NULL;
  end = NULL;
  if (decimal_point[0] != '.' || decimal_point[1] != 0) {
      p = nptr;
      while (g_ascii_isspace(*p)) p++;
      if (*p == '+' || *p == '-') p++;
      if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
          p += 2;
          while(g_ascii_isxdigit(*p)) p++;
          if (*p == '.') decimal_point_pos = p++;
          while(g_ascii_isxdigit(*p)) p++;
          if (*p == 'p' || *p == 'P') p++;
          if (*p == '+' || *p == '-') p++;
          while(g_ascii_isdigit(*p)) p++;
          end = p;
      } else if (g_ascii_isdigit(*p) || *p == '.') {
          while(g_ascii_isdigit(*p)) p++;
          if (*p == '.') decimal_point_pos = p++;
          while(g_ascii_isdigit(*p)) p++;
          if (*p == 'e' || *p == 'E') p++;
          if (*p == '+' || *p == '-') p++;
          while(g_ascii_isdigit(*p)) p++;
          end = p;
      }
  }
  if (decimal_point_pos) {
      char *copy, *c;
      copy = g_malloc(end - nptr + 1 + decimal_point_len);
      c = copy;
      memcpy(c, nptr, decimal_point_pos - nptr);
      c += decimal_point_pos - nptr;
      memcpy(c, decimal_point, decimal_point_len);
      c += decimal_point_len;
      memcpy(c, decimal_point_pos + 1, end - (decimal_point_pos + 1));
      c += end - (decimal_point_pos + 1);
      *c = 0;
      errno = 0;
      val = strtod(copy, &fail_pos);
      strtod_errno = errno;
      if (fail_pos) {
          if (fail_pos - copy > decimal_point_pos - nptr) fail_pos = (char*)nptr + (fail_pos - copy) - (decimal_point_len - 1);
          else fail_pos = (char*)nptr + (fail_pos - copy);
      }
      g_free (copy);
  } else if (end) {
      char *copy;
      copy = g_malloc(end - (char*)nptr + 1);
      memcpy(copy, nptr, end - nptr);
      *(copy + (end - (char*)nptr)) = 0;
      errno = 0;
      val = strtod(copy, &fail_pos);
      strtod_errno = errno;
      if (fail_pos) fail_pos = (char*)nptr + (fail_pos - copy);
      g_free (copy);
  } else {
      errno = 0;
      val = strtod(nptr, &fail_pos);
      strtod_errno = errno;
  }
  if (endptr) *endptr = fail_pos;
  errno = strtod_errno;
  return val;
}
gchar* g_ascii_dtostr(gchar *buffer, gint buf_len, gdouble d) {
  return g_ascii_formatd(buffer, buf_len, "%.17g", d);
}
gchar* g_ascii_formatd(gchar *buffer, gint buf_len, const gchar *format, gdouble d) {
  struct lconv *locale_data;
  const char *decimal_point;
  int decimal_point_len;
  gchar *p;
  int rest_len;
  gchar format_char;
  g_return_val_if_fail(buffer != NULL, NULL);
  g_return_val_if_fail(format[0] == '%', NULL);
  g_return_val_if_fail(strpbrk(format + 1, "'l%") == NULL, NULL);
  format_char = format[strlen(format) - 1];
  g_return_val_if_fail(format_char == 'e' || format_char == 'E' || format_char == 'f' || format_char == 'F' || format_char == 'g' || format_char == 'G',
                       NULL);
  if (format[0] != '%') return NULL;
  if (strpbrk (format + 1, "'l%")) return NULL;
  if (!(format_char == 'e' || format_char == 'E' || format_char == 'f' || format_char == 'F' || format_char == 'g' || format_char == 'G')) return NULL;
  _g_snprintf(buffer, buf_len, format, d);
  locale_data = localeconv();
  decimal_point = locale_data->decimal_point;
  decimal_point_len = strlen(decimal_point);
  g_assert(decimal_point_len != 0);
  if (decimal_point[0] != '.' || decimal_point[1] != 0) {
      p = buffer;
      while(g_ascii_isspace(*p)) p++;
      if (*p == '+' || *p == '-') p++;
      while(isdigit((guchar)*p)) p++;
      if (strncmp (p, decimal_point, decimal_point_len) == 0) {
          *p = '.';
          p++;
          if (decimal_point_len > 1) {
              rest_len = strlen (p + (decimal_point_len-1));
              memmove (p, p + (decimal_point_len-1), rest_len);
              p[rest_len] = 0;
          }
      }
  }
  return buffer;
}
static unsigned long long g_parse_long_long(const gchar *nptr, const gchar **endptr, guint base, gboolean *negative) {
  #define ISSPACE(c)  ((c) == ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == '\v')
  #define ISUPPER(c)  ((c) >= 'A' && (c) <= 'Z')
  #define ISLOWER(c)  ((c) >= 'a' && (c) <= 'z')
  #define ISALPHA(c)  (ISUPPER(c) || ISLOWER(c))
  #define TOUPPER(c)  (ISLOWER(c) ? (c) - 'a' + 'A' : (c))
  #define TOLOWER(c)  (ISUPPER(c) ? (c) - 'A' + 'a' : (c))
  gint overflow;
  unsigned long long cutoff;
  unsigned long long cutlim;
  unsigned long long ui64;
  const gchar *s, *save;
  guchar c;
  g_return_val_if_fail(nptr != NULL, 0);
  *negative = FALSE;
  if (base == 1 || base > 36) {
      errno = EINVAL;
      if (endptr) *endptr = nptr;
      return 0;
  }
  save = s = nptr;
  while(ISSPACE (*s)) ++s;
  if (G_UNLIKELY (!*s)) goto noconv;
  if (*s == '-') {
      *negative = TRUE;
      ++s;
  } else if (*s == '+') ++s;
  if (*s == '0') {
      if ((base == 0 || base == 16) && TOUPPER (s[1]) == 'X') {
          s += 2;
          base = 16;
      } else if (base == 0) base = 8;
  } else if (base == 0) base = 10;
  save = s;
  cutoff = G_MAXUINT64 / base;
  cutlim = G_MAXUINT64 % base;
  overflow = FALSE;
  ui64 = 0;
  c = *s;
  for (; c; c = *++s) {
      if (c >= '0' && c <= '9') c -= '0';
      else if (ISALPHA(c)) c = TOUPPER(c) - 'A' + 10;
      else break;
      if (c >= base) break;
      if (ui64 > cutoff || (ui64 == cutoff && c > cutlim)) overflow = TRUE;
      else {
          ui64 *= base;
          ui64 += c;
      }
  }
  if (s == save) goto noconv;
  if (endptr) *endptr = s;
  if (G_UNLIKELY(overflow)) {
      errno = ERANGE;
      return G_MAXUINT64;
  }
  return ui64;
  noconv:
  if (endptr) {
      if (save - nptr >= 2 && TOUPPER (save[-1]) == 'X' && save[-2] == '0') *endptr = &save[-1];
      else *endptr = nptr;
  }
  return 0;
}
unsigned long long g_ascii_strtoull(const gchar *nptr, gchar **endptr, guint base) {
  gboolean negative;
  unsigned long long result;
  result = g_parse_long_long(nptr, (const gchar**)endptr, base, &negative);
  return negative ? -result : result;
}
gint64 g_ascii_strtoll(const gchar *nptr, gchar **endptr, guint base) {
  gboolean negative;
  unsigned long long result;
  result = g_parse_long_long(nptr, (const gchar **)endptr, base, &negative);
  if (negative && result > (guint64)G_MININT64) {
      errno = ERANGE;
      return G_MININT64;
  } else if (!negative && result > (guint64)G_MAXINT64) {
      errno = ERANGE;
      return G_MAXINT64;
  } else if (negative) return - (gint64)result;
  else return (gint64)result;
}
G_CONST_RETURN gchar* g_strerror(gint errnum) {
  static GStaticPrivate msg_private = G_STATIC_PRIVATE_INIT;
  char *msg;
  int saved_errno = errno;
#ifdef HAVE_STRERROR
  const char *msg_locale;
  msg_locale = strerror(errnum);
  if (g_get_charset (NULL)) {
      errno = saved_errno;
      return msg_locale;
  } else {
      gchar *msg_utf8 = g_locale_to_utf8(msg_locale, -1, NULL, NULL, NULL);
      if (msg_utf8) {
          GQuark msg_quark = g_quark_from_string(msg_utf8);
          g_free(msg_utf8);
          msg_utf8 = (gchar *)g_quark_to_string(msg_quark);
          errno = saved_errno;
          return msg_utf8;
      }
  }
#elif NO_SYS_ERRLIST
  switch (errnum) {
  #ifdef E2BIG
      case E2BIG: return "argument list too long";
  #endif
  #ifdef EACCES
      case EACCES: return "permission denied";
  #endif
  #ifdef EADDRINUSE
      case EADDRINUSE: return "address already in use";
  #endif
  #ifdef EADDRNOTAVAIL
      case EADDRNOTAVAIL: return "can't assign requested address";
  #endif
  #ifdef EADV
      case EADV: return "advertise error";
  #endif
  #ifdef EAFNOSUPPORT
      case EAFNOSUPPORT: return "address family not supported by protocol family";
  #endif
  #ifdef EAGAIN
      case EAGAIN: return "try again";
  #endif
  #ifdef EALIGN
      case EALIGN: return "EALIGN";
  #endif
  #ifdef EALREADY
      case EALREADY: return "operation already in progress";
  #endif
  #ifdef EBADE
      case EBADE: return "bad exchange descriptor";
  #endif
  #ifdef EBADF
      case EBADF: return "bad file number";
  #endif
  #ifdef EBADFD
      case EBADFD: return "file descriptor in bad state";
  #endif
  #ifdef EBADMSG
      case EBADMSG: return "not a data message";
  #endif
  #ifdef EBADR
      case EBADR: return "bad request descriptor";
  #endif
  #ifdef EBADRPC
      case EBADRPC: return "RPC structure is bad";
  #endif
  #ifdef EBADRQC
      case EBADRQC: return "bad request code";
  #endif
  #ifdef EBADSLT
      case EBADSLT: return "invalid slot";
  #endif
  #ifdef EBFONT
      case EBFONT: return "bad font file format";
  #endif
  #ifdef EBUSY
      case EBUSY: return "mount device busy";
  #endif
  #ifdef ECHILD
      case ECHILD: return "no children";
  #endif
  #ifdef ECHRNG
      case ECHRNG: return "channel number out of range";
  #endif
  #ifdef ECOMM
      case ECOMM: return "communication error on send";
  #endif
  #ifdef ECONNABORTED
      case ECONNABORTED: return "software caused connection abort";
  #endif
  #ifdef ECONNREFUSED
      case ECONNREFUSED: return "connection refused";
  #endif
  #ifdef ECONNRESET
      case ECONNRESET: return "connection reset by peer";
  #endif
  #if defined(EDEADLK) && (!defined(EWOULDBLOCK) || (EDEADLK != EWOULDBLOCK))
      case EDEADLK: return "resource deadlock avoided";
  #endif
  #if defined(EDEADLOCK) && (!defined(EDEADLK) || (EDEADLOCK != EDEADLK))
      case EDEADLOCK: return "resource deadlock avoided";
  #endif
  #ifdef EDESTADDRREQ
      case EDESTADDRREQ: return "destination address required";
  #endif
  #ifdef EDIRTY
      case EDIRTY: return "mounting a dirty fs w/o force";
  #endif
  #ifdef EDOM
      case EDOM: return "math argument out of range";
  #endif
  #ifdef EDOTDOT
      case EDOTDOT: return "cross mount point";
  #endif
  #ifdef EDQUOT
      case EDQUOT: return "disk quota exceeded";
  #endif
  #ifdef EDUPPKG
      case EDUPPKG: return "duplicate package name";
  #endif
  #ifdef EEXIST
      case EEXIST: return "file already exists";
  #endif
  #ifdef EFAULT
      case EFAULT: return "bad address in system call argument";
  #endif
  #ifdef EFBIG
      case EFBIG: return "file too large";
  #endif
  #ifdef EHOSTDOWN
      case EHOSTDOWN: return "host is down";
  #endif
  #ifdef EHOSTUNREACH
      case EHOSTUNREACH: return "host is unreachable";
  #endif
  #ifdef EIDRM
      case EIDRM: return "identifier removed";
  #endif
  #ifdef EINIT
      case EINIT: return "initialization error";
  #endif
  #ifdef EINPROGRESS
      case EINPROGRESS: return "operation now in progress";
  #endif
  #ifdef EINTR
      case EINTR: return "interrupted system call";
  #endif
  #ifdef EINVAL
      case EINVAL: return "invalid argument";
  #endif
  #ifdef EIO
      case EIO: return "I/O error";
  #endif
  #ifdef EISCONN
      case EISCONN: return "socket is already connected";
  #endif
  #ifdef EISDIR
      case EISDIR: return "is a directory";
  #endif
  #ifdef EISNAME
      case EISNAM: return "is a name file";
  #endif
  #ifdef ELBIN
      case ELBIN: return "ELBIN";
  #endif
  #ifdef EL2HLT
      case EL2HLT: return "level 2 halted";
  #endif
  #ifdef EL2NSYNC
      case EL2NSYNC: return "level 2 not synchronized";
  #endif
  #ifdef EL3HLT
      case EL3HLT: return "level 3 halted";
  #endif
  #ifdef EL3RST
      case EL3RST: return "level 3 reset";
  #endif
  #ifdef ELIBACC
      case ELIBACC: return "can not access a needed shared library";
  #endif
  #ifdef ELIBBAD
      case ELIBBAD: return "accessing a corrupted shared library";
  #endif
  #ifdef ELIBEXEC
      case ELIBEXEC: return "can not exec a shared library directly";
  #endif
  #ifdef ELIBMAX
      case ELIBMAX: return "attempting to link in more shared libraries than system limit";
  #endif
  #ifdef ELIBSCN
      case ELIBSCN: return ".lib section in a.out corrupted";
  #endif
  #ifdef ELNRNG
      case ELNRNG: return "link number out of range";
  #endif
  #ifdef ELOOP
      case ELOOP: return "too many levels of symbolic links";
  #endif
  #ifdef EMFILE
      case EMFILE: return "too many open files";
  #endif
  #ifdef EMLINK
      case EMLINK: return "too many links";
  #endif
  #ifdef EMSGSIZE
      case EMSGSIZE: return "message too long";
  #endif
  #ifdef EMULTIHOP
      case EMULTIHOP: return "multihop attempted";
  #endif
  #ifdef ENAMETOOLONG
      case ENAMETOOLONG: return "file name too long";
  #endif
  #ifdef ENAVAIL
      case ENAVAIL: return "not available";
  #endif
  #ifdef ENET
      case ENET: return "ENET";
  #endif
  #ifdef ENETDOWN
      case ENETDOWN: return "network is down";
  #endif
  #ifdef ENETRESET
      case ENETRESET: return "network dropped connection on reset";
  #endif
  #ifdef ENETUNREACH
      case ENETUNREACH: return "network is unreachable";
  #endif
  #ifdef ENFILE
      case ENFILE: return "file table overflow";
  #endif
  #ifdef ENOANO
      case ENOANO: return "anode table overflow";
  #endif
  #if defined(ENOBUFS) && (!defined(ENOSR) || (ENOBUFS != ENOSR))
      case ENOBUFS: return "no buffer space available";
  #endif
  #ifdef ENOCSI
      case ENOCSI: return "no CSI structure available";
  #endif
  #ifdef ENODATA
      case ENODATA: return "no data available";
  #endif
  #ifdef ENODEV
      case ENODEV: return "no such device";
  #endif
  #ifdef ENOENT
      case ENOENT: return "no such file or directory";
  #endif
  #ifdef ENOEXEC
      case ENOEXEC: return "exec format error";
  #endif
  #ifdef ENOLCK
      case ENOLCK: return "no locks available";
  #endif
  #ifdef ENOLINK
      case ENOLINK: return "link has be severed";
  #endif
  #ifdef ENOMEM
      case ENOMEM: return "not enough memory";
  #endif
  #ifdef ENOMSG
      case ENOMSG: return "no message of desired type";
  #endif
  #ifdef ENONET
      case ENONET: return "machine is not on the network";
  #endif
  #ifdef ENOPKG
      case ENOPKG: return "package not installed";
  #endif
  #ifdef ENOPROTOOPT
      case ENOPROTOOPT: return "bad proocol option";
  #endif
  #ifdef ENOSPC
      case ENOSPC: return "no space left on device";
  #endif
  #ifdef ENOSR
      case ENOSR: return "out of stream resources";
  #endif
  #ifdef ENOSTR
      case ENOSTR: return "not a stream device";
  #endif
  #ifdef ENOSYM
      case ENOSYM: return "unresolved symbol name";
  #endif
  #ifdef ENOSYS
      case ENOSYS: return "function not implemented";
  #endif
  #ifdef ENOTBLK
      case ENOTBLK: return "block device required";
  #endif
  #ifdef ENOTCONN
      case ENOTCONN: return "socket is not connected";
  #endif
  #ifdef ENOTDIR
      case ENOTDIR: return "not a directory";
  #endif
  #ifdef ENOTEMPTY
      case ENOTEMPTY: return "directory not empty";
  #endif
  #ifdef ENOTNAM
      case ENOTNAM: return "not a name file";
  #endif
  #ifdef ENOTSOCK
      case ENOTSOCK: return "socket operation on non-socket";
  #endif
  #ifdef ENOTTY
      case ENOTTY: return "inappropriate device for ioctl";
  #endif
  #ifdef ENOTUNIQ
      case ENOTUNIQ: return "name not unique on network";
  #endif
  #ifdef ENXIO
      case ENXIO: return "no such device or address";
  #endif
  #ifdef EOPNOTSUPP
      case EOPNOTSUPP: return "operation not supported on socket";
  #endif
  #ifdef EPERM
      case EPERM: return "not owner";
  #endif
  #ifdef EPFNOSUPPORT
      case EPFNOSUPPORT: return "protocol family not supported";
  #endif
  #ifdef EPIPE
      case EPIPE: return "broken pipe";
  #endif
  #ifdef EPROCLIM
      case EPROCLIM: return "too many processes";
  #endif
  #ifdef EPROCUNAVAIL
      case EPROCUNAVAIL: return "bad procedure for program";
  #endif
  #ifdef EPROGMISMATCH
      case EPROGMISMATCH: return "program version wrong";
  #endif
  #ifdef EPROGUNAVAIL
      case EPROGUNAVAIL: return "RPC program not available";
  #endif
  #ifdef EPROTO
      case EPROTO: return "protocol error";
  #endif
  #ifdef EPROTONOSUPPORT
      case EPROTONOSUPPORT: return "protocol not suppored";
  #endif
  #ifdef EPROTOTYPE
      case EPROTOTYPE: return "protocol wrong type for socket";
  #endif
  #ifdef ERANGE
      case ERANGE: return "math result unrepresentable";
  #endif
  #if defined(EREFUSED) && (!defined(ECONNREFUSED) || (EREFUSED != ECONNREFUSED))
      case EREFUSED: return "EREFUSED";
  #endif
  #ifdef EREMCHG
      case EREMCHG: return "remote address changed";
  #endif
  #ifdef EREMDEV
      case EREMDEV: return "remote device";
  #endif
  #ifdef EREMOTE
      case EREMOTE: return "pathname hit remote file system";
  #endif
  #ifdef EREMOTEIO
      case EREMOTEIO: return "remote i/o error";
  #endif
  #ifdef EREMOTERELEASE
      case EREMOTERELEASE: return "EREMOTERELEASE";
  #endif
  #ifdef EROFS
      case EROFS: return "read-only file system";
  #endif
  #ifdef ERPCMISMATCH
      case ERPCMISMATCH: return "RPC version is wrong";
  #endif
  #ifdef ERREMOTE
      case ERREMOTE: return "object is remote";
  #endif
  #ifdef ESHUTDOWN
      case ESHUTDOWN: return "can't send afer socket shutdown";
  #endif
  #ifdef ESOCKTNOSUPPORT
      case ESOCKTNOSUPPORT: return "socket type not supported";
  #endif
  #ifdef ESPIPE
      case ESPIPE: return "invalid seek";
  #endif
  #ifdef ESRCH
      case ESRCH: return "no such process";
  #endif
  #ifdef ESRMNT
      case ESRMNT: return "srmount error";
  #endif
  #ifdef ESTALE
      case ESTALE: return "stale remote file handle";
  #endif
  #ifdef ESUCCESS
      case ESUCCESS: return "Error 0";
  #endif
  #ifdef ETIME
      case ETIME: return "timer expired";
  #endif
  #ifdef ETIMEDOUT
      case ETIMEDOUT: return "connection timed out";
  #endif
  #ifdef ETOOMANYREFS
      case ETOOMANYREFS: return "too many references: can't splice";
  #endif
  #ifdef ETXTBSY
      case ETXTBSY: return "text file or pseudo-device busy";
  #endif
  #ifdef EUCLEAN
      case EUCLEAN: return "structure needs cleaning";
  #endif
  #ifdef EUNATCH
      case EUNATCH: return "protocol driver not attached";
  #endif
  #ifdef EUSERS
      case EUSERS: return "too many users";
  #endif
  #ifdef EVERSION
      case EVERSION: return "version mismatch";
  #endif
  #if defined(EWOULDBLOCK) && (!defined(EAGAIN) || (EWOULDBLOCK != EAGAIN))
      case EWOULDBLOCK: return "operation would block";
  #endif
  #ifdef EXDEV
      case EXDEV: return "cross-domain link";
  #endif
  #ifdef EXFULL
      case EXFULL: return "message tables full";
  #endif
  }
#else
  extern int sys_nerr;
  extern char *sys_errlist[];
  if ((errnum > 0) && (errnum <= sys_nerr)) return sys_errlist[errnum];
#endif
  msg = g_static_private_get(&msg_private);
  if (!msg) {
      msg = g_new(gchar, 64);
      g_static_private_set(&msg_private, msg, g_free);
  }
  _g_sprintf(msg, "unknown error (%d)", errnum);
  errno = saved_errno;
  return msg;
}
G_CONST_RETURN gchar* g_strsignal(gint signum) {
  static GStaticPrivate msg_private = G_STATIC_PRIVATE_INIT;
  char *msg;
#ifdef HAVE_STRSIGNAL
  const char *msg_locale;
#if defined(G_OS_BEOS) || defined(G_WITH_CYGWIN)
  extern const char *strsignal(int);
#else
  extern char *strsignal (int sig);
#endif
  msg_locale = strsignal (signum);
  if (g_get_charset (NULL)) return msg_locale;
  else {
      gchar *msg_utf8 = g_locale_to_utf8(msg_locale, -1, NULL, NULL, NULL);
      if (msg_utf8) {
          GQuark msg_quark = g_quark_from_string(msg_utf8);
          g_free(msg_utf8);
          return g_quark_to_string (msg_quark);
      }
  }
#elif NO_SYS_SIGLIST
  switch (signum) {
  #ifdef SIGHUP
      case SIGHUP: return "Hangup";
  #endif
  #ifdef SIGINT
      case SIGINT: return "Interrupt";
  #endif
  #ifdef SIGQUIT
      case SIGQUIT: return "Quit";
  #endif
  #ifdef SIGILL
      case SIGILL: return "Illegal instruction";
  #endif
  #ifdef SIGTRAP
      case SIGTRAP: return "Trace/breakpoint trap";
  #endif
  #ifdef SIGABRT
      case SIGABRT: return "IOT trap/Abort";
  #endif
  #ifdef SIGBUS
      case SIGBUS: return "Bus error";
  #endif
  #ifdef SIGFPE
      case SIGFPE: return "Floating point exception";
  #endif
  #ifdef SIGKILL
      case SIGKILL: return "Killed";
  #endif
  #ifdef SIGUSR1
      case SIGUSR1: return "User defined signal 1";
  #endif
  #ifdef SIGSEGV
      case SIGSEGV: return "Segmentation fault";
  #endif
  #ifdef SIGUSR2
      case SIGUSR2: return "User defined signal 2";
  #endif
  #ifdef SIGPIPE
      case SIGPIPE: return "Broken pipe";
  #endif
  #ifdef SIGALRM
      case SIGALRM: return "Alarm clock";
  #endif
  #ifdef SIGTERM
      case SIGTERM: return "Terminated";
  #endif
  #ifdef SIGSTKFLT
      case SIGSTKFLT: return "Stack fault";
  #endif
  #ifdef SIGCHLD
      case SIGCHLD: return "Child exited";
  #endif
  #ifdef SIGCONT
      case SIGCONT: return "Continued";
  #endif
  #ifdef SIGSTOP
      case SIGSTOP: return "Stopped (signal)";
  #endif
  #ifdef SIGTSTP
      case SIGTSTP: return "Stopped";
  #endif
  #ifdef SIGTTIN
      case SIGTTIN: return "Stopped (tty input)";
  #endif
  #ifdef SIGTTOU
      case SIGTTOU: return "Stopped (tty output)";
  #endif
  #ifdef SIGURG
      case SIGURG: return "Urgent condition";
  #endif
  #ifdef SIGXCPU
      case SIGXCPU: return "CPU time limit exceeded";
  #endif
  #ifdef SIGXFSZ
      case SIGXFSZ: return "File size limit exceeded";
  #endif
  #ifdef SIGVTALRM
      case SIGVTALRM: return "Virtual time alarm";
  #endif
  #ifdef SIGPROF
      case SIGPROF: return "Profile signal";
  #endif
  #ifdef SIGWINCH
      case SIGWINCH: return "Window size changed";
  #endif
  #ifdef SIGIO
      case SIGIO: return "Possible I/O";
  #endif
  #ifdef SIGPWR
      case SIGPWR: return "Power failure";
  #endif
  #ifdef SIGUNUSED
      case SIGUNUSED: return "Unused signal";
  #endif
  }
#else
#ifdef NO_SYS_SIGLIST_DECL
  extern char *sys_siglist[];
#endif
  return (char*)sys_siglist [signum];
#endif
  msg = g_static_private_get (&msg_private);
  if (!msg) {
      msg = g_new (gchar, 64);
      g_static_private_set (&msg_private, msg, g_free);
  }
  _g_sprintf (msg, "unknown signal (%d)", signum);
  return msg;
}
#ifdef HAVE_STRLCPY
gsize g_strlcpy(gchar *dest, const gchar *src, gsize dest_size) {
  g_return_val_if_fail(dest != NULL, 0);
  g_return_val_if_fail(src  != NULL, 0);
  return strlcpy(dest, src, dest_size);
}
gsize g_strlcat(gchar *dest, const gchar *src, gsize dest_size) {
  g_return_val_if_fail(dest != NULL, 0);
  g_return_val_if_fail(src  != NULL, 0);
  return strlcat(dest, src, dest_size);
}
#else
gsize g_strlcpy(gchar *dest, const gchar *src, gsize dest_size) {
  register gchar *d = dest;
  register const gchar *s = src;
  register gsize n = dest_size;
  g_return_val_if_fail(dest != NULL, 0);
  g_return_val_if_fail(src  != NULL, 0);
  if (n != 0 && --n != 0)
    do {
        register gchar c = *s++;
        *d++ = c;
        if (c == 0) break;
    } while (--n != 0);
  if (n == 0) {
      if (dest_size != 0) {
          *d = 0;
      } while(*s++);
  }
  return s - src - 1;
}
gsize g_strlcat(gchar *dest, const gchar *src, gsize dest_size) {
  register gchar *d = dest;
  register const gchar *s = src;
  register gsize bytes_left = dest_size;
  gsize dlength;
  g_return_val_if_fail(dest != NULL, 0);
  g_return_val_if_fail(src  != NULL, 0);
  while(*d != 0 && bytes_left-- != 0) d++;
  dlength = d - dest;
  bytes_left = dest_size - dlength;
  if (bytes_left == 0) return dlength + strlen(s);
  while(*s != 0) {
      if (bytes_left != 1) {
          *d++ = *s;
          bytes_left--;
      }
      s++;
  }
  *d = 0;
  return dlength + (s - src);
}
#endif
gchar* g_ascii_strdown(const gchar *str, gssize len) {
  gchar *result, *s;
  g_return_val_if_fail(str != NULL, NULL);
  if (len < 0) len = strlen (str);
  result = g_strndup (str, len);
  for (s = result; *s; s++)
    *s = g_ascii_tolower(*s);
  return result;
}
gchar*
g_ascii_strup (const gchar *str, gssize len) {
  gchar *result, *s;
  g_return_val_if_fail(str != NULL, NULL);
  if (len < 0) len = strlen(str);
  result = g_strndup (str, len);
  for (s = result; *s; s++)
    *s = g_ascii_toupper(*s);
  return result;
}
gchar* g_strdown(gchar *string) {
  register guchar *s;
  g_return_val_if_fail(string != NULL, NULL);
  s = (guchar *)string;
  while(*s) {
      if (isupper(*s)) *s = tolower(*s);
      s++;
  }
  return (gchar*)string;
}
gchar* g_strup(gchar *string) {
  register guchar *s;
  g_return_val_if_fail(string != NULL, NULL);
  s = (guchar*)string;
  while(*s) {
      if (islower(*s)) *s = toupper(*s);
      s++;
  }
  return (gchar*)string;
}
gchar* g_strreverse(gchar *string) {
  g_return_val_if_fail(string != NULL, NULL);
  if (*string) {
      register gchar *h, *t;
      h = string;
      t = string + strlen(string) - 1;
      while(h < t) {
          register gchar c;
          c = *h;
          *h = *t;
          h++;
          *t = c;
          t--;
      }
  }
  return string;
}
gchar g_ascii_tolower(gchar c) {
  return g_ascii_isupper(c) ? c - 'A' + 'a' : c;
}
gchar g_ascii_toupper(gchar c) {
  return g_ascii_islower(c) ? c - 'a' + 'A' : c;
}
int g_ascii_digit_value(gchar c) {
  if (g_ascii_isdigit(c)) return c - '0';
  return -1;
}
int g_ascii_xdigit_value(gchar c) {
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return g_ascii_digit_value(c);
}
gint g_ascii_strcasecmp(const gchar *s1, const gchar *s2) {
  gint c1, c2;
  g_return_val_if_fail(s1 != NULL, 0);
  g_return_val_if_fail(s2 != NULL, 0);
  while(*s1 && *s2) {
      c1 = (gint)(guchar)TOLOWER(*s1);
      c2 = (gint)(guchar)TOLOWER(*s2);
      if (c1 != c2) return (c1 - c2);
      s1++; s2++;
  }
  return (((gint)(guchar)*s1) - ((gint)(guchar)*s2));
}
gint g_ascii_strncasecmp(const gchar *s1, const gchar *s2, gsize n) {
  gint c1, c2;
  g_return_val_if_fail(s1 != NULL, 0);
  g_return_val_if_fail(s2 != NULL, 0);
  while (n && *s1 && *s2) {
      n -= 1;
      c1 = (gint)(guchar)TOLOWER(*s1);
      c2 = (gint)(guchar)TOLOWER(*s2);
      if (c1 != c2) return (c1 - c2);
      s1++; s2++;
  }
  if (n) return (((gint)(guchar)*s1) - ((gint)(guchar)*s2));
  else return 0;
}
gint g_strcasecmp(const gchar *s1, const gchar *s2) {
#ifdef HAVE_STRCASECMP
  g_return_val_if_fail(s1 != NULL, 0);
  g_return_val_if_fail(s2 != NULL, 0);
  return strcasecmp(s1, s2);
#else
  gint c1, c2;
  g_return_val_if_fail(s1 != NULL, 0);
  g_return_val_if_fail(s2 != NULL, 0);
  while(*s1 && *s2) {
      c1 = isupper((guchar)*s1) ? tolower((guchar)*s1) : *s1;
      c2 = isupper((guchar)*s2) ? tolower((guchar)*s2) : *s2;
      if (c1 != c2) return (c1 - c2);
      s1++; s2++;
  }
  return (((gint)(guchar)*s1) - ((gint)(guchar)*s2));
#endif
}
gint g_strncasecmp(const gchar *s1, const gchar *s2, guint n) {
#ifdef HAVE_STRNCASECMP
  return strncasecmp(s1, s2, n);
#else
  gint c1, c2;
  g_return_val_if_fail(s1 != NULL, 0);
  g_return_val_if_fail(s2 != NULL, 0);
  while (n && *s1 && *s2) {
      n -= 1;
      c1 = isupper ((guchar)*s1) ? tolower ((guchar)*s1) : *s1;
      c2 = isupper ((guchar)*s2) ? tolower ((guchar)*s2) : *s2;
      if (c1 != c2) return (c1 - c2);
      s1++; s2++;
  }
  if (n) return (((gint)(guchar)*s1) - ((gint)(guchar)*s2));
  else return 0;
#endif
}
gchar* g_strdelimit(gchar *string, const gchar *delimiters, gchar new_delim) {
  register gchar *c;
  g_return_val_if_fail(string != NULL, NULL);
  if (!delimiters) delimiters = G_STR_DELIMITERS;
  for (c = string; *c; c++) {
      if (strchr(delimiters, *c)) *c = new_delim;
  }
  return string;
}
gchar* g_strcanon(gchar *string, const gchar *valid_chars, gchar substitutor) {
  register gchar *c;
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(valid_chars != NULL, NULL);
  for (c = string; *c; c++) {
      if (!strchr(valid_chars, *c)) *c = substitutor;
  }
  return string;
}
gchar* g_strcompress(const gchar *source) {
  const gchar *p = source, *octal;
  gchar *dest = g_malloc(strlen (source) + 1);
  gchar *q = dest;
  while(*p) {
      if (*p == '\\') {
          p++;
          switch(*p) {
            case '\0':
              g_warning ("g_strcompress: trailing \\");
              goto out;
            case '0':  case '1':  case '2':  case '3':  case '4': case '5':  case '6':  case '7':
              *q = 0;
              octal = p;
              while ((p < octal + 3) && (*p >= '0') && (*p <= '7')) {
                  *q = (*q * 8) + (*p - '0');
                  p++;
              }
              q++;
              p--;
              break;
            case 'b': *q++ = '\b'; break;
            case 'f': *q++ = '\f'; break;
            case 'n': *q++ = '\n'; break;
            case 'r': *q++ = '\r'; break;
            case 't': *q++ = '\t'; break;
            default: *q++ = *p; break;
          }
      } else *q++ = *p;
      p++;
  }
out:
  *q = 0;
  return dest;
}
gchar* g_strescape(const gchar *source, const gchar *exceptions) {
  const guchar *p;
  gchar *dest;
  gchar *q;
  guchar excmap[256];
  g_return_val_if_fail(source != NULL, NULL);
  p = (guchar*)source;
  q = dest = g_malloc (strlen (source) * 4 + 1);
  memset(excmap, 0, 256);
  if (exceptions) {
      guchar *e = (guchar*)exceptions;
      while(*e) {
          excmap[*e] = 1;
          e++;
      }
  }
  while(*p) {
      if (excmap[*p]) *q++ = *p;
      else {
          switch(*p) {
            case '\b': *q++ = '\\'; *q++ = 'b'; break;
            case '\f': *q++ = '\\'; *q++ = 'f'; break;
            case '\n': *q++ = '\\'; *q++ = 'n'; break;
            case '\r': *q++ = '\\'; *q++ = 'r'; break;
            case '\t': *q++ = '\\'; *q++ = 't'; break;
            case '\\': *q++ = '\\'; *q++ = '\\'; break;
            case '"': *q++ = '\\'; *q++ = '"'; break;
            default:
              if ((*p < ' ') || (*p >= 0177)) {
                  *q++ = '\\';
                  *q++ = '0' + (((*p) >> 6) & 07);
                  *q++ = '0' + (((*p) >> 3) & 07);
                  *q++ = '0' + ((*p) & 07);
              } else *q++ = *p;
              break;
          }
      }
      p++;
  }
  *q = 0;
  return dest;
}
gchar* g_strchug(gchar *string) {
  guchar *start;
  g_return_val_if_fail(string != NULL, NULL);
  for (start = (guchar*)string; *start && g_ascii_isspace(*start); start++);
  g_memmove (string, start, strlen ((gchar *) start) + 1);
  return string;
}
gchar* g_strchomp(gchar *string) {
  gsize len;
  g_return_val_if_fail(string != NULL, NULL);
  len = strlen(string);
  while(len--) {
      if (g_ascii_isspace((guchar)string[len])) string[len] = '\0';
      else break;
  }
  return string;
}
gchar** g_strsplit(const gchar *string, const gchar *delimiter, gint max_tokens) {
  GSList *string_list = NULL, *slist;
  gchar **str_array, *s;
  guint n = 0;
  const gchar *remainder;
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(delimiter != NULL, NULL);
  g_return_val_if_fail(delimiter[0] != '\0', NULL);
  if (max_tokens < 1) max_tokens = G_MAXINT;
  remainder = string;
  s = strstr(remainder, delimiter);
  if (s) {
      gsize delimiter_len = strlen(delimiter);
      while(--max_tokens && s) {
          gsize len;
          len = s - remainder;
          string_list = g_slist_prepend(string_list, g_strndup(remainder, len));
          n++;
          remainder = s + delimiter_len;
          s = strstr(remainder, delimiter);
      }
  }
  if (*string) {
      n++;
      string_list = g_slist_prepend(string_list, g_strdup(remainder));
  }
  str_array = g_new(gchar*, n + 1);
  str_array[n--] = NULL;
  for (slist = string_list; slist; slist = slist->next)
    str_array[n--] = slist->data;
  g_slist_free(string_list);
  return str_array;
}
gchar** g_strsplit_set(const gchar *string, const gchar *delimiters, gint max_tokens) {
  gboolean delim_table[256];
  GSList *tokens, *list;
  gint n_tokens;
  const gchar *s;
  const gchar *current;
  gchar *token;
  gchar **result;
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(delimiters != NULL, NULL);
  if (max_tokens < 1) max_tokens = G_MAXINT;
  if (*string == '\0') {
      result = g_new(char*, 1);
      result[0] = NULL;
      return result;
  }
  memset (delim_table, FALSE, sizeof (delim_table));
  for (s = delimiters; *s != '\0'; ++s) delim_table[*(guchar *)s] = TRUE;
  tokens = NULL;
  n_tokens = 0;
  s = current = string;
  while(*s != '\0') {
      if (delim_table[*(guchar *)s] && n_tokens + 1 < max_tokens) {
          token = g_strndup(current, s - current);
          tokens = g_slist_prepend(tokens, token);
          ++n_tokens;
          current = s + 1;
      }
      ++s;
  }
  token = g_strndup (current, s - current);
  tokens = g_slist_prepend (tokens, token);
  ++n_tokens;
  result = g_new(gchar*, n_tokens + 1);
  result[n_tokens] = NULL;
  for (list = tokens; list != NULL; list = list->next) result[--n_tokens] = list->data;
  g_slist_free(tokens);
  return result;
}
void g_strfreev(gchar **str_array) {
  if (str_array) {
      int i;
      for (i = 0; str_array[i] != NULL; i++) g_free(str_array[i]);
      g_free(str_array);
  }
}
gchar** g_strdupv(gchar **str_array) {
  if (str_array) {
      gint i;
      gchar **retval;
      i = 0;
      while(str_array[i]) ++i;
      retval = g_new(gchar*, i + 1);
      i = 0;
      while(str_array[i]) {
          retval[i] = g_strdup(str_array[i]);
          ++i;
      }
      retval[i] = NULL;
      return retval;
  } else return NULL;
}
gchar* g_strjoinv(const gchar *separator, gchar **str_array) {
  gchar *string;
  gchar *ptr;
  g_return_val_if_fail(str_array != NULL, NULL);
  if (separator == NULL) separator = "";
  if (*str_array) {
      gint i;
      gsize len;
      gsize separator_len;
      separator_len = strlen(separator);
      len = 1 + strlen(str_array[0]);
      for (i = 1; str_array[i] != NULL; i++) len += strlen(str_array[i]);
      len += separator_len * (i - 1);
      string = g_new(gchar, len);
      ptr = g_stpcpy(string, *str_array);
      for (i = 1; str_array[i] != NULL; i++) {
          ptr = g_stpcpy(ptr, separator);
          ptr = g_stpcpy(ptr, str_array[i]);
      }
  } else string = g_strdup ("");
  return string;
}
gchar* g_strjoin(const gchar  *separator, ...) {
  gchar *string, *s;
  va_list args;
  gsize len;
  gsize separator_len;
  gchar *ptr;
  if (separator == NULL) separator = "";
  separator_len = strlen(separator);
  va_start(args, separator);
  s = va_arg(args, gchar*);
  if (s) {
      len = 1 + strlen (s);
      s = va_arg(args, gchar*);
      while(s) {
          len += separator_len + strlen(s);
          s = va_arg(args, gchar*);
      }
      va_end(args);
      string = g_new(gchar, len);
      va_start(args, separator);
      s = va_arg(args, gchar*);
      ptr = g_stpcpy(string, s);
      s = va_arg(args, gchar*);
      while(s) {
          ptr = g_stpcpy(ptr, separator);
          ptr = g_stpcpy(ptr, s);
          s = va_arg(args, gchar*);
      }
  } else string = g_strdup("");
  va_end(args);
  return string;
}
gchar* g_strstr_len(const gchar *haystack, gssize haystack_len, const gchar *needle) {
  g_return_val_if_fail(haystack != NULL, NULL);
  g_return_val_if_fail(needle != NULL, NULL);
  if (haystack_len < 0) return strstr(haystack, needle);
  else {
      const gchar *p = haystack;
      gsize needle_len = strlen(needle);
      const gchar *end;
      gsize i;
      if (needle_len == 0) return (gchar*)haystack;
      if (haystack_len < needle_len) return NULL;
      end = haystack + haystack_len - needle_len;
      while (p <= end && *p) {
          for (i = 0; i < needle_len; i++)
            if (p[i] != needle[i]) goto next;
          return (gchar *)p;
        next:
          p++;
      }
      return NULL;
  }
}
gchar* g_strrstr(const gchar *haystack, const gchar *needle) {
  gsize i;
  gsize needle_len;
  gsize haystack_len;
  const gchar *p;
  g_return_val_if_fail(haystack != NULL, NULL);
  g_return_val_if_fail(needle != NULL, NULL);
  needle_len = strlen(needle);
  haystack_len = strlen(haystack);
  if (needle_len == 0) return (gchar*)haystack;
  if (haystack_len < needle_len) return NULL;
  p = haystack + haystack_len - needle_len;
  while (p >= haystack) {
      for (i = 0; i < needle_len; i++)
        if (p[i] != needle[i]) goto next;
      return (gchar *)p;
    next:
      p--;
  }
  return NULL;
}
gchar* g_strrstr_len(const gchar *haystack, gssize haystack_len, const gchar *needle) {
  g_return_val_if_fail(haystack != NULL, NULL);
  g_return_val_if_fail(needle != NULL, NULL);
  if (haystack_len < 0) return g_strrstr(haystack, needle);
  else {
      gsize needle_len = strlen(needle);
      const gchar *haystack_max = haystack + haystack_len;
      const gchar *p = haystack;
      gsize i;
      while (p < haystack_max && *p) p++;
      if (p < haystack + needle_len) return NULL;
      p -= needle_len;
      while(p >= haystack) {
          for (i = 0; i < needle_len; i++)
            if (p[i] != needle[i]) goto next;
          return (gchar *)p;
        next:
          p--;
      }
      return NULL;
  }
}
gboolean g_str_has_suffix(const gchar *str, const gchar *suffix) {
  int str_len;
  int suffix_len;
  g_return_val_if_fail(str != NULL, FALSE);
  g_return_val_if_fail(suffix != NULL, FALSE);
  str_len = strlen(str);
  suffix_len = strlen(suffix);
  if (str_len < suffix_len) return FALSE;
  return strcmp(str + str_len - suffix_len, suffix) == 0;
}
gboolean g_str_has_prefix(const gchar *str, const gchar *prefix) {
  int str_len;
  int prefix_len;
  g_return_val_if_fail(str != NULL, FALSE);
  g_return_val_if_fail(prefix != NULL, FALSE);
  str_len = strlen(str);
  prefix_len = strlen(prefix);
  if (str_len < prefix_len) return FALSE;
  return strncmp(str, prefix, prefix_len) == 0;
}
G_CONST_RETURN gchar* g_strip_context(const gchar *msgid, const gchar *msgval) {
  if (msgval == msgid) {
      const char *c = strchr(msgid, '|');
      if (c != NULL) return c + 1;
  }
  return msgval;
}
guint g_strv_length(gchar **str_array) {
  guint i = 0;
  g_return_val_if_fail(str_array != NULL, 0);
  while(str_array[i]) ++i;
  return i;
}
G_CONST_RETURN gchar* g_dpgettext(const gchar *domain, const gchar *msgctxtid, gsize msgidoffset) {
  const gchar *translation;
  gchar *sep;
  translation = g_dgettext (domain, msgctxtid);
  if (translation == msgctxtid) {
      if (msgidoffset > 0) return msgctxtid + msgidoffset;
      sep = strchr (msgctxtid, '|');
      if (sep) {
          gchar *tmp = g_alloca(strlen (msgctxtid) + 1);
          strcpy(tmp, msgctxtid);
          tmp[sep - msgctxtid] = '\004';
          translation = g_dgettext(domain, tmp);
          if (translation == tmp) return sep + 1;
      }
  }
  return translation;
}
G_CONST_RETURN char* g_dpgettext2(const char *domain, const char *msgctxt, const char *msgid) {
  size_t msgctxt_len = strlen(msgctxt) + 1;
  size_t msgid_len = strlen(msgid) + 1;
  const char *translation;
  char* msg_ctxt_id;
  msg_ctxt_id = g_alloca(msgctxt_len + msgid_len);
  memcpy(msg_ctxt_id, msgctxt, msgctxt_len - 1);
  msg_ctxt_id[msgctxt_len - 1] = '\004';
  memcpy(msg_ctxt_id + msgctxt_len, msgid, msgid_len);
  translation = g_dgettext(domain, msg_ctxt_id);
  if (translation == msg_ctxt_id) {
      msg_ctxt_id[msgctxt_len - 1] = '|';
      translation = g_dgettext(domain, msg_ctxt_id);
      if (translation == msg_ctxt_id) return msgid;
  }
  return translation;
}
static gboolean _g_dgettext_should_translate(void) {
  static gsize translate = 0;
  enum {
    SHOULD_TRANSLATE = 1,
    SHOULD_NOT_TRANSLATE = 2
  };
  if (G_UNLIKELY(g_once_init_enter (&translate))) {
      gboolean should_translate = TRUE;
      const char *default_domain     = textdomain(NULL);
      const char *translator_comment = gettext("");
  #ifndef G_OS_WIN32
      const char *translate_locale = setlocale(LC_MESSAGES, NULL);
  #else
      const char *translate_locale = g_win32_getlocale();
  #endif
      if (0 != strcmp(default_domain, "messages") && '\0' == *translator_comment && 0 != strncmp(translate_locale, "en_", 3) && 0 != strcmp(translate_locale, "C"))
          should_translate = FALSE;
      g_once_init_leave(&translate, should_translate ? SHOULD_TRANSLATE : SHOULD_NOT_TRANSLATE);
  }
  return translate == SHOULD_TRANSLATE;
}
G_CONST_RETURN gchar* g_dgettext(const gchar *domain, const gchar *msgid) {
  if (domain && G_UNLIKELY(!_g_dgettext_should_translate ())) return msgid;
  return dgettext(domain, msgid);
}
G_CONST_RETURN gchar* g_dcgettext(const gchar *domain, const gchar *msgid, int category) {
  if (domain && G_UNLIKELY(!_g_dgettext_should_translate())) return msgid;
  return dcgettext(domain, msgid, category);
}
G_CONST_RETURN gchar* g_dngettext(const gchar *domain, const gchar *msgid, const gchar *msgid_plural, gulong n) {
  if (domain && G_UNLIKELY(!_g_dgettext_should_translate ())) return n == 1 ? msgid : msgid_plural;
  return dngettext(domain, msgid, msgid_plural, n);
}