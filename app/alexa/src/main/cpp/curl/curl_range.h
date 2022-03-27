#ifndef HEADER_CURL_RANGE_H
#define HEADER_CURL_RANGE_H

#include "curl_setup.h"
#include "urldata.h"

CURLcode Curl_range(struct connectdata *conn);

#endif