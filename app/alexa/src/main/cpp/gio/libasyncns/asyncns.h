#ifndef fooasyncnshfoo
#define fooasyncnshfoo

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#ifdef  __cplusplus
extern "C" {
#endif
typedef struct asyncns _g_asyncns_t;
typedef struct _g_asyncns_query _g_asyncns_query_t;
typedef unsigned int size_t;
_g_asyncns_t* _g_asyncns_new(unsigned n_proc);
void _g_asyncns_free(_g_asyncns_t *asyncns);
int _g_asyncns_fd(_g_asyncns_t *asyncns);
int _g_asyncns_wait(_g_asyncns_t *asyncns, int block);
_g_asyncns_query_t* _g_asyncns_getaddrinfo(_g_asyncns_t *asyncns, const char *node, const char *service, const struct addrinfo *hints);
int _g_asyncns_getaddrinfo_done(_g_asyncns_t *asyncns, _g_asyncns_query_t* q, struct addrinfo **ret_res);
_g_asyncns_query_t* _g_asyncns_getnameinfo(_g_asyncns_t *asyncns, const struct sockaddr *sa, socklen_t salen, int flags, int gethost, int getserv);
int _g_asyncns_getnameinfo_done(_g_asyncns_t *asyncns, _g_asyncns_query_t* q, char *ret_host, size_t hostlen, char *ret_serv, size_t servlen);
_g_asyncns_query_t* _g_asyncns_res_query(_g_asyncns_t *asyncns, const char *dname, int clas, int type);
_g_asyncns_query_t* _g_asyncns_res_search(_g_asyncns_t *asyncns, const char *dname, int clas, int type);
int _g_asyncns_res_done(_g_asyncns_t *asyncns, _g_asyncns_query_t* q, unsigned char **answer);
_g_asyncns_query_t* _g_asyncns_getnext(_g_asyncns_t *asyncns);
int _g_asyncns_getnqueries(_g_asyncns_t *asyncns);
void _g_asyncns_cancel(_g_asyncns_t *asyncns, _g_asyncns_query_t* q);
void _g_asyncns_freeaddrinfo(struct addrinfo *ai);
void _g_asyncns_freeanswer(unsigned char *answer);
int _g_asyncns_isdone(_g_asyncns_t *asyncns, _g_asyncns_query_t*q);
void _g_asyncns_setuserdata(_g_asyncns_t *asyncns, _g_asyncns_query_t *q, void *userdata);
void* _g_asyncns_getuserdata(_g_asyncns_t *asyncns, _g_asyncns_query_t *q);
#ifdef  __cplusplus
}
#endif

#endif