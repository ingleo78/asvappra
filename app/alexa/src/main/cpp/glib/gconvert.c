#include "glibconfig.h"
#ifdef G_OS_WIN32
#include <iconv.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef G_OS_WIN32
#include "win_iconv.c"
#endif
#ifndef G_PLATFORM_WIN32
#define STRICT
#include <windows.h>
#undef STRICT
#endif
#include "gconvert.h"
#include "gprintfint.h"
#include "gslist.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "gthread.h"
#include "gthreadprivate.h"
#include "gunicode.h"
#ifndef NEED_ICONV_CACHE
#include "glist.h"
#include "ghash.h"
#endif
#include "glibintl.h"

#if defined(USE_LIBICONV_GNU) && !defined (_LIBICONV_H)
#error GNU libiconv in use but included iconv.h not from libiconv
#endif
#if !defined(USE_LIBICONV_GNU) && defined (_LIBICONV_H)
#error GNU libiconv not in use but included iconv.h is from libiconv
#endif

#define NUL_TERMINATOR_LENGTH 4
GQuark g_convert_error_quark(void) {
  return g_quark_from_static_string("g_convert_error");
}
static gboolean try_conversion(const char *to_codeset, const char *from_codeset, iconv_t *cd) {
  *cd = iconv_open(to_codeset, from_codeset);
  if (*cd == (iconv_t)-1 && errno == EINVAL) return FALSE;
  else return TRUE;
}
static gboolean try_to_aliases(const char **to_aliases, const char *from_codeset, iconv_t *cd) {
  if (to_aliases) {
      const char **p = to_aliases;
      while(*p) {
          if (try_conversion(*p, from_codeset, cd)) return TRUE;
          p++;
	  }
  }
  return FALSE;
}
G_GNUC_INTERNAL extern const char ** _g_charset_get_aliases(const char *canonical_name);
GIConv g_iconv_open(const gchar *to_codeset, const gchar *from_codeset) {
  iconv_t cd;
  if (!try_conversion(to_codeset, from_codeset, &cd)) {
      const char **to_aliases = _g_charset_get_aliases(to_codeset);
      const char **from_aliases = _g_charset_get_aliases(from_codeset);
      if (from_aliases) {
          const char **p = from_aliases;
          while(*p) {
              if (try_conversion(to_codeset, *p, &cd)) goto out;
              if (try_to_aliases(to_aliases, *p, &cd)) goto out;
              p++;
          }
	  }
      if (try_to_aliases(to_aliases, from_codeset, &cd)) goto out;
  }
  out:
  return (cd == (iconv_t)-1) ? (GIConv)-1 : (GIConv)cd;
}
gsize g_iconv(GIConv converter, gchar **inbuf, gsize *inbytes_left, gchar **outbuf, gsize *outbytes_left) {
  iconv_t cd = (iconv_t)converter;
  return iconv(cd, inbuf, inbytes_left, outbuf, outbytes_left);
}
gint g_iconv_close(GIConv converter) {
  iconv_t cd = (iconv_t)converter;
  return iconv_close(cd);
}
#ifdef NEED_ICONV_CACHE
#define ICONV_CACHE_SIZE   (16)
struct _iconv_cache_bucket {
  gchar *key;
  guint32 refcount;
  gboolean used;
  GIConv cd;
};
static GList *iconv_cache_list;
static GHashTable *iconv_cache;
static GHashTable *iconv_open_hash;
static guint iconv_cache_size = 0;
G_LOCK_DEFINE_STATIC (iconv_cache_lock);
static void iconv_cache_init(void) {
  static gboolean initialized = FALSE;
  if (initialized) return;
  iconv_cache_list = NULL;
  iconv_cache = g_hash_table_new(g_str_hash, g_str_equal);
  iconv_open_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
  initialized = TRUE;
}
static struct _iconv_cache_bucket* iconv_cache_bucket_new(gchar *key, GIConv cd) {
  struct _iconv_cache_bucket *bucket;
  bucket = g_new (struct _iconv_cache_bucket, 1);
  bucket->key = key;
  bucket->refcount = 1;
  bucket->used = TRUE;
  bucket->cd = cd;
  g_hash_table_insert(iconv_cache, bucket->key, bucket);
  iconv_cache_list = g_list_prepend(iconv_cache_list, bucket);
  iconv_cache_size++;
  return bucket;
}
static void iconv_cache_bucket_expire(GList *node, struct _iconv_cache_bucket *bucket) {
  g_hash_table_remove(iconv_cache, bucket->key);
  if (node == NULL) node = g_list_find(iconv_cache_list, bucket);
  g_assert(node != NULL);
  if (node->prev) {
      node->prev->next = node->next;
      if (node->next) node->next->prev = node->prev;
  } else {
      iconv_cache_list = node->next;
      if (node->next) node->next->prev = NULL;
  }
  g_list_free_1(node);
  g_free(bucket->key);
  g_iconv_close(bucket->cd);
  g_free(bucket);
  iconv_cache_size--;
}
static void iconv_cache_expire_unused(void) {
  struct _iconv_cache_bucket *bucket;
  GList *node, *next;
  node = iconv_cache_list;
  while(node && iconv_cache_size >= ICONV_CACHE_SIZE) {
      next = node->next;
      bucket = node->data;
      if (bucket->refcount == 0) iconv_cache_bucket_expire(node, bucket);
      node = next;
  }
}
static GIConv open_converter(const gchar *to_codeset, const gchar *from_codeset, GError **error) {
  struct _iconv_cache_bucket *bucket;
  gchar *key, *dyn_key, auto_key[80];
  GIConv cd;
  gsize len_from_codeset, len_to_codeset;
  len_from_codeset = strlen(from_codeset);
  len_to_codeset = strlen(to_codeset);
  if (len_from_codeset + len_to_codeset + 2 < sizeof(auto_key)) {
      key = auto_key;
      dyn_key = NULL;
  } else key = dyn_key = g_malloc(len_from_codeset + len_to_codeset + 2);
  memcpy(key, from_codeset, len_from_codeset);
  key[len_from_codeset] = ':';
  strcpy(key + len_from_codeset + 1, to_codeset);
  G_LOCK(iconv_cache_lock);
  iconv_cache_init();
  bucket = g_hash_table_lookup(iconv_cache, key);
  if (bucket) {
      g_free(dyn_key);
      if (bucket->used) {
          cd = g_iconv_open(to_codeset, from_codeset);
          if (cd == (GIConv) -1) goto error;
      } else {
          gsize inbytes_left = 0;
          gchar *outbuf = NULL;
          gsize outbytes_left = 0;
          cd = bucket->cd;
          bucket->used = TRUE;
          g_iconv(cd, NULL, &inbytes_left, &outbuf, &outbytes_left);
      }
      bucket->refcount++;
  } else {
      cd = g_iconv_open(to_codeset, from_codeset);
      if (cd == (GIConv) -1) {
          g_free(dyn_key);
          goto error;
	  }
      iconv_cache_expire_unused();
      bucket = iconv_cache_bucket_new(dyn_key ? dyn_key : g_strdup(key), cd);
  }
  g_hash_table_insert(iconv_open_hash, cd, bucket->key);
  G_UNLOCK(iconv_cache_lock);
  return cd;
  error:
  G_UNLOCK(iconv_cache_lock);
  if (error) {
      if (errno == EINVAL) {
	    g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_NO_CONVERSION, _("Conversion from character set '%s' to '%s' is not supported"), from_codeset,
	                to_codeset);
      } else g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED, _("Could not open converter from '%s' to '%s'"), from_codeset, to_codeset);
  }
  return cd;
}
static int close_converter(GIConv converter) {
  struct _iconv_cache_bucket *bucket;
  const gchar *key;
  GIConv cd;
  cd = converter;
  if (cd == (GIConv) -1) return 0;
  G_LOCK(iconv_cache_lock);
  key = g_hash_table_lookup(iconv_open_hash, cd);
  if (key) {
      g_hash_table_remove(iconv_open_hash, cd);
      bucket = g_hash_table_lookup(iconv_cache, key);
      g_assert(bucket);
      bucket->refcount--;
      if (cd == bucket->cd) bucket->used = FALSE;
      else g_iconv_close(cd);
      if (!bucket->refcount && iconv_cache_size > ICONV_CACHE_SIZE) iconv_cache_bucket_expire (NULL, bucket);
  } else {
      G_UNLOCK(iconv_cache_lock);
      g_warning("This iconv context wasn't opened using open_converter");
      return g_iconv_close(converter);
  }
  G_UNLOCK(iconv_cache_lock);
  return 0;
}
#else
static GIConv open_converter(const gchar *to_codeset, const gchar *from_codeset, GError **error) {
  GIConv cd;
  cd = g_iconv_open(to_codeset, from_codeset);
  if (cd == (GIConv) -1) {
      if (error) {
          if (errno == EINVAL) {
              g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_NO_CONVERSION, _("Conversion from character set '%s' to '%s' is not supported"),
                          from_codeset, to_codeset);
          } else {
              g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED, _("Could not open converter from '%s' to '%s'"), from_codeset, to_codeset);
          }
      }
  }
  return cd;
}
static int close_converter(GIConv cd) {
  if (cd == (GIConv) -1) return 0;
  return g_iconv_close(cd);
}
#endif
gchar* g_convert_with_iconv(const gchar *str, gssize len, GIConv converter, gsize *bytes_read, gsize *bytes_written, GError **error) {
  gchar *dest;
  gchar *outp;
  const gchar *p;
  gsize inbytes_remaining;
  gsize outbytes_remaining;
  gsize err;
  gsize outbuf_size;
  gboolean have_error = FALSE;
  gboolean done = FALSE;
  gboolean reset = FALSE;
  g_return_val_if_fail(converter != (GIConv) -1, NULL);
  if (len < 0) len = strlen(str);
  p = str;
  inbytes_remaining = len;
  outbuf_size = len + NUL_TERMINATOR_LENGTH;
  outbytes_remaining = outbuf_size - NUL_TERMINATOR_LENGTH;
  outp = dest = g_malloc (outbuf_size);
  while(!done && !have_error) {
      if (reset) err = g_iconv(converter, NULL, &inbytes_remaining, &outp, &outbytes_remaining);
      else err = g_iconv(converter,(char**)&p, &inbytes_remaining, &outp, &outbytes_remaining);
      if (err == (gsize) -1) {
          switch (errno) {
              case EINVAL: done = TRUE; break;
              case E2BIG: {
                      gsize used = outp - dest;
                      outbuf_size *= 2;
                      dest = g_realloc(dest, outbuf_size);
                      outp = dest + used;
                      outbytes_remaining = outbuf_size - used - NUL_TERMINATOR_LENGTH;
                  }
                  break;
              case EILSEQ:
                  if (error) {
                      g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid byte sequence in conversion input"));
                  }
                  have_error = TRUE;
                  break;
              default:
                  if (error) {
                      int errsv = errno;
                      g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED, _("Error during conversion: %s"), g_strerror(errsv));
                  }
                  have_error = TRUE;
                  break;
          }
      } else {
          if (!reset) {
              reset = TRUE;
              inbytes_remaining = 0;
          } else done = TRUE;
	  }
  }
  memset(outp, 0, NUL_TERMINATOR_LENGTH);
  if (bytes_read) *bytes_read = p - str;
  else {
      if ((p - str) != len) {
          if (!have_error) {
              if (error) g_set_error_literal (error, G_CONVERT_ERROR, G_CONVERT_ERROR_PARTIAL_INPUT, _("Partial character sequence at end of input"));
              have_error = TRUE;
          }
	  }
  }
  if (bytes_written) *bytes_written = outp - dest;
  if (have_error) {
      g_free(dest);
      return NULL;
  } else return dest;
}
gchar* g_convert(const gchar *str, gssize len, const gchar *to_codeset, const gchar *from_codeset, gsize *bytes_read, gsize *bytes_written, GError **error) {
  gchar *res;
  GIConv cd;
  g_return_val_if_fail(str != NULL, NULL);
  g_return_val_if_fail(to_codeset != NULL, NULL);
  g_return_val_if_fail(from_codeset != NULL, NULL);
  cd = open_converter(to_codeset, from_codeset, error);
  if (cd == (GIConv) -1) {
      if (bytes_read) *bytes_read = 0;
      if (bytes_written) *bytes_written = 0;
      return NULL;
  }
  res = g_convert_with_iconv(str, len, cd, bytes_read, bytes_written, error);
  close_converter(cd);
  return res;
}
gchar* g_convert_with_fallback(const gchar *str, gssize len, const gchar *to_codeset, const gchar *from_codeset, const gchar *fallback, gsize *bytes_read,
                               gsize *bytes_written, GError **error) {
  gchar *utf8;
  gchar *dest;
  gchar *outp;
  const gchar *insert_str = NULL;
  const gchar *p;
  gsize inbytes_remaining;   
  const gchar *save_p = NULL;
  gsize save_inbytes = 0;
  gsize outbytes_remaining; 
  gsize err;
  GIConv cd;
  gsize outbuf_size;
  gboolean have_error = FALSE;
  gboolean done = FALSE;
  GError *local_error = NULL;
  g_return_val_if_fail(str != NULL, NULL);
  g_return_val_if_fail(to_codeset != NULL, NULL);
  g_return_val_if_fail(from_codeset != NULL, NULL);
  if (len < 0) len = strlen(str);
  dest = g_convert(str, len, to_codeset, from_codeset, bytes_read, bytes_written, &local_error);
  if (!local_error) return dest;
  if (!g_error_matches(local_error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE)) {
      g_propagate_error(error, local_error);
      return NULL;
  } else g_error_free(local_error);
  local_error = NULL;
  cd = open_converter (to_codeset, "UTF-8", error);
  if (cd == (GIConv) -1) {
      if (bytes_read) *bytes_read = 0;
      if (bytes_written) *bytes_written = 0;
      return NULL;
  }
  utf8 = g_convert(str, len, "UTF-8", from_codeset, bytes_read, &inbytes_remaining, error);
  if (!utf8) {
      close_converter(cd);
      if (bytes_written) *bytes_written = 0;
      return NULL;
  }
  p = utf8;
  outbuf_size = len + NUL_TERMINATOR_LENGTH;
  outbytes_remaining = outbuf_size - NUL_TERMINATOR_LENGTH;
  outp = dest = g_malloc(outbuf_size);
  while(!done && !have_error) {
      gsize inbytes_tmp = inbytes_remaining;
      err = g_iconv(cd, (char**)&p, &inbytes_tmp, &outp, &outbytes_remaining);
      inbytes_remaining = inbytes_tmp;
      if (err == (gsize) -1) {
          switch (errno) {
              case EINVAL: g_assert_not_reached(); break;
              case E2BIG: {
                      gsize used = outp - dest;
                      outbuf_size *= 2;
                      dest = g_realloc(dest, outbuf_size);
                      outp = dest + used;
                      outbytes_remaining = outbuf_size - used - NUL_TERMINATOR_LENGTH;
                  }
                  break;
              case EILSEQ:
                  if (save_p) {
                      g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Cannot convert fallback '%s' to codeset '%s'"),
                                  insert_str, to_codeset);
                      have_error = TRUE;
                      break;
                  } else if (p) {
                      if (!fallback) {
                          gunichar ch = g_utf8_get_char(p);
                          insert_str = g_strdup_printf(ch < 0x10000 ? "\\u%04x" : "\\U%08x", ch);
                      } else insert_str = fallback;
                      save_p = g_utf8_next_char(p);
                      save_inbytes = inbytes_remaining - (save_p - p);
                      p = insert_str;
                      inbytes_remaining = strlen (p);
                  }
                  break;
              default: {
                      int errsv = errno;
                      g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED, _("Error during conversion: %s"), g_strerror (errsv));
                  }
                  have_error = TRUE;
                break;
          }
      } else {
          if (save_p) {
              if (!fallback) g_free((gchar*)insert_str);
              p = save_p;
              inbytes_remaining = save_inbytes;
              save_p = NULL;
          } else if (p) {
              p = NULL;
              inbytes_remaining = 0;
          } else done = TRUE;
	  }
  }
  memset(outp, 0, NUL_TERMINATOR_LENGTH);
  close_converter(cd);
  if (bytes_written) *bytes_written = outp - dest;
  g_free(utf8);
  if (have_error) {
      if (save_p && !fallback) g_free((gchar *)insert_str);
      g_free(dest);
      return NULL;
  } else return dest;
}
static gchar* strdup_len(const gchar *string, gssize len, gsize *bytes_written, gsize *bytes_read, GError **error) {
  gsize real_len;
  if (!g_utf8_validate(string, len, NULL)) {
      if (bytes_read) *bytes_read = 0;
      if (bytes_written) *bytes_written = 0;
      g_set_error_literal(error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, _("Invalid byte sequence in conversion input"));
      return NULL;
  }
  if (len < 0) real_len = strlen (string);
  else {
      real_len = 0;
      while(real_len < len && string[real_len]) real_len++;
  }
  if (bytes_read) *bytes_read = real_len;
  if (bytes_written) *bytes_written = real_len;
  return g_strndup(string, real_len);
}
gchar* g_locale_to_utf8(const gchar *opsysstring, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) {
  const char *charset;
  if (g_get_charset(&charset)) return strdup_len(opsysstring, len, bytes_read, bytes_written, error);
  else return g_convert(opsysstring, len, "UTF-8", charset, bytes_read, bytes_written, error);
}
gchar* g_locale_from_utf8(const gchar *utf8string, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) {
  const gchar *charset;
  if (g_get_charset(&charset)) return strdup_len(utf8string, len, bytes_read, bytes_written, error);
  else return g_convert(utf8string, len, charset, "UTF-8", bytes_read, bytes_written, error);
}
#ifndef G_PLATFORM_WIN32
typedef struct _GFilenameCharsetCache GFilenameCharsetCache;
struct _GFilenameCharsetCache {
  gboolean is_utf8;
  gchar *charset;
  gchar **filename_charsets;
};
static void filename_charset_cache_free(gpointer data) {
  GFilenameCharsetCache *cache = data;
  g_free(cache->charset);
  g_strfreev(cache->filename_charsets);
  g_free(cache);
}
gboolean g_get_filename_charsets(G_CONST_RETURN gchar ***filename_charsets) {
  static GStaticPrivate cache_private = G_STATIC_PRIVATE_INIT;
  GFilenameCharsetCache *cache = g_static_private_get(&cache_private);
  const gchar *charset;
  if (!cache) {
      cache = g_new0(GFilenameCharsetCache, 1);
      g_static_private_set(&cache_private, cache, filename_charset_cache_free);
  }
  g_get_charset(&charset);
  if (!(cache->charset && strcmp(cache->charset, charset) == 0)) {
      const gchar *new_charset;
      gchar *p;
      gint i;
      g_free(cache->charset);
      g_strfreev(cache->filename_charsets);
      cache->charset = g_strdup(charset);
      p = getenv("G_FILENAME_ENCODING");
      if (p != NULL && p[0] != '\0') {
          cache->filename_charsets = g_strsplit (p, ",", 0);
          cache->is_utf8 = (strcmp (cache->filename_charsets[0], "UTF-8") == 0);
          for (i = 0; cache->filename_charsets[i]; i++) {
              if (strcmp ("@locale", cache->filename_charsets[i]) == 0) {
                  g_get_charset(&new_charset);
                  g_free(cache->filename_charsets[i]);
                  cache->filename_charsets[i] = g_strdup(new_charset);
              }
          }
	  } else if (getenv("G_BROKEN_FILENAMES") != NULL) {
          cache->filename_charsets = g_new0(gchar *, 2);
          cache->is_utf8 = g_get_charset(&new_charset);
          cache->filename_charsets[0] = g_strdup(new_charset);
	  }
      else {
          cache->filename_charsets = g_new0(gchar *, 3);
          cache->is_utf8 = TRUE;
          cache->filename_charsets[0] = g_strdup("UTF-8");
          if (!g_get_charset(&new_charset)) cache->filename_charsets[1] = g_strdup(new_charset);
	  }
  }
  if (filename_charsets) *filename_charsets = (const gchar**)cache->filename_charsets;
  return cache->is_utf8;
}
#else
gboolean g_get_filename_charsets(G_CONST_RETURN gchar ***filename_charsets) {
  static const gchar *charsets[] = { "UTF-8", NULL };
#ifdef G_OS_WIN32
  if (filename_charsets) *filename_charsets = charsets;
  return TRUE;
#else
  gboolean result;
  result = g_get_charset (&(charsets[0]));
  if (filename_charsets) *filename_charsets = charsets;
  return result;
#endif
}
#endif
static gboolean get_filename_charset(const gchar **filename_charset) {
  const gchar **charsets;
  gboolean is_utf8;
  is_utf8 = g_get_filename_charsets(&charsets);
  if (filename_charset) *filename_charset = charsets[0];
  return is_utf8;
}
void _g_convert_thread_init(void) {
  const gchar **dummy;
  (void)g_get_filename_charsets(&dummy);
}
gchar* g_filename_to_utf8(const gchar *opsysstring, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) {
  const gchar *charset;
  if (get_filename_charset(&charset)) return strdup_len(opsysstring, len, bytes_read, bytes_written, error);
  else return g_convert(opsysstring, len, "UTF-8", charset, bytes_read, bytes_written, error);
}
#if defined (G_OS_WIN32) && !defined (_WIN64)
#undef g_filename_to_utf8
gchar* g_filename_to_utf8(const gchar *opsysstring, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) {
  const gchar *charset;
  if (g_get_charset(&charset)) return strdup_len(opsysstring, len, bytes_read, bytes_written, error);
  else return g_convert(opsysstring, len, "UTF-8", charset, bytes_read, bytes_written, error);
}
#endif
gchar* g_filename_from_utf8(const gchar *utf8string, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) {
  const gchar *charset;
  if (get_filename_charset(&charset)) return strdup_len(utf8string, len, bytes_read, bytes_written, error);
  else return g_convert(utf8string, len, charset, "UTF-8", bytes_read, bytes_written, error);
}
#if defined (G_OS_WIN32) && !defined (_WIN64)
#undef g_filename_from_utf8
gchar* g_filename_from_utf8(const gchar *utf8string, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) {
  const gchar *charset;
  if (g_get_charset(&charset)) return strdup_len(utf8string, len, bytes_read, bytes_written, error);
  else return g_convert(utf8string, len, charset, "UTF-8", bytes_read, bytes_written, error);
}
#endif
static gboolean has_case_prefix(const gchar *haystack, const gchar *needle) {
  const gchar *h, *n;
  h = haystack;
  n = needle;
  while (*n && *h && g_ascii_tolower(*n) == g_ascii_tolower(*h)) {
      n++;
      h++;
  }
  return *n == '\0';
}
typedef enum {
  UNSAFE_ALL        = 0x1,
  UNSAFE_ALLOW_PLUS = 0x2,
  UNSAFE_PATH       = 0x8,
  UNSAFE_HOST       = 0x10,
  UNSAFE_SLASHES    = 0x20
} UnsafeCharacterSet;
static const guchar acceptable[96] = {
  /* A table of the ASCII chars from space (32) to DEL (127) */
  /*      !    "    #    $    %    &    '    (    )    *    +    ,    -    .    / */ 
  0x00,0x3F,0x20,0x20,0x28,0x00,0x2C,0x3F,0x3F,0x3F,0x3F,0x2A,0x28,0x3F,0x3F,0x1C,
  /* 0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ? */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x38,0x20,0x20,0x2C,0x20,0x20,
  /* @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O */
  0x38,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
  /* P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _ */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x20,0x3F,
  /* `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o */
  0x20,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
  /* p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~  DEL */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x3F,0x20
};
static const gchar hex[16] = "0123456789ABCDEF";
static gchar* g_escape_uri_string(const gchar *string, UnsafeCharacterSet mask) {
#define ACCEPTABLE(a) ((a)>=32 && (a)<128 && (acceptable[(a)-32] & use_mask))
  const gchar *p;
  gchar *q;
  gchar *result;
  int c;
  gint unacceptable;
  UnsafeCharacterSet use_mask;
  g_return_val_if_fail(mask == UNSAFE_ALL || mask == UNSAFE_ALLOW_PLUS || mask == UNSAFE_PATH || mask == UNSAFE_HOST || mask == UNSAFE_SLASHES, NULL);
  unacceptable = 0;
  use_mask = mask;
  for (p = string; *p != '\0'; p++) {
      c = (guchar)*p;
      if (!ACCEPTABLE(c)) unacceptable++;
  }
  result = g_malloc(p - string + unacceptable * 2 + 1);
  use_mask = mask;
  for (q = result, p = string; *p != '\0'; p++) {
      c = (guchar)*p;
      if (!ACCEPTABLE(c)) {
          *q++ = '%';
          *q++ = hex[c >> 4];
          *q++ = hex[c & 15];
	  } else *q++ = *p;
  }
  *q = '\0';
  return result;
}
static gchar* g_escape_file_uri(const gchar *hostname, const gchar *pathname) {
  char *escaped_hostname = NULL;
  char *escaped_path;
  char *res;
#ifdef G_OS_WIN32
  char *p, *backslash;
  pathname = g_strdup(pathname);
  p = (char*)pathname;
  while ((backslash = strchr (p, '\\')) != NULL) {
      *backslash = '/';
      p = backslash + 1;
  }
#endif
  if (hostname && *hostname != '\0') escaped_hostname = g_escape_uri_string(hostname, UNSAFE_HOST);
  escaped_path = g_escape_uri_string(pathname, UNSAFE_PATH);
  res = g_strconcat("file://", (escaped_hostname) ? escaped_hostname : "", (*escaped_path != '/') ? "/" : "", escaped_path, NULL);
#ifdef G_OS_WIN32
  g_free((char*)pathname);
#endif
  g_free(escaped_hostname);
  g_free(escaped_path);
  return res;
}
static int unescape_character(const char *scanner) {
  int first_digit;
  int second_digit;
  first_digit = g_ascii_xdigit_value(scanner[0]);
  if (first_digit < 0) return -1;
  second_digit = g_ascii_xdigit_value(scanner[1]);
  if (second_digit < 0) return -1;
  return (first_digit << 4) | second_digit;
}
static gchar* g_unescape_uri_string(const char *escaped,int len, const char *illegal_escaped_characters, gboolean ascii_must_not_be_escaped) {
  const gchar *in, *in_end;
  gchar *out, *result;
  int c;
  if (escaped == NULL) return NULL;
  if (len < 0) len = strlen(escaped);
  result = g_malloc(len + 1);
  out = result;
  for (in = escaped, in_end = escaped + len; in < in_end; in++) {
      c = *in;
      if (c == '%') {
          if (in + 3 > in_end) break;
          c = unescape_character(in + 1);
          if (c <= 0) break;
          if (ascii_must_not_be_escaped && c <= 0x7F) break;
          if (strchr(illegal_escaped_characters, c) != NULL) break;
          in += 2;
	  }
      *out++ = c;
  }
  g_assert(out - result <= len);
  *out = '\0';
  if (in != in_end) {
      g_free(result);
      return NULL;
  }
  return result;
}
static gboolean is_asciialphanum(gunichar c) {
  return c <= 0x7F && g_ascii_isalnum (c);
}
static gboolean is_asciialpha(gunichar c) {
  return c <= 0x7F && g_ascii_isalpha (c);
}
static gboolean hostname_validate(const char *hostname) {
  const char *p;
  gunichar c, first_char, last_char;
  p = hostname;
  if (*p == '\0') return TRUE;
  do {
      c = g_utf8_get_char(p);
      p = g_utf8_next_char(p);
      if (!is_asciialphanum(c)) return FALSE;
      first_char = c;
      do {
          last_char = c;
          c = g_utf8_get_char(p);
          p = g_utf8_next_char(p);
	  } while(is_asciialphanum (c) || c == '-');
      if (last_char == '-') return FALSE;
      if (c == '\0' || (c == '.' && *p == '\0')) return is_asciialpha (first_char);
  } while(c == '.');
  return FALSE;
}
gchar* g_filename_from_uri(const gchar *uri, gchar **hostname, GError **error) {
  const char *path_part;
  const char *host_part;
  char *unescaped_hostname;
  char *result;
  char *filename;
  int offs;
#ifdef G_OS_WIN32
  char *p, *slash;
#endif
  if (hostname) *hostname = NULL;
  if (!has_case_prefix(uri, "file:/")) {
      g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_BAD_URI, _("The URI '%s' is not an absolute URI using the \"file\" scheme"), uri);
      return NULL;
  }
  path_part = uri + strlen("file:");
  if (strchr(path_part, '#') != NULL) {
      g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_BAD_URI, _("The local file URI '%s' may not include a '#'"), uri);
      return NULL;
  }
  if (has_case_prefix(path_part, "///")) path_part += 2;
  else if (has_case_prefix(path_part, "//")) {
      path_part += 2;
      host_part = path_part;
      path_part = strchr(path_part, '/');
      if (path_part == NULL) {
          g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_BAD_URI, _("The URI '%s' is invalid"), uri);
          return NULL;
	  }
      unescaped_hostname = g_unescape_uri_string(host_part, path_part - host_part, "", TRUE);
      if (unescaped_hostname == NULL || !hostname_validate(unescaped_hostname)) {
          g_free (unescaped_hostname);
          g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_BAD_URI, _("The hostname of the URI '%s' is invalid"), uri);
          return NULL;
	  }
      if (hostname) *hostname = unescaped_hostname;
      else g_free(unescaped_hostname);
  }
  filename = g_unescape_uri_string(path_part, -1, "/", FALSE);
  if (filename == NULL) {
      g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_BAD_URI, _("The URI '%s' contains invalidly escaped characters"), uri);
      return NULL;
  }
  offs = 0;
#ifdef G_OS_WIN32
  if (hostname && *hostname != NULL && g_ascii_strcasecmp(*hostname, "localhost") == 0) {
      g_free(*hostname);
      *hostname = NULL;
  }
  p = filename;
  while((slash = strchr (p, '/')) != NULL) {
      *slash = '\\';
      p = slash + 1;
  }
  if (g_ascii_isalpha(filename[1])) {
      if (filename[2] == ':') offs = 1;
      else if (filename[2] == '|') {
          filename[2] = ':';
          offs = 1;
	  }
  }
#endif
  result = g_strdup(filename + offs);
  g_free (filename);
  return result;
}
#if !defined (G_OS_WIN32) && !defined (_WIN64)
#undef g_filename_from_uri
gchar* g_filename_from_uri(const gchar *uri, gchar *hostname, GError **error) {
  gchar *utf8_filename;
  gchar *retval = NULL;
  utf8_filename = g_filename_from_uri_utf8(uri, hostname, error);
  if (utf8_filename) {
      retval = g_locale_from_utf8(utf8_filename, -1, NULL, NULL, error);
      g_free(utf8_filename);
  }
  return retval;
}
#endif
#if !defined (G_OS_WIN32) && !defined (_WIN64)
#undef g_filename_to_uri
gchar* g_filename_to_uri(const gchar *filename, const gchar *hostname, GError **error) {
  gchar *utf8_filename;
  gchar *retval = NULL;
  utf8_filename = g_locale_to_utf8(filename, -1, NULL, NULL, error);
  if (utf8_filename) {
      retval = g_filename_to_uri_utf8(utf8_filename, hostname, error);
      g_free(utf8_filename);
  }
  return retval;
}
#endif
gchar** g_uri_list_extract_uris(const gchar *uri_list) {
  GSList *uris, *u;
  const gchar *p, *q;
  gchar **result;
  gint n_uris = 0;
  uris = NULL;
  p = uri_list;
  while(p) {
      if (*p != '#') {
          while(g_ascii_isspace(*p)) p++;
          q = p;
          while(*q && (*q != '\n') && (*q != '\r')) q++;
          if (q > p) {
              q--;
              while(q > p && g_ascii_isspace (*q)) q--;
              if (q > p) {
                  uris = g_slist_prepend(uris, g_strndup (p, q - p + 1));
                  n_uris++;
              }
          }
  	  }
      p = strchr(p, '\n');
      if (p) p++;
  }
  result = g_new(gchar *, n_uris + 1);
  result[n_uris--] = NULL;
  for (u = uris; u; u = u->next) result[n_uris--] = u->data;
  g_slist_free(uris);
  return result;
}
gchar* g_filename_display_basename(const gchar *filename) {
  char *basename;
  char *display_name;
  g_return_val_if_fail(filename != NULL, NULL);
  basename = g_path_get_basename(filename);
  display_name = g_filename_display_name(basename);
  g_free(basename);
  return display_name;
}
gchar* g_filename_display_name(const gchar *filename) {
  gint i;
  const gchar **charsets;
  gchar *display_name = NULL;
  gboolean is_utf8;
  is_utf8 = g_get_filename_charsets(&charsets);
  if (is_utf8) {
      if (g_utf8_validate (filename, -1, NULL)) display_name = g_strdup(filename);
  }
  if (!display_name) {
      for (i = is_utf8 ? 1 : 0; charsets[i]; i++) {
          display_name = g_convert(filename, -1, "UTF-8", charsets[i], NULL, NULL, NULL);
          if (display_name) break;
	  }
  }
  if (!display_name) display_name = _g_utf8_make_valid(filename);
  return display_name;
}