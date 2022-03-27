#ifndef HEADER_CURL_SMTP_H
#define HEADER_CURL_SMTP_H

#include "pingpong.h"
#include "curl_sasl.h"

typedef enum {
    SMTP_STOP,
    SMTP_SERVERGREET,
    SMTP_EHLO,
    SMTP_HELO,
    SMTP_STARTTLS,
    SMTP_UPGRADETLS,
    SMTP_AUTH,
    SMTP_COMMAND,
    SMTP_MAIL,
    SMTP_RCPT,
    SMTP_DATA,
    SMTP_POSTDATA,
    SMTP_QUIT,
    SMTP_LAST
} smtpstate;
struct SMTP {
    curl_pp_transfer transfer;
    char *custom;
    struct curl_slist *rcpt;
    bool rcpt_had_ok;
    int rcpt_last_error;
    size_t eob;
    bool trailing_crlf;
};
struct smtp_conn {
    struct pingpong pp;
    smtpstate state;
    bool ssldone;
    char *domain;
    struct SASL sasl;
    bool tls_supported;
    bool size_supported;
    bool utf8_supported;
    bool auth_supported;
};
extern const struct Curl_handler Curl_handler_smtp;
extern const struct Curl_handler Curl_handler_smtps;
#define SMTP_EOB "\x0d\x0a\x2e\x0d\x0a"
#define SMTP_EOB_LEN 5
#define SMTP_EOB_FIND_LEN 3
#define SMTP_EOB_REPL "\x0d\x0a\x2e\x2e"
#define SMTP_EOB_REPL_LEN 4
CURLcode Curl_smtp_escape_eob(struct connectdata *conn, const ssize_t nread);

#endif