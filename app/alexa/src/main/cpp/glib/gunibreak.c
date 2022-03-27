#include <stdlib.h>
#include "gunibreak.h"

#define TPROP_PART1(Page, Char) ((break_property_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (break_property_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (break_property_data[break_property_table_part1[Page]][Char]))
#define TPROP_PART2(Page, Char) ((break_property_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (break_property_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (break_property_data[break_property_table_part2[Page]][Char]))
#define PROP(Char) (((Char) <= G_UNICODE_LAST_CHAR_PART1) ? TPROP_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR)  ? TPROP_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
   : G_UNICODE_BREAK_UNKNOWN))
GUnicodeBreakType g_unichar_break_type(gunichar c) {
  return PROP (c);
}