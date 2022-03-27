#ifndef HEADER_CURL_VQUIC_QUICHE_H
#define HEADER_CURL_VQUIC_QUICHE_H

#include "../curl_setup.h"

#ifdef USE_QUICHE
struct quic_handshake {
    char *buf;
    size_t alloclen;
    size_t len;
    size_t nread;
};
struct quicsocket {
    quiche_config *cfg;
    quiche_conn *conn;
    quiche_h3_conn *h3c;
    quiche_h3_config *h3config;
    uint8_t scid[QUICHE_MAX_CONN_ID_LEN];
    uint32_t version;
};
#endif
#endif