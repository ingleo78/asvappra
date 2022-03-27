#ifndef __G_DEBUG_H__
#define __G_DEBUG_H__

G_BEGIN_DECLS
typedef enum {
  G_DEBUG_FATAL_WARNINGS  = 1 << 0,
  G_DEBUG_FATAL_CRITICALS = 1 << 1
} GDebugFlag;
#ifdef G_ENABLE_DEBUG
#define G_NOTE(type, action)                             \
    G_STMT_START {                                       \
        if (!_g_debug_initialized) _g_debug_init();      \
        if (_g_debug_flags & G_DEBUG_##type) action;     \
    } G_STMT_END
#else
#define G_NOTE(type, action)
#endif
GLIB_VAR gboolean _g_debug_initialized;
GLIB_VAR guint _g_debug_flags;
G_GNUC_INTERNAL void _g_debug_init(void);
G_END_DECLS

#endif