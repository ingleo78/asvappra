#ifndef HEADER_CURL_IMAP_H
#define HEADER_CURL_IMAP_H

#include "pingpong.h"
#include "curl_sasl.h"

typedef enum {
    IMAP_STOP,
    IMAP_SERVERGREET,
    IMAP_CAPABILITY,
    IMAP_STARTTLS,
    IMAP_UPGRADETLS,
    IMAP_AUTHENTICATE,
    IMAP_LOGIN,
    IMAP_LIST,
    IMAP_SELECT,
    IMAP_FETCH,
    IMAP_FETCH_FINAL,
    IMAP_APPEND,
    IMAP_APPEND_FINAL,
    IMAP_SEARCH,
    IMAP_LOGOUT,
    IMAP_LAST
} imapstate;
struct IMAP {
    curl_pp_transfer transfer;
    char *mailbox;
    char *uidvalidity;
    char *uid;
    char *mindex;
    char *section;
    char *partial;
    char *query;
    char *custom;
    char *custom_params;
};
struct imap_conn {
    struct pingpong pp;
    imapstate state;
    bool ssldone;
    bool preauth;
    struct SASL sasl;
    unsigned int preftype;
    int cmdid;
    char resptag[5];
    bool tls_supported;
    bool login_disabled;
    bool ir_supported;
    char *mailbox;
    char *mailbox_uidvalidity;
};
extern const struct Curl_handler Curl_handler_imap;
extern const struct Curl_handler Curl_handler_imaps;
#define IMAP_TYPE_CLEARTEXT (1 << 0)
#define IMAP_TYPE_SASL      (1 << 1)
#define IMAP_TYPE_NONE      0
#define IMAP_TYPE_ANY       ~0U
#endif