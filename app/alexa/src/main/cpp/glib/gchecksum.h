#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_CHECKSUM_H__
#define __G_CHECKSUM_H__

#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
typedef enum {
  G_CHECKSUM_MD5,
  G_CHECKSUM_SHA1,
  G_CHECKSUM_SHA256
} GChecksumType;
typedef struct _GChecksum       GChecksum;
gssize g_checksum_type_get_length(GChecksumType checksum_type);
GChecksum* g_checksum_new(GChecksumType checksum_type);
void g_checksum_reset(GChecksum *checksum);
GChecksum* g_checksum_copy(const GChecksum *checksum);
void g_checksum_free(GChecksum *checksum);
void g_checksum_update(GChecksum *checksum, const guchar *data, gssize length);
G_CONST_RETURN gchar *g_checksum_get_string(GChecksum *checksum);
void g_checksum_get_digest(GChecksum *checksum, guint8 *buffer, gsize *digest_len);
gchar *g_compute_checksum_for_data(GChecksumType checksum_type, const guchar *data, gsize length);
gchar *g_compute_checksum_for_string(GChecksumType checksum_type, const gchar *str, gssize length);
G_END_DECLS
#endif