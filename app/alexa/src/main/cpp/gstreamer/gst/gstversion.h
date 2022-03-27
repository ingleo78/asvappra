#ifndef __GST_VERSION_H__
#define __GST_VERSION_H__

#include <glib/glib.h>

G_BEGIN_DECLS
#define GST_VERSION_MAJOR 1
#define GST_VERSION_MINOR 19
#define GST_VERSION_MICRO 3
#define GST_VERSION_NANO 1193
#define	GST_CHECK_VERSION(major,minor,micro)	\
    (GST_VERSION_MAJOR > (major) || \
     (GST_VERSION_MAJOR == (major) && GST_VERSION_MINOR > (minor)) || \
     (GST_VERSION_MAJOR == (major) && GST_VERSION_MINOR == (minor) && \
      GST_VERSION_MICRO >= (micro)) || \
     (GST_VERSION_MAJOR == (major) && GST_VERSION_MINOR == (minor) && \
      GST_VERSION_MICRO + 1 == (micro) && GST_VERSION_NANO > 0))
G_END_DECLS

#endif