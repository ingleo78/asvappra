#include <stdlib.h>
#ifdef HAVE_CODESET
#include <langinfo.h>
#endif
#include <string.h>
#ifdef G_PLATFORM_WIN32
#include <stdio.h>
#define STRICT
#include <windows.h>
#undef STRICT
#endif
#include "libcharset/libcharset.h"
#include "gconvert.h"
#include "ghash.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "gtypes.h"
#include "gthread.h"
#include "glibintl.h"

#define UTF8_COMPUTE(Char, Mask, Len) \
  if (Char < 128) {                   \
      Len = 1;                        \
      Mask = 0x7f;                    \
  } else if ((Char & 0xe0) == 0xc0)	{ \
      Len = 2;						  \
      Mask = 0x1f;					  \
  }	else if ((Char & 0xf0) == 0xe0)	{ \
      Len = 3;						  \
      Mask = 0x0f;					  \
  }	else if ((Char & 0xf8) == 0xf0)	{ \
      Len = 4;						  \
      Mask = 0x07;					  \
  }	else if ((Char & 0xfc) == 0xf8)	{ \
      Len = 5;						  \
      Mask = 0x03;					  \
  }	else if ((Char & 0xfe) == 0xfc)	{ \
      Len = 6;						  \
      Mask = 0x01;					  \
  }	else Len = -1;
#define UTF8_LENGTH(Char)  ((Char) < 0x80 ? 1 : ((Char) < 0x800 ? 2 : ((Char) < 0x10000 ? 3 : ((Char) < 0x200000 ? 4 : ((Char) < 0x4000000 ? 5 : 6)))))
#define UTF8_GET(Result, Chars, Count, Mask, Len) \
  (Result) = (Chars)[0] & (Mask);				  \
  for ((Count) = 1; (Count) < (Len); ++(Count))	{ \
      if (((Chars)[(Count)] & 0xc0) != 0x80) {	  \
          (Result) = -1;						  \
          break;							      \
	  }								              \
      (Result) <<= 6;							  \
      (Result) |= ((Chars)[(Count)] & 0x3f);	  \
  }
#define UNICODE_VALID(Char)  ((Char) < 0x110000 && (((Char) & 0xFFFFF800) != 0xD800) && ((Char) < 0xFDD0 || (Char) > 0xFDEF) && ((Char) & 0xFFFE) != 0xFFFE)
const gchar utf8_skip_data[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};
const gchar* const g_utf8_skip = utf8_skip_data;
gchar* g_utf8_find_prev_char(const char *str, const char *p) {
  for (--p; p >= str; --p) {
      if ((*p & 0xc0) != 0x80) return (gchar*)p;
  }
  return NULL;
}
gchar* g_utf8_find_next_char(const gchar *p, const gchar *end) {
  if (*p) {
      if (end) for (++p; p < end && (*p & 0xc0) == 0x80; ++p);
      else for (++p; (*p & 0xc0) == 0x80; ++p);
  }
  return (p == end) ? NULL : (gchar*)p;
}
gchar* g_utf8_prev_char(const gchar *p) {
  while(TRUE) {
      p--;
      if ((*p & 0xc0) != 0x80) return (gchar*)p;
  }
}
glong g_utf8_strlen(const gchar *p, gssize max) {
  glong len = 0;
  const gchar *start = p;
  g_return_val_if_fail(p != NULL || max == 0, 0);
  if (max < 0) {
      while(*p) {
          p = g_utf8_next_char(p);
          ++len;
      }
  } else {
      if (max == 0 || !*p) return 0;
      p = g_utf8_next_char (p);
      while (p - start < max && *p) {
          ++len;
          p = g_utf8_next_char (p);
      }
      if (p - start <= max) ++len;
  }
  return len;
}
gunichar g_utf8_get_char(const gchar *p) {
  int i, mask = 0, len;
  gunichar result;
  unsigned char c = (unsigned char)*p;
  UTF8_COMPUTE(c, mask, len);
  if (len == -1) return (gunichar)-1;
  UTF8_GET(result, p, i, mask, len);
  return result;
}
gchar* g_utf8_offset_to_pointer(const gchar *str, glong offset) {
  const gchar *s = str;
  if (offset > 0) {
      while(offset--) s = g_utf8_next_char (s);
  } else {
      const char *s1;
      while(offset) {
          s1 = s;
          s += offset;
          while((*s & 0xc0) == 0x80) s--;
          offset += g_utf8_pointer_to_offset(s, s1);
	  }
  }
  return (gchar *)s;
}
glong g_utf8_pointer_to_offset(const gchar *str, const gchar *pos) {
  const gchar *s = str;
  glong offset = 0;
  if (pos < str) offset = - g_utf8_pointer_to_offset(pos, str);
  else {
      while(s < pos) {
          s = g_utf8_next_char(s);
          offset++;
      }
  }
  return offset;
}
gchar* g_utf8_strncpy(gchar *dest, const gchar *src, gsize n) {
  const gchar *s = src;
  while(n && *s) {
      s = g_utf8_next_char(s);
      n--;
  }
  strncpy(dest, src, s - src);
  dest[s - src] = 0;
  return dest;
}
G_LOCK_DEFINE_STATIC(aliases);
static GHashTable* get_alias_hash(void) {
  static GHashTable *alias_hash = NULL;
  const char *aliases;
  G_LOCK(aliases);
  if (!alias_hash) {
      alias_hash = g_hash_table_new (g_str_hash, g_str_equal);
      aliases = _g_locale_get_charset_aliases ();
      while(*aliases != '\0') {
          const char *canonical;
          const char *alias;
          const char **alias_array;
          int count = 0;
          alias = aliases;
          aliases += strlen (aliases) + 1;
          canonical = aliases;
          aliases += strlen (aliases) + 1;
          alias_array = g_hash_table_lookup (alias_hash, canonical);
          if (alias_array) {
              while(alias_array[count])
              count++;
          }
          alias_array = g_renew(const char *, alias_array, count + 2);
          alias_array[count] = alias;
          alias_array[count + 1] = NULL;
          g_hash_table_insert(alias_hash, (char *)canonical, alias_array);
	  }
  }
  G_UNLOCK(aliases);
  return alias_hash;
}
G_GNUC_INTERNAL const char** _g_charset_get_aliases(const char *canonical_name) {
  GHashTable *alias_hash = get_alias_hash();
  return g_hash_table_lookup(alias_hash, canonical_name);
}
static gboolean g_utf8_get_charset_internal(const char *raw_data, const char **a) {
  const char *charset = getenv("CHARSET");
  if (charset && *charset) {
      *a = charset;
      if (charset && strstr (charset, "UTF-8")) return TRUE;
      else return FALSE;
  }
  G_LOCK(aliases);
  charset = _g_locale_charset_unalias(raw_data);
  G_UNLOCK(aliases);
  if (charset && *charset) {
      *a = charset;
      if (charset && strstr(charset, "UTF-8")) return TRUE;
      else return FALSE;
  }
  *a = "US-ASCII";
  return FALSE;
}
typedef struct _GCharsetCache GCharsetCache;
struct _GCharsetCache {
  gboolean is_utf8;
  gchar *raw;
  gchar *charset;
};
static void charset_cache_free(gpointer data) {
  GCharsetCache *cache = data;
  g_free(cache->raw);
  g_free(cache->charset);
  g_free(cache);
}
gboolean g_get_charset(G_CONST_RETURN char **charset) {
  static GStaticPrivate cache_private = G_STATIC_PRIVATE_INIT;
  GCharsetCache *cache = g_static_private_get(&cache_private);
  const gchar *raw;
  if (!cache) {
      cache = g_new0(GCharsetCache, 1);
      g_static_private_set(&cache_private, cache, charset_cache_free);
  }
  raw = _g_locale_charset_raw();
  if (!(cache->raw && strcmp(cache->raw, raw) == 0)) {
      const gchar *new_charset;
      g_free(cache->raw);
      g_free(cache->charset);
      cache->raw = g_strdup(raw);
      cache->is_utf8 = g_utf8_get_charset_internal(raw, &new_charset);
      cache->charset = g_strdup(new_charset);
  }
  if (charset) *charset = cache->charset;
  return cache->is_utf8;
}
int g_unichar_to_utf8(gunichar c, gchar *outbuf) {
  guint len = 0;    
  int first;
  int i;
  if (c < 0x80) {
      first = 0;
      len = 1;
  } else if (c < 0x800) {
      first = 0xc0;
      len = 2;
  } else if (c < 0x10000) {
      first = 0xe0;
      len = 3;
  } else if (c < 0x200000) {
      first = 0xf0;
      len = 4;
  } else if (c < 0x4000000) {
      first = 0xf8;
      len = 5;
  } else {
      first = 0xfc;
      len = 6;
  }
  if (outbuf) {
      for (i = len - 1; i > 0; --i) {
          outbuf[i] = (c & 0x3f) | 0x80;
          c >>= 6;
	  }
      outbuf[0] = c | first;
  }
  return len;
}
gchar* g_utf8_strchr(const char *p, gssize len, gunichar c) {
  gchar ch[10];
  gint charlen = g_unichar_to_utf8(c, ch);
  ch[charlen] = '\0';
  return g_strstr_len(p, len, ch);
}
gchar* g_utf8_strrchr(const char *p, gssize len, gunichar c) {
  gchar ch[10];
  gint charlen = g_unichar_to_utf8(c, ch);
  ch[charlen] = '\0';
  return g_strrstr_len(p, len, ch);
}
static inline gunichar g_utf8_get_char_extended(const gchar *p, gssize max_len) {
  guint i, len;
  gunichar min_code;
  gunichar wc = (guchar)*p;
  if (wc < 0x80) return wc;
  else if (G_UNLIKELY (wc < 0xc0)) return (gunichar)-1;
  else if (wc < 0xe0) {
      len = 2;
      wc &= 0x1f;
      min_code = 1 << 7;
  } else if (wc < 0xf0) {
      len = 3;
      wc &= 0x0f;
      min_code = 1 << 11;
  } else if (wc < 0xf8) {
      len = 4;
      wc &= 0x07;
      min_code = 1 << 16;
  } else if (wc < 0xfc) {
      len = 5;
      wc &= 0x03;
      min_code = 1 << 21;
  } else if (wc < 0xfe) {
      len = 6;
      wc &= 0x01;
      min_code = 1 << 26;
  } else return (gunichar)-1;
  if (G_UNLIKELY(max_len >= 0 && len > max_len)) {
      for (i = 1; i < max_len; i++) {
	      if ((((guchar *)p)[i] & 0xc0) != 0x80) return (gunichar)-1;
	  }
      return (gunichar)-2;
  }
  for (i = 1; i < len; ++i) {
      gunichar ch = ((guchar*)p)[i];
      if (G_UNLIKELY ((ch & 0xc0) != 0x80)) {
          if (ch) return (gunichar)-1;
          else return (gunichar)-2;
	  }
      wc <<= 6;
      wc |= (ch & 0x3f);
  }
  if (G_UNLIKELY(wc < min_code)) return (gunichar)-1;
  return wc;
}
gunichar g_utf8_get_char_validated(const  gchar *p, gssize max_len) {
  gunichar result;
  if (max_len == 0) return (gunichar)-2;
  result = g_utf8_get_char_extended(p, max_len);
  if (result & 0x80000000) return result;
  else if (!UNICODE_VALID(result)) return (gunichar)-1;
  else return result;
}
gunichar* g_utf8_to_ucs4_fast(const gchar *str, glong len, glong *items_written) {
  gint j, charlen;
  gunichar *result;
  gint n_chars, i;
  const gchar *p;
  g_return_val_if_fail(str != NULL, NULL);
  p = str;
  n_chars = 0;
  if (len < 0) {
      while(*p) {
          p = g_utf8_next_char(p);
          ++n_chars;
	  }
  } else {
      while(p < str + len && *p) {
          p = g_utf8_next_char(p);
          ++n_chars;
	  }
  }
  result = g_new(gunichar, n_chars + 1);
  p = str;
  for (i=0; i < n_chars; i++) {
      gunichar wc = ((unsigned char*)p)[0];
      if (wc < 0x80) {
	  result[i] = wc;
	  p++;
	  } else {
          if (wc < 0xe0) {
              charlen = 2;
              wc &= 0x1f;
          } else if (wc < 0xf0) {
              charlen = 3;
              wc &= 0x0f;
          } else if (wc < 0xf8) {
              charlen = 4;
              wc &= 0x07;
          } else if (wc < 0xfc) {
              charlen = 5;
              wc &= 0x03;
          } else {
              charlen = 6;
              wc &= 0x01;
          }
          for (j = 1; j < charlen; j++) {
              wc <<= 6;
              wc |= ((unsigned char *)p)[j] & 0x3f;
          }
          result[i] = wc;
          p += charlen;
	  }
  }
  result[i] = 0;
  if (items_written) *items_written = i;
  return result;
}
gunichar* g_utf8_to_ucs4(const gchar *str, glong len, glong *items_read, glong *items_written, GError **error) {
  gunichar *result = NULL;
  gint n_chars, i;
  const gchar *in;
  in = str;
  n_chars = 0;
  while ((len < 0 || str + len - in > 0) && *in) {
      gunichar wc = g_utf8_get_char_extended(in, len < 0 ? 6 : str + len - in);
      if (wc & 0x80000000) {
          if (wc == (gunichar)-2) {
              if (items_read) break;
              else g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_PARTIAL_INPUT, _("Partial character sequence at end of input"));
          } else g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid byte sequence in conversion input"));
          goto err_out;
	  }
      n_chars++;
      in = g_utf8_next_char(in);
  }
  result = g_new(gunichar, n_chars + 1);
  in = str;
  for (i=0; i < n_chars; i++) {
      result[i] = g_utf8_get_char(in);
      in = g_utf8_next_char(in);
  }
  result[i] = 0;
  if (items_written) *items_written = n_chars;
  err_out:
  if (items_read) *items_read = in - str;
  return result;
}
gchar* g_ucs4_to_utf8(const gunichar *str, glong len, glong *items_read, glong *items_written, GError **error) {
  gint result_length;
  gchar *result = NULL;
  gchar *p;
  gint i;
  result_length = 0;
  for (i = 0; len < 0 || i < len ; i++) {
      if (!str[i]) break;
      if (str[i] >= 0x80000000) {
          g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Character out of range for UTF-8"));
          goto err_out;
	  }
      result_length += UTF8_LENGTH (str[i]);
  }
  result = g_malloc(result_length + 1);
  p = result;
  i = 0;
  while(p < result + result_length) p += g_unichar_to_utf8 (str[i++], p);
  *p = '\0';
  if (items_written) *items_written = p - result;
   err_out:
  if (items_read) *items_read = i;
  return result;
}
#define SURROGATE_VALUE(h,l) (((h) - 0xd800) * 0x400 + (l) - 0xdc00 + 0x10000)
gchar* g_utf16_to_utf8(const gunichar2 *str, glong len, glong *items_read, glong *items_written, GError **error) {
  const gunichar2 *in;
  gchar *out;
  gchar *result = NULL;
  gint n_bytes;
  gunichar high_surrogate;
  g_return_val_if_fail(str != NULL, NULL);
  n_bytes = 0;
  in = str;
  high_surrogate = 0;
  while((len < 0 || in - str < len) && *in) {
      gunichar2 c = *in;
      gunichar wc;
      if (c >= 0xdc00 && c < 0xe000) {
          if (high_surrogate) {
              wc = SURROGATE_VALUE(high_surrogate, c);
              high_surrogate = 0;
          } else {
              g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid sequence in conversion input"));
              goto err_out;
          }
	  } else {
          if (high_surrogate) {
              g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid sequence in conversion input"));
              goto err_out;
          }
          if (c >= 0xd800 && c < 0xdc00) {
              high_surrogate = c;
              goto next1;
          } else wc = c;
	  }
      n_bytes += UTF8_LENGTH (wc);
      next1:
      in++;
  }
  if (high_surrogate && !items_read) {
      g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_PARTIAL_INPUT, _("Partial character sequence at end of input"));
      goto err_out;
  }
  result = g_malloc(n_bytes + 1);
  high_surrogate = 0;
  out = result;
  in = str;
  while (out < result + n_bytes) {
      gunichar2 c = *in;
      gunichar wc;
      if (c >= 0xdc00 && c < 0xe000) {
          wc = SURROGATE_VALUE (high_surrogate, c);
          high_surrogate = 0;
	  } else if (c >= 0xd800 && c < 0xdc00) {
          high_surrogate = c;
          goto next2;
	  } else wc = c;
      out += g_unichar_to_utf8(wc, out);
      next2:
      in++;
  }
  *out = '\0';
  if (items_written) *items_written = out - result;
  err_out:
  if (items_read) *items_read = in - str;
  return result;
}
gunichar* g_utf16_to_ucs4(const gunichar2 *str, glong len, glong *items_read, glong *items_written, GError **error) {
  const gunichar2 *in;
  gchar *out;
  gchar *result = NULL;
  gint n_bytes;
  gunichar high_surrogate;
  g_return_val_if_fail(str != NULL, NULL);
  n_bytes = 0;
  in = str;
  high_surrogate = 0;
  while((len < 0 || in - str < len) && *in) {
      gunichar2 c = *in;
      gunichar wc;
      if (c >= 0xdc00 && c < 0xe000) {
          if (high_surrogate) {
              wc = SURROGATE_VALUE (high_surrogate, c);
              high_surrogate = 0;
          } else {
              g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid sequence in conversion input"));
              goto err_out;
          }
      } else {
          if (high_surrogate) {
              g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid sequence in conversion input"));
              goto err_out;
          }
          if (c >= 0xd800 && c < 0xdc00)  {
              high_surrogate = c;
              goto next1;
          } else wc = c;
	  }
      n_bytes += sizeof (gunichar);
      next1:
      in++;
  }
  if (high_surrogate && !items_read) {
      g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_PARTIAL_INPUT, _("Partial character sequence at end of input"));
      goto err_out;
  }
  result = g_malloc(n_bytes + 4);
  high_surrogate = 0;
  out = result;
  in = str;
  while (out < result + n_bytes) {
      gunichar2 c = *in;
      gunichar wc;
      if (c >= 0xdc00 && c < 0xe000) {
          wc = SURROGATE_VALUE (high_surrogate, c);
          high_surrogate = 0;
	  } else if (c >= 0xd800 && c < 0xdc00) {
          high_surrogate = c;
          goto next2;
	  } else wc = c;
      *(gunichar *)out = wc;
      out += sizeof (gunichar);
      next2:
      in++;
  }
  *(gunichar *)out = 0;
  if (items_written) *items_written = (out - result) / sizeof (gunichar);
  err_out:
  if (items_read) *items_read = in - str;
  return (gunichar *)result;
}
gunichar2* g_utf8_to_utf16(const gchar *str, glong len, glong *items_read, glong *items_written, GError **error) {
  gunichar2 *result = NULL;
  gint n16;
  const gchar *in;
  gint i;
  g_return_val_if_fail(str != NULL, NULL);
  in = str;
  n16 = 0;
  while ((len < 0 || str + len - in > 0) && *in) {
      gunichar wc = g_utf8_get_char_extended (in, len < 0 ? 6 : str + len - in);
      if (wc & 0x80000000) {
          if (wc == (gunichar)-2) {
              if (items_read) break;
              else g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_PARTIAL_INPUT, _("Partial character sequence at end of input"));
          } else g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid byte sequence in conversion input"));
          goto err_out;
	  }
      if (wc < 0xd800) n16 += 1;
      else if (wc < 0xe000) {
          g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid sequence in conversion input"));
          goto err_out;
	  } else if (wc < 0x10000) n16 += 1;
      else if (wc < 0x110000) n16 += 2;
      else {
          g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Character out of range for UTF-16"));
          goto err_out;
	  }
      in = g_utf8_next_char (in);
  }
  result = g_new (gunichar2, n16 + 1);
  in = str;
  for (i = 0; i < n16;) {
      gunichar wc = g_utf8_get_char(in);
      if (wc < 0x10000) result[i++] = wc;
      else {
          result[i++] = (wc - 0x10000) / 0x400 + 0xd800;
          result[i++] = (wc - 0x10000) % 0x400 + 0xdc00;
	  }
      in = g_utf8_next_char (in);
  }
  result[i] = 0;
  if (items_written) *items_written = n16;
  err_out:
  if (items_read) *items_read = in - str;
  return result;
}
gunichar2* g_ucs4_to_utf16(const gunichar *str, glong len, glong *items_read, glong *items_written, GError **error) {
  gunichar2 *result = NULL;
  gint n16;
  gint i, j;
  n16 = 0;
  i = 0;
  while((len < 0 || i < len) && str[i]) {
      gunichar wc = str[i];
      if (wc < 0xd800) n16 += 1;
      else if (wc < 0xe000) {
          g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid sequence in conversion input"));
          goto err_out;
	  } else if (wc < 0x10000) n16 += 1;
      else if (wc < 0x110000) n16 += 2;
      else {
          g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Character out of range for UTF-16"));
          goto err_out;
	  }
      i++;
  }
  result = g_new (gunichar2, n16 + 1);
  for (i = 0, j = 0; j < n16; i++) {
      gunichar wc = str[i];
      if (wc < 0x10000) result[j++] = wc;
      else {
          result[j++] = (wc - 0x10000) / 0x400 + 0xd800;
          result[j++] = (wc - 0x10000) % 0x400 + 0xdc00;
	  }
  }
  result[j] = 0;
  if (items_written) *items_written = n16;
  err_out:
  if (items_read) *items_read = i;
  return result;
}
#define CONTINUATION_CHAR                           \
 G_STMT_START {                                     \
    if ((*(guchar*)p & 0xc0) != 0x80) goto error;   \
    val <<= 6;                                      \
    val |= (*(guchar*)p) & 0x3f;                    \
 } G_STMT_END
static const gchar* fast_validate(const char *str) {
  gunichar val = 0;
  gunichar min = 0;
  const gchar *p;
  for (p = str; *p; p++) {
      if (*(guchar*)p < 128);
      else {
          const gchar *last;
          last = p;
          if ((*(guchar*)p & 0xe0) == 0xc0) {
              if (G_UNLIKELY((*(guchar*)p & 0x1e) == 0)) goto error;
              p++;
              if (G_UNLIKELY((*(guchar*)p & 0xc0) != 0x80)) goto error;
          } else {
              if ((*(guchar*)p & 0xf0) == 0xe0) {
                  min = (1 << 11);
                  val = *(guchar*)p & 0x0f;
                  goto TWO_REMAINING;
              } else if ((*(guchar*)p & 0xf8) == 0xf0) {
                  min = (1 << 16);
                  val = *(guchar*)p & 0x07;
              } else goto error;
              p++;
              CONTINUATION_CHAR;
              TWO_REMAINING:
              p++;
              CONTINUATION_CHAR;
              p++;
              CONTINUATION_CHAR;
              if (G_UNLIKELY(val < min)) goto error;
              if (G_UNLIKELY(!UNICODE_VALID(val))) goto error;
          }
          continue;
          error:
          return last;
	  }
  }
  return p;
}
static const gchar* fast_validate_len(const char *str, gssize max_len) {
  gunichar val = 0;
  gunichar min = 0;
  const gchar *p;
  g_assert(max_len >= 0);
  for (p = str; ((p - str) < max_len) && *p; p++) {
      if (*(guchar*)p < 128);
      else {
          const gchar *last;
          last = p;
          if ((*(guchar*)p & 0xe0) == 0xc0) {
              if (G_UNLIKELY(max_len - (p - str) < 2)) goto error;
              if (G_UNLIKELY((*(guchar*)p & 0x1e) == 0)) goto error;
              p++;
              if (G_UNLIKELY((*(guchar*)p & 0xc0) != 0x80)) goto error;
          } else {
              if ((*(guchar*)p & 0xf0) == 0xe0) {
                  if (G_UNLIKELY(max_len - (p - str) < 3)) goto error;
                  min = (1 << 11);
                  val = *(guchar*)p & 0x0f;
                  goto TWO_REMAINING;
              } else if ((*(guchar*)p & 0xf8) == 0xf0) {
              if (G_UNLIKELY (max_len - (p - str) < 4)) goto error;
                  min = (1 << 16);
                  val = *(guchar*)p & 0x07;
              } else goto error;
              p++;
              CONTINUATION_CHAR;
              TWO_REMAINING:
              p++;
              CONTINUATION_CHAR;
              p++;
              CONTINUATION_CHAR;
              if (G_UNLIKELY(val < min)) goto error;
              if (G_UNLIKELY(!UNICODE_VALID(val))) goto error;
          }
          continue;
          error:
          return last;
	  }
  }
  return p;
}
gboolean g_utf8_validate(const char *str, gssize max_len, const gchar **end) {
  const gchar *p;
  if (max_len < 0) p = fast_validate(str);
  else p = fast_validate_len(str, max_len);
  if (end) *end = p;
  if ((max_len >= 0 && p != str + max_len) || (max_len < 0 && *p != '\0')) return FALSE;
  else return TRUE;
}
gboolean g_unichar_validate(gunichar ch) {
  return UNICODE_VALID(ch);
}
gchar* g_utf8_strreverse(const gchar *str, gssize len) {
  gchar *r, *result;
  const gchar *p;
  if (len < 0) len = strlen(str);
  result = g_new(gchar, len + 1);
  r = result + len;
  p = str;
  while(r > result) {
      gchar *m, skip = g_utf8_skip[*(guchar*) p];
      r -= skip;
      for (m = r; skip; skip--) *m++ = *p++;
  }
  result[len] = 0;
  return result;
}
gchar* _g_utf8_make_valid(const gchar *name) {
  GString *string;
  const gchar *remainder, *invalid;
  gint remaining_bytes, valid_bytes;
  g_return_val_if_fail(name != NULL, NULL);
  string = NULL;
  remainder = name;
  remaining_bytes = strlen(name);
  while(remaining_bytes != 0) {
      if (g_utf8_validate(remainder, remaining_bytes, &invalid)) break;
      valid_bytes = invalid - remainder;
      if (string == NULL) string = g_string_sized_new(remaining_bytes);
      g_string_append_len(string, remainder, valid_bytes);
      g_string_append(string, "\357\277\275");
      remaining_bytes -= valid_bytes + 1;
      remainder = invalid + 1;
  }
  if (string == NULL) return g_strdup(name);
  g_string_append(string, remainder);
  g_assert(g_utf8_validate(string->str, -1, NULL));
  return g_string_free(string, FALSE);
}