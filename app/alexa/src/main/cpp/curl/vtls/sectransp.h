#ifndef HEADER_CURL_SECTRANSP_H
#define HEADER_CURL_SECTRANSP_H

#include "../curl_setup.h"
#include "../urldata.h"
#include "../curl_base64.h"
#include "../strtok.h"
#include "../multiif.h"

#ifdef USE_SECTRANSP
extern const struct Curl_ssl Curl_ssl_sectransp;
#endif
#endif