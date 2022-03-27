#ifndef HEADER_CURL_PARSEDATE_H
#define HEADER_CURL_PARSEDATE_H

#include <sys/types.h>
#include "curl.h"

extern const char * const Curl_wkday[7];
extern const char * const Curl_month[12];

CURLcode Curl_gmtime(time_t intime, struct tm *store);
time_t Curl_getdate_capped(const char *p);

#endif