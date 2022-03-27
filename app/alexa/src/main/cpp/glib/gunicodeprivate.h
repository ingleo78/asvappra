#ifndef __G_UNICODE_PRIVATE_H__
#define __G_UNICODE_PRIVATE_H__

#include "gtypes.h"

G_BEGIN_DECLS
G_GNUC_INTERNAL gunichar *_g_utf8_normalize_wc(const gchar *str, gssize max_len, GNormalizeMode mode);
G_END_DECLS

#endif