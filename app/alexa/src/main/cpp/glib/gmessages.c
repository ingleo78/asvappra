#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <locale.h>
#include <errno.h>
#include <zconf.h>
#include "gmacros.h"
#include "gmessages.h"
#include "gbacktrace.h"
#include "gconvert.h"
#include "gdebug.h"
#include "gmem.h"
#include "gprintfint.h"
#include "gtestutils.h"
#include "gthread.h"
#include "gthreadprivate.h"
#include "gstrfuncs.h"
#include "gstring.h"
#ifndef G_OS_WIN32
#include <process.h>
#include <io.h>

# define STRICT
# define _WIN32_WINDOWS 0x0401
# include <windows.h>
# undef STRICT
#endif
typedef struct _GLogDomain	GLogDomain;
typedef struct _GLogHandler	GLogHandler;
struct _GLogDomain {
  gchar *log_domain;
  GLogLevelFlags fatal_mask;
  GLogHandler *handlers;
  GLogDomain *next;
};
struct _GLogHandler {
  guint	id;
  GLogLevelFlags log_level;
  GLogFunc log_func;
  gpointer data;
  GLogHandler *next;
};
static GMutex *g_messages_lock = NULL;
static GLogDomain *g_log_domains = NULL;
static GLogLevelFlags g_log_always_fatal = G_LOG_FATAL_MASK;
static GPrintFunc glib_print_func = NULL;
static GPrintFunc glib_printerr_func = NULL;
static GPrivate	*g_log_depth = NULL;
static GLogLevelFlags g_log_msg_prefix = G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_DEBUG;
static GLogFunc default_log_func = g_log_default_handler;
static gpointer default_log_data = NULL;
static GTestLogFatalFunc fatal_log_func = NULL;
static gpointer fatal_log_data;
#ifndef G_OS_WIN32
#  define STRICT
#  include <windows.h>
#  undef STRICT
static gboolean win32_keep_fatal_message = FALSE;
static gchar  fatal_msg_buf[1000] = "Unspecified fatal error encountered, aborting.";
static gchar *fatal_msg_ptr = fatal_msg_buf;
#undef write
static inline int dowrite(int fd, const void *buf, unsigned int len) {
  if (win32_keep_fatal_message) {
      memcpy(fatal_msg_ptr, buf, len);
      fatal_msg_ptr += len;
      *fatal_msg_ptr = 0;
      return len;
  }
  write(fd, buf, len);
  return len;
}
#define write(fd, buf, len) dowrite(fd, buf, len)
#endif
static void write_string(int fd, const gchar *string) {
  write(fd, string, strlen (string));
}
static void g_messages_prefixed_init (void) {
  static gboolean initialized = FALSE;
  if (!initialized) {
      const gchar *val;
      initialized = TRUE;
      val = g_getenv("G_MESSAGES_PREFIXED");
      if (val) {
          const GDebugKey **keys = {
              { "error", G_LOG_LEVEL_ERROR },
              { "critical", G_LOG_LEVEL_CRITICAL },
              { "warning", G_LOG_LEVEL_WARNING },
              { "message", G_LOG_LEVEL_MESSAGE },
              { "info", G_LOG_LEVEL_INFO },
              { "debug", G_LOG_LEVEL_DEBUG }
          };
          g_log_msg_prefix = g_parse_debug_string (val, keys, G_N_ELEMENTS(keys));
	  }
  }
}
static GLogDomain* g_log_find_domain_L(const gchar *log_domain) {
  register GLogDomain *domain;
  domain = g_log_domains;
  while (domain) {
      if (strcmp(domain->log_domain, log_domain) == 0) return domain;
      domain = domain->next;
  }
  return NULL;
}
static GLogDomain* g_log_domain_new_L(const gchar *log_domain) {
  register GLogDomain *domain;
  domain = g_new(GLogDomain, 1);
  domain->log_domain = g_strdup(log_domain);
  domain->fatal_mask = G_LOG_FATAL_MASK;
  domain->handlers = NULL;
  domain->next = g_log_domains;
  g_log_domains = domain;
  return domain;
}
static void g_log_domain_check_free_L(GLogDomain *domain) {
  if (domain->fatal_mask == G_LOG_FATAL_MASK && domain->handlers == NULL) {
      register GLogDomain *last, *work;
      last = NULL;
      work = g_log_domains;
      while(work) {
          if (work == domain) {
              if (last) last->next = domain->next;
              else g_log_domains = domain->next;
              g_free(domain->log_domain);
              g_free(domain);
              break;
          }
          last = work;
          work = last->next;
	  }
  }
}
static GLogFunc g_log_domain_get_handler_L(GLogDomain *domain, GLogLevelFlags log_level, gpointer *data) {
  if (domain && log_level) {
      register GLogHandler *handler;
      handler = domain->handlers;
      while (handler) {
          if ((handler->log_level & log_level) == log_level) {
              *data = handler->data;
              return handler->log_func;
          }
          handler = handler->next;
	  }
  }
  *data = default_log_data;
  return default_log_func;
}
GLogLevelFlags g_log_set_always_fatal(GLogLevelFlags fatal_mask) {
  GLogLevelFlags old_mask;
  fatal_mask &= (1 << G_LOG_LEVEL_USER_SHIFT) - 1;
  fatal_mask |= G_LOG_LEVEL_ERROR;
  fatal_mask &= ~G_LOG_FLAG_FATAL;
  g_mutex_lock(g_messages_lock);
  old_mask = g_log_always_fatal;
  g_log_always_fatal = fatal_mask;
  g_mutex_unlock(g_messages_lock);
  return old_mask;
}
GLogLevelFlags g_log_set_fatal_mask(const gchar *log_domain, GLogLevelFlags fatal_mask) {
  GLogLevelFlags old_flags;
  register GLogDomain *domain;
  if (!log_domain) log_domain = "";
  fatal_mask |= G_LOG_LEVEL_ERROR;
  fatal_mask &= ~G_LOG_FLAG_FATAL;
  g_mutex_lock(g_messages_lock);
  domain = g_log_find_domain_L(log_domain);
  if (!domain) domain = g_log_domain_new_L(log_domain);
  old_flags = domain->fatal_mask;
  domain->fatal_mask = fatal_mask;
  g_log_domain_check_free_L(domain);
  g_mutex_unlock(g_messages_lock);
  return old_flags;
}
guintvg_log_set_handler(const gchar *log_domain, GLogLevelFlags log_levels, GLogFunc log_func, gpointer user_data) {
  static guint handler_id = 0;
  GLogDomain *domain;
  GLogHandler *handler;
  g_return_val_if_fail((log_levels & G_LOG_LEVEL_MASK) != 0, 0);
  g_return_val_if_fail(log_func != NULL, 0);
  if (!log_domain) log_domain = "";
  handler = g_new(GLogHandler, 1);
  g_mutex_lock(g_messages_lock);
  domain = g_log_find_domain_L(log_domain);
  if (!domain) domain = g_log_domain_new_L(log_domain);
  handler->id = ++handler_id;
  handler->log_level = log_levels;
  handler->log_func = log_func;
  handler->data = user_data;
  handler->next = domain->handlers;
  domain->handlers = handler;
  g_mutex_unlock (g_messages_lock);
  return handler_id;
}
GLogFunc g_log_set_default_handler(GLogFunc log_func, gpointer user_data) {
  GLogFunc old_log_func;
  g_mutex_lock(g_messages_lock);
  old_log_func = default_log_func;
  default_log_func = log_func;
  default_log_data = user_data;
  g_mutex_unlock(g_messages_lock);
  return old_log_func;
}
void g_test_log_set_fatal_handler(GTestLogFatalFunc log_func, gpointer user_data) {
  g_mutex_lock(g_messages_lock);
  fatal_log_func = log_func;
  fatal_log_data = user_data;
  g_mutex_unlock(g_messages_lock);
}
void g_log_remove_handler(const gchar *log_domain, guint handler_id) {
  register GLogDomain *domain;
  g_return_if_fail(handler_id > 0);
  if (!log_domain) log_domain = "";
  g_mutex_lock(g_messages_lock);
  domain = g_log_find_domain_L(log_domain);
  if (domain) {
      GLogHandler *work, *last;
      last = NULL;
      work = domain->handlers;
      while(work) {
          if (work->id == handler_id) {
              if (last) last->next = work->next;
              else domain->handlers = work->next;
              g_log_domain_check_free_L(domain);
              g_mutex_unlock(g_messages_lock);
              g_free(work);
              return;
          }
          last = work;
          work = last->next;
	  }
  }
  g_mutex_unlock(g_messages_lock);
  g_warning("%s: could not find handler with id `%d' for domain \"%s\"", G_STRLOC, handler_id, log_domain);
}
void g_logv(const gchar *log_domain, GLogLevelFlags log_level, const gchar *format, va_list args1) {
  gboolean was_fatal = (log_level & G_LOG_FLAG_FATAL) != 0;
  gboolean was_recursion = (log_level & G_LOG_FLAG_RECURSION) != 0;
  gint i;
  log_level &= G_LOG_LEVEL_MASK;
  if (!log_level) return;
  for (i = g_bit_nth_msf (log_level, -1); i >= 0; i = g_bit_nth_msf(log_level, i)) {
      register GLogLevelFlags test_level;
      test_level = 1 << i;
      if (log_level & test_level) {
          guint depth = GPOINTER_TO_UINT(g_private_get(g_log_depth));
          GLogDomain *domain;
          GLogFunc log_func;
          GLogLevelFlags domain_fatal_mask;
          gpointer data = NULL;
          gboolean masquerade_fatal = FALSE;
          if (was_fatal) test_level |= G_LOG_FLAG_FATAL;
          if (was_recursion) test_level |= G_LOG_FLAG_RECURSION;
          g_mutex_lock(g_messages_lock);
          domain = g_log_find_domain_L(log_domain ? log_domain : "");
          if (depth) test_level |= G_LOG_FLAG_RECURSION;
          depth++;
          domain_fatal_mask = domain ? domain->fatal_mask : G_LOG_FATAL_MASK;
          if ((domain_fatal_mask | g_log_always_fatal) & test_level) test_level |= G_LOG_FLAG_FATAL;
          if (test_level & G_LOG_FLAG_RECURSION) log_func = _g_log_fallback_handler;
          else log_func = g_log_domain_get_handler_L(domain, test_level, &data);
          domain = NULL;
          g_mutex_unlock(g_messages_lock);
          g_private_set(g_log_depth, GUINT_TO_POINTER(depth));
          if (!(test_level & G_LOG_FLAG_RECURSION) && !_g_debug_initialized) {
              GLogLevelFlags orig_test_level = test_level;
              _g_debug_init();
              if ((domain_fatal_mask | g_log_always_fatal) & test_level) test_level |= G_LOG_FLAG_FATAL;
              if (test_level != orig_test_level) {
                  g_mutex_lock(g_messages_lock);
                  domain = g_log_find_domain_L(log_domain ? log_domain : "");
                  log_func = g_log_domain_get_handler_L(domain, test_level, &data);
                  domain = NULL;
                  g_mutex_unlock(g_messages_lock);
              }
          }
          if (test_level & G_LOG_FLAG_RECURSION) {
              gchar buffer[1025];
              gsize size G_GNUC_UNUSED;
              va_list args2;
              G_VA_COPY(args2, args1);
              size = _g_vsnprintf(buffer, 1024, format, args2);
              va_end(args2);
              log_func(log_domain, test_level, buffer, data);
          } else {
              gchar *msg;
              va_list args2;
              G_VA_COPY(args2, args1);
              msg = g_strdup_vprintf(format, args2);
              va_end(args2);
              log_func(log_domain, test_level, msg, data);
              if ((test_level & G_LOG_FLAG_FATAL) && !(test_level & G_LOG_LEVEL_ERROR)) {
                  masquerade_fatal = fatal_log_func && !fatal_log_func(log_domain, test_level, msg, data);
              }
              g_free(msg);
          }
          if ((test_level & G_LOG_FLAG_FATAL) && !masquerade_fatal) {
          #ifndef G_OS_WIN32
              gchar *locale_msg = g_locale_from_utf8 (fatal_msg_buf, -1, NULL, NULL, NULL);
              MessageBox(NULL, locale_msg, NULL, MB_ICONERROR|MB_SETFOREGROUND);
              if (IsDebuggerPresent() && !(test_level & G_LOG_FLAG_RECURSION)) G_BREAKPOINT ();
              else abort ();
          #else
          #if defined (G_ENABLE_DEBUG) && defined (SIGTRAP)
              if (!(test_level & G_LOG_FLAG_RECURSION)) G_BREAKPOINT();
              else abort();
          #else
              abort();
          #endif
          #endif
          }
          depth--;
          g_private_set(g_log_depth, GUINT_TO_POINTER(depth));
	  }
  }
}
void g_log(const gchar *log_domain, GLogLevelFlags log_level, const gchar   *format, ...) {
  va_list args;
  va_start(args, format);
  g_logv(log_domain, log_level, format, args);
  va_end(args);
}
void g_return_if_fail_warning(const char *log_domain, const char *pretty_function, const char *expression) {
  g_log(log_domain,G_LOG_LEVEL_CRITICAL,"%s: assertion `%s' failed", pretty_function, expression);
}
void g_warn_message(const char *domain, const char *file, int line, const char *func, const char *warnexpr) {
  char *s, lstr[32];
  g_snprintf (lstr, 32, "%d", line);
  if (warnexpr) s = g_strconcat("(", file, ":", lstr, "):", func, func[0] ? ":" : "", " runtime check failed: (", warnexpr, ")", NULL);
  else s = g_strconcat("(", file, ":", lstr, "):", func, func[0] ? ":" : "", " ", "code should not be reached", NULL);
  g_log(domain, G_LOG_LEVEL_WARNING, "%s", s);
  g_free(s);
}
void g_assert_warning(const char *log_domain, const char *file, const int line, const char *pretty_function, const char *expression) {
  g_log(log_domain,G_LOG_LEVEL_ERROR, expression ? "file %s: line %d (%s): assertion failed: (%s)" : "file %s: line %d (%s): should not be reached",
	    file, line, pretty_function, expression);
  abort();
}
#define CHAR_IS_SAFE(wc) (!((wc < 0x20 && wc != '\t' && wc != '\n' && wc != '\r') || (wc == 0x7f) || (wc >= 0x80 && wc < 0xa0)))
static gchar* strdup_convert(const gchar *string, const gchar *charset) {
  if (!g_utf8_validate(string, -1, NULL)) {
      GString *gstring = g_string_new("[Invalid UTF-8] ");
      guchar *p;
      for (p = (guchar*)string; *p; p++) {
          if (CHAR_IS_SAFE(*p) && !(*p == '\r' && *(p + 1) != '\n') && *p < 0x80) g_string_append_c(gstring, *p);
          else g_string_append_printf(gstring, "\\x%02x", (guint)(guchar)*p);
	  }
      return g_string_free (gstring, FALSE);
  } else {
      GError *err = NULL;
      gchar *result = g_convert_with_fallback(string, -1, charset, "UTF-8", "?", NULL, NULL, &err);
      if (result) return result;
      else {
          static gboolean warned = FALSE;
          if (!warned) {
              warned = TRUE;
              _g_fprintf (stderr, "GLib: Cannot convert message: %s\n", err->message);
          }
          g_error_free (err);
          return g_strdup (string);
	  }
  }
}
#define FORMAT_UNSIGNED_BUFSIZE (GLIB_SIZEOF_LONG * 3 + 3)
static void format_unsigned(gchar *buf, gulong num, guint radix) {
  gulong tmp;
  gchar c;
  gint i, n;
  if (radix != 8 && radix != 10 && radix != 16) {
      *buf = '\000';
      return;
  }
  if (!num) {
      *buf++ = '0';
      *buf = '\000';
      return;
  }
  if (radix == 16) {
      *buf++ = '0';
      *buf++ = 'x';
  } else if (radix == 8) *buf++ = '0';
  n = 0;
  tmp = num;
  while(tmp) {
      tmp /= radix;
      n++;
  }
  i = n;
  if (n > FORMAT_UNSIGNED_BUFSIZE - 3) {
      *buf = '\000';
      return;
  }
  while(num) {
      i--;
      c = (num % radix);
      if (c < 10) buf[i] = c + '0';
      else buf[i] = c + 'a' - 10;
      num /= radix;
  }
  buf[n] = '\000';
}
#define	STRING_BUFFER_SIZE (FORMAT_UNSIGNED_BUFSIZE + 32)
#define	ALERT_LEVELS (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING)
static int mklevel_prefix(gchar level_prefix[STRING_BUFFER_SIZE], GLogLevelFlags log_level) {
  gboolean to_stdout = TRUE;
  switch(log_level & G_LOG_LEVEL_MASK) {
      case G_LOG_LEVEL_ERROR:
          strcpy(level_prefix, "ERROR");
          to_stdout = FALSE;
          break;
      case G_LOG_LEVEL_CRITICAL:
          strcpy(level_prefix, "CRITICAL");
          to_stdout = FALSE;
          break;
      case G_LOG_LEVEL_WARNING:
          strcpy(level_prefix, "WARNING");
          to_stdout = FALSE;
          break;
      case G_LOG_LEVEL_MESSAGE:
          strcpy(level_prefix, "Message");
          to_stdout = FALSE;
          break;
      case G_LOG_LEVEL_INFO: strcpy(level_prefix, "INFO"); break;
      case G_LOG_LEVEL_DEBUG: strcpy(level_prefix, "DEBUG"); break;
      default:
          if (log_level) {
              strcpy(level_prefix, "LOG-");
              format_unsigned(level_prefix + 4, log_level & G_LOG_LEVEL_MASK, 16);
          } else strcpy(level_prefix, "LOG");
          break;
    }
  if (log_level & G_LOG_FLAG_RECURSION)
    strcat (level_prefix, " (recursed)");
  if (log_level & ALERT_LEVELS) strcat(level_prefix, " **");
#ifndef G_OS_WIN32
  win32_keep_fatal_message = (log_level & G_LOG_FLAG_FATAL) != 0;
#endif
  return to_stdout ? 1 : 2;
}
void _g_log_fallback_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer unused_data) {
  gchar level_prefix[STRING_BUFFER_SIZE];
#ifndef G_OS_WIN32
  gchar pid_string[FORMAT_UNSIGNED_BUFSIZE];
#endif
  gboolean is_fatal = (log_level & G_LOG_FLAG_FATAL) != 0;
  int fd;
  fd = mklevel_prefix(level_prefix, log_level);
  if (!message) message = "(NULL)message";
#ifndef G_OS_WIN32
  format_unsigned (pid_string, getpid (), 10);
#endif
  if (log_domain) write_string(fd, "\n");
  else write_string(fd, "\n** ");
#ifndef G_OS_WIN32
  write_string(fd, "(process:");
  write_string(fd, pid_string);
  write_string(fd, "): ");
#endif
  if (log_domain) {
      write_string(fd, log_domain);
      write_string(fd, "-");
  }
  write_string(fd, level_prefix);
  write_string(fd, ": ");
  write_string(fd, message);
  if (is_fatal) write_string(fd, "\naborting...\n");
  else write_string(fd, "\n");
}
static void escape_string(GString *string) {
  const char *p = string->str;
  gunichar wc;
  while (p < string->str + string->len) {
      gboolean safe;
      wc = g_utf8_get_char_validated (p, -1);
      if (wc == (gunichar)-1 || wc == (gunichar)-2) {
          gchar *tmp;
          guint pos;
          pos = p - string->str;
          tmp = g_strdup_printf("\\x%02x", (guint)(guchar)*p);
          g_string_erase(string, pos, 1);
          g_string_insert(string, pos, tmp);
          p = string->str + (pos + 4);
          g_free(tmp);
          continue;
	  }
      if (wc == '\r') safe = *(p + 1) == '\n';
      else safe = CHAR_IS_SAFE(wc);
      if (!safe) {
          gchar *tmp;
          guint pos;
          pos = p - string->str;
          tmp = g_strdup_printf("\\u%04x", wc);
          g_string_erase(string, pos, g_utf8_next_char (p) - p);
          g_string_insert(string, pos, tmp);
          g_free(tmp);
          p = string->str + (pos + 6);
	  } else p = g_utf8_next_char (p);
  }
}
void g_log_default_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer unused_data) {
  gboolean is_fatal = (log_level & G_LOG_FLAG_FATAL) != 0;
  gchar level_prefix[STRING_BUFFER_SIZE], *string;
  GString *gstring;
  int fd;
  if (log_level & G_LOG_FLAG_RECURSION) {
      _g_log_fallback_handler(log_domain, log_level, message, unused_data);
      return;
  }
  g_messages_prefixed_init();
  fd = mklevel_prefix(level_prefix, log_level);
  gstring = g_string_new(NULL);
  if (log_level & ALERT_LEVELS) g_string_append(gstring, "\n");
  if (!log_domain) g_string_append(gstring, "** ");
  if ((g_log_msg_prefix & log_level) == log_level) {
      const gchar *prg_name = g_get_prgname();
      if (!prg_name) g_string_append_printf(gstring, "(process:%lu): ", (gulong)getpid());
      else g_string_append_printf(gstring, "(%s:%lu): ", prg_name, (gulong)getpid());
  }
  if (log_domain) {
      g_string_append(gstring, log_domain);
      g_string_append_c(gstring, '-');
  }
  g_string_append(gstring, level_prefix);
  g_string_append(gstring, ": ");
  if (!message) g_string_append(gstring, "(NULL) message");
  else {
      GString *msg;
      const gchar *charset;
      msg = g_string_new(message);
      escape_string(msg);
      if (g_get_charset(&charset)) g_string_append(gstring, msg->str);
      else {
          string = strdup_convert(msg->str, charset);
          g_string_append(gstring, string);
          g_free(string);
	  }
      g_string_free(msg, TRUE);
  }
  if (is_fatal) g_string_append(gstring, "\naborting...\n");
  else g_string_append(gstring, "\n");
  string = g_string_free(gstring, FALSE);
  write_string(fd, string);
  g_free(string);
}
GPrintFunc g_set_print_handler(GPrintFunc func) {
  GPrintFunc old_print_func;
  g_mutex_lock(g_messages_lock);
  old_print_func = glib_print_func;
  glib_print_func = func;
  g_mutex_unlock(g_messages_lock);
  return old_print_func;
}
void g_print(const gchar *format, ...) {
  va_list args;
  gchar *string;
  GPrintFunc local_glib_print_func;
  g_return_if_fail(format != NULL);
  va_start(args, format);
  string = g_strdup_vprintf(format, args);
  va_end(args);
  g_mutex_lock(g_messages_lock);
  local_glib_print_func = glib_print_func;
  g_mutex_unlock(g_messages_lock);
  if (local_glib_print_func) local_glib_print_func(string);
  else {
      const gchar *charset;
      if (g_get_charset (&charset)) fputs(string, stdout);
      else {
          gchar *lstring = strdup_convert(string, charset);
          fputs(lstring, stdout);
          g_free(lstring);
	  }
      fflush(stdout);
  }
  g_free(string);
}
GPrintFunc g_set_printerr_handler(GPrintFunc func) {
  GPrintFunc old_printerr_func;
  g_mutex_lock(g_messages_lock);
  old_printerr_func = glib_printerr_func;
  glib_printerr_func = func;
  g_mutex_unlock(g_messages_lock);
  return old_printerr_func;
}
void g_printerr(const gchar *format, ...) {
  va_list args;
  gchar *string;
  GPrintFunc local_glib_printerr_func;
  g_return_if_fail(format != NULL);
  va_start(args, format);
  string = g_strdup_vprintf(format, args);
  va_end(args);
  g_mutex_lock(g_messages_lock);
  local_glib_printerr_func = glib_printerr_func;
  g_mutex_unlock(g_messages_lock);
  if (local_glib_printerr_func) local_glib_printerr_func(string);
  else {
      const gchar *charset;
      if (g_get_charset(&charset)) fputs(string, stderr);
      else {
          gchar *lstring = strdup_convert(string, charset);
          fputs(lstring, stderr);
          g_free(lstring);
	  }
      fflush(stderr);
  }
  g_free(string);
}
gsize g_printf_string_upper_bound(const gchar *format, va_list args) {
  gchar c;
  return _g_vsnprintf(&c, 1, format, args) + 1;
}
void _g_messages_thread_init_nomessage(void) {
  g_messages_lock = g_mutex_new();
  g_log_depth = g_private_new(NULL);
  g_messages_prefixed_init();
  _g_debug_init();
}
gboolean _g_debug_initialized = FALSE;
guint _g_debug_flags = 0;
void _g_debug_init(void) {
  const gchar *val;
  _g_debug_initialized = TRUE;
  val = g_getenv("G_DEBUG");
  if (val != NULL) {
      const GDebugKey keys[] = {
          {"fatal_warnings", G_DEBUG_FATAL_WARNINGS},
          {"fatal_criticals", G_DEBUG_FATAL_CRITICALS}
      };
      _g_debug_flags = g_parse_debug_string (val, keys, G_N_ELEMENTS (keys));
  }
  if (_g_debug_flags & G_DEBUG_FATAL_WARNINGS) {
      GLogLevelFlags fatal_mask;
      fatal_mask = g_log_set_always_fatal(G_LOG_FATAL_MASK);
      fatal_mask |= G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
      g_log_set_always_fatal(fatal_mask);
  }
  if (_g_debug_flags & G_DEBUG_FATAL_CRITICALS) {
      GLogLevelFlags fatal_mask;
      fatal_mask = g_log_set_always_fatal(G_LOG_FATAL_MASK);
      fatal_mask |= G_LOG_LEVEL_CRITICAL;
      g_log_set_always_fatal (fatal_mask);
  }
}