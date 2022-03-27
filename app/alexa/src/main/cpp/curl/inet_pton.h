#ifndef HEADER_CURL_INET_PTON_H
#define HEADER_CURL_INET_PTON_H

#include "curl_setup.h"

int Curl_inet_pton(int, const char *, void *);
#ifdef HAVE_INET_PTON
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#elif defined(HAVE_WS2TCPIP_H)
#include <ws2tcpip.h>
#endif
#define Curl_inet_pton(x,y,z) inet_pton(x,y,z)
#endif
#endif