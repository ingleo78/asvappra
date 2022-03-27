#ifndef HEADER_CURL_TOOL_SETOPT_H
#define HEADER_CURL_TOOL_SETOPT_H

#include "tool_setup.h"
#include "tool_formparse.h"
#include "easy.h"

#define SETOPT_CHECK(v,opt) do {                \
        if(!tool_setopt_skip(opt)) {            \
          result = (v);                         \
          if(result) break;                     \
        }                                       \
    } while(0);

bool tool_setopt_skip(CURLoption tag);
#ifndef CURL_DISABLE_LIBCURL_OPTION
struct NameValue {
    const char *name;
    long value;
};
struct NameValueUnsigned {
    const char *name;
    unsigned long value;
};
extern const struct NameValue setopt_nv_CURLPROXY[];
extern const struct NameValue setopt_nv_CURL_SOCKS_PROXY[];
extern const struct NameValue setopt_nv_CURL_HTTP_VERSION[];
extern const struct NameValue setopt_nv_CURL_SSLVERSION[];
extern const struct NameValue setopt_nv_CURL_TIMECOND[];
extern const struct NameValue setopt_nv_CURLFTPSSL_CCC[];
extern const struct NameValue setopt_nv_CURLUSESSL[];
extern const struct NameValueUnsigned setopt_nv_CURLSSLOPT[];
extern const struct NameValue setopt_nv_CURL_NETRC[];
extern const struct NameValue setopt_nv_CURLPROTO[];
extern const struct NameValueUnsigned setopt_nv_CURLAUTH[];
#define setopt_nv_CURLOPT_HTTP_VERSION setopt_nv_CURL_HTTP_VERSION
#define setopt_nv_CURLOPT_HTTPAUTH setopt_nv_CURLAUTH
#define setopt_nv_CURLOPT_SSLVERSION setopt_nv_CURL_SSLVERSION
#define setopt_nv_CURLOPT_PROXY_SSLVERSION setopt_nv_CURL_SSLVERSION
#define setopt_nv_CURLOPT_TIMECONDITION setopt_nv_CURL_TIMECOND
#define setopt_nv_CURLOPT_FTP_SSL_CCC setopt_nv_CURLFTPSSL_CCC
#define setopt_nv_CURLOPT_USE_SSL setopt_nv_CURLUSESSL
#define setopt_nv_CURLOPT_SSL_OPTIONS setopt_nv_CURLSSLOPT
#define setopt_nv_CURLOPT_NETRC setopt_nv_CURL_NETRC
#define setopt_nv_CURLOPT_PROTOCOLS setopt_nv_CURLPROTO
#define setopt_nv_CURLOPT_REDIR_PROTOCOLS setopt_nv_CURLPROTO
#define setopt_nv_CURLOPT_PROXYTYPE setopt_nv_CURLPROXY
#define setopt_nv_CURLOPT_PROXYAUTH setopt_nv_CURLAUTH
#define setopt_nv_CURLOPT_SOCKS5_AUTH setopt_nv_CURLAUTH
CURLcode tool_setopt_enum(CURL *curl, struct GlobalConfig *config, const char *name, CURLoption tag, const struct NameValue *nv, long lval);
CURLcode tool_setopt_flags(CURL *curl, struct GlobalConfig *config, const char *name, CURLoption tag, const struct NameValue *nv, long lval);
CURLcode tool_setopt_bitmask(CURL *curl, struct GlobalConfig *config, const char *name, CURLoption tag, const struct NameValueUnsigned *nv, long lval);
CURLcode tool_setopt_mimepost(CURL *curl, struct GlobalConfig *config, const char *name, CURLoption tag, curl_mime *mimepost);
CURLcode tool_setopt_slist(CURL *curl, struct GlobalConfig *config, const char *name, CURLoption tag, struct curl_slist *list);
CURLcode tool_setopt(CURL *curl, bool str, struct GlobalConfig *config, const char *name, CURLoption tag, ...);
#define my_setopt(x,y,z) SETOPT_CHECK(tool_setopt(x, FALSE, global, #y, y, z), y)
#define my_setopt_str(x,y,z) SETOPT_CHECK(tool_setopt(x, TRUE, global, #y, y, z), y)
#define my_setopt_enum(x,y,z) SETOPT_CHECK(tool_setopt_enum(x, global, #y, y, setopt_nv_ ## y, z), y)
#define my_setopt_flags(x,y,z) SETOPT_CHECK(tool_setopt_flags(x, global, #y, y, setopt_nv_ ## y, z), y)
#define my_setopt_bitmask(x,y,z) SETOPT_CHECK(tool_setopt_bitmask(x, global, #y, y, setopt_nv_ ## y, z), y)
#define my_setopt_mimepost(x,y,z) SETOPT_CHECK(tool_setopt_mimepost(x, global, #y, y, z), y)
#define my_setopt_slist(x,y,z) SETOPT_CHECK(tool_setopt_slist(x, global, #y, y, z), y)
#define res_setopt(x,y,z) tool_setopt(x, FALSE, global, #y, y, z)
#define res_setopt_str(x,y,z) tool_setopt(x, TRUE, global, #y, y, z)
#else
#define my_setopt(x,y,z) SETOPT_CHECK(curl_easy_setopt(x, y, z), y)
#define my_setopt_str(x,y,z) SETOPT_CHECK(curl_easy_setopt(x, y, z), y)
#define my_setopt_enum(x,y,z) SETOPT_CHECK(curl_easy_setopt(x, y, z), y)
#define my_setopt_flags(x,y,z) SETOPT_CHECK(curl_easy_setopt(x, y, z), y)
#define my_setopt_bitmask(x,y,z) SETOPT_CHECK(curl_easy_setopt(x, y, z), y)
#define my_setopt_mimepost(x,y,z) SETOPT_CHECK(curl_easy_setopt(x, y, z), y)
#define my_setopt_slist(x,y,z) SETOPT_CHECK(curl_easy_setopt(x, y, z), y)
#define res_setopt(x,y,z) curl_easy_setopt(x,y,z)
#define res_setopt_str(x,y,z) curl_easy_setopt(x,y,z)
#endif
#endif