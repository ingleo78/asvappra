#ifndef HEADER_CURL_TOOL_URLGLOB_H
#define HEADER_CURL_TOOL_URLGLOB_H

#include "tool_setup.h"
#include "urlapi.h"

typedef enum {
    UPTSet = 1,
    UPTCharRange,
    UPTNumRange
} URLPatternType;
struct URLPattern {
    URLPatternType type;
    int globindex;
    union {
        struct {
            char **elements;
            int size;
            int ptr_s;
        } Set;
        struct {
            char min_c;
            char max_c;
            char ptr_c;
            int step;
        } CharRange;
        struct {
            unsigned long min_n;
            unsigned long max_n;
            int padlength;
            unsigned long ptr_n;
            unsigned long step;
        } NumRange;
    } content;
};
#define GLOB_PATTERN_NUM 100
struct URLGlob {
    struct URLPattern pattern[GLOB_PATTERN_NUM];
    size_t size;
    size_t urllen;
    char *glob_buffer;
    char beenhere;
    const char *error;
    size_t pos;
};
CURLcode glob_url(struct URLGlob**, char *, unsigned long *, FILE *);
CURLcode glob_next_url(char **, struct URLGlob *);
CURLcode glob_match_url(char **, char *, struct URLGlob *);
void glob_cleanup(struct URLGlob *glob);

#endif