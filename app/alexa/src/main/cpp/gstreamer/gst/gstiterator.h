#ifndef __GST_ITERATOR_H__
#define __GST_ITERATOR_H__

#include <glib/glib-object.h>
#include "gstconfig.h"

G_BEGIN_DECLS
#define GST_TYPE_ITERATOR (gst_iterator_get_type ())
typedef enum {
  GST_ITERATOR_DONE     = 0,
  GST_ITERATOR_OK       = 1,
  GST_ITERATOR_RESYNC   = 2,
  GST_ITERATOR_ERROR    = 3
} GstIteratorResult;
typedef struct _GstIterator GstIterator;
typedef enum {
  GST_ITERATOR_ITEM_SKIP        = 0,
  GST_ITERATOR_ITEM_PASS        = 1,
  GST_ITERATOR_ITEM_END         = 2
} GstIteratorItem;
typedef void              (*GstIteratorCopyFunction) (const GstIterator *it, GstIterator *copy);
typedef GstIteratorItem   (*GstIteratorItemFunction)    (GstIterator *it, const GValue * item);
typedef GstIteratorResult (*GstIteratorNextFunction)    (GstIterator *it, GValue *result);
typedef void              (*GstIteratorResyncFunction)  (GstIterator *it);
typedef void              (*GstIteratorFreeFunction)    (GstIterator *it);
typedef void         (*GstIteratorForeachFunction)     (const GValue * item, gpointer user_data);
typedef gboolean          (*GstIteratorFoldFunction)    (const GValue * item, GValue * ret, gpointer user_data);
#define GST_ITERATOR(it)                ((GstIterator*)(it))
#define GST_ITERATOR_LOCK(it)           (GST_ITERATOR(it)->lock)
#define GST_ITERATOR_COOKIE(it)         (GST_ITERATOR(it)->cookie)
#define GST_ITERATOR_ORIG_COOKIE(it)    (GST_ITERATOR(it)->master_cookie)
struct _GstIterator {
  GstIteratorCopyFunction copy;
  GstIteratorNextFunction next;
  GstIteratorItemFunction item;
  GstIteratorResyncFunction resync;
  GstIteratorFreeFunction free;
  GstIterator *pushed;
  GType     type;
  GMutex   *lock;
  guint32   cookie;
  guint32  *master_cookie;
  guint     size;
  gpointer _gst_reserved[GST_PADDING];
};
GType                   gst_iterator_get_type           (void);
GstIterator*            gst_iterator_new                (guint size,
                                                         GType type,
                                                         GMutex *lock,
                                                         guint32 *master_cookie,
                                                         GstIteratorCopyFunction copy,
                                                         GstIteratorNextFunction next,
                                                         GstIteratorItemFunction item,
                                                         GstIteratorResyncFunction resync,
                                                         GstIteratorFreeFunction free) G_GNUC_MALLOC;
GstIterator*            gst_iterator_new_list           (GType type,
                                                         GMutex *lock,
                                                         guint32 *master_cookie,
                                                         GList **list,
                                                         GObject * owner,
                                                         GstIteratorItemFunction item) G_GNUC_MALLOC;
GstIterator*            gst_iterator_new_single         (GType type,
                                                         const GValue * object) G_GNUC_MALLOC;
GstIterator*            gst_iterator_copy               (const GstIterator *it) G_GNUC_MALLOC;
GstIteratorResult       gst_iterator_next               (GstIterator *it, GValue * elem);
void                    gst_iterator_resync             (GstIterator *it);
void                    gst_iterator_free               (GstIterator *it);
void                    gst_iterator_push               (GstIterator *it, GstIterator *other);
GstIterator*            gst_iterator_filter             (GstIterator *it, GCompareFunc func,
                                                         const GValue * user_data) G_GNUC_MALLOC;
GstIteratorResult       gst_iterator_fold               (GstIterator *it,
                                                         GstIteratorFoldFunction func,
                                                         GValue *ret, gpointer user_data);
GstIteratorResult       gst_iterator_foreach            (GstIterator *it,
                                                         GstIteratorForeachFunction func, gpointer user_data);
gboolean                gst_iterator_find_custom        (GstIterator *it, GCompareFunc func,
                                                         GValue *elem, gpointer user_data);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstIterator, gst_iterator_free)
#endif
G_END_DECLS

#endif