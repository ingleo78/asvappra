#ifndef HEADER_CURL_GETINFO_H
#define HEADER_CURL_GETINFO_H

CURLcode Curl_getinfo(struct Curl_easy *data, CURLINFO info, ...);
CURLcode Curl_initinfo(struct Curl_easy *data);

#endif