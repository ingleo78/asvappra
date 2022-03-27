#include <stdbool.h>
#include "curl_setup.h"
#if !defined(CURL_DISABLE_IMAP) || !defined(CURL_DISABLE_SMTP) || !defined(CURL_DISABLE_POP3)
#include "curl.h"
#include "urldata.h"
#include "curl_base64.h"
#include "curl_md5.h"
#include "vauth/vauth.h"
#include "vtls/vtls.h"
#include "curl_hmac.h"
#include "curl_sasl.h"
#include "warnless.h"
#include "strtok.h"
#include "sendf.h"
#include "non-ascii.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static const struct {
    const char   *name;
    size_t        len;
    unsigned int  bit;
} mechtable[] = {
    { "LOGIN",        5,  SASL_MECH_LOGIN },
    { "PLAIN",        5,  SASL_MECH_PLAIN },
    { "CRAM-MD5",     8,  SASL_MECH_CRAM_MD5 },
    { "DIGEST-MD5",   10, SASL_MECH_DIGEST_MD5 },
    { "GSSAPI",       6,  SASL_MECH_GSSAPI },
    { "EXTERNAL",     8,  SASL_MECH_EXTERNAL },
    { "NTLM",         4,  SASL_MECH_NTLM },
    { "XOAUTH2",      7,  SASL_MECH_XOAUTH2 },
    { "OAUTHBEARER",  11, SASL_MECH_OAUTHBEARER },
    { ZERO_NULL,      0,  0 }
};
void Curl_sasl_cleanup(struct connectdata *conn, unsigned int authused) {
#if defined(USE_KERBEROS5)
    if (authused == SASL_MECH_GSSAPI) Curl_auth_cleanup_gssapi(&conn->krb5);
#endif
#if defined(USE_NTLM)
    if (authused == SASL_MECH_NTLM) Curl_auth_cleanup_ntlm(&conn->ntlm);
#endif
#if !defined(USE_KERBEROS5) && !defined(USE_NTLM)
    (void)conn;
    (void)authused;
#endif
}
unsigned int Curl_sasl_decode_mech(const char *ptr, size_t maxlen, size_t *len) {
    unsigned int i;
    char c;
    for (i = 0; mechtable[i].name; i++) {
        if (maxlen >= mechtable[i].len && !memcmp(ptr, mechtable[i].name, mechtable[i].len)) {
            if (len) *len = mechtable[i].len;
            if (maxlen == mechtable[i].len) return mechtable[i].bit;
            c = ptr[mechtable[i].len];
            if (!ISUPPER(c) && !ISDIGIT(c) && c != '-' && c != '_') return mechtable[i].bit;
        }
    }
    return 0;
}
CURLcode Curl_sasl_parse_url_auth_option(struct SASL *sasl, const char *value, size_t len) {
    CURLcode result = CURLE_OK;
    size_t mechlen;
    if (!len) return CURLE_URL_MALFORMAT;
    if (sasl->resetprefs) {
        sasl->resetprefs = FALSE;
        sasl->prefmech = SASL_AUTH_NONE;
    }
    if (!strncmp(value, "*", len)) sasl->prefmech = SASL_AUTH_DEFAULT;
    else {
        unsigned int mechbit = Curl_sasl_decode_mech(value, len, &mechlen);
        if (mechbit && mechlen == len) sasl->prefmech |= mechbit;
        else result = CURLE_URL_MALFORMAT;
    }
    return result;
}
void Curl_sasl_init(struct SASL *sasl, const struct SASLproto *params) {
    sasl->params = params;
    sasl->state = SASL_STOP;
    sasl->authmechs = SASL_AUTH_NONE;
    sasl->prefmech = SASL_AUTH_DEFAULT;
    sasl->authused = SASL_AUTH_NONE;
    sasl->resetprefs = TRUE;
    sasl->mutual_auth = FALSE;
    sasl->force_ir = FALSE;
}
static void state(struct SASL *sasl, struct connectdata *conn, saslstate newstate) {
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
    static const char * const names[]={
        "STOP",
        "PLAIN",
        "LOGIN",
        "LOGIN_PASSWD",
        "EXTERNAL",
        "CRAMMD5",
        "DIGESTMD5",
        "DIGESTMD5_RESP",
        "NTLM",
        "NTLM_TYPE2MSG",
        "GSSAPI",
        "GSSAPI_TOKEN",
        "GSSAPI_NO_DATA",
        "OAUTH2",
        "OAUTH2_RESP",
        "CANCEL",
        "FINAL"
    };
    if(sasl->state != newstate) infof(conn->data, "SASL %p state change from %s to %s\n", (void *)sasl, names[sasl->state], names[newstate]);
#else
    (void) conn;
#endif
    sasl->state = newstate;
}
bool Curl_sasl_can_authenticate(struct SASL *sasl, struct connectdata *conn) {
    if(conn->bits.user_passwd) return TRUE;
    if(sasl->authmechs & sasl->prefmech & SASL_MECH_EXTERNAL) return TRUE;
    return FALSE;
}
CURLcode Curl_sasl_start(struct SASL *sasl, struct connectdata *conn, bool force_ir, saslprogress *progress) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    unsigned int enabledmechs;
    const char *mech = NULL;
    char *resp = NULL;
    size_t len = 0;
    saslstate state1 = SASL_STOP;
    saslstate state2 = SASL_FINAL;
#ifndef CURL_DISABLE_PROXY
    const char * const hostname = SSL_IS_PROXY() ? conn->http_proxy.host.name : conn->host.name;
    const long int port = SSL_IS_PROXY() ? conn->port : conn->remote_port;
#else
    const char * const hostname = conn->host.name;
    const long int port = conn->remote_port;
#endif
#if defined(USE_KERBEROS5) || defined(USE_NTLM)
    const char *service = data->set.str[STRING_SERVICE_NAME] ? data->set.str[STRING_SERVICE_NAME] : sasl->params->service;
#endif
    const char *oauth_bearer = data->set.str[STRING_BEARER];
    sasl->force_ir = force_ir;
    sasl->authused = 0;
    enabledmechs = sasl->authmechs & sasl->prefmech;
    *progress = SASL_IDLE;
    if((enabledmechs & SASL_MECH_EXTERNAL) && !conn->passwd[0]) {
        mech = SASL_MECH_STRING_EXTERNAL;
        state1 = SASL_EXTERNAL;
        sasl->authused = SASL_MECH_EXTERNAL;
        if(force_ir || data->set.sasl_ir) result = Curl_auth_create_external_message(data, conn->user, &resp, &len);
    } else if(conn->bits.user_passwd) {
    #if defined(USE_KERBEROS5)
        if((enabledmechs & SASL_MECH_GSSAPI) && Curl_auth_is_gssapi_supported() && Curl_auth_user_contains_domain(conn->user)) {
            sasl->mutual_auth = FALSE;
            mech = SASL_MECH_STRING_GSSAPI;
            state1 = SASL_GSSAPI;
            state2 = SASL_GSSAPI_TOKEN;
            sasl->authused = SASL_MECH_GSSAPI;
            if(force_ir || data->set.sasl_ir)
                result = Curl_auth_create_gssapi_user_message(data, conn->user, conn->passwd, service, data->conn->host.name, sasl->mutual_auth,
                                                              NULL, &conn->krb5, &resp, &len);
        } else
    #endif
    #ifndef CURL_DISABLE_CRYPTO_AUTH
        if((enabledmechs & SASL_MECH_DIGEST_MD5) && Curl_auth_is_digest_supported()) {
            mech = SASL_MECH_STRING_DIGEST_MD5;
            state1 = SASL_DIGESTMD5;
            sasl->authused = SASL_MECH_DIGEST_MD5;
        } else if(enabledmechs & SASL_MECH_CRAM_MD5) {
            mech = SASL_MECH_STRING_CRAM_MD5;
            state1 = SASL_CRAMMD5;
            sasl->authused = SASL_MECH_CRAM_MD5;
        } else
    #endif
    #ifdef USE_NTLM
        if((enabledmechs & SASL_MECH_NTLM) && Curl_auth_is_ntlm_supported()) {
            mech = SASL_MECH_STRING_NTLM;
            state1 = SASL_NTLM;
            state2 = SASL_NTLM_TYPE2MSG;
            sasl->authused = SASL_MECH_NTLM;
            if(force_ir || data->set.sasl_ir) result = Curl_auth_create_ntlm_type1_message(data, conn->user, conn->passwd, service, hostname, &conn->ntlm, &resp, &len);
        } else
    #endif
        if((enabledmechs & SASL_MECH_OAUTHBEARER) && oauth_bearer) {
            mech = SASL_MECH_STRING_OAUTHBEARER;
            state1 = SASL_OAUTH2;
            state2 = SASL_OAUTH2_RESP;
            sasl->authused = SASL_MECH_OAUTHBEARER;
            if(force_ir || data->set.sasl_ir) result = Curl_auth_create_oauth_bearer_message(data, conn->user, hostname, port, oauth_bearer, &resp, &len);
        } else if((enabledmechs & SASL_MECH_XOAUTH2) && oauth_bearer) {
            mech = SASL_MECH_STRING_XOAUTH2;
            state1 = SASL_OAUTH2;
            sasl->authused = SASL_MECH_XOAUTH2;
            if(force_ir || data->set.sasl_ir) result = Curl_auth_create_xoauth_bearer_message(data, conn->user, oauth_bearer, &resp, &len);
        } else if(enabledmechs & SASL_MECH_PLAIN) {
            mech = SASL_MECH_STRING_PLAIN;
            state1 = SASL_PLAIN;
            sasl->authused = SASL_MECH_PLAIN;
            if(force_ir || data->set.sasl_ir) result = Curl_auth_create_plain_message(data, conn->sasl_authzid, conn->user, conn->passwd, &resp, &len);
        } else if(enabledmechs & SASL_MECH_LOGIN) {
            mech = SASL_MECH_STRING_LOGIN;
            state1 = SASL_LOGIN;
            state2 = SASL_LOGIN_PASSWD;
            sasl->authused = SASL_MECH_LOGIN;
            if(force_ir || data->set.sasl_ir) result = Curl_auth_create_login_message(data, conn->user, &resp, &len);
        }
    }
    if(!result && mech) {
        if(resp && sasl->params->maxirlen &&
            strlen(mech) + len > sasl->params->maxirlen) {
            free(resp);
            resp = NULL;
        }
        result = sasl->params->sendauth(conn, mech, resp);
        if(!result) {
            *progress = SASL_INPROGRESS;
            state(sasl, conn, resp ? state2 : state1);
        }
    }
    free(resp);
    return result;
}
CURLcode Curl_sasl_continue(struct SASL *sasl, struct connectdata *conn, int code, saslprogress *progress) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    saslstate newstate = SASL_FINAL;
    char *resp = NULL;
#ifndef CURL_DISABLE_PROXY
    const char * const hostname = SSL_IS_PROXY() ? conn->http_proxy.host.name : conn->host.name;
    const long int port = SSL_IS_PROXY() ? conn->port : conn->remote_port;
#else
    const char * const hostname = conn->host.name;
    const long int port = conn->remote_port;
#endif
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
    char *chlg = NULL;
    size_t chlglen = 0;
#endif
#if !defined(CURL_DISABLE_CRYPTO_AUTH) || defined(USE_KERBEROS5) || defined(USE_NTLM)
    const char *service = data->set.str[STRING_SERVICE_NAME] ? data->set.str[STRING_SERVICE_NAME] : sasl->params->service;
    char *serverdata;
#endif
    size_t len = 0;
    const char *oauth_bearer = data->set.str[STRING_BEARER];
    *progress = SASL_INPROGRESS;
    if (sasl->state == SASL_FINAL) {
        if (code != sasl->params->finalcode) result = CURLE_LOGIN_DENIED;
        *progress = SASL_DONE;
        state(sasl, conn, SASL_STOP);
        return result;
    }
    if(sasl->state != SASL_CANCEL && sasl->state != SASL_OAUTH2_RESP &&
        code != sasl->params->contcode) {
        *progress = SASL_DONE;
        state(sasl, conn, SASL_STOP);
        return CURLE_LOGIN_DENIED;
    }
    switch(sasl->state) {
        case SASL_STOP:
            *progress = SASL_DONE;
            return result;
        case SASL_PLAIN:
            result = Curl_auth_create_plain_message(data, conn->sasl_authzid, conn->user, conn->passwd, &resp, &len);
            break;
        case SASL_LOGIN:
            result = Curl_auth_create_login_message(data, conn->user, &resp, &len);
            newstate = SASL_LOGIN_PASSWD;
            break;
        case SASL_LOGIN_PASSWD:
            result = Curl_auth_create_login_message(data, conn->passwd, &resp, &len);
            break;
        case SASL_EXTERNAL:
            result = Curl_auth_create_external_message(data, conn->user, &resp, &len);
            break;
    #ifndef CURL_DISABLE_CRYPTO_AUTH
        case SASL_CRAMMD5:
            sasl->params->getmessage(data->state.buffer, &serverdata);
            result = Curl_auth_decode_cram_md5_message(serverdata, &chlg, &chlglen);
            if (!result) result = Curl_auth_create_cram_md5_message(data, chlg, conn->user, conn->passwd, &resp, &len);
            free(chlg);
            break;
        case SASL_DIGESTMD5:
            sasl->params->getmessage(data->state.buffer, &serverdata);
            result = Curl_auth_create_digest_md5_message(data, serverdata, conn->user, conn->passwd, service, &resp, &len);
            newstate = SASL_DIGESTMD5_RESP;
            break;
        case SASL_DIGESTMD5_RESP:
            resp = strdup("");
            if (!resp) result = CURLE_OUT_OF_MEMORY;
            break;
    #endif
    #ifdef USE_NTLM
        case SASL_NTLM:
            result = Curl_auth_create_ntlm_type1_message(data, conn->user, conn->passwd, service, hostname, &conn->ntlm, &resp, &len);
            newstate = SASL_NTLM_TYPE2MSG;
            break;
        case SASL_NTLM_TYPE2MSG:
            sasl->params->getmessage(data->state.buffer, &serverdata);
            result = Curl_auth_decode_ntlm_type2_message(data, serverdata, &conn->ntlm);
            if (!result) result = Curl_auth_create_ntlm_type3_message(data, conn->user, conn->passwd, &conn->ntlm, &resp, &len);
            break;
    #endif
    #if defined(USE_KERBEROS5)
        case SASL_GSSAPI:
            result = Curl_auth_create_gssapi_user_message(data, conn->user, conn->passwd, service, data->conn->host.name, sasl->mutual_auth, NULL,
                                                          &conn->krb5, &resp, &len);
            newstate = SASL_GSSAPI_TOKEN;
            break;
        case SASL_GSSAPI_TOKEN:
            sasl->params->getmessage(data->state.buffer, &serverdata);
            if (sasl->mutual_auth) {
                result = Curl_auth_create_gssapi_user_message(data, NULL, NULL, NULL, NULL, sasl->mutual_auth, serverdata, &conn->krb5, &resp, &len);
                newstate = SASL_GSSAPI_NO_DATA;
            } else result = Curl_auth_create_gssapi_security_message(data, serverdata, &conn->krb5, &resp, &len);
            break;
        case SASL_GSSAPI_NO_DATA:
            sasl->params->getmessage(data->state.buffer, &serverdata);
            result = Curl_auth_create_gssapi_security_message(data, serverdata, &conn->krb5, &resp, &len);
            break;
    #endif
        case SASL_OAUTH2:
            if(sasl->authused == SASL_MECH_OAUTHBEARER) {
                result = Curl_auth_create_oauth_bearer_message(data, conn->user, hostname, port, oauth_bearer, &resp, &len);
                newstate = SASL_OAUTH2_RESP;
            } else result = Curl_auth_create_xoauth_bearer_message(data, conn->user, oauth_bearer, &resp, &len);
            break;
        case SASL_OAUTH2_RESP:
            if(code == sasl->params->finalcode) {
                *progress = SASL_DONE;
                state(sasl, conn, SASL_STOP);
                return result;
            } else if(code == sasl->params->contcode) {
                resp = strdup("AQ==");
                if(!resp) result = CURLE_OUT_OF_MEMORY;
                break;
            } else {
                *progress = SASL_DONE;
                state(sasl, conn, SASL_STOP);
                return CURLE_LOGIN_DENIED;
            }
        case SASL_CANCEL:
            sasl->authmechs ^= sasl->authused;
            result = Curl_sasl_start(sasl, conn, sasl->force_ir, progress);
            newstate = sasl->state;
            break;
        default:
            failf(data, "Unsupported SASL authentication mechanism");
            result = CURLE_UNSUPPORTED_PROTOCOL;  /* Should not happen */
            break;
    }
    switch(result) {
        case CURLE_BAD_CONTENT_ENCODING:
            result = sasl->params->sendcont(conn, "*");
            newstate = SASL_CANCEL;
            break;
        case CURLE_OK:
            if (resp) result = sasl->params->sendcont(conn, resp);
            break;
        default:
            newstate = SASL_STOP;
            *progress = SASL_DONE;
            break;
    }
    free(resp);
    state(sasl, conn, newstate);
    return result;
}
#endif