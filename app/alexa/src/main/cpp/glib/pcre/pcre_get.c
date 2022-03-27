#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_get_stringnumber(const pcre *code, const char *stringname) {
    int rc;
    int entrysize;
    int top, bot;
    uschar *nametable;
    if ((rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMECOUNT, &top)) != 0) return rc;
    if (top <= 0) return PCRE_ERROR_NOSUBSTRING;
    if ((rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMEENTRYSIZE, &entrysize)) != 0) return rc;
    if ((rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMETABLE, &nametable)) != 0) return rc;
    bot = 0;
    while (top > bot)  {
        int mid = (top + bot) / 2;
        uschar *entry = nametable + entrysize*mid;
        int c = strcmp(stringname, (char *)(entry + 2));
        if (c == 0) return (entry[0] << 8) + entry[1];
        if (c > 0) bot = mid + 1; else top = mid;
    }
    return PCRE_ERROR_NOSUBSTRING;
}
PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_get_stringtable_entries(const pcre *code, const char *stringname, char **firstptr, char **lastptr) {
    int rc;
    int entrysize;
    int top, bot;
    uschar *nametable, *lastentry;
    if ((rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMECOUNT, &top)) != 0) return rc;
    if (top <= 0) return PCRE_ERROR_NOSUBSTRING;
    if ((rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMEENTRYSIZE, &entrysize)) != 0) return rc;
    if ((rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMETABLE, &nametable)) != 0) return rc;
    lastentry = nametable + entrysize * (top - 1);
    bot = 0;
    while (top > bot)  {
        int mid = (top + bot) / 2;
        uschar *entry = nametable + entrysize*mid;
        int c = strcmp(stringname, (char *)(entry + 2));
        if (c == 0) {
            uschar *first = entry;
            uschar *last = entry;
            while (first > nametable) {
                if (strcmp(stringname, (char *)(first - entrysize + 2)) != 0) break;
                first -= entrysize;
            }
            while (last < lastentry) {
                if (strcmp(stringname, (char *)(last + entrysize + 2)) != 0) break;
                last += entrysize;
            }
            *firstptr = (char *)first;
            *lastptr = (char *)last;
            return entrysize;
        }
        if (c > 0) bot = mid + 1; else top = mid;
    }
    return PCRE_ERROR_NOSUBSTRING;
}
#ifdef NOT_USED_IN_GLIB
static int get_first_set(const pcre *code, const char *stringname, int *ovector) {
    const real_pcre *re = (const real_pcre *)code;
    int entrysize;
    char *first, *last;
    uschar *entry;
    if ((re->options & PCRE_DUPNAMES) == 0 && (re->flags & PCRE_JCHANGED) == 0) return pcre_get_stringnumber(code, stringname);
    entrysize = pcre_get_stringtable_entries(code, stringname, &first, &last);
    if (entrysize <= 0) return entrysize;
    for (entry = (uschar *)first; entry <= (uschar *)last; entry += entrysize) {
        int n = (entry[0] << 8) + entry[1];
        if (ovector[n*2] >= 0) return n;
    }
    return (first[0] << 8) + first[1];
}
PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_copy_substring(const char *subject, int *ovector, int stringcount, int stringnumber, char *buffer, int size) {
    int yield;
    if (stringnumber < 0 || stringnumber >= stringcount) return PCRE_ERROR_NOSUBSTRING;
    stringnumber *= 2;
    yield = ovector[stringnumber+1] - ovector[stringnumber];
    if (size < yield + 1) return PCRE_ERROR_NOMEMORY;
    memcpy(buffer, subject + ovector[stringnumber], yield);
    buffer[yield] = 0;
    return yield;
}
PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_copy_named_substring(const pcre *code, const char *subject, int *ovector, int stringcount, const char *stringname,
                                                                 char *buffer, int size) {
    int n = get_first_set(code, stringname, ovector);
    if (n <= 0) return n;
    return pcre_copy_substring(subject, ovector, stringcount, n, buffer, size);
}
PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_get_substring_list(const char *subject, int *ovector, int stringcount, const char ***listptr) {
    int i;
    int size = sizeof(char *);
    int double_count = stringcount * 2;
    char **stringlist;
    char *p;
    for (i = 0; i < double_count; i += 2) size += sizeof(char *) + ovector[i+1] - ovector[i] + 1;
    stringlist = (char **)(pcre_malloc)(size);
    if (stringlist == NULL) return PCRE_ERROR_NOMEMORY;
    *listptr = (const char **)stringlist;
    p = (char *)(stringlist + stringcount + 1);
    for (i = 0; i < double_count; i += 2) {
        int len = ovector[i+1] - ovector[i];
        memcpy(p, subject + ovector[i], len);
        *stringlist++ = p;
        p += len;
        *p++ = 0;
    }
    *stringlist = NULL;
    return 0;
}
PCRE_EXP_DEFN void PCRE_CALL_CONVENTION pcre_free_substring_list(const char **pointer) {
    (pcre_free)((void *)pointer);
}
PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_get_substring(const char *subject, int *ovector, int stringcount, int stringnumber, const char **stringptr) {
    int yield;
    char *substring;
    if (stringnumber < 0 || stringnumber >= stringcount) return PCRE_ERROR_NOSUBSTRING;
    stringnumber *= 2;
    yield = ovector[stringnumber+1] - ovector[stringnumber];
    substring = (char *)(pcre_malloc)(yield + 1);
    if (substring == NULL) return PCRE_ERROR_NOMEMORY;
    memcpy(substring, subject + ovector[stringnumber], yield);
    substring[yield] = 0;
    *stringptr = substring;
    return yield;
}
PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_get_named_substring(const pcre *code, const char *subject, int *ovector, int stringcount, const char *stringname,
                                                                const char **stringptr) {
    int n = get_first_set(code, stringname, ovector);
    if (n <= 0) return n;
    return pcre_get_substring(subject, ovector, stringcount, n, stringptr);
}
PCRE_EXP_DEFN void PCRE_CALL_CONVENTION pcre_free_substring(const char *pointer) {
    (pcre_free)((void *)pointer);
}
#endif