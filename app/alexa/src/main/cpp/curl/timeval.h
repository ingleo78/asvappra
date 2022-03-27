#ifndef HEADER_CURL_TIMEVAL_H
#define HEADER_CURL_TIMEVAL_H

#include "curl_setup.h"

typedef curl_off_t timediff_t;
#define CURL_FORMAT_TIMEDIFF_T CURL_FORMAT_CURL_OFF_T
#define TIMEDIFF_T_MAX CURL_OFF_T_MAX
#define TIMEDIFF_T_MIN CURL_OFF_T_MIN

struct curltime {
  time_t tv_sec;
  time_t tv_usec;
};

struct curltime Curl_now(void);
timediff_t Curl_timediff(struct curltime t1, struct curltime t2);
timediff_t Curl_timediff_us(struct curltime newer, struct curltime older);

#endif