#ifndef HEADER_CURL_KEYLOG_H
#define HEADER_CURL_KEYLOG_H

#include <stdbool.h>
#include "../curl_setup.h"
#include "../curl.h"
#include "../curl_memory.h"
#include "../memdebug.h"

void Curl_tls_keylog_open(void);
void Curl_tls_keylog_close(void);
bool Curl_tls_keylog_enabled(void);
bool Curl_tls_keylog_write(const char *label, const unsigned char client_random[32], const unsigned char *secret, size_t secretlen);
bool Curl_tls_keylog_write_line(const char *line);

#endif