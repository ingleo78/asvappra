#include <string.h>
#include "gbase64.h"
#include "gtestutils.h"
#include "glibintl.h"

static const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
gsize g_base64_encode_step(const guchar *in, gsize len, gboolean break_lines, gchar *out, gint *state, gint *save) {
  char *outptr;
  const guchar *inptr;
  g_return_val_if_fail(in != NULL, 0);
  g_return_val_if_fail(out != NULL, 0);
  g_return_val_if_fail(state != NULL, 0);
  g_return_val_if_fail(save != NULL, 0);
  if (len <= 0) return 0;
  inptr = in;
  outptr = out;
  if (len + ((char*)save)[0] > 2) {
      const guchar *inend = in+len-2;
      int c1, c2, c3;
      int already;
      already = *state;
      switch(((char*)save)[0]) {
          case 1:
              c1 = ((unsigned char*)save)[1];
              goto skip1;
          case 2:
              c1 = ((unsigned char*)save)[1];
              c2 = ((unsigned char*)save)[2];
              goto skip2;
      }
      while(inptr < inend) {
          c1 = *inptr++;
          skip1:
          c2 = *inptr++;
          skip2:
          c3 = *inptr++;
          *outptr++ = base64_alphabet[c1 >> 2];
          *outptr++ = base64_alphabet[c2 >> 4 | ((c1&0x3) << 4)];
          *outptr++ = base64_alphabet[((c2 &0x0f) << 2) | (c3 >> 6)];
          *outptr++ = base64_alphabet[c3 & 0x3f];
          if (break_lines && (++already) >= 19) {
              *outptr++ = '\n';
              already = 0;
          }
      }
      ((char*)save)[0] = 0;
      len = 2 - (inptr - inend);
      *state = already;
  }
  if (len>0) {
      char *saveout;
      saveout = &(((char*)save)[1]) + ((char*)save)[0];
      switch(len) {
          case 2: *saveout++ = *inptr++;
          case 1: *saveout++ = *inptr++;
      }
      ((char*)save)[0] += len;
  }
  return outptr - out;
}
gsize g_base64_encode_close(gboolean break_lines, gchar *out, gint *state, gint *save) {
  int c1, c2;
  char *outptr = out;
  g_return_val_if_fail(out != NULL, 0);
  g_return_val_if_fail(state != NULL, 0);
  g_return_val_if_fail(save != NULL, 0);
  c1 = ((unsigned char*)save)[1];
  c2 = ((unsigned char*)save)[2];
  switch(((char*)save)[0]) {
      case 2:
          outptr [2] = base64_alphabet[((c2 &0x0f) << 2)];
          g_assert(outptr [2] != 0);
          goto skip;
      case 1:
          outptr[2] = '=';
          skip:
          outptr[0] = base64_alphabet[ c1 >> 2];
          outptr[1] = base64_alphabet[ c2 >> 4 | ((c1&0x3) << 4)];
          outptr[3] = '=';
          outptr += 4;
          break;
  }
  if (break_lines) *outptr++ = '\n';
  *save = 0;
  *state = 0;
  return outptr - out;
}
gchar* g_base64_encode(const guchar *data, gsize len) {
  gchar *out;
  gint state = 0, outlen;
  gint save = 0;
  g_return_val_if_fail(data != NULL || len == 0, NULL);
  if (len >= ((G_MAXSIZE - 1) / 4 - 1) * 3) g_error("%s: input too large for Base64 encoding (%"G_GSIZE_FORMAT" chars)", G_STRLOC, len);
  out = g_malloc((len / 3 + 1) * 4 + 1);
  outlen = g_base64_encode_step(data, len, FALSE, out, &state, &save);
  outlen += g_base64_encode_close(FALSE, out + outlen, &state, &save);
  out[outlen] = '\0';
  return (gchar*)out;
}
static const unsigned char mime_base64_rank[256] = {
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
  255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};
gsize g_base64_decode_step(const gchar *in, gsize len, guchar *out, gint *state, guint *save) {
  const guchar *inptr;
  guchar *outptr;
  const guchar *inend;
  guchar c, rank;
  guchar last[2];
  unsigned int v;
  int i;
  g_return_val_if_fail(in != NULL, 0);
  g_return_val_if_fail(out != NULL, 0);
  g_return_val_if_fail(state != NULL, 0);
  g_return_val_if_fail(save != NULL, 0);
  if (len <= 0) return 0;
  inend = (const guchar*)in+len;
  outptr = out;
  v=*save;
  i=*state;
  inptr = (const guchar*)in;
  last[0] = last[1] = 0;
  while(inptr < inend) {
      c = *inptr++;
      rank = mime_base64_rank[c];
      if (rank != 0xff) {
          last[1] = last[0];
          last[0] = c;
          v = (v<<6) | rank;
          i++;
          if (i==4) {
              *outptr++ = v>>16;
              if (last[1] != '=') *outptr++ = v>>8;
              if (last[0] != '=') *outptr++ = v;
              i=0;
          }
      }
  }
  *save = v;
  *state = i;
  return outptr - out;
}
guchar* g_base64_decode(const gchar *text, gsize *out_len) {
  guchar *ret;
  gsize input_length;
  gint state = 0;
  guint save = 0;
  g_return_val_if_fail(text != NULL, NULL);
  g_return_val_if_fail(out_len != NULL, NULL);
  input_length = strlen(text);
  ret = g_malloc0((input_length / 4) * 3 + 1);
  *out_len = g_base64_decode_step(text, input_length, ret, &state, &save);
  return ret;
}
guchar* g_base64_decode_inplace(gchar *text, gsize *out_len) {
  gint input_length, state = 0;
  guint save = 0;
  g_return_val_if_fail(text != NULL, NULL);
  g_return_val_if_fail(out_len != NULL, NULL);
  input_length = strlen(text);
  g_return_val_if_fail(input_length > 1, NULL);
  *out_len = g_base64_decode_step(text, input_length, (guchar*)text, &state, &save);
  return (guchar*)text;
}