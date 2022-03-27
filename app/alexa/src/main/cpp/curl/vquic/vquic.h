#ifndef HEADER_CURL_VQUIC_QUIC_H
#define HEADER_CURL_VQUIC_QUIC_H

#include "../curl_setup.h"

#ifdef ENABLE_QUIC
CURLcode Curl_qlogdir(struct Curl_easy *data, unsigned char *scid, size_t scidlen, int *qlogfdp);
#endif

#endif