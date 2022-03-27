#include <limits.h>
#include <stddef.h>

#define XML_MAX_CHUNK_LEN (INT_MAX / 2 + 1)
typedef unsigned int size_t;

#ifdef XML_UNICODE
int filemap(const wchar_t *name, void (*processor)(const void *, size_t, const wchar_t *, void *arg), void *arg);
#else
int filemap(const char *name, void (*processor)(const void *, size_t, const char *, void *arg), void *arg);
#endif