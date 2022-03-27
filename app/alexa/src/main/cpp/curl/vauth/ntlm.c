#include "ntlm.h"

#if defined(USE_NTLM) && !defined(USE_WINDOWS_SSPI)
#define DEBUG_ME 0
#include "../urldata.h"
#include "../non-ascii.h"
#include "../sendf.h"
#include "../curl_base64.h"
#include "../curl_ntlm_core.h"
#include "../curl_gethostname.h"
#include "../curl_multibyte.h"
#include "../curl_md5.h"
#include "../warnless.h"
#include "../rand.h"
#include "../vtls/vtls.h"
#if defined(NTLM_NEEDS_NSS_INIT)
#include "../vtls/nssg.h"
#endif
#define BUILDING_CURL_NTLM_MSGS_C
#include "../curl_endian.h"
#include "../curl_printf.h"
#include "../curl_memory.h"
#include "../memdebug.h"
#include "vauth.h"
#include "ntlm.h"

#define NTLMSSP_SIGNATURE "\x4e\x54\x4c\x4d\x53\x53\x50"
#define SHORTPAIR(x) ((int)((x) & 0xff)), ((int)(((x) >> 8) & 0xff))
#define LONGQUARTET(x) ((int)((x) & 0xff)), ((int)(((x) >> 8) & 0xff)), ((int)(((x) >> 16) & 0xff)), ((int)(((x) >> 24) & 0xff))

#if DEBUG_ME
# define DEBUG_OUT(x) x
static void ntlm_print_flags(FILE *handle, unsigned long flags) {
    if(flags & NTLMFLAG_NEGOTIATE_UNICODE) fprintf(handle, "NTLMFLAG_NEGOTIATE_UNICODE ");
    if(flags & NTLMFLAG_NEGOTIATE_OEM) fprintf(handle, "NTLMFLAG_NEGOTIATE_OEM ");
    if(flags & NTLMFLAG_REQUEST_TARGET) fprintf(handle, "NTLMFLAG_REQUEST_TARGET ");
    if(flags & (1<<3)) fprintf(handle, "NTLMFLAG_UNKNOWN_3 ");
    if(flags & NTLMFLAG_NEGOTIATE_SIGN) fprintf(handle, "NTLMFLAG_NEGOTIATE_SIGN ");
    if(flags & NTLMFLAG_NEGOTIATE_SEAL) fprintf(handle, "NTLMFLAG_NEGOTIATE_SEAL ");
    if(flags & NTLMFLAG_NEGOTIATE_DATAGRAM_STYLE) fprintf(handle, "NTLMFLAG_NEGOTIATE_DATAGRAM_STYLE ");
    if(flags & NTLMFLAG_NEGOTIATE_LM_KEY) fprintf(handle, "NTLMFLAG_NEGOTIATE_LM_KEY ");
    if(flags & NTLMFLAG_NEGOTIATE_NETWARE) fprintf(handle, "NTLMFLAG_NEGOTIATE_NETWARE ");
    if(flags & NTLMFLAG_NEGOTIATE_NTLM_KEY) fprintf(handle, "NTLMFLAG_NEGOTIATE_NTLM_KEY ");
    if(flags & (1<<10)) fprintf(handle, "NTLMFLAG_UNKNOWN_10 ");
    if(flags & NTLMFLAG_NEGOTIATE_ANONYMOUS) fprintf(handle, "NTLMFLAG_NEGOTIATE_ANONYMOUS ");
    if(flags & NTLMFLAG_NEGOTIATE_DOMAIN_SUPPLIED) fprintf(handle, "NTLMFLAG_NEGOTIATE_DOMAIN_SUPPLIED ");
    if(flags & NTLMFLAG_NEGOTIATE_WORKSTATION_SUPPLIED) fprintf(handle, "NTLMFLAG_NEGOTIATE_WORKSTATION_SUPPLIED ");
    if(flags & NTLMFLAG_NEGOTIATE_LOCAL_CALL) fprintf(handle, "NTLMFLAG_NEGOTIATE_LOCAL_CALL ");
    if(flags & NTLMFLAG_NEGOTIATE_ALWAYS_SIGN) fprintf(handle, "NTLMFLAG_NEGOTIATE_ALWAYS_SIGN ");
    if(flags & NTLMFLAG_TARGET_TYPE_DOMAIN) fprintf(handle, "NTLMFLAG_TARGET_TYPE_DOMAIN ");
    if(flags & NTLMFLAG_TARGET_TYPE_SERVER) fprintf(handle, "NTLMFLAG_TARGET_TYPE_SERVER ");
    if(flags & NTLMFLAG_TARGET_TYPE_SHARE) fprintf(handle, "NTLMFLAG_TARGET_TYPE_SHARE ");
    if(flags & NTLMFLAG_NEGOTIATE_NTLM2_KEY) fprintf(handle, "NTLMFLAG_NEGOTIATE_NTLM2_KEY ");
    if(flags & NTLMFLAG_REQUEST_INIT_RESPONSE) fprintf(handle, "NTLMFLAG_REQUEST_INIT_RESPONSE ");
    if(flags & NTLMFLAG_REQUEST_ACCEPT_RESPONSE) fprintf(handle, "NTLMFLAG_REQUEST_ACCEPT_RESPONSE ");
    if(flags & NTLMFLAG_REQUEST_NONNT_SESSION_KEY) fprintf(handle, "NTLMFLAG_REQUEST_NONNT_SESSION_KEY ");
    if(flags & NTLMFLAG_NEGOTIATE_TARGET_INFO) fprintf(handle, "NTLMFLAG_NEGOTIATE_TARGET_INFO ");
    if(flags & (1<<24)) fprintf(handle, "NTLMFLAG_UNKNOWN_24 ");
    if(flags & (1<<25)) fprintf(handle, "NTLMFLAG_UNKNOWN_25 ");
    if(flags & (1<<26)) fprintf(handle, "NTLMFLAG_UNKNOWN_26 ");
    if(flags & (1<<27)) fprintf(handle, "NTLMFLAG_UNKNOWN_27 ");
    if(flags & (1<<28)) fprintf(handle, "NTLMFLAG_UNKNOWN_28 ");
    if(flags & NTLMFLAG_NEGOTIATE_128) fprintf(handle, "NTLMFLAG_NEGOTIATE_128 ");
    if(flags & NTLMFLAG_NEGOTIATE_KEY_EXCHANGE) fprintf(handle, "NTLMFLAG_NEGOTIATE_KEY_EXCHANGE ");
    if(flags & NTLMFLAG_NEGOTIATE_56) fprintf(handle, "NTLMFLAG_NEGOTIATE_56 ");
}
static void ntlm_print_hex(FILE *handle, const char *buf, size_t len) {
    const char *p = buf;
    (void) handle;
    fprintf(stderr, "0x");
    while(len-- > 0) fprintf(stderr, "%02.2x", (unsigned int)*p++);
}
#else
#define DEBUG_OUT(x) Curl_nop_stmt
#endif
static CURLcode ntlm_decode_type2_target(struct Curl_easy *data, unsigned char *buffer, size_t size, struct ntlmdata *ntlm) {
    unsigned short target_info_len = 0;
    unsigned int target_info_offset = 0;
#if defined(CURL_DISABLE_VERBOSE_STRINGS)
    (void) data;
#endif
    if(size >= 48) {
        target_info_len = Curl_read16_le(&buffer[40]);
        target_info_offset = Curl_read32_le(&buffer[44]);
        if(target_info_len > 0) {
            if((target_info_offset >= size) || ((target_info_offset + target_info_len) > size) || (target_info_offset < 48)) {
                infof(data, "NTLM handshake failure (bad type-2 message). Target Info Offset Len is set incorrect by the peer\n");
                return CURLE_BAD_CONTENT_ENCODING;
            }
            ntlm->target_info = malloc(target_info_len);
            if(!ntlm->target_info) return CURLE_OUT_OF_MEMORY;
            memcpy(ntlm->target_info, &buffer[target_info_offset], target_info_len);
        }
    }
    ntlm->target_info_len = target_info_len;
    return CURLE_OK;
}
bool Curl_auth_is_ntlm_supported(void) {
  return TRUE;
}
CURLcode Curl_auth_decode_ntlm_type2_message(struct Curl_easy *data, const char *type2msg, struct ntlmdata *ntlm) {
    static const char type2_marker[] = { 0x02, 0x00, 0x00, 0x00 };
    CURLcode result = CURLE_OK;
    unsigned char *type2 = NULL;
    size_t type2_len = 0;
#if defined(NTLM_NEEDS_NSS_INIT)
    result = Curl_nss_force_init(data);
    if(result) return result;
#elif defined(CURL_DISABLE_VERBOSE_STRINGS)
    (void)data;
#endif
    if(strlen(type2msg) && *type2msg != '=') {
        result = Curl_base64_decode(type2msg, &type2, &type2_len);
        if(result) return result;
    }
    if(!type2) {
        infof(data, "NTLM handshake failure (empty type-2 message)\n");
        return CURLE_BAD_CONTENT_ENCODING;
    }
    ntlm->flags = 0;
    if((type2_len < 32) || (memcmp(type2, NTLMSSP_SIGNATURE, 8) != 0) || (memcmp(type2 + 8, type2_marker, sizeof(type2_marker)) != 0)) {
        free(type2);
        infof(data, "NTLM handshake failure (bad type-2 message)\n");
        return CURLE_BAD_CONTENT_ENCODING;
    }
    ntlm->flags = Curl_read32_le(&type2[20]);
    memcpy(ntlm->nonce, &type2[24], 8);
    if(ntlm->flags & NTLMFLAG_NEGOTIATE_TARGET_INFO) {
        result = ntlm_decode_type2_target(data, type2, type2_len, ntlm);
        if(result) {
            free(type2);
            infof(data, "NTLM handshake failure (bad type-2 message)\n");
            return result;
        }
    }
    DEBUG_OUT({
        fprintf(stderr, "**** TYPE2 header flags=0x%08.8lx ", ntlm->flags);
        ntlm_print_flags(stderr, ntlm->flags);
        fprintf(stderr, "\n                  nonce=");
        ntlm_print_hex(stderr, (char *)ntlm->nonce, 8);
        fprintf(stderr, "\n****\n");
        fprintf(stderr, "**** Header %s\n ", header);
    });
    free(type2);
    return result;
}
static void unicodecpy(unsigned char *dest, const char *src, size_t length) {
    size_t i;
    for(i = 0; i < length; i++) {
        dest[2 * i] = (unsigned char)src[i];
        dest[2 * i + 1] = '\0';
    }
}
CURLcode Curl_auth_create_ntlm_type1_message(struct Curl_easy *data, const char *userp, const char *passwdp, const char *service, const char *hostname,
                                             struct ntlmdata *ntlm, char **outptr, size_t *outlen) {
    size_t size;
    unsigned char ntlmbuf[NTLM_BUFSIZE];
    const char *host = "";
    const char *domain = "";
    size_t hostlen = 0;
    size_t domlen = 0;
    size_t hostoff = 0;
    size_t domoff = hostoff + hostlen;
    (void)userp;
    (void)passwdp;
    (void)service,
    (void)hostname,
    Curl_auth_cleanup_ntlm(ntlm);
#if defined(USE_NTRESPONSES) && defined(USE_NTLM2SESSION)
    #define NTLM2FLAG NTLMFLAG_NEGOTIATE_NTLM2_KEY
#else
    #define NTLM2FLAG 0
#endif
    msnprintf((char *)ntlmbuf, NTLM_BUFSIZE,
              NTLMSSP_SIGNATURE "%c\x01%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%s%s", 0, 0, 0, 0,
              LONGQUARTET(NTLMFLAG_NEGOTIATE_OEM | NTLMFLAG_REQUEST_TARGET | NTLMFLAG_NEGOTIATE_NTLM_KEY | NTLM2FLAG | NTLMFLAG_NEGOTIATE_ALWAYS_SIGN),
              SHORTPAIR(domlen), SHORTPAIR(domlen), SHORTPAIR(domoff), 0, 0, SHORTPAIR(hostlen), SHORTPAIR(hostlen), SHORTPAIR(hostoff), 0, 0,
              host, domain);
    size = 32 + hostlen + domlen;
    DEBUG_OUT({
        fprintf(stderr, "* TYPE1 header flags=0x%02.2x%02.2x%02.2x%02.2x 0x%08.8x ",
                LONGQUARTET(NTLMFLAG_NEGOTIATE_OEM | NTLMFLAG_REQUEST_TARGET | NTLMFLAG_NEGOTIATE_NTLM_KEY | NTLM2FLAG | NTLMFLAG_NEGOTIATE_ALWAYS_SIGN),
                NTLMFLAG_NEGOTIATE_OEM | NTLMFLAG_REQUEST_TARGET | NTLMFLAG_NEGOTIATE_NTLM_KEY | NTLM2FLAG | NTLMFLAG_NEGOTIATE_ALWAYS_SIGN);
        ntlm_print_flags(stderr, NTLMFLAG_NEGOTIATE_OEM | NTLMFLAG_REQUEST_TARGET | NTLMFLAG_NEGOTIATE_NTLM_KEY | NTLM2FLAG | NTLMFLAG_NEGOTIATE_ALWAYS_SIGN);
        fprintf(stderr, "\n****\n");
    });
    return Curl_base64_encode(data, (char *)ntlmbuf, size, outptr, outlen);
}
CURLcode Curl_auth_create_ntlm_type3_message(struct Curl_easy *data, const char *userp, const char *passwdp, struct ntlmdata *ntlm, char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    size_t size;
    unsigned char ntlmbuf[NTLM_BUFSIZE];
    int lmrespoff;
    unsigned char lmresp[24];
#ifdef USE_NTRESPONSES
    int ntrespoff;
    unsigned int ntresplen = 24;
    unsigned char ntresp[24];
    unsigned char *ptr_ntresp = &ntresp[0];
    unsigned char *ntlmv2resp = NULL;
#endif
    bool unicode = (ntlm->flags & NTLMFLAG_NEGOTIATE_UNICODE) ? TRUE : FALSE;
    char host[HOSTNAME_MAX + 1] = "";
    const char *user;
    const char *domain = "";
    size_t hostoff = 0;
    size_t useroff = 0;
    size_t domoff = 0;
    size_t hostlen = 0;
    size_t userlen = 0;
    size_t domlen = 0;
    user = strchr(userp, '\\');
    if(!user) user = strchr(userp, '/');
    if(user) {
        domain = userp;
        domlen = (user - domain);
        user++;
    } else user = userp;
    userlen = strlen(user);
    if(Curl_gethostname(host, sizeof(host))) {
        infof(data, "gethostname() failed, continuing without!\n");
        hostlen = 0;
    } else hostlen = strlen(host);
#if defined(USE_NTRESPONSES) && defined(USE_NTLM_V2)
    if(ntlm->flags & NTLMFLAG_NEGOTIATE_NTLM2_KEY) {
        unsigned char ntbuffer[0x18];
        unsigned char entropy[8];
        unsigned char ntlmv2hash[0x18];
        result = Curl_rand(data, entropy, 8);
        if(result) return result;
        result = Curl_ntlm_core_mk_nt_hash(data, passwdp, ntbuffer);
        if(result) return result;
        result = Curl_ntlm_core_mk_ntlmv2_hash(user, userlen, domain, domlen, ntbuffer, ntlmv2hash);
        if(result) return result;
        result = Curl_ntlm_core_mk_lmv2_resp(ntlmv2hash, entropy, &ntlm->nonce[0], lmresp);
        if(result) return result;
        result = Curl_ntlm_core_mk_ntlmv2_resp(ntlmv2hash, entropy, ntlm, &ntlmv2resp, &ntresplen);
        if(result) return result;
        ptr_ntresp = ntlmv2resp;
    } else
#endif
#if defined(USE_NTRESPONSES) && defined(USE_NTLM2SESSION)
    #define CURL_MD5_DIGEST_LENGTH 16
    if(ntlm->flags & NTLMFLAG_NEGOTIATE_NTLM_KEY) {
        unsigned char ntbuffer[0x18];
        unsigned char tmp[0x18];
        unsigned char md5sum[CURL_MD5_DIGEST_LENGTH];
        unsigned char entropy[8];
        result = Curl_rand(data, entropy, 8);
        if(result) return result;
        memcpy(lmresp, entropy, 8);
        memset(lmresp + 8, 0, 0x10);
        memcpy(tmp, &ntlm->nonce[0], 8);
        memcpy(tmp + 8, entropy, 8);
        Curl_md5it(md5sum, tmp, 16);
        result = Curl_ntlm_core_mk_nt_hash(data, passwdp, ntbuffer);
        if(result) return result;
        Curl_ntlm_core_lm_resp(ntbuffer, md5sum, ntresp);
    } else
#endif
    {
    #ifdef USE_NTRESPONSES
        unsigned char ntbuffer[0x18];
    #endif
        unsigned char lmbuffer[0x18];
    #ifdef USE_NTRESPONSES
        result = Curl_ntlm_core_mk_nt_hash(data, passwdp, ntbuffer);
        if(result) return result;
        Curl_ntlm_core_lm_resp(ntbuffer, &ntlm->nonce[0], ntresp);
    #endif
        result = Curl_ntlm_core_mk_lm_hash(data, passwdp, lmbuffer);
        if(result) return result;
        Curl_ntlm_core_lm_resp(lmbuffer, &ntlm->nonce[0], lmresp);
    }
    if(unicode) {
        domlen = domlen * 2;
        userlen = userlen * 2;
        hostlen = hostlen * 2;
    }
    lmrespoff = 64;
#ifdef USE_NTRESPONSES
    ntrespoff = lmrespoff + 0x18;
    domoff = ntrespoff + ntresplen;
#else
    domoff = lmrespoff + 0x18;
#endif
    useroff = domoff + domlen;
    hostoff = useroff + userlen;
    size = msnprintf((char *)ntlmbuf, NTLM_BUFSIZE,
                   NTLMSSP_SIGNATURE "%c\x03%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                   0, 0, 0, 0, SHORTPAIR(0x18), SHORTPAIR(0x18), SHORTPAIR(lmrespoff), 0x0, 0x0,
                #ifdef USE_NTRESPONSES
                   SHORTPAIR(ntresplen), SHORTPAIR(ntresplen), SHORTPAIR(ntrespoff), 0x0, 0x0,
                #else
                   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                #endif
                   SHORTPAIR(domlen), SHORTPAIR(domlen), SHORTPAIR(domoff), 0x0, 0x0, SHORTPAIR(userlen), SHORTPAIR(userlen), SHORTPAIR(useroff), 0x0, 0x0,
                   SHORTPAIR(hostlen), SHORTPAIR(hostlen), SHORTPAIR(hostoff), 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, LONGQUARTET(ntlm->flags));

    DEBUGASSERT(size == 64);
    DEBUGASSERT(size == (size_t)lmrespoff);
    if(size < (NTLM_BUFSIZE - 0x18)) {
        memcpy(&ntlmbuf[size], lmresp, 0x18);
        size += 0x18;
    }
    DEBUG_OUT({
        fprintf(stderr, "**** TYPE3 header lmresp=");
        ntlm_print_hex(stderr, (char *)&ntlmbuf[lmrespoff], 0x18);
    });
#ifdef USE_NTRESPONSES
    if(ntresplen + size > sizeof(ntlmbuf)) {
        failf(data, "incoming NTLM message too big");
        return CURLE_OUT_OF_MEMORY;
    }
    DEBUGASSERT(size == (size_t)ntrespoff);
    memcpy(&ntlmbuf[size], ptr_ntresp, ntresplen);
    size += ntresplen;
    DEBUG_OUT({
        fprintf(stderr, "\n   ntresp=");
        ntlm_print_hex(stderr, (char *)&ntlmbuf[ntrespoff], ntresplen);
    });
    free(ntlmv2resp);
#endif
    DEBUG_OUT({
        fprintf(stderr, "\n   flags=0x%02.2x%02.2x%02.2x%02.2x 0x%08.8x ",
                LONGQUARTET(ntlm->flags), ntlm->flags);
        ntlm_print_flags(stderr, ntlm->flags);
        fprintf(stderr, "\n****\n");
    });
    if(size + userlen + domlen + hostlen >= NTLM_BUFSIZE) {
        failf(data, "user + domain + host name too big");
        return CURLE_OUT_OF_MEMORY;
    }
    DEBUGASSERT(size == domoff);
    if(unicode) unicodecpy(&ntlmbuf[size], domain, domlen / 2);
    else memcpy(&ntlmbuf[size], domain, domlen);
    size += domlen;
    DEBUGASSERT(size == useroff);
    if(unicode) unicodecpy(&ntlmbuf[size], user, userlen / 2);
    else memcpy(&ntlmbuf[size], user, userlen);
    size += userlen;
    DEBUGASSERT(size == hostoff);
    if(unicode) unicodecpy(&ntlmbuf[size], host, hostlen / 2);
    else memcpy(&ntlmbuf[size], host, hostlen);
    size += hostlen;
    result = Curl_convert_to_network(data, (char *)&ntlmbuf[domoff], size - domoff);
    if(result) return CURLE_CONV_FAILED;
    result = Curl_base64_encode(data, (char *)ntlmbuf, size, outptr, outlen);
    Curl_auth_cleanup_ntlm(ntlm);
    return result;
}
void Curl_auth_cleanup_ntlm(struct ntlmdata *ntlm) {
    Curl_safefree(ntlm->target_info);
    ntlm->target_info_len = 0;
}
#endif