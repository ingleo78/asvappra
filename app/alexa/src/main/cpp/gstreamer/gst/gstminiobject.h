#ifndef __GST_MINI_OBJECT_H__
#define __GST_MINI_OBJECT_H__

#include <glib/glib-object.h>
#include "gstconfig.h"

G_BEGIN_DECLS

#define GST_IS_MINI_OBJECT_TYPE(obj,type)  ((obj) && GST_MINI_OBJECT_TYPE(obj) == (type))
#define GST_MINI_OBJECT_CAST(obj)          ((GstMiniObject*)(obj))
#define GST_MINI_OBJECT_CONST_CAST(obj)    ((const GstMiniObject*)(obj))
#define GST_MINI_OBJECT(obj)               (GST_MINI_OBJECT_CAST(obj))
typedef struct _GstMiniObject GstMiniObject;
typedef GstMiniObject * (*GstMiniObjectCopyFunction) (const GstMiniObject *obj);
typedef gboolean (*GstMiniObjectDisposeFunction) (GstMiniObject *obj);
typedef void (*GstMiniObjectFreeFunction) (GstMiniObject *obj);
typedef void (*GstMiniObjectNotify) (gpointer user_data, GstMiniObject * obj);
#define GST_MINI_OBJECT_TYPE(obj)  (GST_MINI_OBJECT_CAST(obj)->type)
#define GST_MINI_OBJECT_FLAGS(obj)  (GST_MINI_OBJECT_CAST(obj)->flags)
#define GST_MINI_OBJECT_FLAG_IS_SET(obj,flag)        !!(GST_MINI_OBJECT_FLAGS (obj) & (flag))
#define GST_MINI_OBJECT_FLAG_SET(obj,flag)           (GST_MINI_OBJECT_FLAGS (obj) |= (flag))
#define GST_MINI_OBJECT_FLAG_UNSET(obj,flag)         (GST_MINI_OBJECT_FLAGS (obj) &= ~(flag))
typedef enum {
  GST_MINI_OBJECT_FLAG_LOCKABLE      = (1 << 0),
  GST_MINI_OBJECT_FLAG_LOCK_READONLY = (1 << 1),
  GST_MINI_OBJECT_FLAG_LAST          = (1 << 4)
} GstMiniObjectFlags;
#define GST_MINI_OBJECT_IS_LOCKABLE(obj)  GST_MINI_OBJECT_FLAG_IS_SET(obj, GST_MINI_OBJECT_FLAG_LOCKABLE)
typedef enum {
  GST_LOCK_FLAG_READ      = (1 << 0),
  GST_LOCK_FLAG_WRITE     = (1 << 1),
  GST_LOCK_FLAG_EXCLUSIVE = (1 << 2),
  GST_LOCK_FLAG_LAST      = (1 << 8)
} GstLockFlags;
#define GST_LOCK_FLAG_READWRITE  (GST_LOCK_FLAG_READ | GST_LOCK_FLAG_WRITE)
#define GST_MINI_OBJECT_REFCOUNT(obj)           ((GST_MINI_OBJECT_CAST(obj))->refcount)
#define GST_MINI_OBJECT_REFCOUNT_VALUE(obj)     (g_atomic_int_get (&(GST_MINI_OBJECT_CAST(obj))->refcount))
struct _GstMiniObject {
  GType   type;
  gint    refcount;
  gint    lockstate;
  guint   flags;
  GstMiniObjectCopyFunction copy;
  GstMiniObjectDisposeFunction dispose;
  GstMiniObjectFreeFunction free;
  guint n_qdata;
  gpointer qdata;
};
void gst_mini_object_init(GstMiniObject *mini_object, guint flags, GType type, GstMiniObjectCopyFunction copy_func,
                                      GstMiniObjectDisposeFunction dispose_func, GstMiniObjectFreeFunction free_func);
GstMiniObject * gst_mini_object_ref		(GstMiniObject *mini_object);
void            gst_mini_object_unref		(GstMiniObject *mini_object);
void            gst_mini_object_weak_ref        (GstMiniObject *object, GstMiniObjectNotify notify, gpointer data);
void            gst_mini_object_weak_unref	(GstMiniObject *object, GstMiniObjectNotify notify, gpointer data);
gboolean        gst_mini_object_lock            (GstMiniObject *object, GstLockFlags flags);
void            gst_mini_object_unlock          (GstMiniObject *object, GstLockFlags flags);
gboolean        gst_mini_object_is_writable     (const GstMiniObject *mini_object);
GstMiniObject * gst_mini_object_make_writable	(GstMiniObject *mini_object);
GstMiniObject * gst_mini_object_copy		(const GstMiniObject *mini_object) G_GNUC_MALLOC;
void            gst_mini_object_set_qdata       (GstMiniObject *object, GQuark quark, gpointer data, GDestroyNotify destroy);
gpointer        gst_mini_object_get_qdata       (GstMiniObject *object, GQuark quark);
gpointer        gst_mini_object_steal_qdata     (GstMiniObject *object, GQuark quark);
gboolean        gst_mini_object_replace         (GstMiniObject **olddata, GstMiniObject *newdata);
gboolean        gst_mini_object_take            (GstMiniObject **olddata, GstMiniObject *newdata);
GstMiniObject * gst_mini_object_steal           (GstMiniObject **olddata);
#define GST_DEFINE_MINI_OBJECT_TYPE(TypeName,type_name) \
   G_DEFINE_BOXED_TYPE(TypeName,type_name, (GBoxedCopyFunc)gst_mini_object_ref, (GBoxedFreeFunc)gst_mini_object_unref)

G_END_DECLS

#endif