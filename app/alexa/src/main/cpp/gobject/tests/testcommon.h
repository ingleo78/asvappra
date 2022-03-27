#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include "../../glib/glib.h"

G_BEGIN_DECLS

#define DEFINE_TYPE_FULL(name, prefix, class_init, base_init, instance_init, parent_type, interface_decl) \
GType prefix ## _get_type (void) { \
    static GType object_type = 0; \
    if (!object_type) {	\
        static const GTypeInfo object_info = { \
            sizeof (name ## Class),	\
            (GBaseInitFunc) base_init, \
            (GBaseFinalizeFunc) NULL, \
            (GClassInitFunc) class_init, \
            (GClassFinalizeFunc) NULL, \
            NULL, \
            sizeof(name), \
            0, \
            (GInstanceInitFunc) instance_init \
	    }; \
        object_type = g_type_register_static (parent_type, # name, &object_info, 0); \
        interface_decl \
    } \
    return object_type;	\
}
#define DEFINE_TYPE(name, prefix, class_init, base_init, instance_init,	parent_type) \
    DEFINE_TYPE_FULL(name, prefix, class_init, base_init,	instance_init, parent_type, {})
#define DEFINE_IFACE(name, prefix, base_init, dflt_init) \
GType prefix ## _get_type (void) { \
    static GType iface_type = 0; \
    if (!iface_type) { \
        static const GTypeInfo iface_info = { \
            sizeof(name ## Class), \
            (GBaseInitFunc)	base_init, \
            (GBaseFinalizeFunc) NULL, \
            (GClassInitFunc) dflt_init \
        }; \
        iface_type = g_type_register_static(G_TYPE_INTERFACE, # name, &iface_info, 0); \
    } \
    return iface_type; \
}
#define INTERFACE_FULL(type, init_func, iface_type) { \
    static GInterfaceInfo const iface = { \
        (GInterfaceInitFunc) init_func, NULL, NULL \
    }; \
    g_type_add_interface_static (type, iface_type, &iface);	\
}
#define INTERFACE(init_func, iface_type)  INTERFACE_FULL(object_type, init_func, iface_type)
G_END_DECLS
#endif