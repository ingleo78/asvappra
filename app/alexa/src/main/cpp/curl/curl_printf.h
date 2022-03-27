#ifndef HEADER_CURL_PRINTF_H
#define HEADER_CURL_PRINTF_H

#include "mprintf.h"

#undef printf
#undef fprintf
#undef msnprintf
#undef vprintf
#undef vfprintf
#undef vsnprintf
#undef aprintf
#undef vaprintf
#define printf curl_mprintf
#define fprintf curl_mfprintf
#define msnprintf curl_msnprintf
#define vprintf curl_mvprintf
#define vfprintf curl_mvfprintf
#define mvsnprintf curl_mvsnprintf
#define aprintf curl_maprintf
#define vaprintf curl_mvaprintf
#endif