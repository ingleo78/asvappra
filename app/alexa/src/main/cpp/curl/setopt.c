#include "curl_setup.h"
#include <limits.h>
#include <stdbool.h>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_LINUX_TCP_H
#include <linux/tcp.h>
#endif
#include "urldata.h"
#include "url.h"
#include "progress.h"
#include "content_encoding.h"
#include "strcase.h"
#include "share.h"
#include "vtls/vtls.h"
#include "warnless.h"
#include "sendf.h"
#include "http2.h"
#include "setopt.h"
#include "multiif.h"
#include "altsvc.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#include "setup-vms.h"

typedef struct Curl_URL CURLU;
CURLcode Curl_setstropt(char **charp, const char *s) {
    Curl_safefree(*charp);
    if(s) {
        char *str = strdup(s);
        if(str) {
            size_t len = strlen(str);
            if(len > CURL_MAX_INPUT_LENGTH) {
                free(str);
                return CURLE_BAD_FUNCTION_ARGUMENT;
            }
        }
        if(!str) return CURLE_OUT_OF_MEMORY;
        *charp = str;
    }
    return CURLE_OK;
}
CURLcode Curl_setblobopt(struct curl_blob **blobp, const struct curl_blob *blob) {
    Curl_safefree(*blobp);
    if(blob) {
        struct curl_blob *nblob;
        if(blob->len > CURL_MAX_INPUT_LENGTH) return CURLE_BAD_FUNCTION_ARGUMENT;
        nblob = (struct curl_blob *)malloc(sizeof(struct curl_blob) + ((blob->flags & CURL_BLOB_COPY) ? blob->len : 0));
        if(!nblob) return CURLE_OUT_OF_MEMORY;
        *nblob = *blob;
        if(blob->flags & CURL_BLOB_COPY) {
            nblob->data = (char *)nblob + sizeof(struct curl_blob);
            memcpy(nblob->data, blob->data, blob->len);
        }
        *blobp = nblob;
        return CURLE_OK;
    }
    return CURLE_OK;
}
static CURLcode setstropt_userpwd(char *option, char **userp, char **passwdp) {
    CURLcode result = CURLE_OK;
    char *user = NULL;
    char *passwd = NULL;
    if(option) result = Curl_parse_login_details(option, strlen(option),(userp ? &user : NULL),(passwdp ? &passwd : NULL),NULL);
    if(!result) {
        if(userp) {
            if(!user && option && option[0] == ':') {
                user = strdup("");
                if(!user) result = CURLE_OUT_OF_MEMORY;
            }
            Curl_safefree(*userp);
            *userp = user;
        }
        if(passwdp) {
            Curl_safefree(*passwdp);
            *passwdp = passwd;
        }
    }
    return result;
}
#define C_SSLVERSION_VALUE(x) (x & 0xffff)
#define C_SSLVERSION_MAX_VALUE(x) (x & 0xffff0000)
CURLcode Curl_vsetopt(struct Curl_easy *data, CURLoption option, va_list param) {
    char *argptr;
    CURLcode result = CURLE_OK;
    long arg;
    unsigned long uarg;
    curl_off_t bigsize;
    switch(option) {
        case CURLOPT_DNS_CACHE_TIMEOUT:
            arg = va_arg(param, long);
            if(arg < -1) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.dns_cache_timeout = arg;
            break;
        case CURLOPT_DNS_USE_GLOBAL_CACHE: break;
        case CURLOPT_SSL_CIPHER_LIST: result = Curl_setstropt(&data->set.str[STRING_SSL_CIPHER_LIST_ORIG], va_arg(param,char*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSL_CIPHER_LIST: result = Curl_setstropt(&data->set.str[STRING_SSL_CIPHER_LIST_PROXY], va_arg(param,char*)); break;
    #endif
        case CURLOPT_TLS13_CIPHERS:
            if(Curl_ssl_tls13_ciphersuites()) result = Curl_setstropt(&data->set.str[STRING_SSL_CIPHER13_LIST_ORIG], va_arg(param,char*));
            else return CURLE_NOT_BUILT_IN;
            break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_TLS13_CIPHERS:
            if(Curl_ssl_tls13_ciphersuites()) result = Curl_setstropt(&data->set.str[STRING_SSL_CIPHER13_LIST_PROXY], va_arg(param, char*));
            else return CURLE_NOT_BUILT_IN;
            break;
    #endif
        case CURLOPT_RANDOM_FILE: result = Curl_setstropt(&data->set.str[STRING_SSL_RANDOM_FILE], va_arg(param,char*)); break;
        case CURLOPT_EGDSOCKET: result = Curl_setstropt(&data->set.str[STRING_SSL_EGDSOCKET], va_arg(param,char*)); break;
        case CURLOPT_MAXCONNECTS:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.maxconnects = arg;
            break;
        case CURLOPT_FORBID_REUSE: data->set.reuse_forbid = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_FRESH_CONNECT: data->set.reuse_fresh = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_VERBOSE: data->set.verbose = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_HEADER: data->set.include_header = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_NOPROGRESS:
            data->set.hide_progress = (0 != va_arg(param, long)) ? TRUE : FALSE;
            if(data->set.hide_progress) data->progress.flags |= PGRS_HIDE;
            else data->progress.flags &= ~PGRS_HIDE;
            break;
        case CURLOPT_NOBODY:
            data->set.opt_no_body = (0 != va_arg(param, long)) ? TRUE : FALSE;
            if(data->set.opt_no_body) data->set.method = HTTPREQ_HEAD;
            break;
        case CURLOPT_FAILONERROR: data->set.http_fail_on_error = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_KEEP_SENDING_ON_ERROR: data->set.http_keep_sending_on_error = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_UPLOAD:
        case CURLOPT_PUT:
            data->set.upload = (0 != va_arg(param, long)) ? TRUE : FALSE;
            if(data->set.upload) {
              data->set.method = HTTPREQ_PUT;
              data->set.opt_no_body = FALSE;
            } else data->set.method = HTTPREQ_GET;
            break;
        case CURLOPT_REQUEST_TARGET: result = Curl_setstropt(&data->set.str[STRING_TARGET], va_arg(param, char*)); break;
        case CURLOPT_FILETIME: data->set.get_filetime = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_SERVER_RESPONSE_TIMEOUT:
            arg = va_arg(param, long);
            if((arg >= 0) && (arg <= (INT_MAX/1000))) data->set.server_response_timeout = arg * 1000;
            else return CURLE_BAD_FUNCTION_ARGUMENT;
            break;
    #ifndef CURL_DISABLE_TFTP
        case CURLOPT_TFTP_NO_OPTIONS:
            data->set.tftp_no_options = va_arg(param, long) != 0;
            break;
        case CURLOPT_TFTP_BLKSIZE:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.tftp_blksize = arg;
            break;
    #endif
    #ifndef CURL_DISABLE_NETRC
        case CURLOPT_NETRC:
            arg = va_arg(param, long);
            if((arg < CURL_NETRC_IGNORED) || (arg >= CURL_NETRC_LAST)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.use_netrc = (enum CURL_NETRC_OPTION)arg;
            break;
        case CURLOPT_NETRC_FILE: result = Curl_setstropt(&data->set.str[STRING_NETRC_FILE], va_arg(param,char*)); break;
    #endif
        case CURLOPT_TRANSFERTEXT: data->set.prefer_ascii = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_TIMECONDITION:
            arg = va_arg(param, long);
            if((arg < CURL_TIMECOND_NONE) || (arg >= CURL_TIMECOND_LAST)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.timecondition = (curl_TimeCond)arg;
            break;
        case CURLOPT_TIMEVALUE: data->set.timevalue = (time_t)va_arg(param, long); break;
        case CURLOPT_TIMEVALUE_LARGE: data->set.timevalue = (time_t)va_arg(param, curl_off_t); break;
        case CURLOPT_SSLVERSION:
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSLVERSION:
    #endif
        #ifdef USE_SSL
            {
                long version, version_max;
                struct ssl_primary_config *primary = &data->set.ssl.primary;
            #ifndef CURL_DISABLE_PROXY
                if(option != CURLOPT_SSLVERSION) primary = &data->set.proxy_ssl.primary;
            #endif
                arg = va_arg(param, long);
                version = C_SSLVERSION_VALUE(arg);
                version_max = C_SSLVERSION_MAX_VALUE(arg);
                if(version < CURL_SSLVERSION_DEFAULT || version >= CURL_SSLVERSION_LAST || version_max < CURL_SSLVERSION_MAX_NONE ||
                   version_max >= CURL_SSLVERSION_MAX_LAST)
                    return CURLE_BAD_FUNCTION_ARGUMENT;
                primary->version = version;
                primary->version_max = version_max;
            }
         #else
            result = CURLE_UNKNOWN_OPTION;
        #endif
            break;
    #ifndef CURL_DISABLE_HTTP
        case CURLOPT_AUTOREFERER: data->set.http_auto_referer = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_ACCEPT_ENCODING:
            argptr = va_arg(param, char*);
            if(argptr && !*argptr) {
                argptr = Curl_all_content_encodings();
                if(!argptr) result = CURLE_OUT_OF_MEMORY;
                else {
                    result = Curl_setstropt(&data->set.str[STRING_ENCODING], argptr);
                    free(argptr);
                }
            } else result = Curl_setstropt(&data->set.str[STRING_ENCODING], argptr);
            break;
        case CURLOPT_TRANSFER_ENCODING: data->set.http_transfer_encoding = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_FOLLOWLOCATION: data->set.http_follow_location = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_UNRESTRICTED_AUTH: data->set.allow_auth_to_other_hosts = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_MAXREDIRS:
            arg = va_arg(param, long);
            if(arg < -1) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.maxredirs = arg;
            break;
        case CURLOPT_POSTREDIR:
            arg = va_arg(param, long);
            if(arg < CURL_REDIR_GET_ALL) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.keep_post = arg & CURL_REDIR_POST_ALL;
            break;
        case CURLOPT_POST:
            if(va_arg(param, long)) {
                data->set.method = HTTPREQ_POST;
                data->set.opt_no_body = FALSE;
            } else data->set.method = HTTPREQ_GET;
            break;
        case CURLOPT_COPYPOSTFIELDS:
            argptr = va_arg(param, char *);
            if(!argptr || data->set.postfieldsize == -1) result = Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], argptr);
            else {
                if((data->set.postfieldsize < 0) || ((sizeof(curl_off_t) != sizeof(size_t)) && (data->set.postfieldsize > (curl_off_t)((size_t)-1)))) {
                    result = CURLE_OUT_OF_MEMORY;
                } else {
                    char *p;
                    (void) Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], NULL);
                    p = malloc((size_t)(data->set.postfieldsize ? data->set.postfieldsize : 1));
                    if(!p) result = CURLE_OUT_OF_MEMORY;
                    else {
                        if(data->set.postfieldsize) memcpy(p, argptr, (size_t)data->set.postfieldsize);
                        data->set.str[STRING_COPYPOSTFIELDS] = p;
                    }
                }
            }
            data->set.postfields = data->set.str[STRING_COPYPOSTFIELDS];
            data->set.method = HTTPREQ_POST;
            break;
        case CURLOPT_POSTFIELDS:
            data->set.postfields = va_arg(param, void *);
            (void)Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], NULL);
            data->set.method = HTTPREQ_POST;
            break;
        case CURLOPT_POSTFIELDSIZE:
            bigsize = va_arg(param, long);
            if(bigsize < -1) return CURLE_BAD_FUNCTION_ARGUMENT;
            if(data->set.postfieldsize < bigsize && data->set.postfields == data->set.str[STRING_COPYPOSTFIELDS]) {
                (void)Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], NULL);
                data->set.postfields = NULL;
            }
            data->set.postfieldsize = bigsize;
            break;
        case CURLOPT_POSTFIELDSIZE_LARGE:
            bigsize = va_arg(param, curl_off_t);
            if(bigsize < -1) return CURLE_BAD_FUNCTION_ARGUMENT;
            if(data->set.postfieldsize < bigsize && data->set.postfields == data->set.str[STRING_COPYPOSTFIELDS]) {
                (void)Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], NULL);
                data->set.postfields = NULL;
            }
            data->set.postfieldsize = bigsize;
            break;
        case CURLOPT_HTTPPOST:
            data->set.httppost = va_arg(param, struct curl_httppost *);
            data->set.method = HTTPREQ_POST_FORM;
            data->set.opt_no_body = FALSE;
            break;
    #endif
        case CURLOPT_MIMEPOST:
            result = Curl_mime_set_subparts(&data->set.mimepost, va_arg(param, curl_mime *), FALSE);
            if(!result) {
              data->set.method = HTTPREQ_POST_MIME;
              data->set.opt_no_body = FALSE;
            }
            break;
        case CURLOPT_REFERER:
            if(data->change.referer_alloc) {
              Curl_safefree(data->change.referer);
              data->change.referer_alloc = FALSE;
            }
            result = Curl_setstropt(&data->set.str[STRING_SET_REFERER], va_arg(param, char*));
            data->change.referer = data->set.str[STRING_SET_REFERER];
            break;
        case CURLOPT_USERAGENT: result = Curl_setstropt(&data->set.str[STRING_USERAGENT], va_arg(param, char*)); break;
        case CURLOPT_HTTPHEADER: data->set.headers = va_arg(param, struct curl_slist*); break;
    #ifndef CURL_DISABLE_HTTP
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXYHEADER: data->set.proxyheaders = va_arg(param, struct curl_slist*); break;
    #endif
        case CURLOPT_HEADEROPT:
            arg = va_arg(param, long);
            data->set.sep_headers = (bool)((arg & CURLHEADER_SEPARATE)? TRUE: FALSE);
            break;
        case CURLOPT_HTTP200ALIASES: data->set.http200aliases = va_arg(param, struct curl_slist*); break;
    #if !defined(CURL_DISABLE_COOKIES)
        case CURLOPT_COOKIE: result = Curl_setstropt(&data->set.str[STRING_COOKIE], va_arg(param, char*)); break;
        case CURLOPT_COOKIEFILE:
            argptr = (char *)va_arg(param, void *);
            if(argptr) {
                struct curl_slist *cl;
                cl = curl_slist_append(data->change.cookielist, argptr);
                if(!cl) {
                    curl_slist_free_all(data->change.cookielist);
                    data->change.cookielist = NULL;
                    return CURLE_OUT_OF_MEMORY;
                }
                data->change.cookielist = cl;
            }
            break;
        case CURLOPT_COOKIEJAR:
            {
                struct CookieInfo *newcookies;
                result = Curl_setstropt(&data->set.str[STRING_COOKIEJAR], va_arg(param, char*));
                newcookies = Curl_cookie_init(data, NULL, data->cookies, data->set.cookiesession);
                if(!newcookies) result = CURLE_OUT_OF_MEMORY;
                data->cookies = newcookies;
            }
            break;
        case CURLOPT_COOKIESESSION: data->set.cookiesession = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_COOKIELIST:
            argptr = va_arg(param, char*);
            if(argptr == NULL) break;
            if(strcasecompare(argptr, "ALL")) {
                Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
                Curl_cookie_clearall(data->cookies);
                Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
            } else if(strcasecompare(argptr, "SESS")) {
                Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
                Curl_cookie_clearsess(data->cookies);
                Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
            } else if(strcasecompare(argptr, "FLUSH")) Curl_flush_cookies(data, FALSE);
            else if(strcasecompare(argptr, "RELOAD")) {
                Curl_cookie_loadfiles(data);
                break;
            } else {
                if(!data->cookies) data->cookies = Curl_cookie_init(data, NULL, NULL, TRUE);
                argptr = strdup(argptr);
                if(!argptr || !data->cookies) {
                    result = CURLE_OUT_OF_MEMORY;
                    free(argptr);
                } else {
                    Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
                    if(checkprefix("Set-Cookie:", argptr)) {
                        Curl_cookie_add(data, data->cookies, TRUE, FALSE, argptr + 11, NULL, NULL, TRUE);
                    } else Curl_cookie_add(data, data->cookies,FALSE,FALSE, argptr,NULL,NULL,TRUE);
                    Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
                    free(argptr);
                }
            }
            break;
    #endif
        case CURLOPT_HTTPGET:
            if(va_arg(param, long)) {
                data->set.method = HTTPREQ_GET;
                data->set.upload = FALSE;
                data->set.opt_no_body = FALSE;
            }
            break;
        case CURLOPT_HTTP_VERSION:
            arg = va_arg(param, long);
            if(arg < CURL_HTTP_VERSION_NONE) return CURLE_BAD_FUNCTION_ARGUMENT;
        #ifdef ENABLE_QUIC
            if(arg == CURL_HTTP_VERSION_3);
            else
        #endif
        #ifndef USE_NGHTTP2
            if(arg >= CURL_HTTP_VERSION_2) return CURLE_UNSUPPORTED_PROTOCOL;
        #else
            if(arg >= CURL_HTTP_VERSION_LAST) return CURLE_UNSUPPORTED_PROTOCOL;
            if(arg == CURL_HTTP_VERSION_NONE) arg = CURL_HTTP_VERSION_2TLS;
        #endif
            data->set.httpversion = arg;
            break;
        case CURLOPT_EXPECT_100_TIMEOUT_MS:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.expect_100_timeout = arg;
            break;
        case CURLOPT_HTTP09_ALLOWED:
            arg = va_arg(param, unsigned long);
            if(arg > 1L) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.http09_allowed = arg ? TRUE : FALSE;
            break;
    #endif
        case CURLOPT_HTTPAUTH:
            {
                int bitcheck;
                bool authbits;
                unsigned long auth = va_arg(param, unsigned long);
                if(auth == CURLAUTH_NONE) {
                    data->set.httpauth = auth;
                    break;
                }
                data->state.authhost.iestyle = (bool)((auth & CURLAUTH_DIGEST_IE) ? TRUE : FALSE);
                if(auth & CURLAUTH_DIGEST_IE) {
                    auth |= CURLAUTH_DIGEST;
                    auth &= ~CURLAUTH_DIGEST_IE;
                }
            #ifndef USE_NTLM
                auth &= ~CURLAUTH_NTLM;
                auth &= ~CURLAUTH_NTLM_WB;
            #elif !defined(NTLM_WB_ENABLED)
                auth &= ~CURLAUTH_NTLM_WB;
            #endif
            #ifndef USE_SPNEGO
                auth &= ~CURLAUTH_NEGOTIATE;
            #endif
                bitcheck = 0;
                authbits = FALSE;
                while(bitcheck < 31) {
                    if(auth & (1UL << bitcheck++)) {
                        authbits = TRUE;
                        break;
                    }
                }
                if(!authbits) return CURLE_NOT_BUILT_IN;
                data->set.httpauth = auth;
            }
            break;
        case CURLOPT_CUSTOMREQUEST:
            result = Curl_setstropt(&data->set.str[STRING_CUSTOMREQUEST], va_arg(param, char*));
            break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_HTTPPROXYTUNNEL:
            data->set.tunnel_thru_httpproxy = (0 != va_arg(param, long)) ? TRUE : FALSE;
            break;
        case CURLOPT_PROXYPORT:
            arg = va_arg(param, long);
            if((arg < 0) || (arg > 65535)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.proxyport = arg;
            break;
        case CURLOPT_PROXYAUTH:
            {
                int bitcheck;
                bool authbits;
                unsigned long auth = va_arg(param, unsigned long);
                if(auth == CURLAUTH_NONE) {
                    data->set.proxyauth = auth;
                    break;
                }
                data->state.authproxy.iestyle = (bool)((auth & CURLAUTH_DIGEST_IE) ? TRUE : FALSE);
                if(auth & CURLAUTH_DIGEST_IE) {
                    auth |= CURLAUTH_DIGEST;
                    auth &= ~CURLAUTH_DIGEST_IE;
                }
            #ifndef USE_NTLM
                auth &= ~CURLAUTH_NTLM;
                auth &= ~CURLAUTH_NTLM_WB;
            #elif !defined(NTLM_WB_ENABLED)
                auth &= ~CURLAUTH_NTLM_WB; /* no NTLM_WB support */
            #endif
            #ifndef USE_SPNEGO
                auth &= ~CURLAUTH_NEGOTIATE;
            #endif
                bitcheck = 0;
                authbits = FALSE;
                while(bitcheck < 31) {
                    if(auth & (1UL << bitcheck++)) {
                        authbits = TRUE;
                        break;
                    }
                }
                if(!authbits) return CURLE_NOT_BUILT_IN;
                data->set.proxyauth = auth;
            }
            break;
        case CURLOPT_PROXY: result = Curl_setstropt(&data->set.str[STRING_PROXY], va_arg(param, char*)); break;
        case CURLOPT_PRE_PROXY: result = Curl_setstropt(&data->set.str[STRING_PRE_PROXY], va_arg(param, char*)); break;
        case CURLOPT_PROXYTYPE:
            arg = va_arg(param, long);
            if((arg < CURLPROXY_HTTP) || (arg > CURLPROXY_SOCKS5_HOSTNAME)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.proxytype = (curl_proxytype)arg;
            break;
        case CURLOPT_PROXY_TRANSFER_MODE:
            switch(va_arg(param, long)) {
                case 0: data->set.proxy_transfer_mode = FALSE; break;
                case 1: data->set.proxy_transfer_mode = TRUE; break;
                default: result = CURLE_UNKNOWN_OPTION; break;
            }
            break;
    #endif
        case CURLOPT_SOCKS5_AUTH:
            data->set.socks5auth = va_arg(param, unsigned long);
            if(data->set.socks5auth & ~(CURLAUTH_BASIC | CURLAUTH_GSSAPI)) result = CURLE_NOT_BUILT_IN;
            break;
    #if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
        case CURLOPT_SOCKS5_GSSAPI_NEC:
            data->set.socks5_gssapi_nec = (0 != va_arg(param, long)) ? TRUE : FALSE;
            break;
    #endif
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_SOCKS5_GSSAPI_SERVICE: case CURLOPT_PROXY_SERVICE_NAME:
            result = Curl_setstropt(&data->set.str[STRING_PROXY_SERVICE_NAME], va_arg(param, char*));
            break;
    #endif
        case CURLOPT_SERVICE_NAME: result = Curl_setstropt(&data->set.str[STRING_SERVICE_NAME], va_arg(param, char*)); break;
        case CURLOPT_HEADERDATA: data->set.writeheader = (void *)va_arg(param, void *); break;
        case CURLOPT_ERRORBUFFER: data->set.errorbuffer = va_arg(param, char *); break;
        case CURLOPT_WRITEDATA: data->set.out = va_arg(param, void *); break;
        case CURLOPT_DIRLISTONLY: data->set.ftp_list_only = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_APPEND: data->set.ftp_append = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
    #ifndef CURL_DISABLE_FTP
        case CURLOPT_FTP_FILEMETHOD:
            arg = va_arg(param, long);
            if((arg < CURLFTPMETHOD_DEFAULT) || (arg >= CURLFTPMETHOD_LAST)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.ftp_filemethod = (curl_ftpfile)arg;
            break;
        case CURLOPT_FTPPORT:
            result = Curl_setstropt(&data->set.str[STRING_FTPPORT], va_arg(param, char*));
            data->set.ftp_use_port = (data->set.str[STRING_FTPPORT]) ? TRUE : FALSE;
            break;
        case CURLOPT_FTP_USE_EPRT: data->set.ftp_use_eprt = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_FTP_USE_EPSV: data->set.ftp_use_epsv = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_FTP_USE_PRET: data->set.ftp_use_pret = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_FTP_SSL_CCC:
            arg = va_arg(param, long);
            if((arg < CURLFTPSSL_CCC_NONE) || (arg >= CURLFTPSSL_CCC_LAST)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.ftp_ccc = (curl_ftpccc)arg;
            break;
        case CURLOPT_FTP_SKIP_PASV_IP: data->set.ftp_skip_ip = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_FTP_ACCOUNT: result = Curl_setstropt(&data->set.str[STRING_FTP_ACCOUNT], va_arg(param, char*)); break;
        case CURLOPT_FTP_ALTERNATIVE_TO_USER: result = Curl_setstropt(&data->set.str[STRING_FTP_ALTERNATIVE_TO_USER], va_arg(param, char*)); break;
        case CURLOPT_FTPSSLAUTH:
            arg = va_arg(param, long);
            if((arg < CURLFTPAUTH_DEFAULT) || (arg >= CURLFTPAUTH_LAST)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.ftpsslauth = (curl_ftpauth)arg;
            break;
        case CURLOPT_KRBLEVEL:
            result = Curl_setstropt(&data->set.str[STRING_KRB_LEVEL], va_arg(param, char*));
            data->set.krb = (data->set.str[STRING_KRB_LEVEL]) ? TRUE : FALSE;
            break;
    #endif
        case CURLOPT_FTP_CREATE_MISSING_DIRS:
            switch(va_arg(param, long)) {
                case 0: data->set.ftp_create_missing_dirs = 0; break;
                case 1: data->set.ftp_create_missing_dirs = 1; break;
                case 2: data->set.ftp_create_missing_dirs = 2; break;
                default: result = CURLE_UNKNOWN_OPTION; break;
            }
            break;
        case CURLOPT_READDATA: data->set.in_set = va_arg(param, void*); break;
        case CURLOPT_INFILESIZE:
            arg = va_arg(param, long);
            if(arg < -1) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.filesize = arg;
            break;
        case CURLOPT_INFILESIZE_LARGE:
            bigsize = va_arg(param, curl_off_t);
            if(bigsize < -1) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.filesize = bigsize;
            break;
        case CURLOPT_LOW_SPEED_LIMIT:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.low_speed_limit = arg;
            break;
        case CURLOPT_MAX_SEND_SPEED_LARGE:
            bigsize = va_arg(param, curl_off_t);
            if(bigsize < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.max_send_speed = bigsize;
            break;
        case CURLOPT_MAX_RECV_SPEED_LARGE:
            bigsize = va_arg(param, curl_off_t);
            if(bigsize < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.max_recv_speed = bigsize;
            break;
        case CURLOPT_LOW_SPEED_TIME:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.low_speed_time = arg;
            break;
        case CURLOPT_CURLU:
            data->set.uh = va_arg(param, CURLU*);
            break;
        case CURLOPT_URL:
            if(data->change.url_alloc) {
                Curl_safefree(data->change.url);
                data->change.url_alloc = FALSE;
            }
            result = Curl_setstropt(&data->set.str[STRING_SET_URL], va_arg(param, char*));
            data->change.url = data->set.str[STRING_SET_URL];
            break;
        case CURLOPT_PORT:
            arg = va_arg(param, long);
            if((arg < 0) || (arg > 65535)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.use_port = arg;
            break;
        case CURLOPT_TIMEOUT:
            arg = va_arg(param, long);
            if((arg >= 0) && (arg <= (INT_MAX/1000))) data->set.timeout = arg * 1000;
            else return CURLE_BAD_FUNCTION_ARGUMENT;
            break;
        case CURLOPT_TIMEOUT_MS:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.timeout = arg;
            break;
        case CURLOPT_CONNECTTIMEOUT:
            arg = va_arg(param, long);
            if((arg >= 0) && (arg <= (INT_MAX/1000))) data->set.connecttimeout = arg * 1000;
            else return CURLE_BAD_FUNCTION_ARGUMENT;
            break;
        case CURLOPT_CONNECTTIMEOUT_MS:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.connecttimeout = arg;
            break;
        case CURLOPT_ACCEPTTIMEOUT_MS:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.accepttimeout = arg;
            break;
        case CURLOPT_USERPWD:
            result = setstropt_userpwd(va_arg(param, char *), &data->set.str[STRING_USERNAME], &data->set.str[STRING_PASSWORD]);
            break;
        case CURLOPT_USERNAME: result = Curl_setstropt(&data->set.str[STRING_USERNAME], va_arg(param, char*)); break;
        case CURLOPT_PASSWORD: result = Curl_setstropt(&data->set.str[STRING_PASSWORD], va_arg(param, char*)); break;
        case CURLOPT_LOGIN_OPTIONS: result = Curl_setstropt(&data->set.str[STRING_OPTIONS], va_arg(param, char*)); break;
        case CURLOPT_XOAUTH2_BEARER: result = Curl_setstropt(&data->set.str[STRING_BEARER], va_arg(param, char*)); break;
        case CURLOPT_POSTQUOTE: data->set.postquote = va_arg(param, struct curl_slist*); break;
        case CURLOPT_PREQUOTE: data->set.prequote = va_arg(param, struct curl_slist*); break;
        case CURLOPT_QUOTE: data->set.quote = va_arg(param, struct curl_slist*); break;
        case CURLOPT_RESOLVE:
            data->set.resolve = va_arg(param, struct curl_slist*);
            data->change.resolve = data->set.resolve;
            break;
        case CURLOPT_PROGRESSFUNCTION:
            data->set.fprogress = va_arg(param, curl_progress_callback);
            if(data->set.fprogress) data->progress.callback = TRUE;
            else data->progress.callback = FALSE;
            break;
        case CURLOPT_XFERINFOFUNCTION:
            data->set.fxferinfo = va_arg(param, curl_xferinfo_callback);
            if(data->set.fxferinfo) data->progress.callback = TRUE;
            else data->progress.callback = FALSE;
            break;
        case CURLOPT_PROGRESSDATA: data->set.progress_client = va_arg(param,void*); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXYUSERPWD:
            result = setstropt_userpwd(va_arg(param,char*), &data->set.str[STRING_PROXYUSERNAME], &data->set.str[STRING_PROXYPASSWORD]);
            break;
        case CURLOPT_PROXYUSERNAME: result = Curl_setstropt(&data->set.str[STRING_PROXYUSERNAME], va_arg(param,char*)); break;
        case CURLOPT_PROXYPASSWORD: result = Curl_setstropt(&data->set.str[STRING_PROXYPASSWORD], va_arg(param,char*)); break;
        case CURLOPT_NOPROXY: result = Curl_setstropt(&data->set.str[STRING_NOPROXY], va_arg(param,char*)); break;
    #endif
        case CURLOPT_RANGE: result = Curl_setstropt(&data->set.str[STRING_SET_RANGE], va_arg(param,char*)); break;
        case CURLOPT_RESUME_FROM:
            arg = va_arg(param, long);
            if(arg < -1) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.set_resume_from = arg;
            break;
        case CURLOPT_RESUME_FROM_LARGE:
            bigsize = va_arg(param, curl_off_t);
            if(bigsize < -1) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.set_resume_from = bigsize;
            break;
        case CURLOPT_DEBUGFUNCTION: data->set.fdebug = va_arg(param, curl_debug_callback); break;
        case CURLOPT_DEBUGDATA: data->set.debugdata = va_arg(param,void*); break;
        case CURLOPT_STDERR:
            data->set.err = va_arg(param, FILE *);
            if(!data->set.err) data->set.err = stderr;
            break;
        case CURLOPT_HEADERFUNCTION: data->set.fwrite_header = va_arg(param, curl_write_callback); break;
        case CURLOPT_WRITEFUNCTION:
            data->set.fwrite_func = va_arg(param, curl_write_callback);
            if(!data->set.fwrite_func) {
                data->set.is_fwrite_set = 0;
                data->set.fwrite_func = (curl_write_callback)fwrite;
            } else data->set.is_fwrite_set = 1;
            break;
        case CURLOPT_READFUNCTION:
            data->set.fread_func_set = va_arg(param, curl_read_callback);
            if(!data->set.fread_func_set) {
                data->set.is_fread_set = 0;
                data->set.fread_func_set = (curl_read_callback)fread;
            } else data->set.is_fread_set = 1;
            break;
        case CURLOPT_SEEKFUNCTION: data->set.seek_func = va_arg(param, curl_seek_callback); break;
        case CURLOPT_SEEKDATA: data->set.seek_client = va_arg(param,void*); break;
        case CURLOPT_CONV_FROM_NETWORK_FUNCTION: data->set.convfromnetwork = va_arg(param, curl_conv_callback); break;
        case CURLOPT_CONV_TO_NETWORK_FUNCTION: data->set.convtonetwork = va_arg(param, curl_conv_callback); break;
        case CURLOPT_CONV_FROM_UTF8_FUNCTION: data->set.convfromutf8 = va_arg(param, curl_conv_callback); break;
        case CURLOPT_IOCTLFUNCTION: data->set.ioctl_func = va_arg(param, curl_ioctl_callback); break;
        case CURLOPT_IOCTLDATA: data->set.ioctl_client = va_arg(param,void*); break;
        case CURLOPT_SSLCERT: result = Curl_setstropt(&data->set.str[STRING_CERT_ORIG], va_arg(param, char*)); break;
        case CURLOPT_SSLCERT_BLOB: result = Curl_setblobopt(&data->set.blobs[BLOB_CERT_ORIG], va_arg(param, struct curl_blob*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSLCERT: result = Curl_setstropt(&data->set.str[STRING_CERT_PROXY], va_arg(param, char*)); break;
        case CURLOPT_PROXY_SSLCERT_BLOB: result = Curl_setblobopt(&data->set.blobs[BLOB_CERT_PROXY], va_arg(param, struct curl_blob*)); break;
    #endif
        case CURLOPT_SSLCERTTYPE: result = Curl_setstropt(&data->set.str[STRING_CERT_TYPE_ORIG], va_arg(param, char*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSLCERTTYPE: result = Curl_setstropt(&data->set.str[STRING_CERT_TYPE_PROXY], va_arg(param, char*)); break;
    #endif
        case CURLOPT_SSLKEY: result = Curl_setstropt(&data->set.str[STRING_KEY_ORIG], va_arg(param, char*)); break;
        case CURLOPT_SSLKEY_BLOB: result = Curl_setblobopt(&data->set.blobs[BLOB_KEY_ORIG], va_arg(param, struct curl_blob*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSLKEY: result = Curl_setstropt(&data->set.str[STRING_KEY_PROXY], va_arg(param, char*)); break;
        case CURLOPT_PROXY_SSLKEY_BLOB: result = Curl_setblobopt(&data->set.blobs[BLOB_KEY_PROXY], va_arg(param, struct curl_blob*)); break;
    #endif
        case CURLOPT_SSLKEYTYPE: result = Curl_setstropt(&data->set.str[STRING_KEY_TYPE_ORIG], va_arg(param, char*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSLKEYTYPE: result = Curl_setstropt(&data->set.str[STRING_KEY_TYPE_PROXY], va_arg(param, char*)); break;
    #endif
        case CURLOPT_KEYPASSWD: result = Curl_setstropt(&data->set.str[STRING_KEY_PASSWD_ORIG], va_arg(param, char*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_KEYPASSWD: result = Curl_setstropt(&data->set.str[STRING_KEY_PASSWD_PROXY], va_arg(param, char*)); break;
    #endif
        case CURLOPT_SSLENGINE:
            argptr = va_arg(param, char*);
            if(argptr && argptr[0]) {
                result = Curl_setstropt(&data->set.str[STRING_SSL_ENGINE], argptr);
                if(!result) result = Curl_ssl_set_engine(data, argptr);
            }
            break;
        case CURLOPT_SSLENGINE_DEFAULT:
        Curl_setstropt(&data->set.str[STRING_SSL_ENGINE], NULL);
        result = Curl_ssl_set_engine_default(data);
        break;
        case CURLOPT_CRLF: data->set.crlf = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_HAPROXYPROTOCOL: data->set.haproxyprotocol = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
    #endif
        case CURLOPT_INTERFACE: result = Curl_setstropt(&data->set.str[STRING_DEVICE], va_arg(param, char*)); break;
        case CURLOPT_LOCALPORT:
            arg = va_arg(param, long);
            if((arg < 0) || (arg > 65535)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.localport = curlx_sltous(arg);
            break;
        case CURLOPT_LOCALPORTRANGE:
            arg = va_arg(param, long);
            if((arg < 0) || (arg > 65535)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.localportrange = curlx_sltosi(arg);
            break;
        case CURLOPT_GSSAPI_DELEGATION:
            arg = va_arg(param, long);
            if(arg < CURLGSSAPI_DELEGATION_NONE) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.gssapi_delegation = arg;
            break;
        case CURLOPT_SSL_VERIFYPEER:
            data->set.ssl.primary.verifypeer = (0 != va_arg(param, long)) ? TRUE : FALSE;
            if(data->conn)  data->conn->ssl_config.verifypeer = data->set.ssl.primary.verifypeer;
            break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSL_VERIFYPEER:
            data->set.proxy_ssl.primary.verifypeer = (0 != va_arg(param, long))?TRUE:FALSE;
            if(data->conn) data->conn->proxy_ssl_config.verifypeer = data->set.proxy_ssl.primary.verifypeer;
            break;
    #endif
        case CURLOPT_SSL_VERIFYHOST:
            arg = va_arg(param, long);
            data->set.ssl.primary.verifyhost = (bool)((arg & 3) ? TRUE : FALSE);
            if(data->conn) data->conn->ssl_config.verifyhost = data->set.ssl.primary.verifyhost;
            break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSL_VERIFYHOST:
            arg = va_arg(param, long);
            data->set.proxy_ssl.primary.verifyhost = (bool)((arg & 3) ? TRUE : FALSE);
            if(data->conn) data->conn->proxy_ssl_config.verifyhost = data->set.proxy_ssl.primary.verifyhost;
            break;
    #endif
        case CURLOPT_SSL_VERIFYSTATUS:
            if(!Curl_ssl_cert_status_request()) {
                result = CURLE_NOT_BUILT_IN;
                break;
            }
            data->set.ssl.primary.verifystatus = (0 != va_arg(param, long)) ? TRUE : FALSE;
            if(data->conn) data->conn->ssl_config.verifystatus = data->set.ssl.primary.verifystatus;
            break;
        case CURLOPT_SSL_CTX_FUNCTION:
    #ifdef USE_SSL
            if(Curl_ssl->supports & SSLSUPP_SSL_CTX) data->set.ssl.fsslctx = va_arg(param, curl_ssl_ctx_callback);
            else
    #endif
            result = CURLE_NOT_BUILT_IN;
            break;
        case CURLOPT_SSL_CTX_DATA:
    #ifdef USE_SSL
            if(Curl_ssl->supports & SSLSUPP_SSL_CTX) data->set.ssl.fsslctxp = va_arg(param, void *);
            else
    #endif
            result = CURLE_NOT_BUILT_IN;
            break;
            case CURLOPT_SSL_FALSESTART:
            if(!Curl_ssl_false_start()) {
                result = CURLE_NOT_BUILT_IN;
                break;
            }
            data->set.ssl.falsestart = (0 != va_arg(param, long)) ? TRUE : FALSE;
            break;
        case CURLOPT_CERTINFO:
    #ifdef USE_SSL
            if(Curl_ssl->supports & SSLSUPP_CERTINFO) data->set.ssl.certinfo = (0 != va_arg(param, long)) ? TRUE : FALSE;
            else
    #endif
            result = CURLE_NOT_BUILT_IN;
            break;
        case CURLOPT_PINNEDPUBLICKEY:
    #ifdef USE_SSL
            if(Curl_ssl->supports & SSLSUPP_PINNEDPUBKEY) result = Curl_setstropt(&data->set.str[STRING_SSL_PINNEDPUBLICKEY_ORIG], va_arg(param, char*));
            else
    #endif
            result = CURLE_NOT_BUILT_IN;
            break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_PINNEDPUBLICKEY:
    #ifdef USE_SSL
            if(Curl_ssl->supports & SSLSUPP_PINNEDPUBKEY) result = Curl_setstropt(&data->set.str[STRING_SSL_PINNEDPUBLICKEY_PROXY], va_arg(param, char *));
            else
    #endif
            result = CURLE_NOT_BUILT_IN;
            break;
    #endif
        case CURLOPT_CAINFO: result = Curl_setstropt(&data->set.str[STRING_SSL_CAFILE_ORIG], va_arg(param, char*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_CAINFO: result = Curl_setstropt(&data->set.str[STRING_SSL_CAFILE_PROXY], va_arg(param, char*)); break;
    #endif
        case CURLOPT_CAPATH:
    #ifdef USE_SSL
            if(Curl_ssl->supports & SSLSUPP_CA_PATH) result = Curl_setstropt(&data->set.str[STRING_SSL_CAPATH_ORIG], va_arg(param, char*));
            else
    #endif
            result = CURLE_NOT_BUILT_IN;
            break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_CAPATH:
    #ifdef USE_SSL
            if(Curl_ssl->supports & SSLSUPP_CA_PATH) result = Curl_setstropt(&data->set.str[STRING_SSL_CAPATH_PROXY], va_arg(param, char *));
            else
    #endif
            result = CURLE_NOT_BUILT_IN;
            break;
    #endif
        case CURLOPT_CRLFILE: result = Curl_setstropt(&data->set.str[STRING_SSL_CRLFILE_ORIG], va_arg(param, char*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_CRLFILE: result = Curl_setstropt(&data->set.str[STRING_SSL_CRLFILE_PROXY], va_arg(param, char*)); break;
    #endif
        case CURLOPT_ISSUERCERT: result = Curl_setstropt(&data->set.str[STRING_SSL_ISSUERCERT_ORIG], va_arg(param, char*)); break;
        case CURLOPT_ISSUERCERT_BLOB: result = Curl_setblobopt(&data->set.blobs[BLOB_SSL_ISSUERCERT_ORIG], va_arg(param, struct curl_blob*)); break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_ISSUERCERT: result = Curl_setstropt(&data->set.str[STRING_SSL_ISSUERCERT_PROXY], va_arg(param, char*)); break;
        case CURLOPT_PROXY_ISSUERCERT_BLOB: result = Curl_setblobopt(&data->set.blobs[BLOB_SSL_ISSUERCERT_PROXY], va_arg(param, struct curl_blob*)); break;
    #endif
    #ifndef CURL_DISABLE_TELNET
        case CURLOPT_TELNETOPTIONS: data->set.telnet_options = va_arg(param, struct curl_slist*); break;
    #endif
        case CURLOPT_BUFFERSIZE:
            arg = va_arg(param, long);
            if(arg > READBUFFER_MAX) arg = READBUFFER_MAX;
            else if(arg < 1) arg = READBUFFER_SIZE;
            else if(arg < READBUFFER_MIN) arg = READBUFFER_MIN;
            if((arg != data->set.buffer_size) && data->state.buffer) {
                char *newbuff = realloc(data->state.buffer, arg + 1);
                if(!newbuff) {
                    DEBUGF(fprintf(stderr, "Error: realloc of buffer failed\n"));
                    result = CURLE_OUT_OF_MEMORY;
                } else data->state.buffer = newbuff;
            }
            data->set.buffer_size = arg;
            break;
        case CURLOPT_UPLOAD_BUFFERSIZE:
            arg = va_arg(param, long);
            if(arg > UPLOADBUFFER_MAX) arg = UPLOADBUFFER_MAX;
            else if(arg < UPLOADBUFFER_MIN) arg = UPLOADBUFFER_MIN;
            data->set.upload_buffer_size = arg;
            Curl_safefree(data->state.ulbuf);
            break;
        case CURLOPT_NOSIGNAL: data->set.no_signal = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_SHARE:
            {
                struct Curl_share *set;
                set = va_arg(param, struct Curl_share*);
                if(data->share) {
                    Curl_share_lock(data, CURL_LOCK_DATA_SHARE, CURL_LOCK_ACCESS_SINGLE);
                    if(data->dns.hostcachetype == HCACHE_SHARED) {
                        data->dns.hostcache = NULL;
                        data->dns.hostcachetype = HCACHE_NONE;
                    }
                #if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
                    if(data->share->cookies == data->cookies) data->cookies = NULL;
                #endif
                    if(data->share->sslsession == data->state.session) data->state.session = NULL;
                #ifdef USE_LIBPSL
                    if(data->psl == &data->share->psl) data->psl = data->multi? &data->multi->psl: NULL;
                #endif
                    data->share->dirty--;
                    Curl_share_unlock(data, CURL_LOCK_DATA_SHARE);
                    data->share = NULL;
                }
                data->share = set;
                if(data->share) {
                    Curl_share_lock(data, CURL_LOCK_DATA_SHARE, CURL_LOCK_ACCESS_SINGLE);
                    data->share->dirty++;
                    if(data->share->specifier & (1<< CURL_LOCK_DATA_DNS)) {
                        data->dns.hostcache = &data->share->hostcache;
                        data->dns.hostcachetype = HCACHE_SHARED;
                    }
                #if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
                    if(data->share->cookies) {
                        Curl_cookie_cleanup(data->cookies);
                        data->cookies = data->share->cookies;
                    }
                #endif
                    if(data->share->sslsession) {
                        data->set.general_ssl.max_ssl_sessions = data->share->max_ssl_sessions;
                        data->state.session = data->share->sslsession;
                    }
                #ifdef USE_LIBPSL
                    if(data->share->specifier & (1 << CURL_LOCK_DATA_PSL)) data->psl = &data->share->psl;
                #endif
                    Curl_share_unlock(data, CURL_LOCK_DATA_SHARE);
                }
            }
            break;
        case CURLOPT_PRIVATE: data->set.private_data = va_arg(param, void*); break;
        case CURLOPT_MAXFILESIZE:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.max_filesize = arg;
            break;
    #ifdef USE_SSL
        case CURLOPT_USE_SSL:
            arg = va_arg(param, long);
            if((arg < CURLUSESSL_NONE) || (arg >= CURLUSESSL_LAST)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.use_ssl = (curl_usessl)arg;
            break;
        case CURLOPT_SSL_OPTIONS:
            arg = va_arg(param, long);
            data->set.ssl.enable_beast = (bool)((arg&CURLSSLOPT_ALLOW_BEAST) ? TRUE : FALSE);
            data->set.ssl.no_revoke = !!(arg & CURLSSLOPT_NO_REVOKE);
            data->set.ssl.no_partialchain = !!(arg & CURLSSLOPT_NO_PARTIALCHAIN);
            data->set.ssl.revoke_best_effort = !!(arg & CURLSSLOPT_REVOKE_BEST_EFFORT);
            data->set.ssl.native_ca_store = !!(arg & CURLSSLOPT_NATIVE_CA);
            break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_SSL_OPTIONS:
            arg = va_arg(param, long);
            data->set.proxy_ssl.enable_beast = (bool)((arg&CURLSSLOPT_ALLOW_BEAST) ? TRUE : FALSE);
            data->set.proxy_ssl.no_revoke = !!(arg & CURLSSLOPT_NO_REVOKE);
            data->set.proxy_ssl.no_partialchain = !!(arg & CURLSSLOPT_NO_PARTIALCHAIN);
            data->set.proxy_ssl.native_ca_store = !!(arg & CURLSSLOPT_NATIVE_CA);
            data->set.proxy_ssl.revoke_best_effort = !!(arg & CURLSSLOPT_REVOKE_BEST_EFFORT);
            break;
    #endif
    #endif
        case CURLOPT_IPRESOLVE:
            arg = va_arg(param, long);
            if((arg < CURL_IPRESOLVE_WHATEVER) || (arg > CURL_IPRESOLVE_V6)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.ipver = arg;
            break;
        case CURLOPT_MAXFILESIZE_LARGE:
            bigsize = va_arg(param, curl_off_t);
            if(bigsize < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.max_filesize = bigsize;
            break;
        case CURLOPT_TCP_NODELAY: data->set.tcp_nodelay = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_IGNORE_CONTENT_LENGTH: data->set.ignorecl = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_CONNECT_ONLY: data->set.connect_only = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_SOCKOPTFUNCTION: data->set.fsockopt = va_arg(param, curl_sockopt_callback); break;
        case CURLOPT_SOCKOPTDATA: data->set.sockopt_client = va_arg(param, void*); break;
        case CURLOPT_OPENSOCKETFUNCTION: data->set.fopensocket = va_arg(param, curl_opensocket_callback); break;
        case CURLOPT_OPENSOCKETDATA: data->set.opensocket_client = va_arg(param, void*); break;
        case CURLOPT_CLOSESOCKETFUNCTION: data->set.fclosesocket = va_arg(param, curl_closesocket_callback); break;
        case CURLOPT_RESOLVER_START_FUNCTION: data->set.resolver_start = va_arg(param, curl_resolver_start_callback); break;
        case CURLOPT_RESOLVER_START_DATA: data->set.resolver_start_client = va_arg(param, void*); break;
        case CURLOPT_CLOSESOCKETDATA: data->set.closesocket_client = va_arg(param, void*); break;
        case CURLOPT_SSL_SESSIONID_CACHE:
            data->set.ssl.primary.sessionid = (0 != va_arg(param, long)) ? TRUE : FALSE;
        #ifndef CURL_DISABLE_PROXY
            data->set.proxy_ssl.primary.sessionid = data->set.ssl.primary.sessionid;
        #endif
            break;
    #ifdef USE_SSH
        case CURLOPT_SSH_AUTH_TYPES: data->set.ssh_auth_types = va_arg(param, long); break;
        case CURLOPT_SSH_PUBLIC_KEYFILE: result = Curl_setstropt(&data->set.str[STRING_SSH_PUBLIC_KEY], va_arg(param, char *)); break;
        case CURLOPT_SSH_PRIVATE_KEYFILE: result = Curl_setstropt(&data->set.str[STRING_SSH_PRIVATE_KEY], va_arg(param, char *)); break;
        case CURLOPT_SSH_HOST_PUBLIC_KEY_MD5: result = Curl_setstropt(&data->set.str[STRING_SSH_HOST_PUBLIC_KEY_MD5], va_arg(param, char *)); break;
        case CURLOPT_SSH_KNOWNHOSTS: result = Curl_setstropt(&data->set.str[STRING_SSH_KNOWNHOSTS], va_arg(param, char *)); break;
        case CURLOPT_SSH_KEYFUNCTION: data->set.ssh_keyfunc = va_arg(param, curl_sshkeycallback); break;
        case CURLOPT_SSH_KEYDATA: data->set.ssh_keyfunc_userp = va_arg(param, void *); break;
        case CURLOPT_SSH_COMPRESSION: data->set.ssh_compression = (0 != va_arg(param, long))?TRUE:FALSE; break;
    #endif
        case CURLOPT_HTTP_TRANSFER_DECODING: data->set.http_te_skip = (0 == va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_HTTP_CONTENT_DECODING: data->set.http_ce_skip = (0 == va_arg(param, long)) ? TRUE : FALSE; break;
    #if !defined(CURL_DISABLE_FTP) || defined(USE_SSH)
        case CURLOPT_NEW_FILE_PERMS:
            arg = va_arg(param, long);
            if((arg < 0) || (arg > 0777)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.new_file_perms = arg;
            break;
        case CURLOPT_NEW_DIRECTORY_PERMS:
            arg = va_arg(param, long);
            if((arg < 0) || (arg > 0777)) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.new_directory_perms = arg;
            break;
    #endif
        case CURLOPT_ADDRESS_SCOPE:
            uarg = va_arg(param, unsigned long);
        #if SIZEOF_LONG > 4
            if(uarg > UINT_MAX) return CURLE_BAD_FUNCTION_ARGUMENT;
        #endif
            data->set.scope_id = (unsigned int)uarg;
            break;
        case CURLOPT_PROTOCOLS: data->set.allowed_protocols = va_arg(param, long); break;
        case CURLOPT_REDIR_PROTOCOLS: data->set.redir_protocols = va_arg(param, long); break;
        case CURLOPT_DEFAULT_PROTOCOL: result = Curl_setstropt(&data->set.str[STRING_DEFAULT_PROTOCOL], va_arg(param, char*)); break;
    #ifndef CURL_DISABLE_SMTP
        case CURLOPT_MAIL_FROM: result = Curl_setstropt(&data->set.str[STRING_MAIL_FROM], va_arg(param, char*)); break;
        case CURLOPT_MAIL_AUTH: result = Curl_setstropt(&data->set.str[STRING_MAIL_AUTH], va_arg(param, char*)); break;
        case CURLOPT_MAIL_RCPT: data->set.mail_rcpt = va_arg(param, struct curl_slist*); break;
        case CURLOPT_MAIL_RCPT_ALLLOWFAILS: data->set.mail_rcpt_allowfails = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
    #endif
        case CURLOPT_SASL_AUTHZID: result = Curl_setstropt(&data->set.str[STRING_SASL_AUTHZID], va_arg(param, char*)); break;
        case CURLOPT_SASL_IR: data->set.sasl_ir = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
    #ifndef CURL_DISABLE_RTSP
        case CURLOPT_RTSP_REQUEST: {
                long curl_rtspreq = va_arg(param, long);
                Curl_RtspReq rtspreq = RTSPREQ_NONE;
                switch(curl_rtspreq) {
                    case CURL_RTSPREQ_OPTIONS: rtspreq = RTSPREQ_OPTIONS; break;
                    case CURL_RTSPREQ_DESCRIBE: rtspreq = RTSPREQ_DESCRIBE; break;
                    case CURL_RTSPREQ_ANNOUNCE: rtspreq = RTSPREQ_ANNOUNCE; break;
                    case CURL_RTSPREQ_SETUP: rtspreq = RTSPREQ_SETUP; break;
                    case CURL_RTSPREQ_PLAY: rtspreq = RTSPREQ_PLAY; break;
                    case CURL_RTSPREQ_PAUSE: rtspreq = RTSPREQ_PAUSE; break;
                    case CURL_RTSPREQ_TEARDOWN: rtspreq = RTSPREQ_TEARDOWN; break;
                    case CURL_RTSPREQ_GET_PARAMETER: rtspreq = RTSPREQ_GET_PARAMETER; break;
                    case CURL_RTSPREQ_SET_PARAMETER: rtspreq = RTSPREQ_SET_PARAMETER; break;
                    case CURL_RTSPREQ_RECORD: rtspreq = RTSPREQ_RECORD; break;
                    case CURL_RTSPREQ_RECEIVE: rtspreq = RTSPREQ_RECEIVE; break;
                    default: rtspreq = RTSPREQ_NONE;
                }
                data->set.rtspreq = rtspreq;
                break;
            }
        case CURLOPT_RTSP_SESSION_ID: result = Curl_setstropt(&data->set.str[STRING_RTSP_SESSION_ID], va_arg(param, char*)); break;
        case CURLOPT_RTSP_STREAM_URI: result = Curl_setstropt(&data->set.str[STRING_RTSP_STREAM_URI], va_arg(param, char*)); break;
        case CURLOPT_RTSP_TRANSPORT: result = Curl_setstropt(&data->set.str[STRING_RTSP_TRANSPORT], va_arg(param, char*)); break;
        case CURLOPT_RTSP_CLIENT_CSEQ: data->state.rtsp_next_client_CSeq = va_arg(param, long); break;
        case CURLOPT_RTSP_SERVER_CSEQ: data->state.rtsp_next_server_CSeq = va_arg(param, long); break;
        case CURLOPT_INTERLEAVEDATA: data->set.rtp_out = va_arg(param, void *); break;
        case CURLOPT_INTERLEAVEFUNCTION: data->set.fwrite_rtp = va_arg(param, curl_write_callback); break;
    #endif
    #ifndef CURL_DISABLE_FTP
        case CURLOPT_WILDCARDMATCH: data->set.wildcard_enabled = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_CHUNK_BGN_FUNCTION: data->set.chunk_bgn = va_arg(param, curl_chunk_bgn_callback); break;
        case CURLOPT_CHUNK_END_FUNCTION: data->set.chunk_end = va_arg(param, curl_chunk_end_callback); break;
        case CURLOPT_FNMATCH_FUNCTION: data->set.fnmatch = va_arg(param, curl_fnmatch_callback); break;
        case CURLOPT_CHUNK_DATA: data->wildcard.customptr = va_arg(param, void*); break;
        case CURLOPT_FNMATCH_DATA: data->set.fnmatch_data = va_arg(param, void*); break;
    #endif
    #ifdef USE_TLS_SRP
        case CURLOPT_TLSAUTH_USERNAME:
            result = Curl_setstropt(&data->set.str[STRING_TLSAUTH_USERNAME_ORIG], va_arg(param, char*));
            if(data->set.str[STRING_TLSAUTH_USERNAME_ORIG] && !data->set.ssl.authtype) data->set.ssl.authtype = CURL_TLSAUTH_SRP;
            break;
        case CURLOPT_PROXY_TLSAUTH_USERNAME:
            result = Curl_setstropt(&data->set.str[STRING_TLSAUTH_USERNAME_PROXY], va_arg(param, char*));
        #ifndef CURL_DISABLE_PROXY
            if(data->set.str[STRING_TLSAUTH_USERNAME_PROXY] && !data->set.proxy_ssl.authtype) data->set.proxy_ssl.authtype = CURL_TLSAUTH_SRP;
        #endif
            break;
        case CURLOPT_TLSAUTH_PASSWORD:
        result = Curl_setstropt(&data->set.str[STRING_TLSAUTH_PASSWORD_ORIG], va_arg(param, char*));
        if(data->set.str[STRING_TLSAUTH_USERNAME_ORIG] && !data->set.ssl.authtype) data->set.ssl.authtype = CURL_TLSAUTH_SRP;
        break;
        case CURLOPT_PROXY_TLSAUTH_PASSWORD:
            result = Curl_setstropt(&data->set.str[STRING_TLSAUTH_PASSWORD_PROXY], va_arg(param, char*));
    #ifndef CURL_DISABLE_PROXY
            if(data->set.str[STRING_TLSAUTH_USERNAME_PROXY] && !data->set.proxy_ssl.authtype) data->set.proxy_ssl.authtype = CURL_TLSAUTH_SRP;
    #endif
            break;
        case CURLOPT_TLSAUTH_TYPE:
            argptr = va_arg(param, char*);
            if(!argptr || strncasecompare(argptr, "SRP", strlen("SRP"))) data->set.ssl.authtype = CURL_TLSAUTH_SRP;
            else data->set.ssl.authtype = CURL_TLSAUTH_NONE;
            break;
    #ifndef CURL_DISABLE_PROXY
        case CURLOPT_PROXY_TLSAUTH_TYPE:
            argptr = va_arg(param, char*);
            if(!argptr || strncasecompare(argptr, "SRP", strlen("SRP"))) data->set.proxy_ssl.authtype = CURL_TLSAUTH_SRP;
            else data->set.proxy_ssl.authtype = CURL_TLSAUTH_NONE;
            break;
    #endif
    #endif
    #ifdef USE_ARES
        case CURLOPT_DNS_SERVERS:
            result = Curl_setstropt(&data->set.str[STRING_DNS_SERVERS], va_arg(param, char*));
            if(result) return result;
            result = Curl_set_dns_servers(data, data->set.str[STRING_DNS_SERVERS]);
            break;
        case CURLOPT_DNS_INTERFACE:
            result = Curl_setstropt(&data->set.str[STRING_DNS_INTERFACE], va_arg(param, char*));
            if(result) return result;
            result = Curl_set_dns_interface(data, data->set.str[STRING_DNS_INTERFACE]);
            break;
        case CURLOPT_DNS_LOCAL_IP4:
            result = Curl_setstropt(&data->set.str[STRING_DNS_LOCAL_IP4], va_arg(param, char*));
            if(result) return result;
            result = Curl_set_dns_local_ip4(data, data->set.str[STRING_DNS_LOCAL_IP4]);
            break;
        case CURLOPT_DNS_LOCAL_IP6:
            result = Curl_setstropt(&data->set.str[STRING_DNS_LOCAL_IP6], va_arg(param, char*));
            if(result) return result;
            result = Curl_set_dns_local_ip6(data, data->set.str[STRING_DNS_LOCAL_IP6]);
            break;
    #endif
        case CURLOPT_TCP_KEEPALIVE: data->set.tcp_keepalive = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_TCP_KEEPIDLE:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.tcp_keepidle = arg;
            break;
        case CURLOPT_TCP_KEEPINTVL:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.tcp_keepintvl = arg;
            break;
        case CURLOPT_TCP_FASTOPEN:
        #if defined(CONNECT_DATA_IDEMPOTENT) || defined(MSG_FASTOPEN) || defined(TCP_FASTOPEN_CONNECT)
            data->set.tcp_fastopen = (0 != va_arg(param, long))?TRUE:FALSE;
        #else
            result = CURLE_NOT_BUILT_IN;
        #endif
            break;
        case CURLOPT_SSL_ENABLE_NPN:
        data->set.ssl_enable_npn = (0 != va_arg(param, long)) ? TRUE : FALSE;
        break;
        case CURLOPT_SSL_ENABLE_ALPN:
        data->set.ssl_enable_alpn = (0 != va_arg(param, long)) ? TRUE : FALSE;
        break;
    #ifdef USE_UNIX_SOCKETS
        case CURLOPT_UNIX_SOCKET_PATH:
            data->set.abstract_unix_socket = FALSE;
            result = Curl_setstropt(&data->set.str[STRING_UNIX_SOCKET_PATH], va_arg(param, char*));
            break;
        case CURLOPT_ABSTRACT_UNIX_SOCKET:
            data->set.abstract_unix_socket = TRUE;
            result = Curl_setstropt(&data->set.str[STRING_UNIX_SOCKET_PATH], va_arg(param, char*));
            break;
    #endif
        case CURLOPT_PATH_AS_IS: data->set.path_as_is = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_PIPEWAIT: data->set.pipewait = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_STREAM_WEIGHT:
    #ifndef USE_NGHTTP2
            return CURLE_NOT_BUILT_IN;
    #else
            arg = va_arg(param, long);
            if((arg >= 1) && (arg <= 256)) data->set.stream_weight = (int)arg;
            break;
    #endif
        case CURLOPT_STREAM_DEPENDS:
        case CURLOPT_STREAM_DEPENDS_E:
            {
            #ifndef USE_NGHTTP2
                return CURLE_NOT_BUILT_IN;
            #else
                struct Curl_easy *dep = va_arg(param, struct Curl_easy*);
                if(!dep || GOOD_EASY_HANDLE(dep)) {
                    if(data->set.stream_depends_on) Curl_http2_remove_child(data->set.stream_depends_on, data);
                    Curl_http2_add_child(dep, data, (option == CURLOPT_STREAM_DEPENDS_E));
                }
                break;
            #endif
            }
        case CURLOPT_CONNECT_TO: data->set.connect_to = va_arg(param, struct curl_slist*); break;
        case CURLOPT_SUPPRESS_CONNECT_HEADERS: data->set.suppress_connect_headers = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
        case CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.happy_eyeballs_timeout = arg;
            break;
    #ifndef CURL_DISABLE_SHUFFLE_DNS
        case CURLOPT_DNS_SHUFFLE_ADDRESSES: data->set.dns_shuffle_addresses = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
    #endif
        case CURLOPT_DISALLOW_USERNAME_IN_URL: data->set.disallow_username_in_url = (0 != va_arg(param, long)) ? TRUE : FALSE; break;
    #ifndef CURL_DISABLE_DOH
        case CURLOPT_DOH_URL:
            result = Curl_setstropt(&data->set.str[STRING_DOH], va_arg(param, char*));
            data->set.doh = data->set.str[STRING_DOH]?TRUE:FALSE;
            break;
    #endif
        case CURLOPT_UPKEEP_INTERVAL_MS:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.upkeep_interval_ms = arg;
            break;
        case CURLOPT_MAXAGE_CONN:
            arg = va_arg(param, long);
            if(arg < 0) return CURLE_BAD_FUNCTION_ARGUMENT;
            data->set.maxage_conn = arg;
            break;
        case CURLOPT_TRAILERFUNCTION:
    #ifndef CURL_DISABLE_HTTP
            data->set.trailer_callback = va_arg(param, curl_trailer_callback);
    #endif
            break;
        case CURLOPT_TRAILERDATA:
    #ifndef CURL_DISABLE_HTTP
            data->set.trailer_data = va_arg(param, void*);
    #endif
            break;
    #ifdef USE_ALTSVC
        case CURLOPT_ALTSVC:
            if(!data->asi) {
                data->asi = Curl_altsvc_init();
                if(!data->asi) return CURLE_OUT_OF_MEMORY;
            }
            argptr = va_arg(param, char *);
            result = Curl_setstropt(&data->set.str[STRING_ALTSVC], argptr);
            if(result) return result;
            if(argptr) (void)Curl_altsvc_load(data->asi, argptr);
            break;
        case CURLOPT_ALTSVC_CTRL:
            if(!data->asi) {
                data->asi = Curl_altsvc_init();
                if(!data->asi) return CURLE_OUT_OF_MEMORY;
            }
            arg = va_arg(param, long);
            result = Curl_altsvc_ctrl(data->asi, arg);
            if(result) return result;
            break;
    #endif
        default: result = CURLE_UNKNOWN_OPTION; break;
    }
    return result;
}
#undef curl_easy_setopt
CURLcode curl_easy_setopt(CURL *data, CURLoption tag, ...) {
    va_list arg;
    CURLcode result;
    if(!data) return CURLE_BAD_FUNCTION_ARGUMENT;
    va_start(arg, tag);
    result = Curl_vsetopt(data, tag, arg);
    va_end(arg);
    return result;
}
