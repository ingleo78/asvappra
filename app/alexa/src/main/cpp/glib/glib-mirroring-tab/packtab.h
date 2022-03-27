#ifndef PACKTAB_H
#define PACKTAB_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#define packtab_version 3
  int pack_table(const signed int *base, long key_num, int key_size, signed int default_key, int max_depth, int tab_width, const char *const *name,
                 const char *key_type_name, const char *table_name, const char *macro_name, FILE *out);
#ifdef	__cplusplus
}
#endif
#endif