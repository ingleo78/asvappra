#ifndef HEADER_CURL_VQUIC_NGTCP2_H
#define HEADER_CURL_VQUIC_NGTCP2_H

#include "../curl_setup.h"

#ifdef USE_NGTCP2
#include <ngtcp2/ngtcp2.h>
#include <nghttp3/nghttp3.h>
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#elif defined(USE_GNUTLS)
#include <gnutls/gnutls.h>
#endif

struct quic_handshake {
    char *buf;
    size_t alloclen;
    size_t len;
    size_t nread;
};
struct quicsocket {
    struct connectdata *conn;
    ngtcp2_conn *qconn;
    ngtcp2_cid dcid;
    ngtcp2_cid scid;
    uint32_t version;
    ngtcp2_settings settings;
#ifdef USE_OPENSSL
    SSL_CTX *sslctx;
    SSL *ssl;
#elif defined(USE_GNUTLS)
    gnutls_certificate_credentials_t cred;
    gnutls_session_t ssl;
#endif
    struct quic_handshake crypto_data[3];
    uint8_t tls_alert;
    struct sockaddr_storage local_addr;
    socklen_t local_addrlen;
    nghttp3_conn *h3conn;
    nghttp3_conn_settings h3settings;
    int qlogfd;
};
#include <curl/urldata.h>
#endif
#endif