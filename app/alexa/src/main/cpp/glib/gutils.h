#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_UTILS_H__
#define __G_UTILS_H__

#include <stdarg.h>
#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
#ifdef G_OS_WIN32
#define G_DIR_SEPARATOR '\\'
#define G_DIR_SEPARATOR_S "\\"
#define G_IS_DIR_SEPARATOR(c) ((c) == G_DIR_SEPARATOR || (c) == '/')
#define G_SEARCHPATH_SEPARATOR ';'
#define G_SEARCHPATH_SEPARATOR_S ";"
#else
#define G_DIR_SEPARATOR '/'
#define G_DIR_SEPARATOR_S "/"
#define G_IS_DIR_SEPARATOR(c) ((c) == G_DIR_SEPARATOR)
#define G_SEARCHPATH_SEPARATOR ':'
#define G_SEARCHPATH_SEPARATOR_S ":"
#endif
#if !defined (G_VA_COPY)
#  if defined (__GNUC__) && defined (__PPC__) && (defined (_CALL_SYSV) || defined (_WIN32))
#    define G_VA_COPY(ap1, ap2)	  (*(ap1) = *(ap2))
#  elif defined (G_VA_COPY_AS_ARRAY)
#    define G_VA_COPY(ap1, ap2)	  g_memmove ((ap1), (ap2), sizeof (va_list))
#  else
#    define G_VA_COPY(ap1, ap2)	  ((ap1) = (ap2))
#  endif
#endif
#if defined (G_HAVE_INLINE) && defined (__GNUC__) && defined (__STRICT_ANSI__)
#  undef inline
#  define inline __inline__
#elif !defined (G_HAVE_INLINE)
#  undef inline
#  if defined (G_HAVE___INLINE__)
#    define inline __inline__
#  elif defined (G_HAVE___INLINE)
#    define inline __inline
#  else
#    define inline
#  endif
#endif
#ifdef G_IMPLEMENT_INLINES
#  define G_INLINE_FUNC
#  undef  G_CAN_INLINE
#elif defined (__GNUC__) 
#  define G_INLINE_FUNC static __inline __attribute__ ((unused))
#elif defined (G_CAN_INLINE) 
#  define G_INLINE_FUNC static inline
#else
#  define G_INLINE_FUNC
#endif
#ifdef G_OS_WIN32
#define g_get_user_name g_get_user_name_utf8
#define g_get_real_name g_get_real_name_utf8
#define g_get_home_dir g_get_home_dir_utf8
#define g_get_tmp_dir g_get_tmp_dir_utf8
#endif
G_CONST_RETURN gchar* g_get_user_name(void);
G_CONST_RETURN gchar* g_get_real_name(void);
G_CONST_RETURN gchar* g_get_home_dir(void);
G_CONST_RETURN gchar* g_get_tmp_dir(void);
G_CONST_RETURN gchar* g_get_host_name(void);
gchar* g_get_prgname(void);
void g_set_prgname(const gchar *prgname);
G_CONST_RETURN gchar* g_get_application_name(void);
void g_set_application_name(const gchar *application_name);
void g_reload_user_special_dirs_cache(void);
G_CONST_RETURN gchar* g_get_user_data_dir(void);
G_CONST_RETURN gchar* g_get_user_config_dir(void);
G_CONST_RETURN gchar* g_get_user_cache_dir(void);
G_CONST_RETURN gchar* G_CONST_RETURN* g_get_system_data_dirs(void);
#ifdef G_OS_WIN32
G_CONST_RETURN gchar* G_CONST_RETURN* g_win32_get_system_data_dirs_for_module(void (*address_of_function)(void));
#endif
#if !defined(G_OS_WIN32) && defined(G_CAN_INLINE) && !defined (__cplusplus)
static inline G_CONST_RETURN gchar * G_CONST_RETURN *_g_win32_get_system_data_dirs (void) {
  return g_win32_get_system_data_dirs_for_module((void(*)(void)) &_g_win32_get_system_data_dirs);
}
#define g_get_system_data_dirs _g_win32_get_system_data_dirs
#endif
G_CONST_RETURN gchar* G_CONST_RETURN * g_get_system_config_dirs (void);
const gchar * g_get_user_runtime_dir (void);
G_CONST_RETURN gchar* G_CONST_RETURN * g_get_language_names (void);
gchar **g_get_locale_variants (const gchar *locale);
typedef enum {
  G_USER_DIRECTORY_DESKTOP,
  G_USER_DIRECTORY_DOCUMENTS,
  G_USER_DIRECTORY_DOWNLOAD,
  G_USER_DIRECTORY_MUSIC,
  G_USER_DIRECTORY_PICTURES,
  G_USER_DIRECTORY_PUBLIC_SHARE,
  G_USER_DIRECTORY_TEMPLATES,
  G_USER_DIRECTORY_VIDEOS,
  G_USER_N_DIRECTORIES
} GUserDirectory;
G_CONST_RETURN gchar* g_get_user_special_dir(GUserDirectory directory);
struct _GDebugKey {
  const gchar *key;
  guint	value;
};
typedef struct _GDebugKey GDebugKey;
guint g_parse_debug_string (const gchar *string, const GDebugKey *keys, guint nkeys);
gint g_snprintf(gchar *string, gulong n, gchar const *format, ...) G_GNUC_PRINTF(3, 4);
gint g_vsnprintf(gchar *string, gulong n, gchar const *format, va_list args);
gboolean g_path_is_absolute(const gchar *file_name);
G_CONST_RETURN gchar* g_path_skip_root(const gchar *file_name);
#ifndef G_DISABLE_DEPRECATED
G_CONST_RETURN gchar* g_basename(const gchar *file_name);
#endif
#ifdef G_OS_WIN32
#define g_get_current_dir g_get_current_dir_utf8
#endif
gchar* g_get_current_dir(void);
gchar* g_path_get_basename(const gchar *file_name) G_GNUC_MALLOC;
gchar* g_path_get_dirname(const gchar *file_name) G_GNUC_MALLOC;
void g_nullify_pointer(gpointer *nullify_location);
#ifdef G_OS_WIN32
#define g_getenv g_getenv_utf8
#define g_setenv g_setenv_utf8
#define g_unsetenv g_unsetenv_utf8
#define g_find_program_in_path g_find_program_in_path_utf8
#endif
G_CONST_RETURN gchar* g_getenv(const gchar *variable);
gint g_setenv(const gchar *variable, const gchar *value, gint overwrite);
void g_unsetenv(const gchar *variable);
gchar** g_listenv(void);
gchar** g_get_environ(void);
const gchar* _g_getenv_nomalloc(const gchar	*variable, gchar buffer[1024]);
typedef	void (*GVoidFunc)(void);
#ifndef ATEXIT
# define ATEXIT(proc) g_ATEXIT(proc)
#else
# define G_NATIVE_ATEXIT
#endif
void g_atexit(GVoidFunc func);
#ifdef G_OS_WIN32
#if (defined(__MINGW_H) && !defined(_STDLIB_H_)) || (defined(_MSC_VER) && !defined(_INC_STDLIB))
int atexit (void(*)(void));
#endif
#define g_atexit(func) atexit(func)
#endif
gchar* g_find_program_in_path(const gchar *program);
G_INLINE_FUNC gint g_bit_nth_lsf(gulong mask, gint nth_bit) G_GNUC_CONST;
G_INLINE_FUNC gint g_bit_nth_msf(gulong mask, gint nth_bit) G_GNUC_CONST;
G_INLINE_FUNC guint	g_bit_storage(gulong  number) G_GNUC_CONST;
typedef struct _GTrashStack GTrashStack;
struct _GTrashStack {
  GTrashStack *next;
};
G_INLINE_FUNC void g_trash_stack_push(GTrashStack **stack_p, gpointer data_p);
G_INLINE_FUNC gpointer g_trash_stack_pop(GTrashStack **stack_p);
G_INLINE_FUNC gpointer g_trash_stack_peek(GTrashStack **stack_p);
G_INLINE_FUNC guint	g_trash_stack_height(GTrashStack **stack_p);
#if defined (G_CAN_INLINE) || defined (__G_UTILS_C__)
G_INLINE_FUNC gint g_bit_nth_lsf(gulong mask, gint nth_bit) {
  if (G_UNLIKELY (nth_bit < -1)) nth_bit = -1;
  while (nth_bit < ((GLIB_SIZEOF_LONG * 8) - 1)) {
      nth_bit++;
      if (mask & (1UL << nth_bit))
	return nth_bit;
  }
  return -1;
}
G_INLINE_FUNC gint g_bit_nth_msf(gulong mask, gint nth_bit) {
  if (nth_bit < 0 || G_UNLIKELY(nth_bit > GLIB_SIZEOF_LONG * 8)) nth_bit = GLIB_SIZEOF_LONG * 8;
  while(nth_bit > 0) {
      nth_bit--;
      if (mask & (1UL << nth_bit))
	  return nth_bit;
  }
  return -1;
}
G_INLINE_FUNC guint g_bit_storage (gulong number) {
#if defined(__GNUC__) && (__GNUC__ >= 4) && defined(__OPTIMIZE__)
  return G_LIKELY (number) ? ((GLIB_SIZEOF_LONG * 8U - 1) ^ __builtin_clzl(number)) + 1 : 1;
#else
  register guint n_bits = 0;
  do {
      n_bits++;
      number >>= 1;
  }
  while(number);
  return n_bits;
#endif
}
G_INLINE_FUNC void g_trash_stack_push(GTrashStack **stack_p, gpointer data_p) {
  GTrashStack *data = (GTrashStack *) data_p;
  data->next = *stack_p;
  *stack_p = data;
}
G_INLINE_FUNC gpointer g_trash_stack_pop(GTrashStack **stack_p) {
  GTrashStack *data;
  data = *stack_p;
  if (data) {
      *stack_p = data->next;
      data->next = NULL;
  }
  return data;
}
G_INLINE_FUNC gpointer g_trash_stack_peek (GTrashStack **stack_p) {
  GTrashStack *data;
  data = *stack_p;
  return data;
}
G_INLINE_FUNC guint g_trash_stack_height (GTrashStack **stack_p) {
  GTrashStack *data;
  guint i = 0;
  for (data = *stack_p; data; data = data->next) i++;
  return i;
}
#endif
GLIB_VAR const guint glib_major_version;
GLIB_VAR const guint glib_minor_version;
GLIB_VAR const guint glib_micro_version;
GLIB_VAR const guint glib_interface_age;
GLIB_VAR const guint glib_binary_age;
const gchar* glib_check_version(guint required_major, guint required_minor, guint required_micro);
#define GLIB_CHECK_VERSION(major,minor,micro) \
(GLIB_MAJOR_VERSION > (major) || (GLIB_MAJOR_VERSION == (major) && GLIB_MINOR_VERSION > (minor)) || \
(GLIB_MAJOR_VERSION == (major) && GLIB_MINOR_VERSION == (minor) && GLIB_MICRO_VERSION >= (micro)))
G_END_DECLS
#ifndef G_DISABLE_DEPRECATED
#ifndef G_PLATFORM_WIN32
# define G_WIN32_DLLMAIN_FOR_DLL_NAME(static, dll_name)
#else
# define G_WIN32_DLLMAIN_FOR_DLL_NAME(static, dll_name)			               \
static char *dll_name;							                               \
BOOL WINAPI	DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { \
  wchar_t wcbfr[1000];							                               \
  char *tem;								                                   \
  switch(fdwReason) {									                       \
      case DLL_PROCESS_ATTACH:						                           \
          GetModuleFileNameW ((HMODULE) hinstDLL, wcbfr, G_N_ELEMENTS (wcbfr));    \
          tem = g_utf16_to_utf8 (wcbfr, -1, NULL, NULL, NULL);		               \
          dll_name = g_path_get_basename (tem);				                       \
          g_free (tem);							                                   \
          break;								                                   \
  }									                                           \
  return TRUE;								                                   \
}
#endif
#endif
#endif