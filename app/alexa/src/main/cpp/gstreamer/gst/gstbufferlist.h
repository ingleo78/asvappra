#ifndef __GST_BUFFER_LIST_H__
#define __GST_BUFFER_LIST_H__

#include "gstbuffer.h"

G_BEGIN_DECLS
GST_EXPORT GType _gst_buffer_list_type;
#define GST_TYPE_BUFFER_LIST      (_gst_buffer_list_type)
#define GST_IS_BUFFER_LIST(obj)   (GST_IS_MINI_OBJECT_TYPE(obj, GST_TYPE_BUFFER_LIST))
#define GST_BUFFER_LIST_CAST(obj) ((GstBufferList *)obj)
#define GST_BUFFER_LIST(obj)      (GST_BUFFER_LIST_CAST(obj))
typedef struct _GstBufferList GstBufferList;
typedef gboolean   (*GstBufferListFunc)   (GstBuffer **buffer, guint idx, gpointer user_data);
static inline GstBufferList *gst_buffer_list_ref (GstBufferList * list) {
  return GST_BUFFER_LIST_CAST (gst_mini_object_ref (GST_MINI_OBJECT_CAST (list)));
}
static inline void gst_buffer_list_unref (GstBufferList * list) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (list));
}
static inline GstBufferList *gst_buffer_list_copy (const GstBufferList * list) {
  return GST_BUFFER_LIST_CAST (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (list)));
}
#define gst_buffer_list_is_writable(list) gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (list))
#define gst_buffer_list_make_writable(list) GST_BUFFER_LIST_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (list)))
GType                    gst_buffer_list_get_type              (void);
GstBufferList *          gst_buffer_list_new                   (void) G_GNUC_MALLOC;
GstBufferList *          gst_buffer_list_new_sized             (guint size) G_GNUC_MALLOC;
guint                    gst_buffer_list_length                (GstBufferList *list);
GstBuffer *              gst_buffer_list_get                   (GstBufferList *list, guint idx);
void                     gst_buffer_list_insert                (GstBufferList *list, gint idx, GstBuffer *buffer);
void                     gst_buffer_list_remove                (GstBufferList *list, guint idx, guint length);
gboolean                 gst_buffer_list_foreach               (GstBufferList *list, GstBufferListFunc func, gpointer user_data);
GstBufferList *          gst_buffer_list_copy_deep             (const GstBufferList * list);
#define gst_buffer_list_add(l,b) gst_buffer_list_insert((l),-1,(b));
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstBufferList, gst_buffer_list_unref)
#endif
G_END_DECLS

#endif