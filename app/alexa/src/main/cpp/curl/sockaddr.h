#ifndef HEADER_CURL_SOCKADDR_H
#define HEADER_CURL_SOCKADDR_H

#include "curl_setup.h"
#include "curl_setup_once.h"

struct Curl_sockaddr_storage {
  union {
    struct sockaddr sa;
    struct sockaddr_in sa_in;
#ifdef ENABLE_IPV6
    struct sockaddr_in6 sa_in6;
#endif
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
    struct sockaddr_storage sa_stor;
#else
    char cbuf[256];
#endif
  } buffer;
};

#endif