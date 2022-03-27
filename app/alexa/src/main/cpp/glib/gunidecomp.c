#include <stdlib.h>
#include "gunicode.h"
#include "gunidecomp.h"
#include "gmem.h"
#include "gunicomp.h"
#include "gunicodeprivate.h"

#define CC_PART1(Page, Char)  ((combining_class_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (combining_class_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) : (cclass_data[combining_class_table_part1[Page]][Char]))
#define CC_PART2(Page, Char)  ((combining_class_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (combining_class_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) : (cclass_data[combining_class_table_part2[Page]][Char]))
#define COMBINING_CLASS(Char)  (((Char) <= G_UNICODE_LAST_CHAR_PART1) ? CC_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR) ? CC_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) : 0))
gint g_unichar_combining_class(gunichar uc) {
  return COMBINING_CLASS(uc);
}
#define SBase 0xAC00
#define LBase 0x1100
#define VBase 0x1161
#define TBase 0x11A7
#define LCount 19 
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)
#define SCount (LCount * NCount)
void g_unicode_canonical_ordering(gunichar *string, gsize len) {
  gsize i;
  int swap = 1;
  while(swap) {
      int last;
      swap = 0;
      last = COMBINING_CLASS(string[0]);
      for (i = 0; i < len - 1; ++i) {
          int next = COMBINING_CLASS(string[i + 1]);
          if (next != 0 && last > next) {
              gsize j;
              for (j = i + 1; j > 0; --j) {
                  gunichar t;
                  if (COMBINING_CLASS(string[j - 1]) <= next) break;
                  t = string[j];
                  string[j] = string[j - 1];
                  string[j - 1] = t;
                  swap = 1;
              }
              next = last;
          }
          last = next;
	  }
  }
}
static void decompose_hangul(gunichar s, gunichar *r, gsize *result_len) {
  gint SIndex = s - SBase;
  if (SIndex < 0 || SIndex >= SCount) {
      if (r) r[0] = s;
      *result_len = 1;
  } else {
      gunichar L = LBase + SIndex / NCount;
      gunichar V = VBase + (SIndex % NCount) / TCount;
      gunichar T = TBase + SIndex % TCount;
      if (r) {
          r[0] = L;
          r[1] = V;
      }
      if (T != TBase) {
          if (r) r[2] = T;
          *result_len = 3;
      } else *result_len = 2;
  }
}
static const gchar* find_decomposition(gunichar ch, gboolean compat) {
  int start = 0;
  int end = G_N_ELEMENTS(decomp_table);
  if (ch >= decomp_table[start].ch && ch <= decomp_table[end - 1].ch) {
      while(TRUE) {
          int half = (start + end) / 2;
          if (ch == decomp_table[half].ch) {
              int offset;
              if (compat) {
                  offset = decomp_table[half].compat_offset;
                  if (offset == G_UNICODE_NOT_PRESENT_OFFSET) offset = decomp_table[half].canon_offset;
              } else {
                  offset = decomp_table[half].canon_offset;
                  if (offset == G_UNICODE_NOT_PRESENT_OFFSET) return NULL;
              }
              return &(decomp_expansion_string[offset]);
          } else if (half == start) break;
          else if (ch > decomp_table[half].ch) start = half;
          else end = half;
	  }
  }
  return NULL;
}
gunichar* g_unicode_canonical_decomposition(gunichar ch, gsize *result_len) {
  const gchar *decomp;
  const gchar *p;
  gunichar *r;
  if (ch >= 0xac00 && ch <= 0xd7a3) {
      decompose_hangul(ch, NULL, result_len);
      r = g_malloc(*result_len * sizeof(gunichar));
      decompose_hangul(ch, r, result_len);
  } else if ((decomp = find_decomposition(ch, FALSE)) != NULL) {
      int i;
      *result_len = g_utf8_strlen(decomp, -1);
      r = g_malloc(*result_len * sizeof(gunichar));
      for (p = decomp, i = 0; *p != '\0'; p = g_utf8_next_char (p), i++) r[i] = g_utf8_get_char (p);
  } else {
      r = g_malloc(sizeof(gunichar));
      *r = ch;
      *result_len = 1;
  }
  return r;
}
static gboolean combine_hangul(gunichar a, gunichar b, gunichar *result) {
  gint LIndex = a - LBase;
  gint SIndex = a - SBase;
  gint VIndex = b - VBase;
  gint TIndex = b - TBase;
  if (0 <= LIndex && LIndex < LCount && 0 <= VIndex && VIndex < VCount) {
      *result = SBase + (LIndex * VCount + VIndex) * TCount;
      return TRUE;
  } else if (0 <= SIndex && SIndex < SCount && (SIndex % TCount) == 0 && 0 < TIndex && TIndex < TCount) {
      *result = a + TIndex;
      return TRUE;
  }
  return FALSE;
}
#define CI(Page, Char) ((compose_table[Page] >= G_UNICODE_MAX_TABLE_INDEX) ? (compose_table[Page] - G_UNICODE_MAX_TABLE_INDEX) \
                        : (compose_data[compose_table[Page]][Char]))
#define COMPOSE_INDEX(Char)  (((Char >> 8) > (COMPOSE_TABLE_LAST)) ? 0 : CI((Char) >> 8, (Char) & 0xff))
static gboolean combine (gunichar a, gunichar b, gunichar *result) {
  gushort index_a, index_b;
  if (combine_hangul(a, b, result)) return TRUE;
  index_a = COMPOSE_INDEX(a);
  if (index_a >= COMPOSE_FIRST_SINGLE_START && index_a < COMPOSE_SECOND_START) {
      if (b == compose_first_single[index_a - COMPOSE_FIRST_SINGLE_START][0]) {
          *result = compose_first_single[index_a - COMPOSE_FIRST_SINGLE_START][1];
          return TRUE;
	  } else return FALSE;
  }
  index_b = COMPOSE_INDEX(b);
  if (index_b >= COMPOSE_SECOND_SINGLE_START) {
      if (a == compose_second_single[index_b - COMPOSE_SECOND_SINGLE_START][0]) {
          *result = compose_second_single[index_b - COMPOSE_SECOND_SINGLE_START][1];
          return TRUE;
	  } else return FALSE;
  }
  if (index_a >= COMPOSE_FIRST_START && index_a < COMPOSE_FIRST_SINGLE_START && index_b >= COMPOSE_SECOND_START && index_b < COMPOSE_SECOND_SINGLE_START) {
      gunichar res = compose_array[index_a - COMPOSE_FIRST_START][index_b - COMPOSE_SECOND_START];
      if (res) {
          *result = res;
          return TRUE;
	  }
  }
  return FALSE;
}
gunichar * _g_utf8_normalize_wc(const gchar *str, gssize max_len, GNormalizeMode mode) {
  gsize n_wc;
  gunichar *wc_buffer;
  const char *p;
  gsize last_start;
  gboolean do_compat = (mode == G_NORMALIZE_NFKC || mode == G_NORMALIZE_NFKD);
  gboolean do_compose = (mode == G_NORMALIZE_NFC || mode == G_NORMALIZE_NFKC);
  n_wc = 0;
  p = str;
  while((max_len < 0 || p < str + max_len) && *p) {
      const gchar *decomp;
      gunichar wc = g_utf8_get_char(p);
      if (wc >= 0xac00 && wc <= 0xd7a3) {
          gsize result_len;
          decompose_hangul(wc, NULL, &result_len);
          n_wc += result_len;
      } else {
          decomp = find_decomposition(wc, do_compat);
          if (decomp) n_wc += g_utf8_strlen(decomp, -1);
          else n_wc++;
      }
      p = g_utf8_next_char(p);
  }
  wc_buffer = g_new(gunichar, n_wc + 1);
  last_start = 0;
  n_wc = 0;
  p = str;
  while ((max_len < 0 || p < str + max_len) && *p) {
      gunichar wc = g_utf8_get_char(p);
      const gchar *decomp;
      int cc;
      gsize old_n_wc = n_wc;
      if (wc >= 0xac00 && wc <= 0xd7a3) {
          gsize result_len;
          decompose_hangul(wc, wc_buffer + n_wc, &result_len);
          n_wc += result_len;
      } else {
          decomp = find_decomposition(wc, do_compat);
          if (decomp) {
              const char *pd;
              for (pd = decomp; *pd != '\0'; pd = g_utf8_next_char(pd)) wc_buffer[n_wc++] = g_utf8_get_char(pd);
          } else wc_buffer[n_wc++] = wc;
      }
      if (n_wc > 0) {
          cc = COMBINING_CLASS (wc_buffer[old_n_wc]);
          if (cc == 0) {
              g_unicode_canonical_ordering(wc_buffer + last_start, n_wc - last_start);
              last_start = old_n_wc;
          }
	  }
      p = g_utf8_next_char (p);
  }
  if (n_wc > 0) {
      g_unicode_canonical_ordering (wc_buffer + last_start, n_wc - last_start);
      last_start = n_wc;
  }
  wc_buffer[n_wc] = 0;
  if (do_compose && n_wc > 0) {
      gsize i, j;
      int last_cc = 0;
      last_start = 0;
      for (i = 0; i < n_wc; i++) {
          int cc = COMBINING_CLASS(wc_buffer[i]);
          if (i > 0 && (last_cc == 0 || last_cc < cc) && combine (wc_buffer[last_start], wc_buffer[i], &wc_buffer[last_start])) {
              for (j = i + 1; j < n_wc; j++) wc_buffer[j-1] = wc_buffer[j];
              n_wc--;
              i--;
              if (i == last_start) last_cc = 0;
              else last_cc = COMBINING_CLASS (wc_buffer[i-1]);
              continue;
          }
          if (cc == 0) last_start = i;
          last_cc = cc;
	  }
  }
  wc_buffer[n_wc] = 0;
  return wc_buffer;
}
gchar* g_utf8_normalize(const gchar *str, gssize len, GNormalizeMode mode) {
  gunichar *result_wc = _g_utf8_normalize_wc(str, len, mode);
  gchar *result;
  result = g_ucs4_to_utf8(result_wc, -1, NULL, NULL, NULL);
  g_free(result_wc);
  return result;
}