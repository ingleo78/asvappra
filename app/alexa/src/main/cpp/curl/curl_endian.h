#ifndef HEADER_CURL_ENDIAN_H
#define HEADER_CURL_ENDIAN_H

unsigned short Curl_read16_le(const unsigned char *buf);
unsigned int Curl_read32_le(const unsigned char *buf);
unsigned short Curl_read16_be(const unsigned char *buf);
#if (CURL_SIZEOF_CURL_OFF_T > 4)
#if defined(HAVE_LONGLONG)
void Curl_write64_le(const long long value, unsigned char *buffer);
#else
void Curl_write64_le(const __int64 value, unsigned char *buffer);
#endif
#endif
#endif