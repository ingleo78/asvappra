#ifndef HEADER_CURL_NETRC_H
#define HEADER_CURL_NETRC_H

#include "curl_setup.h"
#ifndef CURL_DISABLE_NETRC
int Curl_parsenetrc(const char *host, char **loginp, char **passwordp, bool *login_changed, bool *password_changed, char *filename);
#else
#define Curl_parsenetrc(a,b,c,d,e,f) 1
#endif
#endif