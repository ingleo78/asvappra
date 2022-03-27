#ifndef HEADER_CURL_FNMATCH_H
#define HEADER_CURL_FNMATCH_H

#define CURL_FNMATCH_MATCH    0
#define CURL_FNMATCH_NOMATCH  1
#define CURL_FNMATCH_FAIL     2
int Curl_fnmatch(void *ptr, const char *pattern, const char *string);

#endif