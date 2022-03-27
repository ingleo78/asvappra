#ifndef HEADER_CURL_STRERROR_H
#define HEADER_CURL_STRERROR_H

#include "urldata.h"

#define STRERROR_LEN 256

const char *Curl_strerror(int err, char *buf, size_t buflen);
#if defined(WIN32) || defined(_WIN32_WCE)
const char *Curl_winapi_strerror(DWORD err, char *buf, size_t buflen);
#endif
#ifdef USE_WINDOWS_SSPI
const char *Curl_sspi_strerror(int err, char *buf, size_t buflen);
#endif
#endif