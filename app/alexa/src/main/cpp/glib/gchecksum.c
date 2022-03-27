#include <string.h>
#include "gchecksum.h"
#include "gmem.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "gtypes.h"
#include "glibintl.h"

#define IS_VALID_TYPE(type) ((type) >= G_CHECKSUM_MD5 && (type) <= G_CHECKSUM_SHA256)
static const gchar hex_digits[] = "0123456789abcdef";
#define MD5_DATASIZE    64
#define MD5_DIGEST_LEN  16
typedef struct {
  guint32 buf[4];
  guint32 bits[2];
  guchar data[MD5_DATASIZE];
  guchar digest[MD5_DIGEST_LEN];
} Md5sum;
#define SHA1_DATASIZE   64
#define SHA1_DIGEST_LEN 20
typedef struct {
  guint32 buf[5];
  guint32 bits[2];
  guint32 data[16];
  guchar digest[SHA1_DIGEST_LEN];
} Sha1sum;
#define SHA256_DATASIZE         64
#define SHA256_DIGEST_LEN       32
typedef struct {
  guint32 buf[8];
  guint32 bits[2];
  guint8 data[SHA256_DATASIZE];
  guchar digest[SHA256_DIGEST_LEN];
} Sha256sum;
struct _GChecksum {
  GChecksumType type;
  gchar *digest_str;
  union {
    Md5sum md5;
    Sha1sum sha1;
    Sha256sum sha256;
  } sum;
};
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define md5_byte_reverse(buffer,length)
#else
static inline void md5_byte_reverse(guchar *buffer, gulong length) {
  guint32 bit;
  do {
      bit = (guint32)((unsigned)buffer[3] << 8 | buffer[2]) << 16 | ((unsigned)buffer[1] << 8 | buffer[0]);
      *(guint32*)buffer = bit;
      buffer += 4;
  } while(--length);
}
#endif
#if G_BYTE_ORDER == G_BIG_ENDIAN
#define sha_byte_reverse(buffer,length)
#else
static inline void sha_byte_reverse(guint32 *buffer, gint length) {
  length /= sizeof(guint32);
  while (length--) {
      *buffer = GUINT32_SWAP_LE_BE(*buffer);
      ++buffer;
  }
}
#endif
static gchar* digest_to_string(guint8 *digest, gsize digest_len) {
  gint len = digest_len * 2;
  gint i;
  gchar *retval;
  retval = g_new(gchar, len + 1);
  for (i = 0; i < digest_len; i++) {
      guint8 byte = digest[i];
      retval[2 * i] = hex_digits[byte >> 4];
      retval[2 * i + 1] = hex_digits[byte & 0xf];
  }
  retval[len] = 0;
  return retval;
}
static void md5_sum_init(Md5sum *md5) {
  md5->buf[0] = 0x67452301;
  md5->buf[1] = 0xefcdab89;
  md5->buf[2] = 0x98badcfe;
  md5->buf[3] = 0x10325476;
  md5->bits[0] = md5->bits[1] = 0;
}
static void md5_transform(guint32 buf[4], guint32 const in[16]) {
  register guint32 a, b, c, d;
  #define F1(x, y, z) (z ^ (x & (y ^ z)))
  #define F2(x, y, z) F1(z, x, y)
  #define F3(x, y, z) (x ^ y ^ z)
  #define F4(x, y, z) (y ^ (x | ~z))
  #define md5_step(f, w, x, y, z, data, s) (w += f(x, y, z) + data,  w = w << s | w >> (32 - s),  w += x)
  a = buf[0];
  b = buf[1];
  c = buf[2];
  d = buf[3];
  md5_step(F1, a, b, c, d, in[0]  + 0xd76aa478,  7);
  md5_step(F1, d, a, b, c, in[1]  + 0xe8c7b756, 12);
  md5_step(F1, c, d, a, b, in[2]  + 0x242070db, 17);
  md5_step(F1, b, c, d, a, in[3]  + 0xc1bdceee, 22);
  md5_step(F1, a, b, c, d, in[4]  + 0xf57c0faf,  7);
  md5_step(F1, d, a, b, c, in[5]  + 0x4787c62a, 12);
  md5_step(F1, c, d, a, b, in[6]  + 0xa8304613, 17);
  md5_step(F1, b, c, d, a, in[7]  + 0xfd469501, 22);
  md5_step(F1, a, b, c, d, in[8]  + 0x698098d8,  7);
  md5_step(F1, d, a, b, c, in[9]  + 0x8b44f7af, 12);
  md5_step(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
  md5_step(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
  md5_step(F1, a, b, c, d, in[12] + 0x6b901122,  7);
  md5_step(F1, d, a, b, c, in[13] + 0xfd987193, 12);
  md5_step(F1, c, d, a, b, in[14] + 0xa679438e, 17);
  md5_step(F1, b, c, d, a, in[15] + 0x49b40821, 22);
  md5_step(F2, a, b, c, d, in[1]  + 0xf61e2562,  5);
  md5_step(F2, d, a, b, c, in[6]  + 0xc040b340,  9);
  md5_step(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
  md5_step(F2, b, c, d, a, in[0]  + 0xe9b6c7aa, 20);
  md5_step(F2, a, b, c, d, in[5]  + 0xd62f105d,  5);
  md5_step(F2, d, a, b, c, in[10] + 0x02441453,  9);
  md5_step(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
  md5_step(F2, b, c, d, a, in[4]  + 0xe7d3fbc8, 20);
  md5_step(F2, a, b, c, d, in[9]  + 0x21e1cde6,  5);
  md5_step(F2, d, a, b, c, in[14] + 0xc33707d6,  9);
  md5_step(F2, c, d, a, b, in[3]  + 0xf4d50d87, 14);
  md5_step(F2, b, c, d, a, in[8]  + 0x455a14ed, 20);
  md5_step(F2, a, b, c, d, in[13] + 0xa9e3e905,  5);
  md5_step(F2, d, a, b, c, in[2]  + 0xfcefa3f8,  9);
  md5_step(F2, c, d, a, b, in[7]  + 0x676f02d9, 14);
  md5_step(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);
  md5_step(F3, a, b, c, d, in[5]  + 0xfffa3942,  4);
  md5_step(F3, d, a, b, c, in[8]  + 0x8771f681, 11);
  md5_step(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
  md5_step(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
  md5_step(F3, a, b, c, d, in[1]  + 0xa4beea44,  4);
  md5_step(F3, d, a, b, c, in[4]  + 0x4bdecfa9, 11);
  md5_step(F3, c, d, a, b, in[7]  + 0xf6bb4b60, 16);
  md5_step(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
  md5_step(F3, a, b, c, d, in[13] + 0x289b7ec6,  4);
  md5_step(F3, d, a, b, c, in[0]  + 0xeaa127fa, 11);
  md5_step(F3, c, d, a, b, in[3]  + 0xd4ef3085, 16);
  md5_step(F3, b, c, d, a, in[6]  + 0x04881d05, 23);
  md5_step(F3, a, b, c, d, in[9]  + 0xd9d4d039,  4);
  md5_step(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
  md5_step(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
  md5_step(F3, b, c, d, a, in[2]  + 0xc4ac5665, 23);
  md5_step(F4, a, b, c, d, in[0]  + 0xf4292244,  6);
  md5_step(F4, d, a, b, c, in[7]  + 0x432aff97, 10);
  md5_step(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
  md5_step(F4, b, c, d, a, in[5]  + 0xfc93a039, 21);
  md5_step(F4, a, b, c, d, in[12] + 0x655b59c3,  6);
  md5_step(F4, d, a, b, c, in[3]  + 0x8f0ccc92, 10);
  md5_step(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
  md5_step(F4, b, c, d, a, in[1]  + 0x85845dd1, 21);
  md5_step(F4, a, b, c, d, in[8]  + 0x6fa87e4f,  6);
  md5_step(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
  md5_step(F4, c, d, a, b, in[6]  + 0xa3014314, 15);
  md5_step(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
  md5_step(F4, a, b, c, d, in[4]  + 0xf7537e82,  6);
  md5_step(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
  md5_step(F4, c, d, a, b, in[2]  + 0x2ad7d2bb, 15);
  md5_step(F4, b, c, d, a, in[9]  + 0xeb86d391, 21);
  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
  #undef F1
  #undef F2
  #undef F3
  #undef F4
  #undef md5_step
}
static void md5_sum_update(Md5sum *md5, const guchar *data, gsize length) {
  guint32 bit;
  bit = md5->bits[0];
  md5->bits[0] = bit + ((guint32)length << 3);
  if (md5->bits[0] < bit) md5->bits[1] += 1;
  md5->bits[1] += length >> 29;
  bit = (bit >> 3) & 0x3f;
  if (bit) {
      guchar *p = (guchar*)md5->data + bit;
      bit = MD5_DATASIZE - bit;
      if (length < bit) {
          memcpy(p, data, length);
          return;
      }
      memcpy(p, data, bit);
      md5_byte_reverse(md5->data, 16);
      md5_transform(md5->buf, (guint32 *) md5->data);
      data += bit;
      length -= bit;
  }
  while(length >= MD5_DATASIZE) {
      memcpy(md5->data, data, MD5_DATASIZE);
      md5_byte_reverse(md5->data, 16);
      md5_transform(md5->buf, (guint32*)md5->data);
      data += MD5_DATASIZE;
      length -= MD5_DATASIZE;
  }
  memcpy(md5->data, data, length);
}
static void md5_sum_close(Md5sum *md5) {
  guint count;
  guchar *p;
  count = (md5->bits[0] >> 3) & 0x3F;
  p = md5->data + count;
  *p++ = 0x80;
  count = MD5_DATASIZE - 1 - count;
  if (count < 8) {
      memset(p, 0, count);
      md5_byte_reverse(md5->data, 16);
      md5_transform(md5->buf, (guint32*)md5->data);
      memset(md5->data, 0, MD5_DATASIZE - 8);
  } else memset(p, 0, count - 8);
  md5_byte_reverse(md5->data, 14);
  ((guint32*)md5->data)[14] = md5->bits[0];
  ((guint32*)md5->data)[15] = md5->bits[1];
  md5_transform(md5->buf, (guint32*)md5->data);
  md5_byte_reverse((guchar*)md5->buf, 4);
  memcpy(md5->digest, md5->buf, 16);
  memset(md5->buf, 0, sizeof(md5->buf));
  memset(md5->data, 0, sizeof(md5->data));
}
static gchar* md5_sum_to_string(Md5sum *md5) {
  return digest_to_string(md5->digest, MD5_DIGEST_LEN);
}
static void md5_sum_digest(Md5sum *md5, guint8 *digest) {
  gint i;
  for (i = 0; i < MD5_DIGEST_LEN; i++) digest[i] = md5->digest[i];
}
static void sha1_sum_init(Sha1sum *sha1) {
  sha1->buf[0] = 0x67452301L;
  sha1->buf[1] = 0xEFCDAB89L;
  sha1->buf[2] = 0x98BADCFEL;
  sha1->buf[3] = 0x10325476L;
  sha1->buf[4] = 0xC3D2E1F0L;
  sha1->bits[0] = sha1->bits[1] = 0;
}
#define f1(x,y,z)  (z ^ (x & (y ^ z)))
#define f2(x,y,z)  (x ^ y ^ z)
#define f3(x,y,z)  (( x & y) | (z & (x | y)))
#define f4(x,y,z)  (x ^ y ^ z)
#define K1  0x5A827999L
#define K2  0x6ED9EBA1L
#define K3  0x8F1BBCDCL
#define K4  0xCA62C1D6L
#define ROTL(n,X) (((X) << n ) | ((X) >> (32 - n)))
#define expand(W,i) (W[ i & 15 ] = ROTL(1, (W[i & 15] ^ W[(i - 14) & 15] ^ W[(i -  8) & 15] ^ W[(i -  3) & 15])))
#define subRound(a, b, c, d, e, f, k, data)  (e += ROTL (5, a) + f(b, c, d) + k + data, b = ROTL (30, b))
static void sha1_transform(guint32 buf[5], guint32 in[16]) {
  guint32 A, B, C, D, E;
  A = buf[0];
  B = buf[1];
  C = buf[2];
  D = buf[3];
  E = buf[4];
  subRound(A, B, C, D, E, f1, K1, in[0]);
  subRound(E, A, B, C, D, f1, K1, in[1]);
  subRound(D, E, A, B, C, f1, K1, in[2]);
  subRound(C, D, E, A, B, f1, K1, in[3]);
  subRound(B, C, D, E, A, f1, K1, in[4]);
  subRound(A, B, C, D, E, f1, K1, in[5]);
  subRound(E, A, B, C, D, f1, K1, in[6]);
  subRound(D, E, A, B, C, f1, K1, in[7]);
  subRound(C, D, E, A, B, f1, K1, in[8]);
  subRound(B, C, D, E, A, f1, K1, in[9]);
  subRound(A, B, C, D, E, f1, K1, in[10]);
  subRound(E, A, B, C, D, f1, K1, in[11]);
  subRound(D, E, A, B, C, f1, K1, in[12]);
  subRound(C, D, E, A, B, f1, K1, in[13]);
  subRound(B, C, D, E, A, f1, K1, in[14]);
  subRound(A, B, C, D, E, f1, K1, in[15]);
  subRound(E, A, B, C, D, f1, K1, expand(in, 16));
  subRound(D, E, A, B, C, f1, K1, expand(in, 17));
  subRound(C, D, E, A, B, f1, K1, expand(in, 18));
  subRound(B, C, D, E, A, f1, K1, expand(in, 19));
  subRound(A, B, C, D, E, f2, K2, expand(in, 20));
  subRound(E, A, B, C, D, f2, K2, expand(in, 21));
  subRound(D, E, A, B, C, f2, K2, expand(in, 22));
  subRound(C, D, E, A, B, f2, K2, expand(in, 23));
  subRound(B, C, D, E, A, f2, K2, expand(in, 24));
  subRound(A, B, C, D, E, f2, K2, expand(in, 25));
  subRound(E, A, B, C, D, f2, K2, expand(in, 26));
  subRound(D, E, A, B, C, f2, K2, expand(in, 27));
  subRound(C, D, E, A, B, f2, K2, expand(in, 28));
  subRound(B, C, D, E, A, f2, K2, expand(in, 29));
  subRound(A, B, C, D, E, f2, K2, expand(in, 30));
  subRound(E, A, B, C, D, f2, K2, expand(in, 31));
  subRound(D, E, A, B, C, f2, K2, expand(in, 32));
  subRound(C, D, E, A, B, f2, K2, expand(in, 33));
  subRound(B, C, D, E, A, f2, K2, expand(in, 34));
  subRound(A, B, C, D, E, f2, K2, expand(in, 35));
  subRound(E, A, B, C, D, f2, K2, expand(in, 36));
  subRound(D, E, A, B, C, f2, K2, expand(in, 37));
  subRound(C, D, E, A, B, f2, K2, expand(in, 38));
  subRound(B, C, D, E, A, f2, K2, expand(in, 39));
  subRound(A, B, C, D, E, f3, K3, expand(in, 40));
  subRound(E, A, B, C, D, f3, K3, expand(in, 41));
  subRound(D, E, A, B, C, f3, K3, expand(in, 42));
  subRound(C, D, E, A, B, f3, K3, expand(in, 43));
  subRound(B, C, D, E, A, f3, K3, expand(in, 44));
  subRound(A, B, C, D, E, f3, K3, expand(in, 45));
  subRound(E, A, B, C, D, f3, K3, expand(in, 46));
  subRound(D, E, A, B, C, f3, K3, expand(in, 47));
  subRound(C, D, E, A, B, f3, K3, expand(in, 48));
  subRound(B, C, D, E, A, f3, K3, expand(in, 49));
  subRound(A, B, C, D, E, f3, K3, expand(in, 50));
  subRound(E, A, B, C, D, f3, K3, expand(in, 51));
  subRound(D, E, A, B, C, f3, K3, expand(in, 52));
  subRound(C, D, E, A, B, f3, K3, expand(in, 53));
  subRound(B, C, D, E, A, f3, K3, expand(in, 54));
  subRound(A, B, C, D, E, f3, K3, expand(in, 55));
  subRound(E, A, B, C, D, f3, K3, expand(in, 56));
  subRound(D, E, A, B, C, f3, K3, expand(in, 57));
  subRound(C, D, E, A, B, f3, K3, expand(in, 58));
  subRound(B, C, D, E, A, f3, K3, expand(in, 59));
  subRound(A, B, C, D, E, f4, K4, expand(in, 60));
  subRound(E, A, B, C, D, f4, K4, expand(in, 61));
  subRound(D, E, A, B, C, f4, K4, expand(in, 62));
  subRound(C, D, E, A, B, f4, K4, expand(in, 63));
  subRound(B, C, D, E, A, f4, K4, expand(in, 64));
  subRound(A, B, C, D, E, f4, K4, expand(in, 65));
  subRound(E, A, B, C, D, f4, K4, expand(in, 66));
  subRound(D, E, A, B, C, f4, K4, expand(in, 67));
  subRound(C, D, E, A, B, f4, K4, expand(in, 68));
  subRound(B, C, D, E, A, f4, K4, expand(in, 69));
  subRound(A, B, C, D, E, f4, K4, expand(in, 70));
  subRound(E, A, B, C, D, f4, K4, expand(in, 71));
  subRound(D, E, A, B, C, f4, K4, expand(in, 72));
  subRound(C, D, E, A, B, f4, K4, expand(in, 73));
  subRound(B, C, D, E, A, f4, K4, expand(in, 74));
  subRound(A, B, C, D, E, f4, K4, expand(in, 75));
  subRound(E, A, B, C, D, f4, K4, expand(in, 76));
  subRound(D, E, A, B, C, f4, K4, expand(in, 77));
  subRound(C, D, E, A, B, f4, K4, expand(in, 78));
  subRound(B, C, D, E, A, f4, K4, expand(in, 79));
  buf[0] += A;
  buf[1] += B;
  buf[2] += C;
  buf[3] += D;
  buf[4] += E;
}
#undef K1
#undef K2
#undef K3
#undef K4
#undef f1
#undef f2
#undef f3
#undef f4
#undef ROTL
#undef expand
#undef subRound
static void sha1_sum_update(Sha1sum *sha1, const guchar *buffer, gsize count) {
  guint32 tmp;
  guint dataCount;
  tmp = sha1->bits[0];
  if ((sha1->bits[0] = tmp + ((guint32)count << 3) ) < tmp) sha1->bits[1] += 1;
  sha1->bits[1] += count >> 29;
  dataCount = (guint)(tmp >> 3) & 0x3F;
  if (dataCount) {
      guchar *p = (guchar*)sha1->data + dataCount;
      dataCount = SHA1_DATASIZE - dataCount;
      if (count < dataCount) {
          memcpy(p, buffer, count);
          return;
      }
      memcpy(p, buffer, dataCount);
      sha_byte_reverse(sha1->data, SHA1_DATASIZE);
      sha1_transform(sha1->buf, sha1->data);
      buffer += dataCount;
      count -= dataCount;
  }
  while(count >= SHA1_DATASIZE) {
      memcpy(sha1->data, buffer, SHA1_DATASIZE);
      sha_byte_reverse(sha1->data, SHA1_DATASIZE);
      sha1_transform(sha1->buf, sha1->data);
      buffer += SHA1_DATASIZE;
      count -= SHA1_DATASIZE;
  }
  memcpy(sha1->data, buffer, count);
}
static void sha1_sum_close(Sha1sum *sha1) {
  gint count;
  guchar *data_p;
  count = (gint)((sha1->bits[0] >> 3) & 0x3f);
  data_p = (guchar*)sha1->data + count;
  *data_p++ = 0x80;
  count = SHA1_DATASIZE - 1 - count;
  if (count < 8) {
      memset(data_p, 0, count);
      sha_byte_reverse(sha1->data, SHA1_DATASIZE);
      sha1_transform(sha1->buf, sha1->data);
      memset(sha1->data, 0, SHA1_DATASIZE - 8);
  } else memset(data_p, 0, count - 8);
  sha1->data[14] = sha1->bits[1];
  sha1->data[15] = sha1->bits[0];
  sha_byte_reverse(sha1->data, SHA1_DATASIZE - 8);
  sha1_transform(sha1->buf, sha1->data);
  sha_byte_reverse(sha1->buf, SHA1_DIGEST_LEN);
  memcpy(sha1->digest, sha1->buf, SHA1_DIGEST_LEN);
  memset(sha1->buf, 0, sizeof(sha1->buf));
  memset(sha1->data, 0, sizeof(sha1->data));
}
static gchar* sha1_sum_to_string(Sha1sum *sha1) {
  return digest_to_string(sha1->digest, SHA1_DIGEST_LEN);
}
static void sha1_sum_digest(Sha1sum *sha1, guint8 *digest) {
  gint i;
  for (i = 0; i < SHA1_DIGEST_LEN; i++) digest[i] = sha1->digest[i];
}
static void sha256_sum_init(Sha256sum *sha256) {
  sha256->buf[0] = 0x6a09e667;
  sha256->buf[1] = 0xbb67ae85;
  sha256->buf[2] = 0x3c6ef372;
  sha256->buf[3] = 0xa54ff53a;
  sha256->buf[4] = 0x510e527f;
  sha256->buf[5] = 0x9b05688c;
  sha256->buf[6] = 0x1f83d9ab;
  sha256->buf[7] = 0x5be0cd19;
  sha256->bits[0] = sha256->bits[1] = 0;
}
#define GET_UINT32(n,b,i)                                                                                                       \
G_STMT_START {                                                                                                                  \
    (n) = ((guint32) (b)[(i)] << 24) | ((guint32)(b)[(i) + 1] << 16) | ((guint32)(b)[(i) + 2] <<  8) | ((guint32)(b)[(i) + 3]); \
} G_STMT_END
#define PUT_UINT32(n,b,i)                \
G_STMT_START {                           \
    (b)[(i)    ] = (guint8) ((n) >> 24); \
    (b)[(i) + 1] = (guint8) ((n) >> 16); \
    (b)[(i) + 2] = (guint8) ((n) >>  8); \
    (b)[(i) + 3] = (guint8) ((n)      ); \
} G_STMT_END
static void sha256_transform(guint32 buf[8], guint8 const data[64]) {
  guint32 temp1, temp2, W[64];
  guint32 A, B, C, D, E, F, G, H;
  GET_UINT32(W[0],  data,  0);
  GET_UINT32(W[1],  data,  4);
  GET_UINT32(W[2],  data,  8);
  GET_UINT32(W[3],  data, 12);
  GET_UINT32(W[4],  data, 16);
  GET_UINT32(W[5],  data, 20);
  GET_UINT32(W[6],  data, 24);
  GET_UINT32(W[7],  data, 28);
  GET_UINT32(W[8],  data, 32);
  GET_UINT32(W[9],  data, 36);
  GET_UINT32(W[10], data, 40);
  GET_UINT32(W[11], data, 44);
  GET_UINT32(W[12], data, 48);
  GET_UINT32(W[13], data, 52);
  GET_UINT32(W[14], data, 56);
  GET_UINT32(W[15], data, 60);
  #define SHR(x,n)  ((x & 0xFFFFFFFF) >> n)
  #define ROTR(x,n)  (SHR(x,n) | (x << (32 - n)))
  #define S0(x) (ROTR(x, 7) ^ ROTR(x,18) ^  SHR(x, 3))
  #define S1(x) (ROTR(x,17) ^ ROTR(x,19) ^  SHR(x,10))
  #define S2(x) (ROTR(x, 2) ^ ROTR(x,13) ^ ROTR(x,22))
  #define S3(x) (ROTR(x, 6) ^ ROTR(x,11) ^ ROTR(x,25))
  #define F0(x,y,z) ((x & y) | (z & (x | y)))
  #define F1(x,y,z) (z ^ (x & (y ^ z)))
  #define R(t)    (W[t] = S1(W[t -  2]) + W[t -  7] + S0(W[t - 15]) + W[t - 16])
  #define P(a,b,c,d,e,f,g,h,x,K)                 \
      G_STMT_START {                             \
          temp1 = h + S3(e) + F1(e,f,g) + K + x; \
          temp2 = S2(a) + F0(a,b,c);             \
          d += temp1;                            \
          h = temp1 + temp2;                     \
      } G_STMT_END
  A = buf[0];
  B = buf[1];
  C = buf[2];
  D = buf[3];
  E = buf[4];
  F = buf[5];
  G = buf[6];
  H = buf[7];
  P(A, B, C, D, E, F, G, H, W[0], 0x428A2F98);
  P(H, A, B, C, D, E, F, G, W[1], 0x71374491);
  P(G, H, A, B, C, D, E, F, W[2], 0xB5C0FBCF);
  P(F, G, H, A, B, C, D, E, W[3], 0xE9B5DBA5);
  P(E, F, G, H, A, B, C, D, W[4], 0x3956C25B);
  P(D, E, F, G, H, A, B, C, W[5], 0x59F111F1);
  P(C, D, E, F, G, H, A, B, W[6], 0x923F82A4);
  P(B, C, D, E, F, G, H, A, W[7], 0xAB1C5ED5);
  P(A, B, C, D, E, F, G, H, W[8], 0xD807AA98);
  P(H, A, B, C, D, E, F, G, W[9], 0x12835B01);
  P(G, H, A, B, C, D, E, F, W[10], 0x243185BE);
  P(F, G, H, A, B, C, D, E, W[11], 0x550C7DC3);
  P(E, F, G, H, A, B, C, D, W[12], 0x72BE5D74);
  P(D, E, F, G, H, A, B, C, W[13], 0x80DEB1FE);
  P(C, D, E, F, G, H, A, B, W[14], 0x9BDC06A7);
  P(B, C, D, E, F, G, H, A, W[15], 0xC19BF174);
  P(A, B, C, D, E, F, G, H, R(16), 0xE49B69C1);
  P(H, A, B, C, D, E, F, G, R(17), 0xEFBE4786);
  P(G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6);
  P(F, G, H, A, B, C, D, E, R(19), 0x240CA1CC);
  P(E, F, G, H, A, B, C, D, R(20), 0x2DE92C6F);
  P(D, E, F, G, H, A, B, C, R(21), 0x4A7484AA);
  P(C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DC);
  P(B, C, D, E, F, G, H, A, R(23), 0x76F988DA);
  P(A, B, C, D, E, F, G, H, R(24), 0x983E5152);
  P(H, A, B, C, D, E, F, G, R(25), 0xA831C66D);
  P(G, H, A, B, C, D, E, F, R(26), 0xB00327C8);
  P(F, G, H, A, B, C, D, E, R(27), 0xBF597FC7);
  P(E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3);
  P(D, E, F, G, H, A, B, C, R(29), 0xD5A79147);
  P(C, D, E, F, G, H, A, B, R(30), 0x06CA6351);
  P(B, C, D, E, F, G, H, A, R(31), 0x14292967);
  P(A, B, C, D, E, F, G, H, R(32), 0x27B70A85);
  P(H, A, B, C, D, E, F, G, R(33), 0x2E1B2138);
  P(G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFC);
  P(F, G, H, A, B, C, D, E, R(35), 0x53380D13);
  P(E, F, G, H, A, B, C, D, R(36), 0x650A7354);
  P(D, E, F, G, H, A, B, C, R(37), 0x766A0ABB);
  P(C, D, E, F, G, H, A, B, R(38), 0x81C2C92E);
  P(B, C, D, E, F, G, H, A, R(39), 0x92722C85);
  P(A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1);
  P(H, A, B, C, D, E, F, G, R(41), 0xA81A664B);
  P(G, H, A, B, C, D, E, F, R(42), 0xC24B8B70);
  P(F, G, H, A, B, C, D, E, R(43), 0xC76C51A3);
  P(E, F, G, H, A, B, C, D, R(44), 0xD192E819);
  P(D, E, F, G, H, A, B, C, R(45), 0xD6990624);
  P(C, D, E, F, G, H, A, B, R(46), 0xF40E3585);
  P(B, C, D, E, F, G, H, A, R(47), 0x106AA070);
  P(A, B, C, D, E, F, G, H, R(48), 0x19A4C116);
  P(H, A, B, C, D, E, F, G, R(49), 0x1E376C08);
  P(G, H, A, B, C, D, E, F, R(50), 0x2748774C);
  P(F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5);
  P(E, F, G, H, A, B, C, D, R(52), 0x391C0CB3);
  P(D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4A);
  P(C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4F);
  P(B, C, D, E, F, G, H, A, R(55), 0x682E6FF3);
  P(A, B, C, D, E, F, G, H, R(56), 0x748F82EE);
  P(H, A, B, C, D, E, F, G, R(57), 0x78A5636F);
  P(G, H, A, B, C, D, E, F, R(58), 0x84C87814);
  P(F, G, H, A, B, C, D, E, R(59), 0x8CC70208);
  P(E, F, G, H, A, B, C, D, R(60), 0x90BEFFFA);
  P(D, E, F, G, H, A, B, C, R(61), 0xA4506CEB);
  P(C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7);
  P(B, C, D, E, F, G, H, A, R(63), 0xC67178F2);
  #undef SHR
  #undef ROTR
  #undef S0
  #undef S1
  #undef S2
  #undef S3
  #undef F0
  #undef F1
  #undef R
  #undef P
  buf[0] += A;
  buf[1] += B;
  buf[2] += C;
  buf[3] += D;
  buf[4] += E;
  buf[5] += F;
  buf[6] += G;
  buf[7] += H;
}
static void sha256_sum_update(Sha256sum *sha256, const guchar *buffer, gsize length) {
  guint32 left, fill;
  const guint8 *input = buffer;
  if (length == 0) return;
  left = sha256->bits[0] & 0x3F;
  fill = 64 - left;
  sha256->bits[0] += length;
  sha256->bits[0] &= 0xFFFFFFFF;
  if (sha256->bits[0] < length) sha256->bits[1]++;
  if (left > 0 && length >= fill) {
      memcpy((sha256->data + left), input, fill);
      sha256_transform(sha256->buf, sha256->data);
      length -= fill;
      input += fill;
      left = 0;
  }
  while(length >= SHA256_DATASIZE) {
      sha256_transform(sha256->buf, input);
      length -= 64;
      input += 64;
  }
  if (length) memcpy(sha256->data + left, input, length);
}
static guint8 sha256_padding[64] = {
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static void sha256_sum_close(Sha256sum *sha256) {
  guint32 last, padn;
  guint32 high, low;
  guint8 msglen[8];
  high = (sha256->bits[0] >> 29) | (sha256->bits[1] <<  3);
  low  = (sha256->bits[0] <<  3);
  PUT_UINT32(high, msglen, 0);
  PUT_UINT32(low, msglen, 4);
  last = sha256->bits[0] & 0x3F;
  padn = (last < 56) ? (56 - last) : (120 - last);
  sha256_sum_update(sha256, sha256_padding, padn);
  sha256_sum_update(sha256, msglen, 8);
  PUT_UINT32(sha256->buf[0], sha256->digest,  0);
  PUT_UINT32(sha256->buf[1], sha256->digest,  4);
  PUT_UINT32(sha256->buf[2], sha256->digest,  8);
  PUT_UINT32(sha256->buf[3], sha256->digest, 12);
  PUT_UINT32(sha256->buf[4], sha256->digest, 16);
  PUT_UINT32(sha256->buf[5], sha256->digest, 20);
  PUT_UINT32(sha256->buf[6], sha256->digest, 24);
  PUT_UINT32(sha256->buf[7], sha256->digest, 28);
}
#undef PUT_UINT32
#undef GET_UINT32
static gchar* sha256_sum_to_string(Sha256sum *sha256) {
  return digest_to_string (sha256->digest, SHA256_DIGEST_LEN);
}
static void sha256_sum_digest(Sha256sum *sha256, guint8 *digest) {
  gint i;
  for (i = 0; i < SHA256_DIGEST_LEN; i++) digest[i] = sha256->digest[i];
}
gssize g_checksum_type_get_length(GChecksumType checksum_type) {
  gssize len = -1;
  switch(checksum_type) {
    case G_CHECKSUM_MD5: len = MD5_DIGEST_LEN; break;
    case G_CHECKSUM_SHA1: len = SHA1_DIGEST_LEN; break;
    case G_CHECKSUM_SHA256: len = SHA256_DIGEST_LEN; break;
    default: len = -1;
  }
  return len;
}
GChecksum* g_checksum_new(GChecksumType checksum_type) {
  GChecksum *checksum;
  if (!IS_VALID_TYPE(checksum_type)) return NULL;
  checksum = g_slice_new0(GChecksum);
  checksum->type = checksum_type;
  g_checksum_reset(checksum);
  return checksum;
}
void g_checksum_reset(GChecksum *checksum) {
  g_return_if_fail(checksum != NULL);
  g_free(checksum->digest_str);
  checksum->digest_str = NULL;
  switch(checksum->type) {
    case G_CHECKSUM_MD5: md5_sum_init(&(checksum->sum.md5)); break;
    case G_CHECKSUM_SHA1: sha1_sum_init(&(checksum->sum.sha1)); break;
    case G_CHECKSUM_SHA256: sha256_sum_init(&(checksum->sum.sha256)); break;
    default: g_assert_not_reached();
  }
}
GChecksum* g_checksum_copy(const GChecksum *checksum) {
  GChecksum *copy;
  g_return_val_if_fail(checksum != NULL, NULL);
  copy = g_slice_new(GChecksum);
  *copy = *checksum;
  copy->digest_str = g_strdup(checksum->digest_str);
  return copy;
}
void g_checksum_free(GChecksum *checksum) {
  if (G_LIKELY (checksum)) {
      g_free(checksum->digest_str);
      g_slice_free(GChecksum, checksum);
  }
}
void g_checksum_update(GChecksum *checksum, const guchar *data, gssize length) {
  g_return_if_fail(checksum != NULL);
  g_return_if_fail(length == 0 || data != NULL);
  if (length < 0) length = strlen((const gchar*)data);
  if (checksum->digest_str) {
      g_warning("The checksum `%s' has been closed and cannot be updated anymore.", checksum->digest_str);
      return;
  }
  switch (checksum->type) {
    case G_CHECKSUM_MD5: md5_sum_update(&(checksum->sum.md5), data, length); break;
    case G_CHECKSUM_SHA1: sha1_sum_update(&(checksum->sum.sha1), data, length); break;
    case G_CHECKSUM_SHA256: sha256_sum_update(&(checksum->sum.sha256), data, length); break;
    default: g_assert_not_reached();
  }
}
G_CONST_RETURN gchar* g_checksum_get_string(GChecksum *checksum) {
  gchar *str = NULL;
  g_return_val_if_fail(checksum != NULL, NULL);
  if (checksum->digest_str) return checksum->digest_str;
  switch (checksum->type) {
    case G_CHECKSUM_MD5:
      md5_sum_close (&(checksum->sum.md5));
      str = md5_sum_to_string(&(checksum->sum.md5));
      break;
    case G_CHECKSUM_SHA1:
      sha1_sum_close(&(checksum->sum.sha1));
      str = sha1_sum_to_string(&(checksum->sum.sha1));
      break;
    case G_CHECKSUM_SHA256:
      sha256_sum_close(&(checksum->sum.sha256));
      str = sha256_sum_to_string(&(checksum->sum.sha256));
      break;
    default:
      g_assert_not_reached ();
      break;
  }
  checksum->digest_str = str;
  return checksum->digest_str;
}
void g_checksum_get_digest(GChecksum *checksum, guint8 *buffer, gsize *digest_len) {
  gboolean checksum_open = FALSE;
  gchar *str = NULL;
  gsize len;
  g_return_if_fail(checksum != NULL);
  len = g_checksum_type_get_length(checksum->type);
  g_return_if_fail(*digest_len >= len);
  checksum_open = !!(checksum->digest_str == NULL);
  switch (checksum->type) {
    case G_CHECKSUM_MD5:
      if (checksum_open) {
          md5_sum_close(&(checksum->sum.md5));
          str = md5_sum_to_string(&(checksum->sum.md5));
      }
      md5_sum_digest(&(checksum->sum.md5), buffer);
      break;
    case G_CHECKSUM_SHA1:
      if (checksum_open) {
          sha1_sum_close(&(checksum->sum.sha1));
          str = sha1_sum_to_string(&(checksum->sum.sha1));
      }
      sha1_sum_digest(&(checksum->sum.sha1), buffer);
      break;
    case G_CHECKSUM_SHA256:
      if (checksum_open) {
          sha256_sum_close(&(checksum->sum.sha256));
          str = sha256_sum_to_string(&(checksum->sum.sha256));
      }
      sha256_sum_digest(&(checksum->sum.sha256), buffer);
      break;
    default:
      g_assert_not_reached();
      break;
    }

  if (str) checksum->digest_str = str;
  *digest_len = len;
}
gchar* g_compute_checksum_for_data(GChecksumType checksum_type, const guchar *data, gsize length) {
  GChecksum *checksum;
  gchar *retval;
  g_return_val_if_fail(IS_VALID_TYPE (checksum_type), NULL);
  g_return_val_if_fail(length == 0 || data != NULL, NULL);
  checksum = g_checksum_new(checksum_type);
  if (!checksum) return NULL;
  g_checksum_update(checksum, data, length);
  retval = g_strdup(g_checksum_get_string(checksum));
  g_checksum_free(checksum);
  return retval;
}
gchar* g_compute_checksum_for_string(GChecksumType checksum_type, const gchar *str, gssize length) {
  g_return_val_if_fail(IS_VALID_TYPE(checksum_type), NULL);
  g_return_val_if_fail(length == 0 || str != NULL, NULL);
  if (length < 0) length = strlen(str);
  return g_compute_checksum_for_data(checksum_type, (const guchar*)str, length);
}