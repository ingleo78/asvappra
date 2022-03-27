#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_QUARK_H__
#define __G_QUARK_H__

#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
typedef guint32 GQuark;
GQuark g_quark_try_string(const gchar *string);
GQuark g_quark_from_static_string(const gchar *string);
GQuark g_quark_from_string(const gchar *string);
G_CONST_RETURN gchar* g_quark_to_string(GQuark quark) G_GNUC_CONST;
G_CONST_RETURN gchar* g_intern_string(const gchar *string);
G_CONST_RETURN gchar* g_intern_static_string(const gchar *string);
#define G_DEFINE_QUARK(QN, q_n) \
GQuark q_n##_quark(void) { \
    static GQuark q; \
    if (G_UNLIKELY(q == 0)) q = g_quark_from_static_string(#QN); \
    return q;\
}
G_END_DECLS

#endif