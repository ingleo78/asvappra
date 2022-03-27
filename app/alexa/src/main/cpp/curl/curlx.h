#ifndef HEADER_CURL_CURLX_H
#define HEADER_CURL_CURLX_H

#include "mprintf.h"
#include "strcase.h"
#include "strtoofft.h"
#include "nonblock.h"
#include "warnless.h"
#include "curl_multibyte.h"

#define curlx_getenv curl_getenv
#define curlx_mvsnprintf curl_mvsnprintf
#define curlx_msnprintf curl_msnprintf
#define curlx_maprintf curl_maprintf
#define curlx_mvaprintf curl_mvaprintf
#define curlx_msprintf curl_msprintf
#define curlx_mprintf curl_mprintf
#define curlx_mfprintf curl_mfprintf
#define curlx_mvsprintf curl_mvsprintf
#define curlx_mvprintf curl_mvprintf
#define curlx_mvfprintf curl_mvfprintf
#ifdef ENABLE_CURLX_PRINTF
#undef printf
#undef fprintf
#undef sprintf
#undef msnprintf
#undef vprintf
#undef vfprintf
#undef vsprintf
#undef mvsnprintf
#undef aprintf
#undef vaprintf
#define printf curlx_mprintf
#define fprintf curlx_mfprintf
#define sprintf curlx_msprintf
#define msnprintf curlx_msnprintf
#define vprintf curlx_mvprintf
#define vfprintf curlx_mvfprintf
#define mvsnprintf curlx_mvsnprintf
#define aprintf curlx_maprintf
#define vaprintf curlx_mvaprintf
#endif
#endif