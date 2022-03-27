#ifndef HEADER_CURL_NONBLOCK_H
#define HEADER_CURL_NONBLOCK_H

#include "curl.h"

int curlx_nonblock(curl_socket_t sockfd, int nonblock);

#endif