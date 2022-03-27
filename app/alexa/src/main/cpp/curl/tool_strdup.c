#include "tool_strdup.h"

#ifndef HAVE_STRDUP
char *strdup(const char *str) {
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