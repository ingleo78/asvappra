#ifndef __GST_TRACER_FACTORY_H__
#define __GST_TRACER_FACTORY_H__

#include "gstcaps.h"
#include "gstplugin.h"
#include "gstpluginfeature.h"

G_BEGIN_DECLS
#define GST_TYPE_TRACER_FACTORY                 (gst_tracer_factory_get_type())
#define GST_TRACER_FACTORY(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_TRACER_FACTORY, GstTracerFactory))
#define GST_IS_TRACER_FACTORY(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_TRACER_FACTORY))
#define GST_TRACER_FACTORY_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_TRACER_FACTORY, GstTracerFactoryClass))
#define GST_IS_TRACER_FACTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_TRACER_FACTORY))
#define GST_TRACER_FACTORY_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_TRACER_FACTORY, GstTracerFactoryClass))
#define GST_TRACER_FACTORY_CAST(obj)            ((GstTracerFactory *)(obj))
typedef struct _GstTracerFactory GstTracerFactory;
typedef struct _GstTracerFactoryClass GstTracerFactoryClass;
GType           gst_tracer_factory_get_type          (void);
GList *         gst_tracer_factory_get_list          (void);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTracerFactory, gst_object_unref)
#endif
G_END_DECLS

#endif