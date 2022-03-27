#ifndef HEADER_CURL_TOOL_SETUP_H
#define HEADER_CURL_TOOL_SETUP_H
#define CURL_NO_OLDIES
#include "curl_setup.h"
#include "curl.h"

#if defined(macintosh) && defined(__MRC__)
#define main(x,y) curl_main(x,y)
#endif
#ifdef TPF
#undef select
#define select(a,b,c,d,e) tpf_select_bsd(a,b,c,d,e)
#define CONF_DEFAULT (0|CONF_NOPROGRESS)
#endif
#ifndef OS
#define OS "unknown"
#endif
#ifndef UNPRINTABLE_CHAR
#define UNPRINTABLE_CHAR '.'
#endif
#ifndef HAVE_STRDUP
#include "tool_strdup.h"
#endif
#endif