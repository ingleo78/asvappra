#ifndef __GST_TYPE_FIND_FACTORY_H__
#define __GST_TYPE_FIND_FACTORY_H__

#include "gstcaps.h"
#include "gstplugin.h"
#include "gstpluginfeature.h"
#include "gsttypefind.h"

G_BEGIN_DECLS
#define GST_TYPE_TYPE_FIND_FACTORY                 (gst_type_find_factory_get_type())
#define GST_TYPE_FIND_FACTORY(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_TYPE_FIND_FACTORY, GstTypeFindFactory))
#define GST_IS_TYPE_FIND_FACTORY(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_TYPE_FIND_FACTORY))
#define GST_TYPE_FIND_FACTORY_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_TYPE_FIND_FACTORY, GstTypeFindFactoryClass))
#define GST_IS_TYPE_FIND_FACTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_TYPE_FIND_FACTORY))
#define GST_TYPE_FIND_FACTORY_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_TYPE_FIND_FACTORY, GstTypeFindFactoryClass))
typedef struct _GstTypeFindFactory GstTypeFindFactory;
typedef struct _GstTypeFindFactoryClass GstTypeFindFactoryClass;
GType           gst_type_find_factory_get_type          (void);
GList *         gst_type_find_factory_get_list          (void);
const gchar * const * gst_type_find_factory_get_extensions (GstTypeFindFactory *factory);
GstCaps *       gst_type_find_factory_get_caps          (GstTypeFindFactory *factory);
gboolean        gst_type_find_factory_has_function      (GstTypeFindFactory *factory);
void            gst_type_find_factory_call_function     (GstTypeFindFactory *factory, GstTypeFind *find);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTypeFindFactory, gst_object_unref)
#endif
G_END_DECLS

#endif