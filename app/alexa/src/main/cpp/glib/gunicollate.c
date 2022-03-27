#include <locale.h>
#include <string.h>
#ifdef __STDC_ISO_10646__
#include <wchar.h>
#endif
#ifdef HAVE_CARBON
#include <CoreServices/CoreServices.h>
#endif
#include "gmem.h"
#include "gunicode.h"
#include "gunicodeprivate.h"
#include "gstring.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#ifndef __STDC_ISO_10646__
#include "gconvert.h"
#endif
#ifdef _MSC_VER
static gsize msc_strxfrm_wrapper(char *string1, const char *string2, gsize count) {
  if (!string1 || count <= 0) {
      char tmp;
      return strxfrm(&tmp, string2, 1);
  }
  return strxfrm(string1, string2, count);
}
#define strxfrm msc_strxfrm_wrapper
#endif
gint g_utf8_collate(const gchar *str1, const gchar *str2) {
  gint result;
#ifdef HAVE_CARBON
  UniChar *str1_utf16;
  UniChar *str2_utf16;
  glong len1;
  glong len2;
  SInt32 retval = 0;
  g_return_val_if_fail(str1 != NULL, 0);
  g_return_val_if_fail(str2 != NULL, 0);
  str1_utf16 = g_utf8_to_utf16(str1, -1, NULL, &len1, NULL);
  str2_utf16 = g_utf8_to_utf16(str2, -1, NULL, &len2, NULL);
  UCCompareTextDefault(kUCCollateStandardOptions, str1_utf16, len1, str2_utf16, len2, NULL, &retval);
  result = retval;
  g_free(str2_utf16);
  g_free(str1_utf16);
#elif defined(__STDC_ISO_10646__)
  gunichar *str1_norm;
  gunichar *str2_norm;
  g_return_val_if_fail(str1 != NULL, 0);
  g_return_val_if_fail(str2 != NULL, 0);
  str1_norm = _g_utf8_normalize_wc(str1, -1, G_NORMALIZE_ALL_COMPOSE);
  str2_norm = _g_utf8_normalize_wc(str2, -1, G_NORMALIZE_ALL_COMPOSE);
  result = wcscoll((wchar_t*)str1_norm, (wchar_t*)str2_norm);
  g_free(str1_norm);
  g_free(str2_norm);
#else
  const gchar *charset;
  gchar *str1_norm;
  gchar *str2_norm;
  g_return_val_if_fail(str1 != NULL, 0);
  g_return_val_if_fail(str2 != NULL, 0);
  str1_norm = g_utf8_normalize(str1, -1, G_NORMALIZE_ALL_COMPOSE);
  str2_norm = g_utf8_normalize(str2, -1, G_NORMALIZE_ALL_COMPOSE);
  if (g_get_charset(&charset)) result = strcoll(str1_norm, str2_norm);
  else {
      gchar *str1_locale = g_convert(str1_norm, -1, charset, "UTF-8", NULL, NULL, NULL);
      gchar *str2_locale = g_convert(str2_norm, -1, charset, "UTF-8", NULL, NULL, NULL);
      if (str1_locale && str2_locale) result =  strcoll(str1_locale, str2_locale);
      else if (str1_locale) result = -1;
      else if (str2_locale) result = 1;
      else result = strcmp(str1_norm, str2_norm);
      g_free(str1_locale);
      g_free(str2_locale);
  }
  g_free(str1_norm);
  g_free(str2_norm);
#endif
  return result;
}
#if defined(__STDC_ISO_10646__) || defined(HAVE_CARBON)
static inline int utf8_encode(char *buf, wchar_t val) {
  int retval;
  if (val < 0x80) {
      if (buf) *buf++ = (char)val;
      retval = 1;
  } else {
      int step;
      for (step = 2; step < 6; ++step) if ((val & (~(guint32)0 << (5 * step + 1))) == 0) break;
      retval = step;
      if (buf) {
          *buf = (unsigned char) (~0xff >> step);
          --step;
          do {
              buf[step] = 0x80 | (val & 0x3f);
              val >>= 6;
          } while (--step > 0);
          *buf |= val;
	  }
  }
  return retval;
}
#endif
#ifdef HAVE_CARBON
static gchar* collate_key_to_string(UCCollationValue *key, gsize key_len) {
  gchar *result;
  gsize result_len;
  gsize i;
  if (key_len * sizeof(UCCollationValue) <= 8) return g_strdup("");
  result_len = 0;
  for (i = 8; i < key_len * sizeof(UCCollationValue); i++) result_len += utf8_encode(NULL, *((guchar*)key + i) + 1);
  result = g_malloc (result_len + 1);
  result_len = 0;
  for (i = 8; i < key_len * sizeof(UCCollationValue); i++) result_len += utf8_encode(result + result_len, *((guchar*)key + i) + 1);
  result[result_len] = 0;
  return result;
}
static gchar* carbon_collate_key_with_collator(const gchar *str, gssize len, CollatorRef collator) {
  UniChar *str_utf16 = NULL;
  glong len_utf16;
  OSStatus ret;
  UCCollationValue staticbuf[512];
  UCCollationValue *freeme = NULL;
  UCCollationValue *buf;
  ItemCount buf_len;
  ItemCount key_len;
  ItemCount try_len;
  gchar *result = NULL;
  str_utf16 = g_utf8_to_utf16(str, len, NULL, &len_utf16, NULL);
  try_len = len_utf16 * 5 + 2;
  if (try_len <= sizeof staticbuf) {
      buf = staticbuf;
      buf_len = sizeof staticbuf;
  } else {
      freeme = g_new(UCCollationValue, try_len);
      buf = freeme;
      buf_len = try_len;
  }
  ret = UCGetCollationKey(collator, str_utf16, len_utf16, buf_len, &key_len, buf);
  if (ret == kCollateBufferTooSmall) {
      freeme = g_renew(UCCollationValue, freeme, try_len * 2);
      buf = freeme;
      buf_len = try_len * 2;
      ret = UCGetCollationKey(collator, str_utf16, len_utf16, buf_len, &key_len, buf);
  }
  if (ret == 0) result = collate_key_to_string(buf, key_len);
  else result = g_strdup("");
  g_free(freeme);
  g_free(str_utf16);
  return result;
}
static gchar* carbon_collate_key(const gchar *str, gssize len) {
  static CollatorRef collator;
  if (G_UNLIKELY(!collator)) {
      UCCreateCollator(NULL, 0, kUCCollateStandardOptions, &collator);
      if (!collator) {
          static gboolean been_here;
          if (!been_here)
            g_warning("%s: UCCreateCollator failed", G_STRLOC);
          been_here = TRUE;
          return g_strdup("");
      }
  }
  return carbon_collate_key_with_collator(str, len, collator);
}
static gchar* carbon_collate_key_for_filename(const gchar *str, gssize len) {
  static CollatorRef collator;
  if (G_UNLIKELY (!collator)) {
      UCCreateCollator(NULL, 0, kUCCollateComposeInsensitiveMask | kUCCollateWidthInsensitiveMask | kUCCollateCaseInsensitiveMask | kUCCollateDigitsOverrideMask |
                       kUCCollateDigitsAsNumberMask | kUCCollatePunctuationSignificantMask, &collator);
      if (!collator) {
          static gboolean been_here;
          if (!been_here) g_warning("%s: UCCreateCollator failed", G_STRLOC);
          been_here = TRUE;
          return g_strdup("");
      }
  }
  return carbon_collate_key_with_collator(str, len, collator);
}
#endif
gchar* g_utf8_collate_key(const gchar *str, gssize len) {
  gchar *result;
#ifdef HAVE_CARBON
  g_return_val_if_fail (str != NULL, NULL);
  result = carbon_collate_key (str, len);
#elif defined(__STDC_ISO_10646__)
  gsize xfrm_len;
  gunichar *str_norm;
  wchar_t *result_wc;
  gsize i;
  gsize result_len = 0;
  g_return_val_if_fail(str != NULL, NULL);
  str_norm = _g_utf8_normalize_wc(str, len, G_NORMALIZE_ALL_COMPOSE);
  xfrm_len = wcsxfrm(NULL, (wchar_t*)str_norm, 0);
  result_wc = g_new(wchar_t, xfrm_len + 1);
  wcsxfrm(result_wc, (wchar_t*)str_norm, xfrm_len + 1);
  for (i=0; i < xfrm_len; i++) result_len += utf8_encode(NULL, result_wc[i]);
  result = g_malloc(result_len + 1);
  result_len = 0;
  for (i=0; i < xfrm_len; i++) result_len += utf8_encode(result + result_len, result_wc[i]);
  result[result_len] = '\0';
  g_free(result_wc);
  g_free(str_norm);
  return result;
#else
  gsize xfrm_len;
  const gchar *charset;
  gchar *str_norm;
  g_return_val_if_fail(str != NULL, NULL);
  str_norm = g_utf8_normalize(str, len, G_NORMALIZE_ALL_COMPOSE);
  result = NULL;
  if (g_get_charset(&charset)) {
      xfrm_len = strxfrm(NULL, str_norm, 0);
      if (xfrm_len >= 0 && xfrm_len < G_MAXINT - 2) {
          result = g_malloc(xfrm_len + 1);
          strxfrm(result, str_norm, xfrm_len + 1);
      }
  } else {
      gchar *str_locale = g_convert(str_norm, -1, charset, "UTF-8", NULL, NULL, NULL);
      if (str_locale) {
          xfrm_len = strxfrm(NULL, str_locale, 0);
          if (xfrm_len < 0 || xfrm_len >= G_MAXINT - 2) {
              g_free(str_locale);
              str_locale = NULL;
          }
	  }
      if (str_locale) {
          result = g_malloc(xfrm_len + 2);
          result[0] = 'A';
          strxfrm(result + 1, str_locale, xfrm_len + 1);
          g_free(str_locale);
	  }
  }
  if (!result) {
      xfrm_len = strlen(str_norm);
      result = g_malloc(xfrm_len + 2);
      result[0] = 'B';
      memcpy(result + 1, str_norm, xfrm_len);
      result[xfrm_len+1] = '\0';
  }
  g_free(str_norm);
#endif
  return result;
}
#define COLLATION_SENTINEL "\1\1\1"
gchar* g_utf8_collate_key_for_filename(const gchar *str, gssize len) {
#ifndef HAVE_CARBON
  GString *result;
  GString *append;
  const gchar *p;
  const gchar *prev;
  const gchar *end;
  gchar *collate_key;
  gint digits;
  gint leading_zeros;
  if (len < 0) len = strlen (str);
  result = g_string_sized_new(len * 2);
  append = g_string_sized_new(0);
  end = str + len;
  for (prev = p = str; p < end; p++) {
      switch(*p) {
          case '.':
              if (prev != p) {
                  collate_key = g_utf8_collate_key(prev, p - prev);
                  g_string_append(result, collate_key);
                  g_free(collate_key);
              }
              g_string_append(result, COLLATION_SENTINEL "\1");
              prev = p + 1;
              break;
          case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
              if (prev != p) {
                  collate_key = g_utf8_collate_key(prev, p - prev);
                  g_string_append(result, collate_key);
                  g_free(collate_key);
              }
              g_string_append (result, COLLATION_SENTINEL "\2");
              prev = p;
              if (*p == '0') {
                  leading_zeros = 1;
                  digits = 0;
              } else {
                  leading_zeros = 0;
                  digits = 1;
              }
              while(++p < end) {
                  if (*p == '0' && !digits) ++leading_zeros;
                  else if (g_ascii_isdigit(*p)) ++digits;
                  else {
                      if (!digits) {
                          ++digits;
                          --leading_zeros;
                      }
                      break;
                  }
              }
              while (digits > 1) {
                  g_string_append_c(result, ':');
                  --digits;
              }
              if (leading_zeros > 0) {
                  g_string_append_c(append, (char)leading_zeros);
                  prev += leading_zeros;
              }
              g_string_append_len(result, prev, p - prev);
              prev = p;
              --p;
              break;
	  }
  }
  if (prev != p) {
      collate_key = g_utf8_collate_key(prev, p - prev);
      g_string_append(result, collate_key);
      g_free(collate_key);
  }
  g_string_append(result, append->str);
  g_string_free(append, TRUE);
  return g_string_free(result, FALSE);
#else
  return carbon_collate_key_for_filename(str, len);
#endif
}