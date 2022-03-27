#ifndef __G_STDIO_H__
#define __G_STDIO_H__

#include <sys/stat.h>
#include "gprintf.h"

G_BEGIN_DECLS
#if defined(_MSC_VER) && !defined(_WIN64)
typedef struct _stat32 GStatBuf;
#else
typedef struct stat GStatBuf;
#endif
#if defined(G_OS_UNIX) && !defined(G_STDIO_NO_WRAP_ON_UNIX)
#define g_chmod   chmod
#define g_open    open
#define g_creat   creat
#define g_rename  rename
#define g_mkdir   mkdir
#define g_stat    stat
#define g_lstat   lstat
#define g_remove  remove
#define g_fopen   fopen
#define g_freopen freopen
#define g_utime   utime
int g_access(const gchar *filename, int mode);
int g_chdir(const gchar *path);
int g_unlink(const gchar *filename);
int g_rmdir(const gchar *filename);
#else
int g_access(const gchar *filename, int mode);
int g_chmod(const gchar *filename, int mode);
int g_open(const gchar *filename, int flags, int mode);
int g_creat(const gchar *filename, int mode);
int g_rename(const gchar *oldfilename, const gchar *newfilename);
int g_mkdir(const gchar *filename, int mode);
int g_chdir(const gchar *path);
int g_stat(const gchar *filename, GStatBuf *buf);
int g_lstat(const gchar *filename, GStatBuf *buf);
int g_unlink(const gchar *filename);
int g_remove(const gchar *filename);
int g_rmdir(const gchar *filename);
FILE *g_fopen(const gchar *filename, const gchar *mode);
FILE *g_freopen(const gchar *filename, const gchar *mode, FILE *stream);
struct utimbuf;
int g_utime(const gchar *filename, struct utimbuf *utb);
#endif
G_END_DECLS

#endif