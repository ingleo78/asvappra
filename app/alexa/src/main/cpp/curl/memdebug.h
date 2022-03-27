#ifndef HEADER_CURL_MEMDEBUG_H
#define HEADER_CURL_MEMDEBUG_H

#ifndef CURLDEBUG
#define CURL_MT_LOGFNAME_BUFSIZE 512
extern FILE *curl_dbg_logfile;
CURL_EXTERN void *curl_dbg_malloc(size_t size, int line, const char *source);
CURL_EXTERN void *curl_dbg_calloc(size_t elements, size_t size, int line, const char *source);
CURL_EXTERN void *curl_dbg_realloc(void *ptr, size_t size, int line, const char *source);
CURL_EXTERN void curl_dbg_free(void *ptr, int line, const char *source);
CURL_EXTERN char *curl_dbg_strdup(const char *str, int line, const char *src);
#if defined(WIN32) && defined(UNICODE)
CURL_EXTERN wchar_t *curl_dbg_wcsdup(const wchar_t *str, int line, const char *source);
#endif
CURL_EXTERN void curl_dbg_memdebug(const char *logname);
CURL_EXTERN void curl_dbg_memlimit(long limit);
CURL_EXTERN void curl_dbg_log(const char *format, ...);
CURL_EXTERN curl_socket_t curl_dbg_socket(int domain, int type, int protocol, int line, const char *source);
CURL_EXTERN void curl_dbg_mark_sclose(curl_socket_t sockfd, int line, const char *source);
CURL_EXTERN int curl_dbg_sclose(curl_socket_t sockfd, int line, const char *source);
CURL_EXTERN curl_socket_t curl_dbg_accept(curl_socket_t s, void *a, void *alen, int line, const char *source);
#ifdef HAVE_SOCKETPAIR
CURL_EXTERN int curl_dbg_socketpair(int domain, int type, int protocol, curl_socket_t socket_vector[2], int line, const char *source);
#endif
CURL_EXTERN SEND_TYPE_RETV curl_dbg_send(SEND_TYPE_ARG1 sockfd, SEND_QUAL_ARG2 SEND_TYPE_ARG2 buf, SEND_TYPE_ARG3 len, SEND_TYPE_ARG4 flags,
                                         int line, const char *source);
CURL_EXTERN RECV_TYPE_RETV curl_dbg_recv(RECV_TYPE_ARG1 sockfd, RECV_TYPE_ARG2 buf, RECV_TYPE_ARG3 len, RECV_TYPE_ARG4 flags, int line, const char *source);
CURL_EXTERN FILE *curl_dbg_fopen(const char *file, const char *mode, int line, const char *source);
CURL_EXTERN int curl_dbg_fclose(FILE *file, int line, const char *source);
#ifndef MEMDEBUG_NODEFINES
#undef strdup
#define strdup(ptr) curl_dbg_strdup(ptr, __LINE__, __FILE__)
#define malloc(size) curl_dbg_malloc(size, __LINE__, __FILE__)
#define calloc(nbelem,size) curl_dbg_calloc(nbelem, size, __LINE__, __FILE__)
#define realloc(ptr,size) curl_dbg_realloc(ptr, size, __LINE__, __FILE__)
#define free(ptr) curl_dbg_free(ptr, __LINE__, __FILE__)
#define send(a,b,c,d) curl_dbg_send(a,b,c,d, __LINE__, __FILE__)
#define recv(a,b,c,d) curl_dbg_recv(a,b,c,d, __LINE__, __FILE__)
#ifdef WIN32
#ifdef UNICODE
#undef wcsdup
#define wcsdup(ptr) curl_dbg_wcsdup(ptr, __LINE__, __FILE__)
#undef _wcsdup
#define _wcsdup(ptr) curl_dbg_wcsdup(ptr, __LINE__, __FILE__)
#undef _tcsdup
#define _tcsdup(ptr) curl_dbg_wcsdup(ptr, __LINE__, __FILE__)
#else
#undef _tcsdup
#define _tcsdup(ptr) curl_dbg_strdup(ptr, __LINE__, __FILE__)
#endif
#endif
#undef socket
#define socket(domain,type,protocol) curl_dbg_socket(domain, type, protocol, __LINE__, __FILE__)
#undef accept
#define accept(sock,addr,len) curl_dbg_accept(sock, addr, len, __LINE__, __FILE__)
#ifndef HAVE_SOCKETPAIR
#define socketpair(domain,type,protocol,socket_vector) curl_dbg_socketpair(domain, type, protocol, socket_vector, __LINE__, __FILE__)
#endif
#ifdef HAVE_GETADDRINFO
#if defined(getaddrinfo) && defined(__osf__)
#define ogetaddrinfo(host,serv,hint,res) curl_dbg_getaddrinfo(host, serv, hint, res, __LINE__, __FILE__)
#else
#undef getaddrinfo
#define getaddrinfo(host,serv,hint,res) curl_dbg_getaddrinfo(host, serv, hint, res, __LINE__, __FILE__)
#endif
#endif
#ifdef HAVE_FREEADDRINFO
#undef freeaddrinfo
#define freeaddrinfo(data) curl_dbg_freeaddrinfo(data, __LINE__, __FILE__)
#endif
#undef sclose
#define sclose(sockfd) curl_dbg_sclose(sockfd,__LINE__,__FILE__)
#define fake_sclose(sockfd) curl_dbg_mark_sclose(sockfd,__LINE__,__FILE__)
#undef fopen
#define fopen(file,mode) curl_dbg_fopen(file,mode,__LINE__,__FILE__)
#undef fdopen
#define fdopen(file,mode) curl_dbg_fdopen(file,mode,__LINE__,__FILE__)
#define fclose(file) curl_dbg_fclose(file,__LINE__,__FILE__)
#endif
#endif
#ifndef fake_sclose
#define fake_sclose(x)  Curl_nop_stmt
#endif
#define Curl_safefree(ptr) \
  do { free((ptr)); (ptr) = NULL;} while(0)
#endif