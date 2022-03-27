#ifndef HEADER_CURL_MEMORY_H
#define HEADER_CURL_MEMORY_H

#ifdef HEADER_CURL_MEMDEBUG_H
#error "Header memdebug.h shall not be included before curl_memory.h"
#endif
#ifndef CURLX_NO_MEMORY_CALLBACKS
#ifndef CURL_DID_MEMORY_FUNC_TYPEDEFS
typedef void *(*curl_malloc_callback)(size_t size);
typedef void (*curl_free_callback)(void *ptr);
typedef void *(*curl_realloc_callback)(void *ptr, size_t size);
typedef char *(*curl_strdup_callback)(const char *str);
typedef void *(*curl_calloc_callback)(size_t nmemb, size_t size);
#define CURL_DID_MEMORY_FUNC_TYPEDEFS
#endif
extern curl_malloc_callback Curl_cmalloc;
extern curl_free_callback Curl_cfree;
extern curl_realloc_callback Curl_crealloc;
extern curl_strdup_callback Curl_cstrdup;
extern curl_calloc_callback Curl_ccalloc;
#if defined(WIN32) && defined(UNICODE)
extern curl_wcsdup_callback Curl_cwcsdup;
#endif
#ifndef CURLDEBUG
#undef strdup
#define strdup(ptr) Curl_cstrdup(ptr)
#undef malloc
#define malloc(size) Curl_cmalloc(size)
#undef calloc
#define calloc(nbelem,size) Curl_ccalloc(nbelem, size)
#undef realloc
#define realloc(ptr,size) Curl_crealloc(ptr, size)
#undef free
#define free(ptr) Curl_cfree(ptr)
#ifdef WIN32
#ifdef UNICODE
#undef wcsdup
#define wcsdup(ptr) Curl_cwcsdup(ptr)
#undef _wcsdup
#define _wcsdup(ptr) Curl_cwcsdup(ptr)
#undef _tcsdup
#define _tcsdup(ptr) Curl_cwcsdup(ptr)
#else
#undef _tcsdup
#define _tcsdup(ptr) Curl_cstrdup(ptr)
#endif
#endif
#endif
#else
#ifndef MEMDEBUG_NODEFINES
#define MEMDEBUG_NODEFINES
#endif
#endif
#endif