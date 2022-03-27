#ifndef HEADER_CURL_NTLM_WB_H
#define HEADER_CURL_NTLM_WB_H

#include "curl_setup.h"

#if !defined(CURL_DISABLE_HTTP) && defined(USE_NTLM) && defined(NTLM_WB_ENABLED)
CURLcode Curl_input_ntlm_wb(struct connectdata *conn, bool proxy, const char *header);
CURLcode Curl_output_ntlm_wb(struct connectdata *conn, bool proxy);
void Curl_http_auth_cleanup_ntlm_wb(struct connectdata *conn);
#endif
#endif