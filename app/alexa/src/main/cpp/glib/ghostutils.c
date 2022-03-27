#include <string.h>
#include "ghostutils.h"
#include "garray.h"
#include "gmem.h"
#include "gstring.h"
#include "gstrfuncs.h"
#include "glibintl.h"

#define IDNA_ACE_PREFIX     "xn--"
#define IDNA_ACE_PREFIX_LEN 4
#define PUNYCODE_BASE 36
#define PUNYCODE_TMIN 1
#define PUNYCODE_TMAX 26
#define PUNYCODE_SKEW 38
#define PUNYCODE_DAMP 700
#define PUNYCODE_INITIAL_BIAS 72
#define PUNYCODE_INITIAL_N 0x80
#define PUNYCODE_IS_BASIC(cp) ((guint)(cp) < 0x80)
static inline gchar
encode_digit (guint dig) {
  if (dig < 26) return dig + 'a';
  else return dig - 26 + '0';
}
static inline guint decode_digit(gchar dig) {
  if (dig >= 'A' && dig <= 'Z') return dig - 'A';
  else if (dig >= 'a' && dig <= 'z') return dig - 'a';
  else if (dig >= '0' && dig <= '9') return dig - '0' + 26;
  else return G_MAXUINT;
}
static guint adapt(guint delta, guint numpoints, gboolean firsttime) {
  guint k;
  delta = firsttime ? delta / PUNYCODE_DAMP : delta / 2;
  delta += delta / numpoints;
  k = 0;
  while (delta > ((PUNYCODE_BASE - PUNYCODE_TMIN) * PUNYCODE_TMAX) / 2) {
      delta /= PUNYCODE_BASE - PUNYCODE_TMIN;
      k += PUNYCODE_BASE;
  }
  return k + ((PUNYCODE_BASE - PUNYCODE_TMIN + 1) * delta / (delta + PUNYCODE_SKEW));
}
static gboolean punycode_encode(const gchar *input_utf8, gsize input_utf8_length, GString *output) {
  guint delta, handled_chars, num_basic_chars, bias, j, q, k, t, digit;
  gunichar n, m, *input;
  glong input_length;
  gboolean success = FALSE;
  input = g_utf8_to_ucs4 (input_utf8, input_utf8_length, NULL, &input_length, NULL);
  if (!input) return FALSE;
  for (j = num_basic_chars = 0; j < input_length; j++) {
      if (PUNYCODE_IS_BASIC(input[j])) {
          g_string_append_c(output, g_ascii_tolower(input[j]));
          num_basic_chars++;
      }
  }
  if (num_basic_chars) g_string_append_c(output, '-');
  handled_chars = num_basic_chars;
  delta = 0;
  bias = PUNYCODE_INITIAL_BIAS;
  n = PUNYCODE_INITIAL_N;
  while (handled_chars < input_length) {
      for (m = G_MAXUINT, j = 0; j < input_length; j++) {
	      if (input[j] >= n && input[j] < m) m = input[j];
	  }
      if (m - n > (G_MAXUINT - delta) / (handled_chars + 1)) goto fail;
      delta += (m - n) * (handled_chars + 1);
      n = m;
      for (j = 0; j < input_length; j++) {
          if (input[j] < n) {
              if (++delta == 0) goto fail;
          } else if (input[j] == n) {
              q = delta;
              for (k = PUNYCODE_BASE; ; k += PUNYCODE_BASE) {
                  if (k <= bias) t = PUNYCODE_TMIN;
                  else if (k >= bias + PUNYCODE_TMAX) t = PUNYCODE_TMAX;
                  else t = k - bias;
                  if (q < t) break;
                  digit = t + (q - t) % (PUNYCODE_BASE - t);
                  g_string_append_c(output, encode_digit(digit));
                  q = (q - t) / (PUNYCODE_BASE - t);
              }
              g_string_append_c (output, encode_digit(q));
              bias = adapt(delta, handled_chars + 1, handled_chars == num_basic_chars);
              delta = 0;
              handled_chars++;
          }
      }
      delta++;
      n++;
  }
  success = TRUE;
  fail:
  g_free(input);
  return success;
}
#define idna_is_junk(ch) ((ch) == 0x00AD || (ch) == 0x1806 || (ch) == 0x200B || (ch) == 0x2060 || (ch) == 0xFEFF || (ch) == 0x034F || (ch) == 0x180B || \
                          (ch) == 0x180C || (ch) == 0x180D || (ch) == 0x200C || (ch) == 0x200D || ((ch) >= 0xFE00 && (ch) <= 0xFE0F))
static gchar * remove_junk(const gchar *str, gint len) {
  GString *cleaned = NULL;
  const gchar *p;
  gunichar ch;
  for (p = str; len == -1 ? *p : p < str + len; p = g_utf8_next_char(p)) {
      ch = g_utf8_get_char(p);
      if (idna_is_junk(ch)) {
          if (!cleaned) {
              cleaned = g_string_new(NULL);
              g_string_append_len(cleaned, str, p - str);
          }
      } else if (cleaned)
	g_string_append_unichar(cleaned, ch);
  }
  if (cleaned) return g_string_free(cleaned, FALSE);
  else return NULL;
}
static inline gboolean contains_uppercase_letters(const gchar *str, gint len) {
  const gchar *p;
  for (p = str; len == -1 ? *p : p < str + len; p = g_utf8_next_char (p)) {
      if (g_unichar_isupper(g_utf8_get_char (p))) return TRUE;
  }
  return FALSE;
}
static inline gboolean contains_non_ascii(const gchar *str, gint len) {
  const gchar *p;
  for (p = str; len == -1 ? *p : p < str + len; p++) {
      if ((guchar)*p > 0x80) return TRUE;
  }
  return FALSE;
}
static inline gboolean idna_is_prohibited(gunichar ch) {
  switch(g_unichar_type (ch)) {
      case G_UNICODE_CONTROL: case G_UNICODE_FORMAT: case G_UNICODE_UNASSIGNED: case G_UNICODE_PRIVATE_USE: case G_UNICODE_SURROGATE:
      case G_UNICODE_LINE_SEPARATOR: case G_UNICODE_PARAGRAPH_SEPARATOR: case G_UNICODE_SPACE_SEPARATOR:
          return TRUE;
      case G_UNICODE_OTHER_SYMBOL:
          if (ch == 0xFFFC || ch == 0xFFFD || (ch >= 0x2FF0 && ch <= 0x2FFB)) return TRUE;
          return FALSE;
      case G_UNICODE_NON_SPACING_MARK:
          if (ch == 0x0340 || ch == 0x0341) return TRUE;
          return FALSE;
      default: return FALSE;
  }
}
static gchar* nameprep(const gchar *hostname, gint len, gboolean *is_unicode) {
  gchar *name, *tmp = NULL, *p;
  name = remove_junk(hostname, len);
  if (name) {
      tmp = name;
      len = -1;
  } else name = (gchar*)hostname;
  if (contains_uppercase_letters(name, len)) {
      name = g_utf8_strdown(name, len);
      g_free(tmp);
      tmp = name;
      len = -1;
  }
  if (!contains_non_ascii(name, len)) {
      *is_unicode = FALSE;
      if (name == (gchar*)hostname) return len == -1 ? g_strdup(hostname) : g_strndup (hostname, len);
      else return name;
  }
  *is_unicode = TRUE;
  name = g_utf8_normalize(name, len, G_NORMALIZE_NFKC);
  g_free(tmp);
  tmp = name;
  if (!name) return NULL;
  if (contains_uppercase_letters(name, -1)) {
      name = g_utf8_strdown(name, -1);
      g_free(tmp);
      tmp = name;
  }
  for (p = name; *p; p = g_utf8_next_char(p)) {
      if (idna_is_prohibited(g_utf8_get_char(p))) {
	      name = NULL;
          g_free (tmp);
	      goto done;
	  }
  }
  done:
  return name;
}
#define idna_is_dot(str) (((guchar)(str)[0] == '.') ||  ((guchar)(str)[0] == 0xE3 && (guchar)(str)[1] == 0x80 && (guchar)(str)[2] == 0x82) || \
                          ((guchar)(str)[0] == 0xEF && (guchar)(str)[1] == 0xBC && (guchar)(str)[2] == 0x8E) || ((guchar)(str)[0] == 0xEF &&  \
                          (guchar)(str)[1] == 0xBD && (guchar)(str)[2] == 0xA1))
static const gchar* idna_end_of_label (const gchar *str) {
  for (; *str; str = g_utf8_next_char(str)) {
      if (idna_is_dot (str)) return str;
  }
  return str;
}
gchar* g_hostname_to_ascii(const gchar *hostname) {
  gchar *name, *label, *p;
  GString *out;
  gssize llen, oldlen;
  gboolean unicode;
  label = name = nameprep (hostname, -1, &unicode);
  if (!name || !unicode) return name;
  out = g_string_new(NULL);
  do {
      unicode = FALSE;
      for (p = label; *p && !idna_is_dot(p); p++) {
	      if ((guchar)*p > 0x80) unicode = TRUE;
	  }
      oldlen = out->len;
      llen = p - label;
      if (unicode) {
          if (!strncmp (label, IDNA_ACE_PREFIX, IDNA_ACE_PREFIX_LEN)) goto fail;
          g_string_append(out, IDNA_ACE_PREFIX);
          if (!punycode_encode(label, llen, out)) goto fail;
	  } else g_string_append_len(out, label, llen);
      if (out->len - oldlen > 63)
	  goto fail;
      label += llen;
      if (*label) label = g_utf8_next_char(label);
      if (*label) g_string_append_c(out, '.');
  } while(*label);
  g_free(name);
  return g_string_free(out, FALSE);
  fail:
  g_free(name);
  g_string_free(out, TRUE);
  return NULL;
}
gboolean g_hostname_is_non_ascii(const gchar *hostname) {
  return contains_non_ascii(hostname, -1);
}
static gboolean punycode_decode(const gchar *input, gsize input_length, GString *output) {
  GArray *output_chars;
  gunichar n;
  guint i, bias;
  guint oldi, w, k, digit, t;
  const gchar *split;
  n = PUNYCODE_INITIAL_N;
  i = 0;
  bias = PUNYCODE_INITIAL_BIAS;
  split = input + input_length - 1;
  while (split > input && *split != '-') split--;
  if (split > input) {
      output_chars = g_array_sized_new(FALSE, FALSE, sizeof(gunichar),split - input);
      input_length -= (split - input) + 1;
      while(input < split) {
          gunichar ch = (gunichar)*input++;
          if (!PUNYCODE_IS_BASIC(ch)) goto fail;
          g_array_append_val(output_chars, ch);
	  }
      input++;
  } else output_chars = g_array_new(FALSE, FALSE, sizeof(gunichar));
  while(input_length) {
      oldi = i;
      w = 1;
      for (k = PUNYCODE_BASE; ; k += PUNYCODE_BASE) {
          if (!input_length--) goto fail;
          digit = decode_digit(*input++);
          if (digit >= PUNYCODE_BASE) goto fail;
          if (digit > (G_MAXUINT - i) / w) goto fail;
          i += digit * w;
          if (k <= bias) t = PUNYCODE_TMIN;
          else if (k >= bias + PUNYCODE_TMAX) t = PUNYCODE_TMAX;
          else t = k - bias;
          if (digit < t) break;
          if (w > G_MAXUINT / (PUNYCODE_BASE - t)) goto fail;
          w *= (PUNYCODE_BASE - t);
	  }
      bias = adapt(i - oldi, output_chars->len + 1, oldi == 0);
      if (i / (output_chars->len + 1) > G_MAXUINT - n)
	  goto fail;
      n += i / (output_chars->len + 1);
      i %= (output_chars->len + 1);
      g_array_insert_val (output_chars, i++, n);
  }
  for (i = 0; i < output_chars->len; i++) g_string_append_unichar(output, g_array_index(output_chars, gunichar, i));
  g_array_free(output_chars, TRUE);
  return TRUE;
  fail:
  g_array_free(output_chars, TRUE);
  return FALSE;
}
gchar* g_hostname_to_unicode(const gchar *hostname) {
  GString *out;
  gssize llen;
  out = g_string_new(NULL);
  do {
      llen = idna_end_of_label(hostname) - hostname;
      if (!g_ascii_strncasecmp(hostname, IDNA_ACE_PREFIX, IDNA_ACE_PREFIX_LEN)) {
          hostname += IDNA_ACE_PREFIX_LEN;
          llen -= IDNA_ACE_PREFIX_LEN;
          if (!punycode_decode(hostname, llen, out)) {
              g_string_free(out, TRUE);
              return NULL;
          }
	  } else {
          gboolean unicode;
          gchar *canonicalized = nameprep(hostname, llen, &unicode);
          if (!canonicalized) {
              g_string_free(out, TRUE);
              return NULL;
          }
          g_string_append(out, canonicalized);
          g_free(canonicalized);
      }
      hostname += llen;
      if (*hostname) hostname = g_utf8_next_char(hostname);
      if (*hostname) g_string_append_c(out, '.');
  } while(*hostname);
  return g_string_free(out, FALSE);
}
gboolean g_hostname_is_ascii_encoded(const gchar *hostname) {
  while(1) {
      if (!g_ascii_strncasecmp(hostname, IDNA_ACE_PREFIX, IDNA_ACE_PREFIX_LEN)) return TRUE;
      hostname = idna_end_of_label(hostname);
      if (*hostname) hostname = g_utf8_next_char(hostname);
      if (!*hostname) return FALSE;
  }
}
gboolean g_hostname_is_ip_address(const gchar *hostname) {
  gchar *p, *end;
  gint nsegments, octet;
  p = (char*)hostname;
  if (strchr (p, ':')) {
      gboolean skipped;
      nsegments = 0;
      skipped = FALSE;
      while(*p && nsegments < 8) {
          if (p != (char *)hostname || (p[0] == ':' && p[1] == ':')) {
              if (*p != ':') return FALSE;
              p++;
          }
          if (*p == ':' && !skipped) {
              skipped = TRUE;
              nsegments++;
              if (!p[1]) p++;
              continue;
          }
          for (end = p; g_ascii_isxdigit (*end); end++);
          if (end == p || end > p + 4) return FALSE;
          if (*end == '.') {
              if ((nsegments == 6 && !skipped) || (nsegments <= 6 && skipped)) goto parse_ipv4;
              else return FALSE;
          }
          nsegments++;
          p = end;
        }
      return !*p && (nsegments == 8 || skipped);
    }
  parse_ipv4:
  for (nsegments = 0; nsegments < 4; nsegments++) {
      if (nsegments != 0) {
          if (*p != '.') return FALSE;
          p++;
      }
      octet = 0;
      if (*p == '0') end = p + 1;
      else {
          for (end = p; g_ascii_isdigit(*end); end++) octet = 10 * octet + (*end - '0');
      }
      if (end == p || end > p + 3 || octet > 255) return FALSE;
      p = end;
  }
  return !*p;
}