#ifndef _LIBCHARSET_H
#define _LIBCHARSET_H

#include "localcharset.h"

#ifdef __cplusplus
extern "C" {
#endif
extern void libcharset_set_relocation_prefix(const char *orig_prefix, const char *curr_prefix);
#ifdef __cplusplus
}
#endif
#endif