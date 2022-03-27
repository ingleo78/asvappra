#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_WIN32_H__
#define __G_WIN32_H__

#include "gtypes.h"

#ifdef G_PLATFORM_WIN32
G_BEGIN_DECLS
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#ifdef G_OS_WIN32
gint g_win32_ftruncate(gint f, guint size);
#endif
gchar* g_win32_getlocale(void);
gchar* g_win32_error_message(gint error);
#ifndef G_DISABLE_DEPRECATED
#define g_win32_get_package_installation_directory g_win32_get_package_installation_directory_utf8
#define g_win32_get_package_installation_subdirectory g_win32_get_package_installation_subdirectory_utf8
gchar* g_win32_get_package_installation_directory(const gchar *package, const gchar *dll_name);
gchar* g_win32_get_package_installation_subdirectory (const gchar *package, const gchar *dll_name, const gchar *subdir);
#endif
gchar* g_win32_get_package_installation_directory_of_module(gpointer hmodule);
guint g_win32_get_windows_version(void);
gchar* g_win32_locale_filename_from_utf8(const gchar *utf8filename);
#define G_WIN32_IS_NT_BASED() TRUE
#define G_WIN32_HAVE_WIDECHAR_API() TRUE
G_END_DECLS
#endif
#endif