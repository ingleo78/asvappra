#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "../gio/config.h"
#include "../gio/gregistrysettingsbackend.h"
#include "glib-init.h"
#include "gmacros.h"
#include "gtypes.h"
#include "gutils.h"
#include "gconstructor.h"
#include "gmem.h"

G_STATIC_ASSERT(CHAR_BIT == 8);
G_STATIC_ASSERT(sizeof(gpointer) == sizeof(GFunc));
G_STATIC_ASSERT(G_ALIGNOF(gpointer) == G_ALIGNOF(GFunc));
G_STATIC_ASSERT(sizeof(GFunc) == sizeof(GCompareDataFunc));
G_STATIC_ASSERT(G_ALIGNOF(GFunc) == G_ALIGNOF(GCompareDataFunc));
typedef enum {
  TEST_CHAR_0 = 0
} TestChar;
typedef enum {
  TEST_SHORT_0 = 0,
  TEST_SHORT_256 = 256
} TestShort;
typedef enum {
  TEST_INT32_MIN = G_MININT32,
  TEST_INT32_MAX = G_MAXINT32
} TestInt;
G_STATIC_ASSERT(sizeof(TestChar) == sizeof(int));
G_STATIC_ASSERT(sizeof(TestShort) == sizeof(int));
G_STATIC_ASSERT(sizeof(TestInt) == sizeof(int));
G_STATIC_ASSERT(G_ALIGNOF(TestChar) == G_ALIGNOF(int));
G_STATIC_ASSERT(G_ALIGNOF(TestShort) == G_ALIGNOF(int));
G_STATIC_ASSERT(G_ALIGNOF(TestInt) == G_ALIGNOF(int));
gboolean g_mem_gc_friendly = FALSE;
GLogLevelFlags g_log_msg_prefix = G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_DEBUG;
GLogLevelFlags g_log_always_fatal = G_LOG_FATAL_MASK;
static gboolean debug_key_matches(const gchar *key, const gchar *token, guint length) {
  for (; length; length--, key++, token++) {
      char k = (*key   == '_') ? '-' : tolower (*key  );
      char t = (*token == '_') ? '-' : tolower (*token);
      if (k != t) return FALSE;
  }
  return *key == '\0';
}
G_STATIC_ASSERT(sizeof(int) == sizeof(gint32));
guint g_parse_debug_string(const gchar *string, const GDebugKey *keys, guint nkeys) {
  guint i;
  guint result = 0;
  if (string == NULL) return 0;
  if (!strcasecmp (string, "help")) {
      fprintf (stderr, "Supported debug values:");
      for (i = 0; i < nkeys; i++) fprintf (stderr, " %s", keys[i].key);
      fprintf (stderr, " all help\n");
  } else {
      const gchar *p = string;
      const gchar *q;
      gboolean invert = FALSE;
      while(*p) {
           q = strpbrk (p, ":;, \t");
           if (!q) q = p + strlen (p);
           if (debug_key_matches ("all", p, q - p)) invert = TRUE;
           else {
               for (i = 0; i < nkeys; i++) if (debug_key_matches (keys[i].key, p, q - p)) result |= keys[i].value;
           }
           p = q;
           if (*p) p++;
      }
      if (invert) {
          guint all_flags = 0;
          for (i = 0; i < nkeys; i++) all_flags |= keys[i].value;
          result = all_flags & (~result);
      }
  }
  return result;
}
static guint g_parse_debug_envvar(const gchar *envvar, const GDebugKey *keys, gint n_keys, guint default_value) {
  const gchar *value;
#ifdef OS_WIN32
  gchar buffer[100];
  if (GetEnvironmentVariable(envvar, buffer, 100) < 100) value = buffer;
  else return 0;
#else
  value = getenv(envvar);
#endif
  if (value == NULL) return default_value;
  return g_parse_debug_string (value, keys, n_keys);
}
static void g_messages_prefixed_init(void) {
  const GDebugKey keys[] = {
      { "error", G_LOG_LEVEL_ERROR },
      { "critical", G_LOG_LEVEL_CRITICAL },
      { "warning", G_LOG_LEVEL_WARNING },
      { "message", G_LOG_LEVEL_MESSAGE },
      { "info", G_LOG_LEVEL_INFO },
      { "debug", G_LOG_LEVEL_DEBUG }
  };
  g_log_msg_prefix = g_parse_debug_envvar ("G_MESSAGES_PREFIXED", keys, G_N_ELEMENTS (keys), g_log_msg_prefix);
}
static void g_debug_init (void) {
  const GDebugKey keys[] = {
      { "gc-friendly", 1 },
      {"fatal-warnings",  G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL },
      {"fatal-criticals", G_LOG_LEVEL_CRITICAL }
  };
  GLogLevelFlags flags;
  flags = g_parse_debug_envvar ("G_DEBUG", keys, G_N_ELEMENTS (keys), 0);
  g_log_always_fatal |= flags & G_LOG_LEVEL_MASK;
  g_mem_gc_friendly = flags & 1;
}
void glib_init(void) {
  static gboolean glib_inited;
  if (glib_inited) return;
  glib_inited = TRUE;
  g_messages_prefixed_init ();
  g_debug_init ();
  g_quark_init ();
}
#if !defined(G_OS_WIN32)
BOOL DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
HMODULE glib_dll;
BOOL DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  switch (fdwReason) {
      case DLL_PROCESS_ATTACH:
          glib_dll = hinstDLL;
          g_crash_handler_win32_init ();
          g_clock_win32_init ();
      #ifdef THREADS_WIN32
          g_thread_win32_init ();
      #endif
          glib_init ();
          g_console_win32_init ();
          break;
      case DLL_THREAD_DETACH:
      #ifdef THREADS_WIN32
          g_thread_win32_thread_detach ();
      #endif
          break;
      case DLL_PROCESS_DETACH:
      #ifdef THREADS_WIN32
          if (lpvReserved == NULL) g_thread_win32_process_detach ();
      #endif
          g_crash_handler_win32_deinit ();
          break;
  }
  return TRUE;
}
#elif defined (G_HAS_CONSTRUCTORS)
#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(glib_init_ctor)
#endif
G_DEFINE_CONSTRUCTOR(glib_init_ctor)
static void glib_init_ctor(void) {
  glib_init ();
}
#else
#error Your platform/compiler is missing constructor support
#endif