#include <stdbool.h>
#include <errno.h>
#include "tool_setup.h"
#include "strcase.h"
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "tool_cfgable.h"
#include "tool_getparam.h"
#include "tool_getpass.h"
#include "tool_homedir.h"
#include "tool_msgs.h"
#include "tool_paramhlp.h"
#include "tool_version.h"
#include "memdebug.h"

struct getout *new_getout(struct OperationConfig *config) {
    struct getout *node = calloc(1, sizeof(struct getout));
    struct getout *last = config->url_last;
    if(node) {
        if(last) last->next = node;
        else config->url_list = node;
        config->url_last = node;
        node->flags = config->default_node_flags;
    }
    return node;
}
ParameterError file2string(char **bufp, FILE *file) {
    char *string = NULL;
    if(file) {
        char *ptr;
        size_t alloc = 512;
        size_t alloc_needed;
        char buffer[256];
        size_t stringlen = 0;
        string = calloc(1, alloc);
        if(!string) return PARAM_NO_MEM;
        while(fgets(buffer, sizeof(buffer), file)) {
            size_t buflen;
            ptr = strchr(buffer, '\r');
            if(ptr) *ptr = '\0';
            ptr = strchr(buffer, '\n');
            if(ptr) *ptr = '\0';
            buflen = strlen(buffer);
            alloc_needed = stringlen + buflen + 1;
            if(alloc < alloc_needed) {
                #if SIZEOF_SIZE_T < 8
                    if(alloc >= (size_t)SIZE_T_MAX/2) {
                      Curl_safefree(string);
                      return PARAM_NO_MEM;
                    }
                #endif
                alloc *= 2;
                ptr = realloc(string, alloc);
                if(!ptr) {
                    Curl_safefree(string);
                    return PARAM_NO_MEM;
                }
                string = ptr;
            }
            strcpy(string + stringlen, buffer);
            stringlen += buflen;
        }
    }
    *bufp = string;
    return PARAM_OK;
}
ParameterError file2memory(char **bufp, size_t *size, FILE *file) {
    char *newbuf;
    char *buffer = NULL;
    size_t nused = 0;
    if(file) {
        size_t nread;
        size_t alloc = 512;
        do {
            if(!buffer || (alloc == nused)) {
                if(alloc + 1 > ((size_t)-1)/2) {
                    Curl_safefree(buffer);
                    return PARAM_NO_MEM;
                }
                alloc *= 2;
                newbuf = realloc(buffer, alloc + 1);
                if(!newbuf) {
                    Curl_safefree(buffer);
                    return PARAM_NO_MEM;
                }
                buffer = newbuf;
            }
            nread = fread(buffer + nused, 1, alloc-nused, file);
            nused += nread;
        } while(nread);
        buffer[nused] = '\0';
        if(alloc != nused) {
            newbuf = realloc(buffer, nused + 1);
            if(!newbuf) {
                Curl_safefree(buffer);
                return PARAM_NO_MEM;
            }
            buffer = newbuf;
        }
        if(!nused) Curl_safefree(buffer);
    }
    *size = nused;
    *bufp = buffer;
    return PARAM_OK;
}
void cleanarg(char *str) {
#ifdef HAVE_WRITABLE_ARGV
    if(str) {
        size_t len = strlen(str);
        memset(str, ' ', len);
    }
#else
    (void)str;
#endif
}
ParameterError str2num(long *val, const char *str) {
    if(str) {
        char *endptr = NULL;
        long num;
        errno = 0;
        num = strtol(str, &endptr, 10);
        if(errno == ERANGE) return PARAM_NUMBER_TOO_LARGE;
        if((endptr != str) && (endptr == str + strlen(str))) {
            *val = num;
            return PARAM_OK;
        }
    }
    return PARAM_BAD_NUMERIC;
}
ParameterError str2unum(long *val, const char *str) {
    ParameterError result = str2num(val, str);
    if(result != PARAM_OK) return result;
    if(*val < 0) return PARAM_NEGATIVE_NUMERIC;
    return PARAM_OK;
}
ParameterError str2unummax(long *val, const char *str, long max) {
    ParameterError result = str2unum(val, str);
    if(result != PARAM_OK) return result;
    if(*val > max) return PARAM_NUMBER_TOO_LARGE;
    return PARAM_OK;
}
static ParameterError str2double(double *val, const char *str, long max) {
    if(str) {
        char *endptr;
        double num;
        errno = 0;
        num = strtod(str, &endptr);
        if(errno == ERANGE) return PARAM_NUMBER_TOO_LARGE;
        if(num > max) return PARAM_NUMBER_TOO_LARGE;
        if((endptr != str) && (endptr == str + strlen(str))) {
            *val = num;
            return PARAM_OK;
        }
    }
    return PARAM_BAD_NUMERIC;
}
ParameterError str2udouble(double *valp, const char *str, long max) {
    double value;
    ParameterError result = str2double(&value, str, max);
    if(result != PARAM_OK) return result;
    if(value < 0) return PARAM_NEGATIVE_NUMERIC;
    *valp = value;
    return PARAM_OK;
}
long proto2num(struct OperationConfig *config, long *val, const char *str) {
    char *buffer;
    const char *sep = ",";
    char *token;
    static struct sprotos {
        const char *name;
        long bit;
    } const protos[] = {
        { "all", CURLPROTO_ALL },
        { "http", CURLPROTO_HTTP },
        { "https", CURLPROTO_HTTPS },
        { "ftp", CURLPROTO_FTP },
        { "ftps", CURLPROTO_FTPS },
        { "scp", CURLPROTO_SCP },
        { "sftp", CURLPROTO_SFTP },
        { "telnet", CURLPROTO_TELNET },
        { "ldap", CURLPROTO_LDAP },
        { "ldaps", CURLPROTO_LDAPS },
        { "dict", CURLPROTO_DICT },
        { "file", CURLPROTO_FILE },
        { "tftp", CURLPROTO_TFTP },
        { "imap", CURLPROTO_IMAP },
        { "imaps", CURLPROTO_IMAPS },
        { "pop3", CURLPROTO_POP3 },
        { "pop3s", CURLPROTO_POP3S },
        { "smtp", CURLPROTO_SMTP },
        { "smtps", CURLPROTO_SMTPS },
        { "rtsp", CURLPROTO_RTSP },
        { "gopher", CURLPROTO_GOPHER },
        { "smb", CURLPROTO_SMB },
        { "smbs", CURLPROTO_SMBS },
        { NULL, 0 }
    };
    if(!str) return 1;
    buffer = strdup(str);
    if(!buffer) return 1;
    for(token = strtok(buffer, sep);
    token;
    token = strtok(NULL, sep)) {
        enum e_action { allow, deny, set } action = allow;
        struct sprotos const *pp;
        while(!ISALNUM(*token)) {
            switch (*token++) {
                case '=': action = set; break;
                case '-': action = deny; break;
                case '+': action = allow; break;
                default: Curl_safefree(buffer); return 1;
            }
        }
        for(pp = protos; pp->name; pp++) {
            if(curl_strequal(token, pp->name)) {
                switch(action) {
                  case deny: *val &= ~(pp->bit); break;
                  case allow: *val |= pp->bit; break;
                  case set: *val = pp->bit; break;
                }
                break;
            }
        }
        if(!(pp->name)) {
            if(action == set) *val = 0;
            warnf(config->global, "unrecognized protocol '%s'\n", token);
        }
    }
    Curl_safefree(buffer);
    return 0;
}
int check_protocol(const char *str) {
    const char * const *pp;
    const curl_version_info_data *curlinfo = curl_version_info(CURLVERSION_NOW);
    if(!str) return PARAM_REQUIRES_PARAMETER;
    for(pp = curlinfo->protocols; *pp; pp++) {
        if(curl_strequal(*pp, str)) return PARAM_OK;
    }
    return PARAM_LIBCURL_UNSUPPORTED_PROTOCOL;
}
ParameterError str2offset(curl_off_t *val, const char *str) {
    char *endptr;
    if(str[0] == '-') return PARAM_NEGATIVE_NUMERIC;
#if(SIZEOF_CURL_OFF_T > SIZEOF_LONG)
    CURLofft offt = curlx_strtoofft(str, &endptr, 0, val);
    if(CURL_OFFT_FLOW == offt) return PARAM_NUMBER_TOO_LARGE;
    else if(CURL_OFFT_INVAL == offt) return PARAM_BAD_NUMERIC;
#else
    errno = 0;
    *val = strtol(str, &endptr, 0);
    if((*val == LONG_MIN || *val == LONG_MAX) && errno == ERANGE) return PARAM_NUMBER_TOO_LARGE;
#endif
    if((endptr != str) && (endptr == str + strlen(str))) return PARAM_OK;
    return PARAM_BAD_NUMERIC;
}
static CURLcode checkpasswd(const char *kind, const size_t i, const bool last, char **userpwd) {
    char *psep;
    char *osep;
    if(!*userpwd) return CURLE_OK;
    psep = strchr(*userpwd, ':');
    osep = strchr(*userpwd, ';');
    if(!psep && **userpwd != ';') {
        char passwd[256] = "";
        char prompt[256];
        size_t passwdlen;
        size_t userlen = strlen(*userpwd);
        char *passptr;
        if(osep) *osep = '\0';
        if(!i && last) curlx_msnprintf(prompt, sizeof(prompt), "Enter %s password for user '%s':", kind, *userpwd);
        else curlx_msnprintf(prompt, sizeof(prompt), "Enter %s password for user '%s' on URL #%zu:", kind, *userpwd, i + 1);
        getpass_r(prompt, passwd, sizeof(passwd));
        passwdlen = strlen(passwd);
        if(osep) *osep = ';';
        passptr = realloc(*userpwd,passwdlen + 1 + userlen + 1);
        if(!passptr) return CURLE_OUT_OF_MEMORY;
        passptr[userlen] = ':';
        memcpy(&passptr[userlen + 1], passwd, passwdlen + 1);
        *userpwd = passptr;
    }
    return CURLE_OK;
}
ParameterError add2list(struct curl_slist **list, const char *ptr) {
    struct curl_slist *newlist = curl_slist_append(*list, ptr);
    if(newlist) *list = newlist;
    else return PARAM_NO_MEM;
    return PARAM_OK;
}
int ftpfilemethod(struct OperationConfig *config, const char *str) {
    if(curl_strequal("singlecwd", str)) return CURLFTPMETHOD_SINGLECWD;
    if(curl_strequal("nocwd", str)) return CURLFTPMETHOD_NOCWD;
    if(curl_strequal("multicwd", str)) return CURLFTPMETHOD_MULTICWD;
    warnf(config->global, "unrecognized ftp file method '%s', using default\n", str);
    return CURLFTPMETHOD_MULTICWD;
}
int ftpcccmethod(struct OperationConfig *config, const char *str) {
    if(curl_strequal("passive", str)) return CURLFTPSSL_CCC_PASSIVE;
    if(curl_strequal("active", str)) return CURLFTPSSL_CCC_ACTIVE;
    warnf(config->global, "unrecognized ftp CCC method '%s', using default\n", str);
    return CURLFTPSSL_CCC_PASSIVE;
}
long delegation(struct OperationConfig *config, const char *str) {
    if(curl_strequal("none", str)) return CURLGSSAPI_DELEGATION_NONE;
    if(curl_strequal("policy", str)) return CURLGSSAPI_DELEGATION_POLICY_FLAG;
    if(curl_strequal("always", str)) return CURLGSSAPI_DELEGATION_FLAG;
    warnf(config->global, "unrecognized delegation method '%s', using none\n", str);
    return CURLGSSAPI_DELEGATION_NONE;
}
static char *my_useragent(void) {
  return strdup(CURL_NAME "/" CURL_VERSION);
}
CURLcode get_args(struct OperationConfig *config, const size_t i) {
    CURLcode result = CURLE_OK;
    bool last = (config->next ? FALSE : TRUE);
    if(config->userpwd && !config->oauth_bearer) {
        result = checkpasswd("host", i, last, &config->userpwd);
        if(result) return result;
    }
    if(config->proxyuserpwd) {
        result = checkpasswd("proxy", i, last, &config->proxyuserpwd);
        if(result) return result;
    }
    if(!config->useragent) {
        config->useragent = my_useragent();
        if(!config->useragent) {
            errorf(config->global, "out of memory\n");
            result = CURLE_OUT_OF_MEMORY;
        }
    }
    return result;
}
ParameterError str2tls_max(long *val, const char *str) {
    static struct s_tls_max {
        const char *tls_max_str;
        long tls_max;
    } const tls_max_array[] = {
        { "default", CURL_SSLVERSION_MAX_DEFAULT },
        { "1.0",     CURL_SSLVERSION_MAX_TLSv1_0 },
        { "1.1",     CURL_SSLVERSION_MAX_TLSv1_1 },
        { "1.2",     CURL_SSLVERSION_MAX_TLSv1_2 },
        { "1.3",     CURL_SSLVERSION_MAX_TLSv1_3 }
    };
    size_t i = 0;
    if(!str) return PARAM_REQUIRES_PARAMETER;
    for(i = 0; i < sizeof(tls_max_array)/sizeof(tls_max_array[0]); i++) {
        if(!strcmp(str, tls_max_array[i].tls_max_str)) {
            *val = tls_max_array[i].tls_max;
            return PARAM_OK;
        }
    }
    return PARAM_BAD_USE;
}