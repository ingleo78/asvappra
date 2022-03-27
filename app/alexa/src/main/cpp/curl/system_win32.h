#ifndef HEADER_CURL_SYSTEM_WIN32_H
#define HEADER_CURL_SYSTEM_WIN32_H

#include "curl_setup.h"

#if defined(WIN32)
extern LARGE_INTEGER Curl_freq;
extern bool Curl_isVistaOrGreater;
CURLcode Curl_win32_init(long flags);
void Curl_win32_cleanup(long init_flags);
typedef enum {
    VERSION_LESS_THAN,
    VERSION_LESS_THAN_EQUAL,
    VERSION_EQUAL,
    VERSION_GREATER_THAN_EQUAL,
    VERSION_GREATER_THAN
} VersionCondition;
typedef enum {
    PLATFORM_DONT_CARE,
    PLATFORM_WINDOWS,
    PLATFORM_WINNT
} PlatformIdentifier;
typedef unsigned int(WINAPI *IF_NAMETOINDEX_FN)(const char *);
extern IF_NAMETOINDEX_FN Curl_if_nametoindex;
bool Curl_verify_windows_version(const unsigned int majorVersion, const unsigned int minorVersion, const PlatformIdentifier platform,
                                 const VersionCondition condition);
HMODULE Curl_load_library(LPCTSTR filename);
#endif
#endif