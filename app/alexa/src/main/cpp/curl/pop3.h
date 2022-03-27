#ifndef HEADER_CURL_POP3_H
#define HEADER_CURL_POP3_H

#include "pingpong.h"
#include "curl_sasl.h"

typedef enum {
    POP3_STOP,
    POP3_SERVERGREET,
    POP3_CAPA,
    POP3_STARTTLS,
    POP3_UPGRADETLS,
    POP3_AUTH,
    POP3_APOP,
    POP3_USER,
    POP3_PASS,
    POP3_COMMAND,
    POP3_QUIT,
    POP3_LAST
} pop3state;
struct POP3 {
    curl_pp_transfer transfer;
    char *id;
    char *custom;
};
struct pop3_conn {
    struct pingpong pp;
    pop3state state;
    bool ssldone;
    size_t eob;
    size_t strip;
    struct SASL sasl;
    unsigned int authtypes;
    unsigned int preftype;
    char *apoptimestamp;
    bool tls_supported;
};
extern const struct Curl_handler Curl_handler_pop3;
extern const struct Curl_handler Curl_handler_pop3s;
#define POP3_TYPE_CLEARTEXT (1 << 0)
#define POP3_TYPE_APOP      (1 << 1)
#define POP3_TYPE_SASL      (1 << 2)
#define POP3_TYPE_NONE      0
#define POP3_TYPE_ANY       ~0U
#define POP3_EOB "\x0d\x0a\x2e\x0d\x0a"
#define POP3_EOB_LEN 5
CURLcode Curl_pop3_write(struct connectdata *conn, char *str, size_t nread);

#endif