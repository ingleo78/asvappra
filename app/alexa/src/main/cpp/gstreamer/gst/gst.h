#ifndef __GST_H__
#define __GST_H__

#include <glib/glib.h>
#include <gstreamer/gst/glib-compat.h>
#include "gstenumtypes.h"
#include "gstversion.h"
#include "gstatomicqueue.h"
#include "gstbin.h"
#include "gstbuffer.h"
#include "gstbufferlist.h"
#include "gstbufferpool.h"
#include "gstcaps.h"
#include "gstcapsfeatures.h"
#include "gstchildproxy.h"
#include "gstclock.h"
#include "gstcontrolsource.h"
#include "gstdatetime.h"
#include "gstdebugutils.h"
#include "gstdevice.h"
#include "gstdevicemonitor.h"
#include "gstdeviceprovider.h"
#include "gstelement.h"
#include "gstelementmetadata.h"
#include "gsterror.h"
#include "gstevent.h"
#include "gstghostpad.h"
#include "gstinfo.h"
#include "gstiterator.h"
#include "gstmessage.h"
#include "gstmemory.h"
#include "gstmeta.h"
#include "gstminiobject.h"
#include "gstobject.h"
#include "gstpad.h"
#include "gstparamspecs.h"
#include "gstpipeline.h"
#include "gstplugin.h"
#include "gstpoll.h"
#include "gstpreset.h"
#include "gstprotection.h"
#include "gstquery.h"
#include "gstregistry.h"
#include "gstsample.h"
#include "gstsegment.h"
#include "gststructure.h"
#include "gstsystemclock.h"
#include "gsttaglist.h"
#include "gsttagsetter.h"
#include "gsttask.h"
#include "gsttaskpool.h"
#include "gsttoc.h"
#include "gsttocsetter.h"
#include "gsttracer.h"
#include "gsttracerfactory.h"
#include "gsttracerrecord.h"
#include "gsttypefind.h"
#include "gsttypefindfactory.h"
#include "gsturi.h"
#include "gstutils.h"
#include "gstvalue.h"
#include "gstparse.h"
#include "gstcompat.h"

G_BEGIN_DECLS
void gst_init(int *argc, char **argv[]);
gboolean gst_init_check(int *argc, char **argv[], GError ** err);
gboolean gst_is_initialized(void);
GOptionGroup *gst_init_get_option_group(void);
void gst_deinit(void);
void gst_version(guint *major, guint *minor, guint *micro, guint *nano);
gchar *gst_version_string(void);
gboolean gst_segtrap_is_enabled(void);
void gst_segtrap_set_enabled(gboolean enabled);
gboolean gst_registry_fork_is_enabled(void);
void gst_registry_fork_set_enabled(gboolean enabled);
gboolean gst_update_registry(void);
G_END_DECLS

#endif