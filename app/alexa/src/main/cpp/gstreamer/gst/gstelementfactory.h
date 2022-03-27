#ifndef __GST_ELEMENT_FACTORY_H__
#define __GST_ELEMENT_FACTORY_H__

#include "gstconfig.h"
#include "gstpad.h"
#include "gstplugin.h"
#include "gstpluginfeature.h"
#include "gsturi.h"

typedef struct _GstElementFactory GstElementFactory;
typedef struct _GstElementFactoryClass GstElementFactoryClass;
G_BEGIN_DECLS
typedef enum {
    GST_STATE_VOID_PENDING        = 0,
    GST_STATE_NULL                = 1,
    GST_STATE_READY               = 2,
    GST_STATE_PAUSED              = 3,
    GST_STATE_PLAYING             = 4
} GstState;
typedef enum {
    GST_STATE_CHANGE_FAILURE             = 0,
    GST_STATE_CHANGE_SUCCESS             = 1,
    GST_STATE_CHANGE_ASYNC               = 2,
    GST_STATE_CHANGE_NO_PREROLL          = 3
} GstStateChangeReturn;
struct _GstElement {
    GstObject             object;
    GRecMutex             state_lock;
    GCond                 state_cond;
    guint32               state_cookie;
    GstState              target_state;
    GstState              current_state;
    GstState              next_state;
    GstState              pending_state;
    GstStateChangeReturn  last_return;
    GstBus               *bus;
    GstClock             *clock;
    GstClockTimeDiff      base_time;
    GstClockTime          start_time;
    guint16               numpads;
    GList                *pads;
    guint16               numsrcpads;
    GList                *srcpads;
    guint16               numsinkpads;
    GList                *sinkpads;
    guint32               pads_cookie;
    GList                *contexts;
    gpointer _gst_reserved[GST_PADDING-1];
};
typedef struct _GstElement GstElement;
#define GST_TYPE_ELEMENT_FACTORY                (gst_element_factory_get_type())
#define GST_ELEMENT_FACTORY(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ELEMENT_FACTORY,\
                                                 GstElementFactory))
#define GST_ELEMENT_FACTORY_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ELEMENT_FACTORY,\
                                                 GstElementFactoryClass))
#define GST_IS_ELEMENT_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ELEMENT_FACTORY))
#define GST_IS_ELEMENT_FACTORY_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ELEMENT_FACTORY))
#define GST_ELEMENT_FACTORY_CAST(obj)           ((GstElementFactory *)(obj))

GType                   gst_element_factory_get_type            (void);

GstElementFactory *     gst_element_factory_find                (const gchar *name);

GType                   gst_element_factory_get_element_type    (GstElementFactory *factory);

const gchar *           gst_element_factory_get_metadata        (GstElementFactory *factory, const gchar *key);
gchar **                gst_element_factory_get_metadata_keys   (GstElementFactory *factory);

guint                   gst_element_factory_get_num_pad_templates (GstElementFactory *factory);
const GList *           gst_element_factory_get_static_pad_templates (GstElementFactory *factory);

GstURIType              gst_element_factory_get_uri_type        (GstElementFactory *factory);
const gchar * const *   gst_element_factory_get_uri_protocols   (GstElementFactory *factory);

gboolean                gst_element_factory_has_interface       (GstElementFactory *factory,
                                                                 const gchar *interfacename);

GstElement*             gst_element_factory_create              (GstElementFactory *factory,
                                                                 const gchar *name) G_GNUC_MALLOC;
GstElement*             gst_element_factory_make                (const gchar *factoryname, const gchar *name) G_GNUC_MALLOC;

gboolean                gst_element_register                    (GstPlugin *plugin, const gchar *name,
                                                                 guint rank, GType type);
typedef guint64 GstElementFactoryListType;
#define  GST_ELEMENT_FACTORY_TYPE_DECODER        (G_GUINT64_CONSTANT (1) << 0)
#define  GST_ELEMENT_FACTORY_TYPE_ENCODER        (G_GUINT64_CONSTANT (1) << 1)
#define  GST_ELEMENT_FACTORY_TYPE_SINK           (G_GUINT64_CONSTANT (1) << 2)
#define  GST_ELEMENT_FACTORY_TYPE_SRC            (G_GUINT64_CONSTANT (1) << 3)
#define  GST_ELEMENT_FACTORY_TYPE_MUXER          (G_GUINT64_CONSTANT (1) << 4)
#define  GST_ELEMENT_FACTORY_TYPE_DEMUXER        (G_GUINT64_CONSTANT (1) << 5)
#define  GST_ELEMENT_FACTORY_TYPE_PARSER         (G_GUINT64_CONSTANT (1) << 6)
#define  GST_ELEMENT_FACTORY_TYPE_PAYLOADER      (G_GUINT64_CONSTANT (1) << 7)
#define  GST_ELEMENT_FACTORY_TYPE_DEPAYLOADER    (G_GUINT64_CONSTANT (1) << 8)
#define  GST_ELEMENT_FACTORY_TYPE_FORMATTER      (G_GUINT64_CONSTANT (1) << 9)
#define  GST_ELEMENT_FACTORY_TYPE_DECRYPTOR      (G_GUINT64_CONSTANT (1) << 10)
#define  GST_ELEMENT_FACTORY_TYPE_ENCRYPTOR      (G_GUINT64_CONSTANT (1) << 11)

#define  GST_ELEMENT_FACTORY_TYPE_MAX_ELEMENTS   (G_GUINT64_CONSTANT (1) << 48)

#define  GST_ELEMENT_FACTORY_TYPE_MEDIA_VIDEO    (G_GUINT64_CONSTANT (1) << 49)
#define  GST_ELEMENT_FACTORY_TYPE_MEDIA_AUDIO    (G_GUINT64_CONSTANT (1) << 50)
#define  GST_ELEMENT_FACTORY_TYPE_MEDIA_IMAGE    (G_GUINT64_CONSTANT (1) << 51)
#define  GST_ELEMENT_FACTORY_TYPE_MEDIA_SUBTITLE (G_GUINT64_CONSTANT (1) << 52)
#define  GST_ELEMENT_FACTORY_TYPE_MEDIA_METADATA (G_GUINT64_CONSTANT (1) << 53)

/**
 * GST_ELEMENT_FACTORY_TYPE_ANY: (value 562949953421311) (type GstElementFactoryListType)
 *
 * Elements of any of the defined GST_ELEMENT_FACTORY_LIST types
 */
#define  GST_ELEMENT_FACTORY_TYPE_ANY ((G_GUINT64_CONSTANT (1) << 49) - 1)

/**
 * GST_ELEMENT_FACTORY_TYPE_MEDIA_ANY: (value 18446462598732840960) (type GstElementFactoryListType)
 *
 * Elements matching any of the defined GST_ELEMENT_FACTORY_TYPE_MEDIA types
 *
 * Note: Do not use this if you wish to not filter against any of the defined
 * media types. If you wish to do this, simply don't specify any
 * GST_ELEMENT_FACTORY_TYPE_MEDIA flag.
 */
#define GST_ELEMENT_FACTORY_TYPE_MEDIA_ANY (~G_GUINT64_CONSTANT (0) << 48)

/**
 * GST_ELEMENT_FACTORY_TYPE_VIDEO_ENCODER: (value 2814749767106562) (type GstElementFactoryListType)
 *
 * All encoders handling video or image media types
 */
#define GST_ELEMENT_FACTORY_TYPE_VIDEO_ENCODER (GST_ELEMENT_FACTORY_TYPE_ENCODER | GST_ELEMENT_FACTORY_TYPE_MEDIA_VIDEO | GST_ELEMENT_FACTORY_TYPE_MEDIA_IMAGE)

/**
 * GST_ELEMENT_FACTORY_TYPE_AUDIO_ENCODER: (value 1125899906842626) (type GstElementFactoryListType)
 *
 * All encoders handling audio media types
 */
#define GST_ELEMENT_FACTORY_TYPE_AUDIO_ENCODER (GST_ELEMENT_FACTORY_TYPE_ENCODER | GST_ELEMENT_FACTORY_TYPE_MEDIA_AUDIO)

/**
 * GST_ELEMENT_FACTORY_TYPE_AUDIOVIDEO_SINKS: (value 3940649673949188) (type GstElementFactoryListType)
 *
 * All sinks handling audio, video or image media types
 */
#define GST_ELEMENT_FACTORY_TYPE_AUDIOVIDEO_SINKS (GST_ELEMENT_FACTORY_TYPE_SINK | GST_ELEMENT_FACTORY_TYPE_MEDIA_AUDIO | GST_ELEMENT_FACTORY_TYPE_MEDIA_VIDEO | GST_ELEMENT_FACTORY_TYPE_MEDIA_IMAGE)

/**
 * GST_ELEMENT_FACTORY_TYPE_DECODABLE: (value 353) (type GstElementFactoryListType)
 *
 * All elements used to 'decode' streams (decoders, demuxers, parsers, depayloaders)
 */
#define GST_ELEMENT_FACTORY_TYPE_DECODABLE \
  (GST_ELEMENT_FACTORY_TYPE_DECODER | GST_ELEMENT_FACTORY_TYPE_DEMUXER | GST_ELEMENT_FACTORY_TYPE_DEPAYLOADER | GST_ELEMENT_FACTORY_TYPE_PARSER | GST_ELEMENT_FACTORY_TYPE_DECRYPTOR)

/* Element klass defines */
#define GST_ELEMENT_FACTORY_KLASS_DECODER               "Decoder"
#define GST_ELEMENT_FACTORY_KLASS_ENCODER               "Encoder"
#define GST_ELEMENT_FACTORY_KLASS_SINK                  "Sink"
#define GST_ELEMENT_FACTORY_KLASS_SRC                   "Source"
#define GST_ELEMENT_FACTORY_KLASS_MUXER                 "Muxer"
#define GST_ELEMENT_FACTORY_KLASS_DEMUXER               "Demuxer"
#define GST_ELEMENT_FACTORY_KLASS_PARSER                "Parser"
#define GST_ELEMENT_FACTORY_KLASS_PAYLOADER             "Payloader"
#define GST_ELEMENT_FACTORY_KLASS_DEPAYLOADER           "Depayloader"
#define GST_ELEMENT_FACTORY_KLASS_FORMATTER             "Formatter"
#define GST_ELEMENT_FACTORY_KLASS_DECRYPTOR             "Decryptor"
#define GST_ELEMENT_FACTORY_KLASS_ENCRYPTOR             "Encryptor"

#define GST_ELEMENT_FACTORY_KLASS_MEDIA_VIDEO           "Video"
#define GST_ELEMENT_FACTORY_KLASS_MEDIA_AUDIO           "Audio"
#define GST_ELEMENT_FACTORY_KLASS_MEDIA_IMAGE           "Image"
#define GST_ELEMENT_FACTORY_KLASS_MEDIA_SUBTITLE        "Subtitle"
#define GST_ELEMENT_FACTORY_KLASS_MEDIA_METADATA        "Metadata"

gboolean      gst_element_factory_list_is_type      (GstElementFactory *factory,
                                                     GstElementFactoryListType type);

GList *       gst_element_factory_list_get_elements (GstElementFactoryListType type,
                                                     GstRank minrank) G_GNUC_MALLOC;


GList *       gst_element_factory_list_filter       (GList *list, const GstCaps *caps,
                                                     GstPadDirection direction,
                                                     gboolean subsetonly) G_GNUC_MALLOC;
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstElementFactory, gst_object_unref)
#endif

G_END_DECLS

#endif /* __GST_ELEMENT_FACTORY_H__ */
