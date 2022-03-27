#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "gstring.h"
#include "gprintf.h"

struct _GStringChunk {
  GHashTable *const_table;
  GSList *storage_list;
  gsize storage_next;
  gsize this_size;
  gsize default_size;
};
gboolean g_str_equal(gconstpointer v1, gconstpointer v2) {
  const gchar *string1 = v1;
  const gchar *string2 = v2;
  return strcmp(string1, string2) == 0;
}
guint g_str_hash(gconstpointer v) {
  const signed char *p;
  guint32 h = 5381;
  for (p = v; *p != '\0'; p++) h = (h << 5) + h + *p;
  return h;
}
#define MY_MAXSIZE ((gsize)-1)
static inline gsize nearest_power(gsize base, gsize num) {
  if (num > MY_MAXSIZE / 2) return MY_MAXSIZE;
  else {
      gsize n = base;
      while(n < num) n <<= 1;
      return n;
  }
}
GStringChunk* g_string_chunk_new(gsize size) {
  GStringChunk *new_chunk = g_new(GStringChunk, 1);
  gsize actual_size = 1;
  actual_size = nearest_power(1, size);
  new_chunk->const_table = NULL;
  new_chunk->storage_list = NULL;
  new_chunk->storage_next = actual_size;
  new_chunk->default_size = actual_size;
  new_chunk->this_size = actual_size;
  return new_chunk;
}
void g_string_chunk_free(GStringChunk *chunk) {
  GSList *tmp_list;
  g_return_if_fail(chunk != NULL);
  if (chunk->storage_list) {
      for (tmp_list = chunk->storage_list; tmp_list; tmp_list = tmp_list->next) g_free (tmp_list->data);
      g_slist_free(chunk->storage_list);
  }
  if (chunk->const_table) g_hash_table_destroy(chunk->const_table);
  g_free(chunk);
}
void g_string_chunk_clear(GStringChunk *chunk) {
  GSList *tmp_list;
  g_return_if_fail(chunk != NULL);
  if (chunk->storage_list) {
      for (tmp_list = chunk->storage_list; tmp_list; tmp_list = tmp_list->next) g_free(tmp_list->data);
      g_slist_free(chunk->storage_list);
      chunk->storage_list = NULL;
      chunk->storage_next = chunk->default_size;
      chunk->this_size = chunk->default_size;
  }
  if (chunk->const_table) g_hash_table_remove_all(chunk->const_table);
}
gchar* g_string_chunk_insert(GStringChunk *chunk, const gchar *string) {
  g_return_val_if_fail(chunk != NULL, NULL);
  return g_string_chunk_insert_len(chunk, string, -1);
}
gchar* g_string_chunk_insert_const(GStringChunk *chunk, const gchar  *string) {
  char* lookup;
  g_return_val_if_fail(chunk != NULL, NULL);
  if (!chunk->const_table) chunk->const_table = g_hash_table_new(g_str_hash, g_str_equal);
  lookup = (char*)g_hash_table_lookup(chunk->const_table, (gchar*)string);
  if (!lookup) {
      lookup = g_string_chunk_insert(chunk, string);
      g_hash_table_insert(chunk->const_table, lookup, lookup);
  }
  return lookup;
}
gchar* g_string_chunk_insert_len(GStringChunk *chunk, const gchar *string, gssize len) {
  gssize size;
  gchar* pos;
  g_return_val_if_fail(chunk != NULL, NULL);
  if (len < 0) size = strlen (string);
  else size = len;
  if ((chunk->storage_next + size + 1) > chunk->this_size) {
      gsize new_size = nearest_power (chunk->default_size, size + 1);
      chunk->storage_list = g_slist_prepend (chunk->storage_list, g_new (gchar, new_size));
      chunk->this_size = new_size;
      chunk->storage_next = 0;
  }
  pos = ((gchar*)chunk->storage_list->data) + chunk->storage_next;
  *(pos + size) = '\0';
  memcpy(pos, string, size);
  chunk->storage_next += size + 1;
  return pos;
}
static void g_string_maybe_expand(GString* string, gsize len) {
  if (string->len + len >= string->allocated_len) {
      string->allocated_len = nearest_power(1, string->len + len + 1);
      string->str = g_realloc(string->str, string->allocated_len);
  }
}
GString* g_string_sized_new(gsize dfl_size) {
  GString *string = g_slice_new(GString);
  string->allocated_len = 0;
  string->len = 0;
  string->str = NULL;
  g_string_maybe_expand(string, MAX (dfl_size, 2));
  string->str[0] = 0;
  return string;
}
GString* g_string_new(const gchar *init) {
  GString *string;
  if (init == NULL || *init == '\0') string = g_string_sized_new (2);
  else  {
      gint len;
      len = strlen(init);
      string = g_string_sized_new(len + 2);
      g_string_append_len(string, init, len);
  }
  return string;
}
GString* g_string_new_len(const gchar *init, gssize len) {
  GString *string;
  if (len < 0) return g_string_new(init);
  else {
      string = g_string_sized_new(len);
      if (init) g_string_append_len(string, init, len);
      return string;
    }
}
gchar* g_string_free(GString *string, gboolean free_segment) {
  gchar *segment;
  g_return_val_if_fail(string != NULL, NULL);
  if (free_segment) {
      g_free(string->str);
      segment = NULL;
  } else segment = string->str;
  g_slice_free(GString, string);
  return segment;
}
gboolean g_string_equal(const GString *v, const GString *v2) {
  gchar *p, *q;
  GString *string1 = (GString *) v;
  GString *string2 = (GString *) v2;
  gsize i = string1->len;
  if (i != string2->len) return FALSE;
  p = string1->str;
  q = string2->str;
  while(i) {
      if (*p != *q) return FALSE;
      p++;
      q++;
      i--;
  }
  return TRUE;
}
guint g_string_hash(const GString *str) {
  const gchar *p = str->str;
  gsize n = str->len;    
  guint h = 0;
  while (n--) {
      h = (h << 5) - h + *p;
      p++;
  }
  return h;
}
GString* g_string_assign(GString *string, const gchar *rval) {
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(rval != NULL, string);
  if (string->str != rval) {
      g_string_truncate(string, 0);
      g_string_append(string, rval);
  }
  return string;
}
GString* g_string_truncate(GString *string, gsize len) {
  g_return_val_if_fail(string != NULL, NULL);
  string->len = MIN(len, string->len);
  string->str[string->len] = 0;
  return string;
}
GString* g_string_set_size(GString *string, gsize len) {
  g_return_val_if_fail(string != NULL, NULL);
  if (len >= string->allocated_len) g_string_maybe_expand(string, len - string->len);
  string->len = len;
  string->str[len] = 0;
  return string;
}
GString* g_string_insert_len(GString *string, gssize pos, const gchar *val, gssize len) {
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(len == 0 || val != NULL, string);
  if (len == 0) return string;
  if (len < 0) len = strlen(val);
  if (pos < 0) pos = string->len;
  else g_return_val_if_fail(pos <= string->len, string);
  if (val >= string->str && val <= string->str + string->len) {
      gsize offset = val - string->str;
      gsize precount = 0;
      g_string_maybe_expand(string, len);
      val = string->str + offset;
      if (pos < string->len) g_memmove(string->str + pos + len, string->str + pos, string->len - pos);
      if (offset < pos) {
          precount = MIN(len, pos - offset);
          memcpy(string->str + pos, val, precount);
      }
      if (len > precount) memcpy(string->str + pos + precount,val + precount + len,len - precount);
  } else {
      g_string_maybe_expand(string, len);
      if (pos < string->len) g_memmove(string->str + pos + len, string->str + pos, string->len - pos);
      if (len == 1) string->str[pos] = *val;
      else memcpy(string->str + pos, val, len);
  }
  string->len += len;
  string->str[string->len] = 0;
  return string;
}
#define SUB_DELIM_CHARS  "!$&'()*+,;="
static gboolean is_valid(char c, const char *reserved_chars_allowed) {
  if (g_ascii_isalnum (c) || c == '-' || c == '.' || c == '_' || c == '~') return TRUE;
  if (reserved_chars_allowed && strchr (reserved_chars_allowed, c) != NULL) return TRUE;
  return FALSE;
}
static gboolean gunichar_ok(gunichar c) {
  return (c != (gunichar) -2) && (c != (gunichar) -1);
}
GString* g_string_append_uri_escaped(GString *string, const char *unescaped, const char *reserved_chars_allowed, gboolean allow_utf8) {
  unsigned char c;
  const char *end;
  static const gchar hex[16] = "0123456789ABCDEF";
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(unescaped != NULL, NULL);
  end = unescaped + strlen(unescaped);
  while((c = *unescaped) != 0) {
      if (c >= 0x80 && allow_utf8 && gunichar_ok(g_utf8_get_char_validated(unescaped, end - unescaped))) {
          int len = g_utf8_skip [c];
          g_string_append_len(string, unescaped, len);
          unescaped += len;
	  } else if (is_valid(c, reserved_chars_allowed)) {
          g_string_append_c(string, c);
          unescaped++;
	  } else {
          g_string_append_c(string, '%');
          g_string_append_c(string, hex[((guchar)c) >> 4]);
          g_string_append_c(string, hex[((guchar)c) & 0xf]);
          unescaped++;
	  }
  }
  return string;
}
GString* g_string_append(GString *string, const gchar *val) {
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (val != NULL, string);

  return g_string_insert_len (string, -1, val, -1);
}
GString* g_string_append_len(GString *string, const gchar *val, gssize len) {
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(len == 0 || val != NULL, string);
  return g_string_insert_len(string, -1, val, len);
}
#undef g_string_append_c
GString* g_string_append_c(GString *string, gchar c) {
  g_return_val_if_fail(string != NULL, NULL);
  return g_string_insert_c(string, -1, c);
}
GString* g_string_append_unichar(GString *string, gunichar wc) {
  g_return_val_if_fail(string != NULL, NULL);
  return g_string_insert_unichar(string, -1, wc);
}
GString* g_string_prepend(GString *string, const gchar *val) {
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(val != NULL, string);
  return g_string_insert_len(string, 0, val, -1);
}
GString* g_string_prepend_len(GString *string, const gchar *val, gssize len) {
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(val != NULL, string);
  return g_string_insert_len(string, 0, val, len);
}
GString* g_string_prepend_c(GString *string, gchar c) {
  g_return_val_if_fail(string != NULL, NULL);
  return g_string_insert_c(string, 0, c);
}
GString* g_string_prepend_unichar(GString *string, gunichar wc) {
  g_return_val_if_fail(string != NULL, NULL);
  return g_string_insert_unichar(string, 0, wc);
}
GString* g_string_insert(GString *string, gssize pos, const gchar *val) {
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(val != NULL, string);
  if (pos >= 0) g_return_val_if_fail(pos <= string->len, string);
  return g_string_insert_len(string, pos, val, -1);
}
GString* g_string_insert_c(GString *string, gssize pos, gchar c) {
  g_return_val_if_fail(string != NULL, NULL);
  g_string_maybe_expand(string, 1);
  if (pos < 0) pos = string->len;
  else g_return_val_if_fail(pos <= string->len, string);
  if (pos < string->len) g_memmove(string->str + pos + 1, string->str + pos, string->len - pos);
  string->str[pos] = c;
  string->len += 1;
  string->str[string->len] = 0;
  return string;
}
GString* g_string_insert_unichar(GString *string, gssize pos, gunichar wc) {
  gint charlen, first, i;
  gchar *dest;
  g_return_val_if_fail (string != NULL, NULL);
  if (wc < 0x80) {
      first = 0;
      charlen = 1;
  } else if (wc < 0x800) {
      first = 0xc0;
      charlen = 2;
  } else if (wc < 0x10000) {
      first = 0xe0;
      charlen = 3;
  } else if (wc < 0x200000) {
      first = 0xf0;
      charlen = 4;
  } else if (wc < 0x4000000) {
      first = 0xf8;
      charlen = 5;
  } else {
      first = 0xfc;
      charlen = 6;
  }
  g_string_maybe_expand(string, charlen);
  if (pos < 0) pos = string->len;
  else g_return_val_if_fail(pos <= string->len, string);
  if (pos < string->len) g_memmove (string->str + pos + charlen, string->str + pos, string->len - pos);
  dest = string->str + pos;
  for (i = charlen - 1; i > 0; --i) {
      dest[i] = (wc & 0x3f) | 0x80;
      wc >>= 6;
  }
  dest[0] = wc | first;
  string->len += charlen;
  string->str[string->len] = 0;
  return string;
}
GString* g_string_overwrite(GString *string, gsize pos, const gchar *val) {
  g_return_val_if_fail(val != NULL, string);
  return g_string_overwrite_len(string, pos, val, strlen (val));
}
GString* g_string_overwrite_len(GString *string, gsize pos, const gchar *val, gssize len) {
  gsize end;
  g_return_val_if_fail(string != NULL, NULL);
  if (!len) return string;
  g_return_val_if_fail(val != NULL, string);
  g_return_val_if_fail(pos <= string->len, string);
  if (len < 0) len = strlen(val);
  end = pos + len;
  if (end > string->len) g_string_maybe_expand(string, end - string->len);
  memcpy(string->str + pos, val, len);
  if (end > string->len) {
      string->str[end] = '\0';
      string->len = end;
  }
  return string;
}
GString* g_string_erase(GString *string, gssize pos, gssize len) {
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(pos >= 0, string);
  g_return_val_if_fail(pos <= string->len, string);
  if (len < 0) len = string->len - pos;
  else {
      g_return_val_if_fail(pos + len <= string->len, string);
      if (pos + len < string->len) g_memmove(string->str + pos, string->str + pos + len, string->len - (pos + len));
  }
  string->len -= len;
  string->str[string->len] = 0;
  return string;
}
GString* g_string_ascii_down(GString *string) {
  gchar *s;
  gint n;
  g_return_val_if_fail(string != NULL, NULL);
  n = string->len;
  s = string->str;
  while(n) {
      *s = g_ascii_tolower(*s);
      s++;
      n--;
  }
  return string;
}
GString* g_string_ascii_up(GString *string) {
  gchar *s;
  gint n;
  g_return_val_if_fail(string != NULL, NULL);
  n = string->len;
  s = string->str;
  while (n) {
      *s = g_ascii_toupper(*s);
      s++;
      n--;
  }
  return string;
}
GString* g_string_down(GString *string) {
  guchar *s;
  glong n;
  g_return_val_if_fail(string != NULL, NULL);
  n = string->len;    
  s = (guchar*)string->str;
  while(n) {
      if (isupper(*s)) *s = tolower(*s);
      s++;
      n--;
  }
  return string;
}
GString* g_string_up(GString *string) {
  guchar *s;
  glong n;
  g_return_val_if_fail(string != NULL, NULL);
  n = string->len;
  s = (guchar*)string->str;
  while(n) {
      if (islower(*s)) *s = toupper(*s);
      s++;
      n--;
  }
  return string;
}
void g_string_append_vprintf(GString *string, const gchar *format, va_list args) {
  gchar *buf;
  gint len;
  g_return_if_fail(string != NULL);
  g_return_if_fail(format != NULL);
  len = g_vasprintf(&buf, format, args);
  if (len >= 0) {
      g_string_maybe_expand (string, len);
      memcpy (string->str + string->len, buf, len + 1);
      string->len += len;
      g_free (buf);
    }
}
void g_string_vprintf(GString *string, const gchar *format, va_list args) {
  g_string_truncate(string, 0);
  g_string_append_vprintf(string, format, args);
}
void g_string_printf(GString *string, const gchar *format, ...) {
  va_list args;
  g_string_truncate(string, 0);
  va_start(args, format);
  g_string_append_vprintf(string, format, args);
  va_end(args);
}
void g_string_append_printf(GString *string, const gchar *format, ...) {
  va_list args;
  va_start(args, format);
  g_string_append_vprintf(string, format, args);
  va_end(args);
}