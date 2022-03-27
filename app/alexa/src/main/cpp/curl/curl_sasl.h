#ifndef HEADER_CURL_SASL_H
#define HEADER_CURL_SASL_H

#include "curl.h"

struct Curl_easy;
struct connectdata;

#define SASL_MECH_LOGIN             (1 << 0)
#define SASL_MECH_PLAIN             (1 << 1)
#define SASL_MECH_CRAM_MD5          (1 << 2)
#define SASL_MECH_DIGEST_MD5        (1 << 3)
#define SASL_MECH_GSSAPI            (1 << 4)
#define SASL_MECH_EXTERNAL          (1 << 5)
#define SASL_MECH_NTLM              (1 << 6)
#define SASL_MECH_XOAUTH2           (1 << 7)
#define SASL_MECH_OAUTHBEARER       (1 << 8)
#define SASL_AUTH_NONE          0
#define SASL_AUTH_ANY           ~0U
#define SASL_AUTH_DEFAULT       (SASL_AUTH_ANY & ~SASL_MECH_EXTERNAL)
#define SASL_MECH_STRING_LOGIN        "LOGIN"
#define SASL_MECH_STRING_PLAIN        "PLAIN"
#define SASL_MECH_STRING_CRAM_MD5     "CRAM-MD5"
#define SASL_MECH_STRING_DIGEST_MD5   "DIGEST-MD5"
#define SASL_MECH_STRING_GSSAPI       "GSSAPI"
#define SASL_MECH_STRING_EXTERNAL     "EXTERNAL"
#define SASL_MECH_STRING_NTLM         "NTLM"
#define SASL_MECH_STRING_XOAUTH2      "XOAUTH2"
#define SASL_MECH_STRING_OAUTHBEARER  "OAUTHBEARER"
typedef enum {
    SASL_STOP,
    SASL_PLAIN,
    SASL_LOGIN,
    SASL_LOGIN_PASSWD,
    SASL_EXTERNAL,
    SASL_CRAMMD5,
    SASL_DIGESTMD5,
    SASL_DIGESTMD5_RESP,
    SASL_NTLM,
    SASL_NTLM_TYPE2MSG,
    SASL_GSSAPI,
    SASL_GSSAPI_TOKEN,
    SASL_GSSAPI_NO_DATA,
    SASL_OAUTH2,
    SASL_OAUTH2_RESP,
    SASL_CANCEL,
    SASL_FINAL
} saslstate;
typedef enum {
    SASL_IDLE,
    SASL_INPROGRESS,
    SASL_DONE
} saslprogress;
struct SASLproto {
    const char *service;
    int contcode;
    int finalcode;
    size_t maxirlen;
    CURLcode (*sendauth)(struct connectdata *conn, const char *mech, const char *ir);
    CURLcode (*sendcont)(struct connectdata *conn, const char *contauth);
    void (*getmessage)(char *buffer, char **outptr);
};
struct SASL {
    const struct SASLproto *params;
    saslstate state;
    unsigned int authmechs;
    unsigned int prefmech;
    unsigned int authused;
    bool resetprefs;
    bool mutual_auth;
    bool force_ir;
};
#define sasl_mech_equal(line, wordlen, mech) (wordlen == (sizeof(mech) - 1) / sizeof(char) && !memcmp(line, mech, wordlen))
void Curl_sasl_cleanup(struct connectdata *conn, unsigned int authused);
unsigned int Curl_sasl_decode_mech(const char *ptr, size_t maxlen, size_t *len);
CURLcode Curl_sasl_parse_url_auth_option(struct SASL *sasl, const char *value, size_t len);
void Curl_sasl_init(struct SASL *sasl, const struct SASLproto *params);
bool Curl_sasl_can_authenticate(struct SASL *sasl, struct connectdata *conn);
CURLcode Curl_sasl_start(struct SASL *sasl, struct connectdata *conn, bool force_ir, saslprogress *progress);
CURLcode Curl_sasl_continue(struct SASL *sasl, struct connectdata *conn, int code, saslprogress *progress);

#endif