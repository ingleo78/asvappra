#ifndef HEADER_CURL_INET_NTOP_H
#define HEADER_CURL_INET_NTOP_H

#include "curl_setup.h"

char *Curl_inet_ntop(int af, const void *addr, char *buf, size_t size);
#ifdef HAVE_INET_NTOP
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#define Curl_inet_ntop(af,addr,buf,size) inet_ntop(af, addr, buf, (curl_socklen_t)size)
#endif
#endif