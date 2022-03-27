#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include "tool_setup.h"

#if defined(__AMIGA__) && !defined(__amigaos4__)
#undef HAVE_TERMIOS_H
#endif
#ifndef HAVE_GETPASS_R
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#elif defined(HAVE_TERMIO_H)
#include <termio.h>
#endif
#ifdef __VMS
#include <descrip>
#include <starlet>
#include <iodef>
#endif
#ifdef WIN32
#include <conio.h>
#endif
#ifdef NETWARE
#ifdef __NOVELL_LIBC__
#include <screen.h>
#else
#include <nwconio.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "tool_getpass.h"
#include "memdebug.h"
#include "tool_main.h"
#ifdef __VMS
char *getpass_r(const char *prompt, char *buffer, size_t buflen) {
    long sts;
    short chan;
    struct _iosb {
        short int iosb$w_status;
        short int iosb$w_bcnt;
        int       unused;
    } iosb;
    $DESCRIPTOR(ttdesc, "TT");
    buffer[0] = '\0';
    sts = sys$assign(&ttdesc, &chan, 0, 0);
    if(sts & 1) {
        sts = sys$qiow(0, chan, IO$_READPROMPT | IO$M_NOECHO, &iosb, 0, 0, buffer, buflen, 0, 0, prompt, strlen(prompt));
        if((sts & 1) && (iosb.iosb$w_status & 1)) buffer[iosb.iosb$w_bcnt] = '\0';
        sys$dassgn(chan);
    }
    return buffer;
}
#define DONE
#endif
#ifdef __SYMBIAN32__
#define getch() getchar()
#endif
#if defined(WIN32) || defined(__SYMBIAN32__)
char *getpass_r(const char *prompt, char *buffer, size_t buflen) {
    size_t i;
    fputs(prompt, stderr);
    for(i = 0; i < buflen; i++) {
        buffer[i] = (char)getch();
        if(buffer[i] == '\r' || buffer[i] == '\n') {
            buffer[i] = '\0';
            break;
        } else if(buffer[i] == '\b') i = i - (i >= 1 ? 2 : 1);
    }
#ifndef __SYMBIAN32__
    fputs("\n", stderr);
#endif
    if(i == buflen) buffer[buflen-1] = '\0';
    return buffer;
}
#define DONE
#endif
#ifdef NETWARE
#ifdef __NOVELL_LIBC__
char *getpass_r(const char *prompt, char *buffer, size_t buflen) {
    return getpassword(prompt, buffer, buflen);
}
#else
char *getpass_r(const char *prompt, char *buffer, size_t buflen) {
    size_t i = 0;
    printf("%s", prompt);
    do {
    buffer[i++] = getch();
    if(buffer[i-1] == '\b') {
        if(i > 1) {
            printf("\b \b");
            i = i - 2;
        } else {
            RingTheBell();
            i = i - 1;
        }
    } else if(buffer[i-1] != 13) putchar('*');
    } while((buffer[i-1] != 13) && (i < buflen));
    buffer[i-1] = '\0';
    printf("\r\n");
    return buffer;
}
#endif
#define DONE
#endif
#ifndef DONE
#ifdef HAVE_TERMIOS_H
#define struct_term  struct termios
#elif defined(HAVE_TERMIO_H)
#define struct_term  struct termio
#else
#undef  struct_term
#endif
static bool ttyecho(bool enable, int fd) {
#ifdef struct_term
    static struct_term withecho;
    static struct_term noecho;
#endif
    if(!enable) {
    #ifdef HAVE_TERMIOS_H
        tcgetattr(fd, &withecho);
        noecho = withecho;
        noecho.c_lflag &= ~ECHO;
        tcsetattr(fd, TCSANOW, &noecho);
    #elif defined(HAVE_TERMIO_H)
        ioctl(fd, TCGETA, &withecho);
        noecho = withecho;
        noecho.c_lflag &= ~ECHO;
        ioctl(fd, TCSETA, &noecho);
    #else
        (void)fd;
        return FALSE;
    #endif
        return TRUE;
    }
#ifdef HAVE_TERMIOS_H
    tcsetattr(fd, TCSAFLUSH, &withecho);
#elif defined(HAVE_TERMIO_H)
    ioctl(fd, TCSETA, &withecho);
#else
    return FALSE;
#endif
    return TRUE;
}
char *getpass_r(const char *prompt, char *password, size_t buflen) {
    ssize_t nread;
    bool disabled;
    int fd = open("/dev/tty", O_RDONLY);
    if(-1 == fd) fd = STDIN_FILENO;
    disabled = ttyecho(FALSE, fd);
    fputs(prompt, stderr);
    nread = read(fd, password, buflen);
    if(nread > 0) password[--nread] = '\0';
    else password[0] = '\0';
    if(disabled) {
        fputs("\n", stderr);
        (void)ttyecho(TRUE, fd);
    }
    if(STDIN_FILENO != fd) close(fd);
    return password;
}
#endif
#endif