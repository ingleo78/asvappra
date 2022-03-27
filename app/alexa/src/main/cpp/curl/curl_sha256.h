#ifndef HEADER_CURL_SHA256_H
#define HEADER_CURL_SHA256_H

#ifndef CURL_DISABLE_CRYPTO_AUTH
#define SHA256_DIGEST_LENGTH 32
void Curl_sha256it(unsigned char *outbuffer, const unsigned char *input, const size_t len);
#endif
#endif