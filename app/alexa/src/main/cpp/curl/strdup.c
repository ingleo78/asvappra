#include "curl_setup.h"
#include "curl.h"
#include "strdup.h"
#include "curl_memory.h"
#include "memdebug.h"

#ifndef HAVE_STRDUP
char *curlx_strdup(const char *str) {
    size_t len;
    char *newstr;
    if(!str) return (char *)NULL;
    len = strlen(str);
    if(len >= ((size_t)-1) / sizeof(char)) return (char *)NULL;
    newstr = malloc((len + 1)*sizeof(char));
    if(!newstr) return (char *)NULL;
    memcpy(newstr, str, (len + 1)*sizeof(char));
    return newstr;
}
#endif
void *Curl_memdup(const void *src, size_t length) {
    void *buffer = malloc(length);
    if(!buffer) return NULL;
    memcpy(buffer, src, length);
    return buffer;
}
void *Curl_saferealloc(void *ptr, size_t size) {
  void *datap = realloc(ptr, size);
  if(size && !datap) free(ptr);
  return datap;
}