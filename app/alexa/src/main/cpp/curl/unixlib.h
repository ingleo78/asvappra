#ifndef _UNIXLIB_H
#define _UNIXLIB_H

#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif
extern char ** environ __asm ("decc$ga_environ");
#define ___gdecl(_func,_arglist) _func _arglist
char    *getenv(const char *);
char    *getcwd(char *, size_t);
char    *___gdecl(ecvt, (double,int,int *,int *));
char    *___gdecl(fcvt, (double,int,int *,int *));
char    *___gdecl(gcvt, (double,int,char *));
#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif
int      setgid (gid_t), setuid (uid_t);
int      brk(void *);
void    *sbrk(ptrdiff_t __increment);
int      chdir(const char *);
int      chmod(const char *, mode_t);
int      chown(const char *,unsigned, unsigned);
char    *ctermid(char *);
char    *cuserid(char *);
int      mkdir(const char *, mode_t);
int      nice(int);
mode_t      umask(mode_t);
#undef ___gdecl
#ifdef __cplusplus
}
#endif
#endif