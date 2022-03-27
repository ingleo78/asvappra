#include "curl_setup.h"
#include "curl_endian.h"

unsigned short Curl_read16_le(const unsigned char *buf) {
    return (unsigned short)(((unsigned short)buf[0]) | ((unsigned short)buf[1] << 8));
}
unsigned int Curl_read32_le(const unsigned char *buf) {
    return ((unsigned int)buf[0]) | ((unsigned int)buf[1] << 8) | ((unsigned int)buf[2] << 16) | ((unsigned int)buf[3] << 24);
}
unsigned short Curl_read16_be(const unsigned char *buf) {
    return (unsigned short)(((unsigned short)buf[0] << 8) | ((unsigned short)buf[1]));
}
#if (CURL_SIZEOF_CURL_OFF_T > 4)
static void write32_le(const int value, unsigned char *buffer) {
    buffer[0] = (char)(value & 0x000000FF);
    buffer[1] = (char)((value & 0x0000FF00) >> 8);
    buffer[2] = (char)((value & 0x00FF0000) >> 16);
    buffer[3] = (char)((value & 0xFF000000) >> 24);
}
#if defined(HAVE_LONGLONG)
void Curl_write64_le(const long long value, unsigned char *buffer)
#else
void Curl_write64_le(const __int64 value, unsigned char *buffer)
#endif
{
    write32_le((int)value, buffer);
    write32_le((int)(value >> 32), buffer + 4);
}
#endif