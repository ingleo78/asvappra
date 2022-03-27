#ifndef __GST_BUS_H__
#define __GST_BUS_H__

typedef struct _GstBus GstBus;
typedef struct _GstBusPrivate GstBusPrivate;
typedef struct _GstBusClass GstBusClass;

#include "gstmessage.h"
#include "gstclock.h"

G_BEGIN_DECLS
#define GST_TYPE_BUS              (gst_bus_get_type ())
#define GST_BUS(bus)              (G_TYPE_CHECK_INSTANCE_CAST ((bus), GST_TYPE_BUS, GstBus))
#define GST_IS_BUS(bus)           (G_TYPE_CHECK_INSTANCE_TYPE ((bus), GST_TYPE_BUS))
#define GST_BUS_CLASS(bclass)     (G_TYPE_CHECK_CLASS_CAST ((bclass), GST_TYPE_BUS, GstBusClass))
#define GST_IS_BUS_CLASS(bclass)  (G_TYPE_CHECK_CLASS_TYPE ((bclass), GST_TYPE_BUS))
#define GST_BUS_GET_CLASS(bus)    (G_TYPE_INSTANCE_GET_CLASS ((bus), GST_TYPE_BUS, GstBusClass))
#define GST_BUS_CAST(bus)         ((GstBus*)(bus))
typedef enum {
  GST_BUS_FLUSHING      = (GST_OBJECT_FLAG_LAST << 0),
  GST_BUS_FLAG_LAST     = (GST_OBJECT_FLAG_LAST << 1)
} GstBusFlags;
typedef enum
{
  GST_BUS_DROP = 0,
  GST_BUS_PASS = 1,
  GST_BUS_ASYNC = 2
} GstBusSyncReply;
typedef GstBusSyncReply (*GstBusSyncHandler)    (GstBus * bus, GstMessage * message, gpointer user_data);
typedef gboolean        (*GstBusFunc)           (GstBus * bus, GstMessage * message, gpointer user_data);
struct _GstBus {
  GstObject         object;
  GstBusPrivate    *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstBusClass {
  GstObjectClass parent_class;
  void (*message)       (GstBus *bus, GstMessage *message);
  void (*sync_message)  (GstBus *bus, GstMessage *message);
  gpointer _gst_reserved[GST_PADDING];
};
GType                   gst_bus_get_type                (void);
GstBus*                 gst_bus_new                     (void);
gboolean                gst_bus_post                    (GstBus * bus, GstMessage * message);
gboolean                gst_bus_have_pending            (GstBus * bus);
GstMessage *            gst_bus_peek                    (GstBus * bus);
GstMessage *            gst_bus_pop                     (GstBus * bus);
GstMessage *            gst_bus_pop_filtered            (GstBus * bus, GstMessageType types);
GstMessage *            gst_bus_timed_pop               (GstBus * bus, GstClockTime timeout);
GstMessage *            gst_bus_timed_pop_filtered      (GstBus * bus, GstClockTime timeout, GstMessageType types);
void                    gst_bus_set_flushing            (GstBus * bus, gboolean flushing);
void                    gst_bus_set_sync_handler        (GstBus * bus, GstBusSyncHandler func,
                                                         gpointer user_data, GDestroyNotify notify);
GSource *               gst_bus_create_watch            (GstBus * bus);
guint                   gst_bus_add_watch_full          (GstBus * bus,
                                                         gint priority,
                                                         GstBusFunc func,
                                                         gpointer user_data,
                                                         GDestroyNotify notify);
guint                   gst_bus_add_watch               (GstBus * bus,
                                                         GstBusFunc func,
                                                         gpointer user_data);
gboolean                gst_bus_remove_watch            (GstBus * bus);
GstMessage*             gst_bus_poll                    (GstBus *bus, GstMessageType events,
                                                         GstClockTime timeout);
gboolean                gst_bus_async_signal_func       (GstBus *bus, GstMessage *message,
                                                         gpointer data);
GstBusSyncReply         gst_bus_sync_signal_handler     (GstBus *bus, GstMessage *message,
                                                         gpointer data);
void                    gst_bus_add_signal_watch        (GstBus * bus);
void                    gst_bus_add_signal_watch_full   (GstBus * bus, gint priority);
void                    gst_bus_remove_signal_watch     (GstBus * bus);
void                    gst_bus_enable_sync_message_emission (GstBus * bus);
void                    gst_bus_disable_sync_message_emission (GstBus * bus);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstBus, gst_object_unref)
#endif
G_END_DECLS

#endif