#include "tool_setup.h"
#if defined(MSDOS) || defined(WIN32)
#if defined(HAVE_LIBGEN_H) && defined(HAVE_BASENAME)
#include <libgen.h>
#endif
#ifdef WIN32
#include <tlhelp32.h>
#include "tool_cfgable.h"
#include "tool_libinfo.h"
#endif
#include "tool_bname.h"
#include "tool_doswin.h"
#include "memdebug.h"

#ifdef WIN32
#undef  PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#ifndef S_ISCHR
#ifdef S_IFCHR
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#else
#define S_ISCHR(m) (0)
#endif
#endif
#ifdef WIN32
#define _use_lfn(f) (1)
#elif !defined(__DJGPP__) || (__DJGPP__ < 2)
#define _use_lfn(f) (0)
#elif defined(__DJGPP__)
#include <fcntl.h>
#endif

#ifndef UNITTESTS
static SANITIZEcode truncate_dryrun(const char *path, const size_t truncate_pos);
#ifdef MSDOS
static SANITIZEcode msdosify(char **const sanitized, const char *file_name, int flags);
#endif
static SANITIZEcode rename_if_reserved_dos_device_name(char **const sanitized, const char *file_name, int flags);
#endif
SANITIZEcode sanitize_file_name(char **const sanitized, const char *file_name, int flags) {
    char *p, *target;
    size_t len;
    SANITIZEcode sc;
    size_t max_sanitized_len;
    if(!sanitized) return SANITIZE_ERR_BAD_ARGUMENT;
    *sanitized = NULL;
    if(!file_name) return SANITIZE_ERR_BAD_ARGUMENT;
    if((flags & SANITIZE_ALLOW_PATH)) {
    #ifndef MSDOS
        if(file_name[0] == '\\' && file_name[1] == '\\') max_sanitized_len = 32767-1;
        else
    #endif
        max_sanitized_len = PATH_MAX-1;
    } else max_sanitized_len = (PATH_MAX-1 > 255) ? 255 : PATH_MAX-1;
    len = strlen(file_name);
    if(len > max_sanitized_len) {
        if(!(flags & SANITIZE_ALLOW_TRUNCATE) || truncate_dryrun(file_name, max_sanitized_len)) return SANITIZE_ERR_INVALID_PATH;
        len = max_sanitized_len;
    }
    target = malloc(len + 1);
    if(!target) return SANITIZE_ERR_OUT_OF_MEMORY;
    strncpy(target, file_name, len);
    target[len] = '\0';
#ifndef MSDOS
    if((flags & SANITIZE_ALLOW_PATH) && !strncmp(target, "\\\\?\\", 4)) p = target + 4;
    else
#endif
    p = target;
    for( ; *p; ++p) {
        const char *banned;
        if((1 <= *p && *p <= 31) || (!(flags & (SANITIZE_ALLOW_COLONS|SANITIZE_ALLOW_PATH)) && *p == ':') || (!(flags & SANITIZE_ALLOW_PATH) &&
            (*p == '/' || *p == '\\'))) {
            *p = '_';
            continue;
        }
        for(banned = "|<>\"?*"; *banned; ++banned) {
            if(*p == *banned) {
                *p = '_';
                break;
            }
        }
    }
    if(!(flags & SANITIZE_ALLOW_PATH) && len) {
        char *clip = NULL;
        p = &target[len];
        do {
            --p;
            if(*p != ' ' && *p != '.') break;
            clip = p;
        } while(p != target);
        if(clip) {
            *clip = '\0';
            len = clip - target;
        }
    }
#ifdef MSDOS
    sc = msdosify(&p, target, flags);
    free(target);
    if(sc) return sc;
    target = p;
    len = strlen(target);
    if(len > max_sanitized_len) {
        free(target);
        return SANITIZE_ERR_INVALID_PATH;
    }
#endif
    if(!(flags & SANITIZE_ALLOW_RESERVED)) {
        sc = rename_if_reserved_dos_device_name(&p, target, flags);
        free(target);
        if(sc) return sc;
        target = p;
        len = strlen(target);
        if(len > max_sanitized_len) {
            free(target);
            return SANITIZE_ERR_INVALID_PATH;
        }
    }
    *sanitized = target;
    return SANITIZE_ERR_OK;
}
SANITIZEcode truncate_dryrun(const char *path, const size_t truncate_pos) {
    size_t len;
    if(!path) return SANITIZE_ERR_BAD_ARGUMENT;
    len = strlen(path);
    if(truncate_pos > len) return SANITIZE_ERR_BAD_ARGUMENT;
    if(!len || !truncate_pos) return SANITIZE_ERR_INVALID_PATH;
    if(strpbrk(&path[truncate_pos - 1], "\\/:")) return SANITIZE_ERR_INVALID_PATH;
    if(truncate_pos > 1) {
        const char *p = &path[truncate_pos - 1];
        do {
            --p;
            if(*p == ':') return SANITIZE_ERR_INVALID_PATH;
        } while(p != path && *p != '\\' && *p != '/');
    }
    return SANITIZE_ERR_OK;
}
#if defined(MSDOS) || defined(UNITTESTS)
SANITIZEcode msdosify(char **const sanitized, const char *file_name, int flags) {
    char dos_name[PATH_MAX];
    static const char illegal_chars_dos[] = ".+, ;=[]|<>/\\\":?*";
    static const char *illegal_chars_w95 = &illegal_chars_dos[8];
    int idx, dot_idx;
    const char *s = file_name;
    char *d = dos_name;
    const char *const dlimit = dos_name + sizeof(dos_name) - 1;
    const char *illegal_aliens = illegal_chars_dos;
    size_t len = sizeof(illegal_chars_dos) - 1;
    if(!sanitized) return SANITIZE_ERR_BAD_ARGUMENT;
    *sanitized = NULL;
    if(!file_name) return SANITIZE_ERR_BAD_ARGUMENT;
    if(strlen(file_name) > PATH_MAX-1 && (!(flags & SANITIZE_ALLOW_TRUNCATE) || truncate_dryrun(file_name, PATH_MAX-1))) return SANITIZE_ERR_INVALID_PATH;
    if(_use_lfn(file_name)) {
        illegal_aliens = illegal_chars_w95;
        len -= (illegal_chars_w95 - illegal_chars_dos);
    }
    if(s[0] >= 'A' && s[0] <= 'z' && s[1] == ':') {
        *d++ = *s++;
        *d = ((flags & (SANITIZE_ALLOW_COLONS|SANITIZE_ALLOW_PATH))) ? ':' : '_';
        ++d, ++s;
    }
    for(idx = 0, dot_idx = -1; *s && d < dlimit; s++, d++) {
        if(memchr(illegal_aliens, *s, len)) {
            if((flags & (SANITIZE_ALLOW_COLONS|SANITIZE_ALLOW_PATH)) && *s == ':') *d = ':';
            else if((flags & SANITIZE_ALLOW_PATH) && (*s == '/' || *s == '\\')) *d = *s;
            else if(*s == '.') {
                if((flags & SANITIZE_ALLOW_PATH) && idx == 0 && (s[1] == '/' || s[1] == '\\' || (s[1] == '.' && (s[2] == '/' || s[2] == '\\')))) {
                    *d++ = *s++;
                    if(d == dlimit) break;
                    if(*s == '.') {
                        *d++ = *s++;
                        if(d == dlimit) break;
                    }
                    *d = *s;
                } else if(idx == 0) *d = '_';
                else if(dot_idx >= 0) {
                    if(dot_idx < 5) {
                        d[dot_idx - idx] = '_';
                        *d = '.';
                    } else *d = '-';
                } else *d = '.';
                if(*s == '.') dot_idx = idx;
            } else if(*s == '+' && s[1] == '+') {
                if(idx - 2 == dot_idx) {
                    *d++ = 'x';
                    if(d == dlimit) break;
                    *d   = 'x';
                } else {
                    if(dlimit - d < 4) {
                        *d++ = 'x';
                        if(d == dlimit) break;
                        *d   = 'x';
                    } else {
                        memcpy(d, "plus", 4);
                        d += 3;
                    }
                }
                s++;
                idx++;
            } else *d = '_';
        } else *d = *s;
        if(*s == '/' || *s == '\\') {
            idx = 0;
            dot_idx = -1;
        } else idx++;
    }
    *d = '\0';
    if(*s) {
        if(!(flags & SANITIZE_ALLOW_TRUNCATE) || strpbrk(s, "\\/:") || truncate_dryrun(dos_name, d - dos_name)) return SANITIZE_ERR_INVALID_PATH;
    }
    *sanitized = strdup(dos_name);
    return (*sanitized ? SANITIZE_ERR_OK : SANITIZE_ERR_OUT_OF_MEMORY);
}
#endif
SANITIZEcode rename_if_reserved_dos_device_name(char **const sanitized, const char *file_name, int flags) {
    char *p, *base;
    char fname[PATH_MAX];
#ifdef MSDOS
    struct_stat st_buf;
#endif
    if(!sanitized) return SANITIZE_ERR_BAD_ARGUMENT;
    *sanitized = NULL;
    if(!file_name) return SANITIZE_ERR_BAD_ARGUMENT;
#ifndef MSDOS
    if((flags & SANITIZE_ALLOW_PATH) && file_name[0] == '\\' && file_name[1] == '\\') {
        size_t len = strlen(file_name);
        *sanitized = malloc(len + 1);
        if(!*sanitized) return SANITIZE_ERR_OUT_OF_MEMORY;
        strncpy(*sanitized, file_name, len + 1);
        return SANITIZE_ERR_OK;
    }
#endif
    if(strlen(file_name) > PATH_MAX-1 && (!(flags & SANITIZE_ALLOW_TRUNCATE) || truncate_dryrun(file_name, PATH_MAX-1))) return SANITIZE_ERR_INVALID_PATH;
    strncpy(fname, file_name, PATH_MAX-1);
    fname[PATH_MAX-1] = '\0';
    base = basename(fname);
    for(p = fname; p; p = (p == fname && fname != base ? base : NULL)) {
        size_t p_len;
        int x = (curl_strnequal(p, "CON", 3) || curl_strnequal(p, "PRN", 3) || curl_strnequal(p, "AUX", 3) || curl_strnequal(p, "NUL", 3)) ? 3 :
                (curl_strnequal(p, "CLOCK$", 6)) ? 6 : (curl_strnequal(p, "COM", 3) || curl_strnequal(p, "LPT", 3)) ?
                (('1' <= p[3] && p[3] <= '9') ? 4 : 3) : 0;
        if(!x) continue;
        for(; p[x] == ' '; ++x);
        if(p[x] == '.') {
            p[x] = '_';
            continue;
        } else if(p[x] == ':') {
            if(!(flags & (SANITIZE_ALLOW_COLONS|SANITIZE_ALLOW_PATH))) {
                p[x] = '_';
                continue;
            }
            ++x;
        } else if(p[x])  continue;
        p_len = strlen(p);
        if(strlen(fname) == PATH_MAX-1) {
            --p_len;
            if(!(flags & SANITIZE_ALLOW_TRUNCATE) || truncate_dryrun(p, p_len)) return SANITIZE_ERR_INVALID_PATH;
            p[p_len] = '\0';
        }
        memmove(p + 1, p, p_len + 1);
        p[0] = '_';
        ++p_len;
        if(p == fname) base = basename(fname);
    }
#ifdef MSDOS
    if(base && ((stat(base, &st_buf)) == 0) && (S_ISCHR(st_buf.st_mode))) {
        size_t blen = strlen(base);
        if(blen) {
            if(strlen(fname) == PATH_MAX-1) {
                --blen;
                if(!(flags & SANITIZE_ALLOW_TRUNCATE) || truncate_dryrun(base, blen)) return SANITIZE_ERR_INVALID_PATH;
                base[blen] = '\0';
            }
            memmove(base + 1, base, blen + 1);
            base[0] = '_';
        }
    }
#endif
    *sanitized = strdup(fname);
    return (*sanitized ? SANITIZE_ERR_OK : SANITIZE_ERR_OUT_OF_MEMORY);
}
#if defined(MSDOS) && (defined(__DJGPP__) || defined(__GO32__))
char **__crt0_glob_function(char *arg) {
    (void)arg;
    return (char **)0;
}
#endif /* MSDOS && (__DJGPP__ || __GO32__) */
#ifdef WIN32
CURLcode FindWin32CACert(struct OperationConfig *config, curl_sslbackend backend, const char *bundle_file) {
    CURLcode result = CURLE_OK;
    if((curlinfo->features & CURL_VERSION_SSL) && backend != CURLSSLBACKEND_SCHANNEL) {
        DWORD res_len;
        char buf[PATH_MAX];
        char *ptr = NULL;
        buf[0] = '\0';
        res_len = SearchPathA(NULL, bundle_file, NULL, PATH_MAX, buf, &ptr);
        if(res_len > 0) {
            Curl_safefree(config->cacert);
            config->cacert = strdup(buf);
            if(!config->cacert) result = CURLE_OUT_OF_MEMORY;
        }
    }
    return result;
}
struct curl_slist *GetLoadedModulePaths(void) {
    HANDLE hnd = INVALID_HANDLE_VALUE;
    MODULEENTRY32 mod = {0};
    struct curl_slist *slist = NULL;
    mod.dwSize = sizeof(MODULEENTRY32);
    do {
        hnd = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
    } while(hnd == INVALID_HANDLE_VALUE && GetLastError() == ERROR_BAD_LENGTH);
    if(hnd == INVALID_HANDLE_VALUE) goto error;
    if(!Module32First(hnd, &mod)) goto error;
    do {
        char *path;
        struct curl_slist *temp;
    #ifdef UNICODE
        char buffer[sizeof(mod.szExePath) * 2];
        if(!WideCharToMultiByte(CP_ACP, 0, mod.szExePath, -1, buffer, sizeof(buffer), NULL, NULL)) goto error;
        path = buffer;
    #else
        path = mod.szExePath;
    #endif
        temp = curl_slist_append(slist, path);
        if(!temp) goto error;
        slist = temp;
    } while(Module32Next(hnd, &mod));
    goto cleanup;
error:
    curl_slist_free_all(slist);
    slist = NULL;
cleanup:
    if(hnd != INVALID_HANDLE_VALUE) CloseHandle(hnd);
    return slist;
}
LARGE_INTEGER tool_freq;
bool tool_isVistaOrGreater;
CURLcode win32_init(void) {
    OSVERSIONINFOEXA osvi;
    unsigned __int64 mask = 0;
    unsigned char op = VER_GREATER_EQUAL;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = 6;
    VER_SET_CONDITION(mask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(mask, VER_MINORVERSION, op);
    if(VerifyVersionInfoA(&osvi, (VER_MAJORVERSION | VER_MINORVERSION), mask)) tool_isVistaOrGreater = true;
    else if(GetLastError() == ERROR_OLD_WIN_VERSION) tool_isVistaOrGreater = false;
    else return CURLE_FAILED_INIT;
    QueryPerformanceFrequency(&tool_freq);
    return CURLE_OK;
}
#endif /* WIN32 */
#endif