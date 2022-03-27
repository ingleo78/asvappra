#ifndef HEADER_CURL_RTMP_H
#define HEADER_CURL_RTMP_H

#ifdef USE_LIBRTMP
extern const struct Curl_handler Curl_handler_rtmp;
extern const struct Curl_handler Curl_handler_rtmpt;
extern const struct Curl_handler Curl_handler_rtmpe;
extern const struct Curl_handler Curl_handler_rtmpte;
extern const struct Curl_handler Curl_handler_rtmps;
extern const struct Curl_handler Curl_handler_rtmpts;
#endif
#endif