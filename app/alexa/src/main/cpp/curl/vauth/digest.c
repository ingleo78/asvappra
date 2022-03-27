#include <stdbool.h>
#include "../curl_setup.h"
#include "../curl_ctype.h"
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
#include "../curl.h"
#include "../vauth/vauth.h"
#include "../vauth/digest.h"
#include "../urldata.h"
#include "../curl_base64.h"
#include "../curl_hmac.h"
#include "../curl_md5.h"
#include "../curl_sha256.h"
#include "../vtls/vtls.h"
#include "../warnless.h"
#include "../strtok.h"
#include "../strcase.h"
#include "../non-ascii.h"
#include "../curl_printf.h"
#include "../rand.h"
#include "../curl_memory.h"
#include "../memdebug.h"
#include "digest.h"

#if !defined(USE_WINDOWS_SSPI)
#define DIGEST_QOP_VALUE_AUTH             (1 << 0)
#define DIGEST_QOP_VALUE_AUTH_INT         (1 << 1)
#define DIGEST_QOP_VALUE_AUTH_CONF        (1 << 2)
#define DIGEST_QOP_VALUE_STRING_AUTH      "auth"
#define DIGEST_QOP_VALUE_STRING_AUTH_INT  "auth-int"
#define DIGEST_QOP_VALUE_STRING_AUTH_CONF "auth-conf"
#define CURL_OUTPUT_DIGEST_CONV(a, b) \
    result = Curl_convert_to_network(a, b, strlen(b)); \
    if(result) { \
        free(b); \
        return result; \
    }
#endif
bool Curl_auth_digest_get_pair(const char *str, char *value, char *content, const char **endptr) {
    int c;
    bool starts_with_quote = FALSE;
    bool escape = FALSE;
    for(c = DIGEST_MAX_VALUE_LENGTH - 1; (*str && (*str != '=') && c--);) *value++ = *str++;
    *value = 0;
    if('=' != *str++) return FALSE;
    if('\"' == *str) {
        str++;
        starts_with_quote = TRUE;
    }
    for(c = DIGEST_MAX_CONTENT_LENGTH - 1; *str && c--; str++) {
        switch(*str) {
            case '\\':
                if(!escape) {
                  escape = TRUE;
                  *content++ = '\\';
                  continue;
                }
                break;
            case ',':
                if(!starts_with_quote) {
                  c = 0;
                  continue;
                }
                break;
            case '\r': case '\n': c = 0; continue;
            case '\"':
                if(!escape && starts_with_quote) {
                  c = 0;
                  continue;
                }
                break;
        }
        escape = FALSE;
        *content++ = *str;
    }
    *content = 0;
    *endptr = str;
    return TRUE;
}
#if !defined(USE_WINDOWS_SSPI)
static void auth_digest_md5_to_ascii(unsigned char *source, unsigned char *dest) {
    int i;
    for(i = 0; i < 16; i++)
        msnprintf((char*)&dest[i * 2], 3, "%02x", source[i]);
}
static void auth_digest_sha256_to_ascii(unsigned char *source, unsigned char *dest) {
    int i;
    for(i = 0; i < 32; i++)
        msnprintf((char *) &dest[i * 2], 3, "%02x", source[i]);
}
static char *auth_digest_string_quoted(const char *source) {
    char *dest;
    const char *s = source;
    size_t n = 1;
    while(*s) {
        ++n;
        if(*s == '"' || *s == '\\') ++n;
        ++s;
    }
    dest = malloc(n);
    if(dest) {
        char *d = dest;
        s = source;
        while(*s) {
            if(*s == '"' || *s == '\\') *d++ = '\\';
            *d++ = *s++;
        }
        *d = 0;
    }
    return dest;
}
static bool auth_digest_get_key_value(const char *chlg, const char *key, char *value, size_t max_val_len, char end_char) {
    char *find_pos;
    size_t i;
    find_pos = strstr(chlg, key);
    if(!find_pos) return FALSE;
    find_pos += strlen(key);
    for(i = 0; *find_pos && *find_pos != end_char && i < max_val_len - 1; ++i) value[i] = *find_pos++;
    value[i] = '\0';
    return TRUE;
}
static CURLcode auth_digest_get_qop_values(const char *options, int *value) {
    char *tmp;
    char *token;
    char *tok_buf = NULL;
    *value = 0;
    tmp = strdup(options);
    if(!tmp) return CURLE_OUT_OF_MEMORY;
    token = strtok_r(tmp, ",", &tok_buf);
    while(token != NULL) {
        if(strcasecompare(token, DIGEST_QOP_VALUE_STRING_AUTH)) *value |= DIGEST_QOP_VALUE_AUTH;
        else if(strcasecompare(token, DIGEST_QOP_VALUE_STRING_AUTH_INT)) *value |= DIGEST_QOP_VALUE_AUTH_INT;
        else if(strcasecompare(token, DIGEST_QOP_VALUE_STRING_AUTH_CONF)) *value |= DIGEST_QOP_VALUE_AUTH_CONF;
        token = strtok_r(NULL, ",", &tok_buf);
    }
    free(tmp);
    return CURLE_OK;
}
static CURLcode auth_decode_digest_md5_message(const char *chlg64, char *nonce, size_t nlen, char *realm, size_t rlen, char *alg, size_t alen, char *qop,
                                               size_t qlen) {
    CURLcode result = CURLE_OK;
    unsigned char *chlg = NULL;
    size_t chlglen = 0;
    size_t chlg64len = strlen(chlg64);
    if(chlg64len && *chlg64 != '=') {
        result = Curl_base64_decode(chlg64, &chlg, &chlglen);
        if(result) return result;
    }
    if(!chlg) return CURLE_BAD_CONTENT_ENCODING;
    if(!auth_digest_get_key_value((char *) chlg, "nonce=\"", nonce, nlen,'\"')) {
        free(chlg);
        return CURLE_BAD_CONTENT_ENCODING;
    }
    if(!auth_digest_get_key_value((char *) chlg, "realm=\"", realm, rlen,'\"')) strcpy(realm, "");
    if(!auth_digest_get_key_value((char *) chlg, "algorithm=", alg, alen, ',')) {
        free(chlg);
        return CURLE_BAD_CONTENT_ENCODING;
    }
    if(!auth_digest_get_key_value((char *) chlg, "qop=\"", qop, qlen, '\"')) {
        free(chlg);
        return CURLE_BAD_CONTENT_ENCODING;
    }
    free(chlg);
    return CURLE_OK;
}
bool Curl_auth_is_digest_supported(void) {
  return TRUE;
}
CURLcode Curl_auth_create_digest_md5_message(struct Curl_easy *data, const char *chlg64,const char *userp, const char *passwdp, const char *service,
                                             char **outptr, size_t *outlen) {
    size_t i;
    struct MD5_context *ctxt;
    char *response = NULL;
    unsigned char digest[MD5_DIGEST_LEN];
    char HA1_hex[2 * MD5_DIGEST_LEN + 1];
    char HA2_hex[2 * MD5_DIGEST_LEN + 1];
    char resp_hash_hex[2 * MD5_DIGEST_LEN + 1];
    char nonce[64];
    char realm[128];
    char algorithm[64];
    char qop_options[64];
    int qop_values;
    char cnonce[33];
    char nonceCount[] = "00000001";
    char method[]     = "AUTHENTICATE";
    char qop[]        = DIGEST_QOP_VALUE_STRING_AUTH;
    char *spn         = NULL;
    CURLcode result = auth_decode_digest_md5_message(chlg64, nonce, sizeof(nonce), realm, sizeof(realm), algorithm, sizeof(algorithm), qop_options,
                                                     sizeof(qop_options));
    if(result) return result;
    if(strcmp(algorithm, "md5-sess") != 0) return CURLE_BAD_CONTENT_ENCODING;
    result = auth_digest_get_qop_values(qop_options, &qop_values);
    if(result) return result;
    if(!(qop_values & DIGEST_QOP_VALUE_AUTH)) return CURLE_BAD_CONTENT_ENCODING;
    result = Curl_rand_hex(data, (unsigned char *)cnonce, sizeof(cnonce));
    if(result) return result;
    ctxt = Curl_MD5_init(Curl_DIGEST_MD5);
    if(!ctxt) return CURLE_OUT_OF_MEMORY;
    Curl_MD5_update(ctxt, (const unsigned char*)userp, curlx_uztoui(strlen(userp)));
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)realm, curlx_uztoui(strlen(realm)));
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)passwdp, curlx_uztoui(strlen(passwdp)));
    Curl_MD5_final(ctxt, digest);
    ctxt = Curl_MD5_init(Curl_DIGEST_MD5);
    if(!ctxt) return CURLE_OUT_OF_MEMORY;
    Curl_MD5_update(ctxt, (const unsigned char*)digest, MD5_DIGEST_LEN);
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)nonce, curlx_uztoui(strlen(nonce)));
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)cnonce, curlx_uztoui(strlen(cnonce)));
    Curl_MD5_final(ctxt, digest);
    for(i = 0; i < MD5_DIGEST_LEN; i++) msnprintf(&HA1_hex[2 * i], 3, "%02x", digest[i]);
    spn = Curl_auth_build_spn(service, realm, NULL);
    if(!spn) return CURLE_OUT_OF_MEMORY;
    ctxt = Curl_MD5_init(Curl_DIGEST_MD5);
    if(!ctxt) {
        free(spn);
        return CURLE_OUT_OF_MEMORY;
    }
    Curl_MD5_update(ctxt, (const unsigned char *) method, curlx_uztoui(strlen(method)));
    Curl_MD5_update(ctxt, (const unsigned char *) ":", 1);
    Curl_MD5_update(ctxt, (const unsigned char *) spn, curlx_uztoui(strlen(spn)));
    Curl_MD5_final(ctxt, digest);
    for(i = 0; i < MD5_DIGEST_LEN; i++) msnprintf(&HA2_hex[2 * i], 3, "%02x", digest[i]);
    ctxt = Curl_MD5_init(Curl_DIGEST_MD5);
    if(!ctxt) {
      free(spn);
      return CURLE_OUT_OF_MEMORY;
    }
    Curl_MD5_update(ctxt, (const unsigned char*)HA1_hex, 2 * MD5_DIGEST_LEN);
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)nonce, curlx_uztoui(strlen(nonce)));
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)nonceCount, curlx_uztoui(strlen(nonceCount)));
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)cnonce, curlx_uztoui(strlen(cnonce)));
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)qop, curlx_uztoui(strlen(qop)));
    Curl_MD5_update(ctxt, (const unsigned char*)":", 1);
    Curl_MD5_update(ctxt, (const unsigned char*)HA2_hex, 2 * MD5_DIGEST_LEN);
    Curl_MD5_final(ctxt, digest);
    for(i = 0; i < MD5_DIGEST_LEN; i++) msnprintf(&resp_hash_hex[2 * i], 3, "%02x", digest[i]);
    response = aprintf("username=\"%s\",realm=\"%s\",nonce=\"%s\", cnonce=\"%s\",nc=\"%s\",digest-uri=\"%s\",response=%s, qop=%s", userp, realm, nonce,
                       cnonce, nonceCount, spn, resp_hash_hex, qop);
    free(spn);
    if(!response) return CURLE_OUT_OF_MEMORY;
    result = Curl_base64_encode(data, response, 0, outptr, outlen);
    free(response);
    return result;
}
CURLcode Curl_auth_decode_digest_http_message(const char *chlg, struct digestdata *digest) {
    bool before = FALSE;
    bool foundAuth = FALSE;
    bool foundAuthInt = FALSE;
    char *token = NULL;
    char *tmp = NULL;
    if(digest->nonce) before = TRUE;
    Curl_auth_digest_cleanup(digest);
    for( ; ; ) {
        char value[DIGEST_MAX_VALUE_LENGTH];
        char content[DIGEST_MAX_CONTENT_LENGTH];
        while(*chlg && ISSPACE(*chlg)) chlg++;
        if(Curl_auth_digest_get_pair(chlg, value, content, &chlg)) {
            if(strcasecompare(value, "nonce")) {
                free(digest->nonce);
                digest->nonce = strdup(content);
                if(!digest->nonce) return CURLE_OUT_OF_MEMORY;
            } else if(strcasecompare(value, "stale")) {
                if(strcasecompare(content, "true")) {
                    digest->stale = TRUE;
                    digest->nc = 1;
                }
            } else if(strcasecompare(value, "realm")) {
                free(digest->realm);
                digest->realm = strdup(content);
                if(!digest->realm) return CURLE_OUT_OF_MEMORY;
            } else if(strcasecompare(value, "opaque")) {
                free(digest->opaque);
                digest->opaque = strdup(content);
                if(!digest->opaque) return CURLE_OUT_OF_MEMORY;
            } else if(strcasecompare(value, "qop")) {
                char *tok_buf = NULL;
                tmp = strdup(content);
                if(!tmp) return CURLE_OUT_OF_MEMORY;
                token = strtok_r(tmp, ",", &tok_buf);
                while(token != NULL) {
                    if(strcasecompare(token, DIGEST_QOP_VALUE_STRING_AUTH)) foundAuth = TRUE;
                    else if(strcasecompare(token, DIGEST_QOP_VALUE_STRING_AUTH_INT)) foundAuthInt = TRUE;
                    token = strtok_r(NULL, ",", &tok_buf);
                }
                free(tmp);
                if(foundAuth) {
                    free(digest->qop);
                    digest->qop = strdup(DIGEST_QOP_VALUE_STRING_AUTH);
                    if(!digest->qop) return CURLE_OUT_OF_MEMORY;
                } else if(foundAuthInt) {
                    free(digest->qop);
                    digest->qop = strdup(DIGEST_QOP_VALUE_STRING_AUTH_INT);
                    if(!digest->qop) return CURLE_OUT_OF_MEMORY;
                }
            } else if(strcasecompare(value, "algorithm")) {
                free(digest->algorithm);
                digest->algorithm = strdup(content);
                if(!digest->algorithm) return CURLE_OUT_OF_MEMORY;
                if(strcasecompare(content, "MD5-sess")) digest->algo = CURLDIGESTALGO_MD5SESS;
                else if(strcasecompare(content, "MD5")) digest->algo = CURLDIGESTALGO_MD5;
                else if(strcasecompare(content, "SHA-256")) digest->algo = CURLDIGESTALGO_SHA256;
                else if(strcasecompare(content, "SHA-256-SESS")) digest->algo = CURLDIGESTALGO_SHA256SESS;
                else if(strcasecompare(content, "SHA-512-256")) digest->algo = CURLDIGESTALGO_SHA512_256;
                else if(strcasecompare(content, "SHA-512-256-SESS")) digest->algo = CURLDIGESTALGO_SHA512_256SESS;
                else return CURLE_BAD_CONTENT_ENCODING;
            } else if(strcasecompare(value, "userhash")) {
                if(strcasecompare(content, "true")) digest->userhash = TRUE;
            } else {
                /* Unknown specifier, ignore it! */
            }
        } else break;
        while(*chlg && ISSPACE(*chlg)) chlg++;
        if(',' == *chlg) chlg++;
    }
    if(before && !digest->stale) return CURLE_BAD_CONTENT_ENCODING;
    if(!digest->nonce) return CURLE_BAD_CONTENT_ENCODING;
    return CURLE_OK;
}
static CURLcode auth_create_digest_http_message(struct Curl_easy *data, const char *userp, const char *passwdp, const unsigned char *request,
                                                const unsigned char *uripath, struct digestdata *digest, char **outptr, size_t *outlen,
                                                void (*convert_to_ascii)(unsigned char*, unsigned char*),
                                                void (*hash)(unsigned char*, const unsigned char*, const size_t)) {
    CURLcode result;
    unsigned char hashbuf[32];
    unsigned char request_digest[65];
    unsigned char ha1[65];
    unsigned char ha2[65];
    char userh[65];
    char *cnonce = NULL;
    size_t cnonce_sz = 0;
    char *userp_quoted;
    char *response = NULL;
    char *hashthis = NULL;
    char *tmp = NULL;
    if(!digest->nc) digest->nc = 1;
    if(!digest->cnonce) {
        char cnoncebuf[33];
        result = Curl_rand_hex(data, (unsigned char *)cnoncebuf, sizeof(cnoncebuf));
        if(result) return result;
        result = Curl_base64_encode(data, cnoncebuf, strlen(cnoncebuf), &cnonce, &cnonce_sz);
        if(result) return result;
        digest->cnonce = cnonce;
    }
    if(digest->userhash) {
        hashthis = aprintf("%s:%s", userp, digest->realm);
        if(!hashthis) return CURLE_OUT_OF_MEMORY;
        CURL_OUTPUT_DIGEST_CONV(data, hashthis);
        hash(hashbuf, (unsigned char *) hashthis, strlen(hashthis));
        free(hashthis);
        convert_to_ascii(hashbuf, (unsigned char *)userh);
    }
    hashthis = aprintf("%s:%s:%s", digest->userhash ? userh : userp, digest->realm, passwdp);
    if(!hashthis) return CURLE_OUT_OF_MEMORY;
    CURL_OUTPUT_DIGEST_CONV(data, hashthis);
    hash(hashbuf, (unsigned char *) hashthis, strlen(hashthis));
    free(hashthis);
    convert_to_ascii(hashbuf, ha1);
    if(digest->algo == CURLDIGESTALGO_MD5SESS || digest->algo == CURLDIGESTALGO_SHA256SESS || digest->algo == CURLDIGESTALGO_SHA512_256SESS) {
        tmp = aprintf("%s:%s:%s", ha1, digest->nonce, digest->cnonce);
        if(!tmp) return CURLE_OUT_OF_MEMORY;
        CURL_OUTPUT_DIGEST_CONV(data, tmp);
        hash(hashbuf, (unsigned char *) tmp, strlen(tmp));
        free(tmp);
        convert_to_ascii(hashbuf, ha1);
    }
    hashthis = aprintf("%s:%s", request, uripath);
    if(!hashthis) return CURLE_OUT_OF_MEMORY;
    if(digest->qop && strcasecompare(digest->qop, "auth-int")) {
        char hashed[65];
        char *hashthis2;
        hash(hashbuf, (const unsigned char *)"", 0);
        convert_to_ascii(hashbuf, (unsigned char *)hashed);
        hashthis2 = aprintf("%s:%s", hashthis, hashed);
        free(hashthis);
        hashthis = hashthis2;
    }
    if(!hashthis) return CURLE_OUT_OF_MEMORY;
    CURL_OUTPUT_DIGEST_CONV(data, hashthis);
    hash(hashbuf, (unsigned char *) hashthis, strlen(hashthis));
    free(hashthis);
    convert_to_ascii(hashbuf, ha2);
    if(digest->qop) hashthis = aprintf("%s:%s:%08x:%s:%s:%s", ha1, digest->nonce, digest->nc, digest->cnonce, digest->qop, ha2);
    else hashthis = aprintf("%s:%s:%s", ha1, digest->nonce, ha2);
    if(!hashthis) return CURLE_OUT_OF_MEMORY;
    CURL_OUTPUT_DIGEST_CONV(data, hashthis);
    hash(hashbuf, (unsigned char *) hashthis, strlen(hashthis));
    free(hashthis);
    convert_to_ascii(hashbuf, request_digest);
    userp_quoted = auth_digest_string_quoted(digest->userhash ? userh : userp);
    if(!userp_quoted) return CURLE_OUT_OF_MEMORY;
    if(digest->qop) {
        response = aprintf("username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", cnonce=\"%s\", nc=%08x, qop=%s, response=\"%s\"", userp_quoted,
                           digest->realm, digest->nonce, uripath, digest->cnonce, digest->nc, digest->qop, request_digest);
        if(strcasecompare(digest->qop, "auth")) digest->nc++;
    } else {
        response = aprintf("username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", userp_quoted, digest->realm, digest->nonce,
                           uripath, request_digest);
    }
    free(userp_quoted);
    if(!response) return CURLE_OUT_OF_MEMORY;
    if(digest->opaque) {
        tmp = aprintf("%s, opaque=\"%s\"", response, digest->opaque);
        free(response);
        if(!tmp) return CURLE_OUT_OF_MEMORY;
        response = tmp;
    }
    if(digest->algorithm) {
        tmp = aprintf("%s, algorithm=%s", response, digest->algorithm);
        free(response);
        if(!tmp) return CURLE_OUT_OF_MEMORY;
        response = tmp;
    }
    if(digest->userhash) {
        tmp = aprintf("%s, userhash=true", response);
        free(response);
        if(!tmp) return CURLE_OUT_OF_MEMORY;
        response = tmp;
    }
    *outptr = response;
    *outlen = strlen(response);
    return CURLE_OK;
}
CURLcode Curl_auth_create_digest_http_message(struct Curl_easy *data, const char *userp, const char *passwdp, const unsigned char *request,
                                              const unsigned char *uripath, struct digestdata *digest, char **outptr, size_t *outlen) {
    switch(digest->algo) {
        case CURLDIGESTALGO_MD5: case CURLDIGESTALGO_MD5SESS:
            return auth_create_digest_http_message(data, userp, passwdp, request, uripath, digest, outptr, outlen, auth_digest_md5_to_ascii, Curl_md5it);
        case CURLDIGESTALGO_SHA256: case CURLDIGESTALGO_SHA256SESS: case CURLDIGESTALGO_SHA512_256: case CURLDIGESTALGO_SHA512_256SESS:
            return auth_create_digest_http_message(data, userp, passwdp, request, uripath, digest, outptr, outlen, auth_digest_sha256_to_ascii, Curl_sha256it);
        default: return CURLE_UNSUPPORTED_PROTOCOL;
    }
}
void Curl_auth_digest_cleanup(struct digestdata *digest) {
    Curl_safefree(digest->nonce);
    Curl_safefree(digest->cnonce);
    Curl_safefree(digest->realm);
    Curl_safefree(digest->opaque);
    Curl_safefree(digest->qop);
    Curl_safefree(digest->algorithm);
    digest->nc = 0;
    digest->algo = CURLDIGESTALGO_MD5;
    digest->stale = FALSE;
    digest->userhash = FALSE;
}
#endif
#endif