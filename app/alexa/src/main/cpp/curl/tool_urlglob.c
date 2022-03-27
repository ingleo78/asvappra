#include <stdbool.h>
#include <errno.h>
#include "tool_setup.h"
#define ENABLE_CURLX_PRINTF
#include "curl.h"
#include "curlx.h"
#include "curl_ctype.h"
#include "tool_cfgable.h"
#include "tool_doswin.h"
#include "tool_urlglob.h"
#include "tool_vms.h"
#include "memdebug.h"

#define GLOBERROR(string, column, code) glob->error = string, glob->pos = column, code
static CURLcode glob_fixed(struct URLGlob *glob, char *fixed, size_t len) {
    struct URLPattern *pat = &glob->pattern[glob->size];
    pat->type = UPTSet;
    pat->content.Set.size = 1;
    pat->content.Set.ptr_s = 0;
    pat->globindex = -1;
    pat->content.Set.elements = malloc(sizeof(char *));
    if(!pat->content.Set.elements) return GLOBERROR("out of memory", 0, CURLE_OUT_OF_MEMORY);
    pat->content.Set.elements[0] = malloc(len + 1);
    if(!pat->content.Set.elements[0]) return GLOBERROR("out of memory", 0, CURLE_OUT_OF_MEMORY);
    memcpy(pat->content.Set.elements[0], fixed, len);
    pat->content.Set.elements[0][len] = 0;
    return CURLE_OK;
}
static int multiply(unsigned long *amount, long with) {
    unsigned long sum = *amount * with;
    if(!with) {
        *amount = 0;
        return 0;
    }
    if(sum/with != *amount) return 1;
    *amount = sum;
    return 0;
}
static CURLcode glob_set(struct URLGlob *glob, char **patternp, size_t *posp, unsigned long *amount, int globindex) {
    struct URLPattern *pat;
    bool done = FALSE;
    char *buf = glob->glob_buffer;
    char *pattern = *patternp;
    char *opattern = pattern;
    size_t opos = *posp-1;
    pat = &glob->pattern[glob->size];
    pat->type = UPTSet;
    pat->content.Set.size = 0;
    pat->content.Set.ptr_s = 0;
    pat->content.Set.elements = NULL;
    pat->globindex = globindex;
    while(!done) {
        switch (*pattern) {
            case '\0': return GLOBERROR("unmatched brace", opos, CURLE_URL_MALFORMAT);
            case '{': case '[': return GLOBERROR("nested brace", *posp, CURLE_URL_MALFORMAT);
            case '}':
                if(opattern == pattern) return GLOBERROR("empty string within braces", *posp, CURLE_URL_MALFORMAT);
                if(multiply(amount, pat->content.Set.size + 1)) return GLOBERROR("range overflow", 0, CURLE_URL_MALFORMAT);
            case ',':
                *buf = '\0';
                if(pat->content.Set.elements) {
                    char **new_arr = realloc(pat->content.Set.elements,(pat->content.Set.size + 1) * sizeof(char *));
                    if(!new_arr) return GLOBERROR("out of memory", 0, CURLE_OUT_OF_MEMORY);
                    pat->content.Set.elements = new_arr;
                } else pat->content.Set.elements = malloc(sizeof(char *));
                if(!pat->content.Set.elements) return GLOBERROR("out of memory", 0, CURLE_OUT_OF_MEMORY);
                pat->content.Set.elements[pat->content.Set.size] = strdup(glob->glob_buffer);
                if(!pat->content.Set.elements[pat->content.Set.size]) return GLOBERROR("out of memory", 0, CURLE_OUT_OF_MEMORY);
                ++pat->content.Set.size;
                if(*pattern == '}') {
                    pattern++;
                    done = TRUE;
                    continue;
                }
                buf = glob->glob_buffer;
                ++pattern;
                ++(*posp);
                break;
            case ']': return GLOBERROR("unexpected close bracket", *posp, CURLE_URL_MALFORMAT);
            case '\\':
                if(pattern[1]) {
                    ++pattern;
                    ++(*posp);
                }
            default:
              *buf++ = *pattern++;
              ++(*posp);
        }
    }
    *patternp = pattern;
    return CURLE_OK;
}
static CURLcode glob_range(struct URLGlob *glob, char **patternp, size_t *posp, unsigned long *amount, int globindex) {
    struct URLPattern *pat;
    int rc;
    char *pattern = *patternp;
    char *c;
    pat = &glob->pattern[glob->size];
    pat->globindex = globindex;
    if(ISALPHA(*pattern)) {
        char min_c;
        char max_c;
        char end_c;
        unsigned long step = 1;
        pat->type = UPTCharRange;
        rc = sscanf(pattern, "%c-%c%c", &min_c, &max_c, &end_c);
        if(rc == 3) {
            if(end_c == ':') {
                char *endp;
                errno = 0;
                step = strtoul(&pattern[4], &endp, 10);
                if(errno || &pattern[4] == endp || *endp != ']') step = 0;
                else pattern = endp + 1;
            } else if(end_c != ']') rc = 0;
            else pattern += 4;
        }
        *posp += (pattern - *patternp);
        if(rc != 3 || !step || step > (unsigned)INT_MAX || (min_c == max_c && step != 1) || (min_c != max_c && (min_c > max_c || step > (unsigned)(max_c - min_c) ||
           (max_c - min_c) > ('z' - 'a')))) {
            return GLOBERROR("bad range", *posp, CURLE_URL_MALFORMAT);
        }
        pat->content.CharRange.step = (int)step;
        pat->content.CharRange.ptr_c = pat->content.CharRange.min_c = min_c;
        pat->content.CharRange.max_c = max_c;
        if(multiply(amount, ((pat->content.CharRange.max_c - pat->content.CharRange.min_c) / pat->content.CharRange.step + 1)))
            return GLOBERROR("range overflow", *posp, CURLE_URL_MALFORMAT);
    } else if(ISDIGIT(*pattern)) {
        unsigned long min_n;
        unsigned long max_n = 0;
        unsigned long step_n = 0;
        char *endp;
        pat->type = UPTNumRange;
        pat->content.NumRange.padlength = 0;
        if(*pattern == '0') {
            c = pattern;
            while(ISDIGIT(*c)) {
                c++;
                ++pat->content.NumRange.padlength;
            }
        }
        errno = 0;
        min_n = strtoul(pattern, &endp, 10);
        if(errno || (endp == pattern)) endp = NULL;
        else {
            if(*endp != '-') endp = NULL;
            else {
              pattern = endp + 1;
              while(*pattern && ISBLANK(*pattern)) pattern++;
              if(!ISDIGIT(*pattern)) {
                  endp = NULL;
                  goto fail;
              }
              errno = 0;
              max_n = strtoul(pattern, &endp, 10);
              if(errno) endp = NULL;
              else if(*endp == ':') {
                  pattern = endp + 1;
                  errno = 0;
                  step_n = strtoul(pattern, &endp, 10);
                  if(errno) endp = NULL;
              } else step_n = 1;
              if(endp && (*endp == ']'))  pattern = endp + 1;
              else endp = NULL;
            }
        }
        fail:
        *posp += (pattern - *patternp);
        if(!endp || !step_n || (min_n == max_n && step_n != 1) || (min_n != max_n && (min_n > max_n || step_n > (max_n - min_n))))
            return GLOBERROR("bad range", *posp, CURLE_URL_MALFORMAT);
        pat->content.NumRange.ptr_n = pat->content.NumRange.min_n = min_n;
        pat->content.NumRange.max_n = max_n;
        pat->content.NumRange.step = step_n;
        if(multiply(amount, ((pat->content.NumRange.max_n - pat->content.NumRange.min_n) / pat->content.NumRange.step + 1)))
            return GLOBERROR("range overflow", *posp, CURLE_URL_MALFORMAT);
    } else return GLOBERROR("bad range specification", *posp, CURLE_URL_MALFORMAT);
    *patternp = pattern;
    return CURLE_OK;
}
#define MAX_IP6LEN 128
static bool peek_ipv6(const char *str, size_t *skip) {
    char hostname[MAX_IP6LEN];
    CURLU *u;
    char *endbr = strchr(str, ']');
    size_t hlen;
    CURLUcode rc;
    if(!endbr) return FALSE;
    hlen = endbr - str + 1;
    if(hlen >= MAX_IP6LEN) return FALSE;
    u = curl_url();
    if(!u) return FALSE;
    memcpy(hostname, str, hlen);
    hostname[hlen] = 0;
    rc = curl_url_set(u, CURLUPART_URL, hostname, CURLU_GUESS_SCHEME);
    curl_url_cleanup(u);
    if(!rc) *skip = hlen;
    return rc ? FALSE : TRUE;
}
static CURLcode glob_parse(struct URLGlob *glob, char *pattern, size_t pos, unsigned long *amount) {
    CURLcode res = CURLE_OK;
    int globindex = 0;
    *amount = 1;
    while(*pattern && !res) {
        char *buf = glob->glob_buffer;
        size_t sublen = 0;
        while(*pattern && *pattern != '{') {
            if(*pattern == '[') {
                size_t skip = 0;
                if(!peek_ipv6(pattern, &skip) && (pattern[1] == ']')) skip = 2;
                if(skip) {
                    memcpy(buf, pattern, skip);
                    buf += skip;
                    pattern += skip;
                    sublen += skip;
                    continue;
                }
                break;
            }
            if(*pattern == '}' || *pattern == ']') return GLOBERROR("unmatched close brace/bracket", pos, CURLE_URL_MALFORMAT);
            if(*pattern == '\\' && (*(pattern + 1) == '{' || *(pattern + 1) == '[' || *(pattern + 1) == '}' || *(pattern + 1) == ']') ) {
                ++pattern;
                ++pos;
            }
            *buf++ = *pattern++;
            ++pos;
            sublen++;
        }
        if(sublen) {
            *buf = '\0';
            res = glob_fixed(glob, glob->glob_buffer, sublen);
        } else {
            switch (*pattern) {
                case '\0': break;
                case '{':
                    pattern++;
                    pos++;
                    res = glob_set(glob, &pattern, &pos, amount, globindex++);
                    break;
                case '[':
                    pattern++;
                    pos++;
                    res = glob_range(glob, &pattern, &pos, amount, globindex++);
                    break;
            }
        }
        if(++glob->size >= GLOB_PATTERN_NUM) return GLOBERROR("too many globs", pos, CURLE_URL_MALFORMAT);
    }
    return res;
}
CURLcode glob_url(struct URLGlob **glob, char *url, unsigned long *urlnum, FILE *error) {
    struct URLGlob *glob_expand;
    unsigned long amount = 0;
    char *glob_buffer;
    CURLcode res;
    *glob = NULL;
    glob_buffer = malloc(strlen(url) + 1);
    if(!glob_buffer) return CURLE_OUT_OF_MEMORY;
    glob_buffer[0] = 0;
    glob_expand = calloc(1, sizeof(struct URLGlob));
    if(!glob_expand) {
        Curl_safefree(glob_buffer);
        return CURLE_OUT_OF_MEMORY;
    }
    glob_expand->urllen = strlen(url);
    glob_expand->glob_buffer = glob_buffer;
    res = glob_parse(glob_expand, url, 1, &amount);
    if(!res) *urlnum = amount;
    else {
        if(error && glob_expand->error) {
            char text[512];
            const char *t;
            if(glob_expand->pos) {
                msnprintf(text, sizeof(text), "%s in URL position %zu:\n%s\n%*s^", glob_expand->error, glob_expand->pos, url, glob_expand->pos - 1, " ");
                t = text;
            } else t = glob_expand->error;
            fprintf(error, "curl: (%d) %s\n", res, t);
        }
        glob_cleanup(glob_expand);
        *urlnum = 1;
        return res;
    }
    *glob = glob_expand;
    return CURLE_OK;
}
void glob_cleanup(struct URLGlob *glob) {
    size_t i;
    int elem;
    if(!glob) return;
    for(i = 0; i < glob->size; i++) {
        if((glob->pattern[i].type == UPTSet) && (glob->pattern[i].content.Set.elements)) {
            for(elem = glob->pattern[i].content.Set.size - 1; elem >= 0; --elem) {
                Curl_safefree(glob->pattern[i].content.Set.elements[elem]);
            }
            Curl_safefree(glob->pattern[i].content.Set.elements);
        }
    }
    Curl_safefree(glob->glob_buffer);
    Curl_safefree(glob);
}
CURLcode glob_next_url(char **globbed, struct URLGlob *glob) {
    struct URLPattern *pat;
    size_t i;
    size_t len;
    size_t buflen = glob->urllen + 1;
    char *buf = glob->glob_buffer;
    *globbed = NULL;
    if(!glob->beenhere) glob->beenhere = 1;
    else {
        bool carry = TRUE;
        for(i = 0; carry && (i < glob->size); i++) {
            carry = FALSE;
            pat = &glob->pattern[glob->size - 1 - i];
            switch(pat->type) {
                case UPTSet:
                    if((pat->content.Set.elements) && (++pat->content.Set.ptr_s == pat->content.Set.size)) {
                        pat->content.Set.ptr_s = 0;
                        carry = TRUE;
                    }
                    break;
                case UPTCharRange:
                    pat->content.CharRange.ptr_c = (char)(pat->content.CharRange.step + (int)((unsigned char)pat->content.CharRange.ptr_c));
                    if(pat->content.CharRange.ptr_c > pat->content.CharRange.max_c) {
                        pat->content.CharRange.ptr_c = pat->content.CharRange.min_c;
                        carry = TRUE;
                    }
                    break;
                case UPTNumRange:
                    pat->content.NumRange.ptr_n += pat->content.NumRange.step;
                    if(pat->content.NumRange.ptr_n > pat->content.NumRange.max_n) {
                        pat->content.NumRange.ptr_n = pat->content.NumRange.min_n;
                        carry = TRUE;
                    }
                    break;
                default:
                    printf("internal error: invalid pattern type (%d)\n", (int)pat->type);
                    return CURLE_FAILED_INIT;
            }
        }
        if(carry) return CURLE_OK;
    }
    for(i = 0; i < glob->size; ++i) {
        pat = &glob->pattern[i];
        switch(pat->type) {
            case UPTSet:
                if(pat->content.Set.elements) {
                    msnprintf(buf, buflen, "%s", pat->content.Set.elements[pat->content.Set.ptr_s]);
                    len = strlen(buf);
                    buf += len;
                    buflen -= len;
                }
                break;
            case UPTCharRange:
                if(buflen) {
                    *buf++ = pat->content.CharRange.ptr_c;
                    *buf = '\0';
                    buflen--;
                }
                break;
            case UPTNumRange:
                msnprintf(buf, buflen, "%0*lu", pat->content.NumRange.padlength, pat->content.NumRange.ptr_n);
                len = strlen(buf);
                buf += len;
                buflen -= len;
                break;
            default:
                printf("internal error: invalid pattern type (%d)\n", (int)pat->type);
                return CURLE_FAILED_INIT;
        }
    }
    *globbed = strdup(glob->glob_buffer);
    if(!*globbed) return CURLE_OUT_OF_MEMORY;
    return CURLE_OK;
}
CURLcode glob_match_url(char **result, char *filename, struct URLGlob *glob) {
    char *target;
    size_t allocsize;
    char numbuf[18];
    char *appendthis = (char*)"";
    size_t appendlen = 0;
    size_t stringlen = 0;
    *result = NULL;
    allocsize = strlen(filename) + 1;
    target = malloc(allocsize);
    if(!target) return CURLE_OUT_OF_MEMORY;
    while(*filename) {
        if(*filename == '#' && ISDIGIT(filename[1])) {
            char *ptr = filename;
            unsigned long num = strtoul(&filename[1], &filename, 10);
            struct URLPattern *pat = NULL;
            if(num && (num < glob->size)) {
                unsigned long i;
                num--;
                for(i = 0; i<glob->size; i++) {
                    if(glob->pattern[i].globindex == (int)num) {
                        pat = &glob->pattern[i];
                        break;
                    }
                }
            }
            if(pat) {
                switch(pat->type) {
                    case UPTSet:
                        if(pat->content.Set.elements) {
                            appendthis = pat->content.Set.elements[pat->content.Set.ptr_s];
                            appendlen = strlen(pat->content.Set.elements[pat->content.Set.ptr_s]);
                        }
                        break;
                    case UPTCharRange:
                        numbuf[0] = pat->content.CharRange.ptr_c;
                        numbuf[1] = 0;
                        appendthis = numbuf;
                        appendlen = 1;
                        break;
                    case UPTNumRange:
                        msnprintf(numbuf, sizeof(numbuf), "%0*lu", pat->content.NumRange.padlength, pat->content.NumRange.ptr_n);
                        appendthis = numbuf;
                        appendlen = strlen(numbuf);
                        break;
                    default:
                        fprintf(stderr, "internal error: invalid pattern type (%d)\n", (int)pat->type);
                        Curl_safefree(target);
                        return CURLE_FAILED_INIT;
                }
            } else {
                filename = ptr;
                appendthis = filename++;
                appendlen = 1;
            }
        } else {
            appendthis = filename++;
            appendlen = 1;
        }
        if(appendlen + stringlen >= allocsize) {
            char *newstr;
            allocsize = (appendlen + stringlen) * 2;
            newstr = realloc(target, allocsize + 1);
            if(!newstr) {
                Curl_safefree(target);
                return CURLE_OUT_OF_MEMORY;
            }
            target = newstr;
        }
        memcpy(&target[stringlen], appendthis, appendlen);
        stringlen += appendlen;
    }
    target[stringlen]= '\0';
#if defined(MSDOS) || defined(WIN32)
    char *sanitized;
    SANITIZEcode sc = sanitize_file_name(&sanitized, target, (SANITIZE_ALLOW_PATH | SANITIZE_ALLOW_RESERVED));
    Curl_safefree(target);
    if(sc) return CURLE_URL_MALFORMAT;
    target = sanitized;
#endif
    *result = target;
    return CURLE_OK;
}