#ifndef HEADER_CURL_PATH_H
#define HEADER_CURL_PATH_H

#include "curl_setup.h"
#include "curl.h"
#include "urldata.h"

#ifdef WIN32
#undef  PATH_MAX
#define PATH_MAX MAX_PATH
#ifndef R_OK
#define R_OK 4
#endif
#endif
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
CURLcode Curl_getworkingpath(struct connectdata *conn, char *homedir, char **path);
CURLcode Curl_get_pathname(const char **cpp, char **path, char *homedir);
#endif