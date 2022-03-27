#ifndef HEADER_CURL_ESCAPE_H
#define HEADER_CURL_ESCAPE_H

bool Curl_isunreserved(unsigned char in);
enum urlreject {
    REJECT_NADA = 2,
    REJECT_CTRL,
    REJECT_ZERO
};
CURLcode Curl_urldecode(struct Curl_easy *data, const char *string, size_t length, char **ostring, size_t *olen, enum urlreject ctrl);

#endif