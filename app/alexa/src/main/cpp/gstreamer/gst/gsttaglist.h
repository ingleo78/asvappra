#ifndef __GST_TAGLIST_H__
#define __GST_TAGLIST_H__

#include "gstdatetime.h"
#include "gstsample.h"
#include "glib-compat.h"

G_BEGIN_DECLS
typedef enum {
  GST_TAG_MERGE_UNDEFINED,
  GST_TAG_MERGE_REPLACE_ALL,
  GST_TAG_MERGE_REPLACE,
  GST_TAG_MERGE_APPEND,
  GST_TAG_MERGE_PREPEND,
  GST_TAG_MERGE_KEEP,
  GST_TAG_MERGE_KEEP_ALL,
  GST_TAG_MERGE_COUNT
} GstTagMergeMode;
#define GST_TAG_MODE_IS_VALID(mode)     (((mode) > GST_TAG_MERGE_UNDEFINED) && ((mode) < GST_TAG_MERGE_COUNT))
typedef enum {
  GST_TAG_FLAG_UNDEFINED,
  GST_TAG_FLAG_META,
  GST_TAG_FLAG_ENCODED,
  GST_TAG_FLAG_DECODED,
  GST_TAG_FLAG_COUNT
} GstTagFlag;
#define GST_TAG_FLAG_IS_VALID(flag)     (((flag) > GST_TAG_FLAG_UNDEFINED) && ((flag) < GST_TAG_FLAG_COUNT))
typedef struct _GstTagList GstTagList;
struct _GstTagList {
  GstMiniObject mini_object;
};
GST_EXPORT GType _gst_tag_list_type;
#define GST_TAG_LIST(x)       ((GstTagList *) (x))
#define GST_TYPE_TAG_LIST     (_gst_tag_list_type)
#define GST_IS_TAG_LIST(obj)  (GST_IS_MINI_OBJECT_TYPE((obj), GST_TYPE_TAG_LIST))
typedef void (*GstTagForeachFunc) (const GstTagList * list, const gchar      * tag, gpointer  user_data);
typedef void (* GstTagMergeFunc) (GValue *dest, const GValue *src);
GType        gst_tag_list_get_type (void);
void gst_tag_register(const gchar *name, GstTagFlag flag, GType type, const gchar *nick, const gchar *blurb, GstTagMergeFunc func);
void gst_tag_register_static(const gchar *name, GstTagFlag flag, GType type, const gchar *nick, const gchar *blurb, GstTagMergeFunc func);
void      gst_tag_merge_use_first          (GValue * dest, const GValue * src);
void      gst_tag_merge_strings_with_comma (GValue * dest, const GValue * src);
gboolean               gst_tag_exists          (const gchar * tag);
GType                  gst_tag_get_type        (const gchar * tag);
const gchar *          gst_tag_get_nick        (const gchar * tag);
const gchar *          gst_tag_get_description (const gchar * tag);
GstTagFlag             gst_tag_get_flag        (const gchar * tag);
gboolean               gst_tag_is_fixed        (const gchar * tag);
typedef enum {
  GST_TAG_SCOPE_STREAM,
  GST_TAG_SCOPE_GLOBAL
} GstTagScope;
GstTagList * gst_tag_list_new_empty         (void) G_GNUC_MALLOC;
GstTagList * gst_tag_list_new               (const gchar * tag, ...) G_GNUC_MALLOC;
GstTagList * gst_tag_list_new_valist        (va_list var_args) G_GNUC_MALLOC;
void         gst_tag_list_set_scope         (GstTagList * list, GstTagScope scope);
GstTagScope  gst_tag_list_get_scope         (const GstTagList * list);
gchar      * gst_tag_list_to_string         (const GstTagList * list) G_GNUC_MALLOC;
GstTagList * gst_tag_list_new_from_string   (const gchar      * str) G_GNUC_MALLOC;
gint         gst_tag_list_n_tags            (const GstTagList * list);
const gchar* gst_tag_list_nth_tag_name      (const GstTagList * list, guint index);
gboolean     gst_tag_list_is_empty          (const GstTagList * list);
gboolean     gst_tag_list_is_equal          (const GstTagList * list1, const GstTagList * list2);
void         gst_tag_list_insert            (GstTagList       * into, const GstTagList * from, GstTagMergeMode    mode);
GstTagList * gst_tag_list_merge             (const GstTagList * list1, const GstTagList * list2, GstTagMergeMode    mode) G_GNUC_MALLOC;
guint        gst_tag_list_get_tag_size      (const GstTagList * list, const gchar      * tag);
void         gst_tag_list_add(GstTagList *list, GstTagMergeMode mode, const gchar *tag, ...) G_GNUC_NULL_TERMINATED;
void         gst_tag_list_add_values        (GstTagList *list, GstTagMergeMode mode, const gchar *tag, ...) G_GNUC_NULL_TERMINATED;
void         gst_tag_list_add_valist        (GstTagList *list, GstTagMergeMode mode, const gchar *tag, va_list var_args);
void         gst_tag_list_add_valist_values (GstTagList *list, GstTagMergeMode mode, const gchar *tag, va_list var_args);
void         gst_tag_list_add_value         (GstTagList *list, GstTagMergeMode mode, const gchar *tag, const GValue *value);
void         gst_tag_list_remove_tag        (GstTagList *list, const gchar *tag);
void         gst_tag_list_foreach           (const GstTagList *list, GstTagForeachFunc func, gpointer user_data);
const GValue *gst_tag_list_get_value_index(const GstTagList *list, const gchar *tag, guint index);
gboolean     gst_tag_list_copy_value        (GValue           *dest, const GstTagList *list, const gchar *tag);
gboolean     gst_tag_list_get_boolean       (const GstTagList *list, const gchar *tag, gboolean *value);
gboolean     gst_tag_list_get_boolean_index (const GstTagList *list, const gchar *tag, guint index, gboolean *value);
gboolean     gst_tag_list_get_int           (const GstTagList *list, const gchar *tag, gint *value);
gboolean     gst_tag_list_get_int_index     (const GstTagList *list, const gchar *tag, guint index, gint *value);
gboolean     gst_tag_list_get_uint          (const GstTagList *list, const gchar *tag, guint *value);
gboolean     gst_tag_list_get_uint_index    (const GstTagList *list,const gchar *tag,guint index, guint *value);
gboolean     gst_tag_list_get_int64         (const GstTagList *list, const gchar *tag, gint64 *value);
gboolean     gst_tag_list_get_int64_index   (const GstTagList *list, const gchar *tag, guint index, gint64 *value);
gboolean     gst_tag_list_get_uint64        (const GstTagList *list, const gchar *tag, guint64 *value);
gboolean     gst_tag_list_get_uint64_index  (const GstTagList *list, const gchar *tag, guint index, guint64 *value);
gboolean     gst_tag_list_get_float         (const GstTagList *list, const gchar *tag, gfloat *value);
gboolean     gst_tag_list_get_float_index   (const GstTagList *list, const gchar *tag, guint index, gfloat *value);
gboolean     gst_tag_list_get_double        (const GstTagList *list, const gchar *tag, gdouble *value);
gboolean     gst_tag_list_get_double_index  (const GstTagList *list, const gchar *tag, guint index, gdouble *value);
gboolean     gst_tag_list_get_string        (const GstTagList *list, const gchar *tag, gchar **value);
gboolean     gst_tag_list_get_string_index  (const GstTagList *list, const gchar *tag, guint index, gchar **value);
gboolean     gst_tag_list_peek_string_index (const GstTagList *list, const gchar *tag, guint index, const gchar **value);
gboolean     gst_tag_list_get_pointer       (const GstTagList *list, const gchar *tag, gpointer *value);
gboolean     gst_tag_list_get_pointer_index (const GstTagList *list, const gchar *tag, guint index, gpointer *value);
gboolean     gst_tag_list_get_date          (const GstTagList *list, const gchar *tag, GDate **value);
gboolean     gst_tag_list_get_date_index    (const GstTagList *list, const gchar *tag, guint index, GDate **value);
gboolean     gst_tag_list_get_date_time     (const GstTagList *list, const gchar *tag, GstDateTime **value);
gboolean     gst_tag_list_get_date_time_index (const GstTagList *list, const gchar *tag, guint index, GstDateTime **value);
gboolean     gst_tag_list_get_sample        (const GstTagList *list, const gchar *tag, GstSample **sample);
gboolean     gst_tag_list_get_sample_index  (const GstTagList *list, const gchar *tag, guint index, GstSample **sample);
static inline GstTagList *gst_tag_list_ref(GstTagList *taglist) {
  return (GstTagList *) gst_mini_object_ref (GST_MINI_OBJECT_CAST (taglist));
}
static inline void gst_tag_list_unref(GstTagList *taglist) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (taglist));
}
static inline GstTagList *gst_tag_list_copy (const GstTagList * taglist) {
  return GST_TAG_LIST (gst_mini_object_copy (GST_MINI_OBJECT_CAST (taglist)));
}
#define gst_tag_list_is_writable(taglist)    gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (taglist))
#define gst_tag_list_make_writable(taglist)   GST_TAG_LIST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (taglist)))
#define GST_TAG_TITLE                  "title"
#define GST_TAG_TITLE_SORTNAME         "title-sortname"
#define GST_TAG_ARTIST                 "artist"
#define GST_TAG_ARTIST_SORTNAME        "artist-sortname"
#define GST_TAG_ALBUM                  "album"
#define GST_TAG_ALBUM_SORTNAME         "album-sortname"
#define GST_TAG_ALBUM_ARTIST           "album-artist"
#define GST_TAG_ALBUM_ARTIST_SORTNAME  "album-artist-sortname"
#define GST_TAG_COMPOSER               "composer"
#define GST_TAG_CONDUCTOR               "conductor"
#define GST_TAG_DATE                   "date"
#define GST_TAG_DATE_TIME              "datetime"
#define GST_TAG_GENRE                  "genre"
#define GST_TAG_COMMENT                "comment"
#define GST_TAG_EXTENDED_COMMENT       "extended-comment"
#define GST_TAG_TRACK_NUMBER           "track-number"
#define GST_TAG_TRACK_COUNT            "track-count"
#define GST_TAG_ALBUM_VOLUME_NUMBER    "album-disc-number"
#define GST_TAG_ALBUM_VOLUME_COUNT    "album-disc-count"
#define GST_TAG_LOCATION               "location"
#define GST_TAG_HOMEPAGE               "homepage"
#define GST_TAG_DESCRIPTION            "description"
#define GST_TAG_VERSION                "version"
#define GST_TAG_ISRC                   "isrc"
#define GST_TAG_ORGANIZATION           "organization"
#define GST_TAG_COPYRIGHT              "copyright"
#define GST_TAG_COPYRIGHT_URI          "copyright-uri"
#define GST_TAG_ENCODED_BY             "encoded-by"
#define GST_TAG_CONTACT                "contact"
#define GST_TAG_LICENSE                "license"
#define GST_TAG_LICENSE_URI            "license-uri"
#define GST_TAG_PERFORMER              "performer"
#define GST_TAG_DURATION               "duration"
#define GST_TAG_CODEC                  "codec"
#define GST_TAG_VIDEO_CODEC            "video-codec"
#define GST_TAG_AUDIO_CODEC            "audio-codec"
#define GST_TAG_SUBTITLE_CODEC         "subtitle-codec"
#define GST_TAG_CONTAINER_FORMAT       "container-format"
#define GST_TAG_BITRATE                "bitrate"
#define GST_TAG_NOMINAL_BITRATE        "nominal-bitrate"
#define GST_TAG_MINIMUM_BITRATE        "minimum-bitrate"
#define GST_TAG_MAXIMUM_BITRATE        "maximum-bitrate"
#define GST_TAG_SERIAL                 "serial"
#define GST_TAG_ENCODER                "encoder"
#define GST_TAG_ENCODER_VERSION        "encoder-version"
#define GST_TAG_TRACK_GAIN             "replaygain-track-gain"
#define GST_TAG_TRACK_PEAK             "replaygain-track-peak"
#define GST_TAG_ALBUM_GAIN             "replaygain-album-gain"
#define GST_TAG_ALBUM_PEAK             "replaygain-album-peak"
#define GST_TAG_REFERENCE_LEVEL        "replaygain-reference-level"
#define GST_TAG_LANGUAGE_CODE          "language-code"
#define GST_TAG_LANGUAGE_NAME          "language-name"
#define GST_TAG_IMAGE                  "image"
#define GST_TAG_PREVIEW_IMAGE          "preview-image"
#define GST_TAG_ATTACHMENT             "attachment"
#define GST_TAG_BEATS_PER_MINUTE       "beats-per-minute"
#define GST_TAG_KEYWORDS               "keywords"
#define GST_TAG_GEO_LOCATION_NAME               "geo-location-name"
#define GST_TAG_GEO_LOCATION_LATITUDE               "geo-location-latitude"
#define GST_TAG_GEO_LOCATION_LONGITUDE               "geo-location-longitude"
#define GST_TAG_GEO_LOCATION_ELEVATION               "geo-location-elevation"
#define GST_TAG_GEO_LOCATION_COUNTRY                 "geo-location-country"
#define GST_TAG_GEO_LOCATION_CITY                    "geo-location-city"
#define GST_TAG_GEO_LOCATION_SUBLOCATION             "geo-location-sublocation"
#define GST_TAG_GEO_LOCATION_HORIZONTAL_ERROR   "geo-location-horizontal-error"
#define GST_TAG_GEO_LOCATION_MOVEMENT_SPEED       "geo-location-movement-speed"
#define GST_TAG_GEO_LOCATION_MOVEMENT_DIRECTION "geo-location-movement-direction"
#define GST_TAG_GEO_LOCATION_CAPTURE_DIRECTION  "geo-location-capture-direction"
#define GST_TAG_SHOW_NAME                         "show-name"
#define GST_TAG_SHOW_SORTNAME                     "show-sortname"
#define GST_TAG_SHOW_EPISODE_NUMBER               "show-episode-number"
#define GST_TAG_SHOW_SEASON_NUMBER                "show-season-number"
#define GST_TAG_LYRICS                            "lyrics"
#define GST_TAG_COMPOSER_SORTNAME                 "composer-sortname"
#define GST_TAG_GROUPING                          "grouping"
#define GST_TAG_USER_RATING                       "user-rating"
#define GST_TAG_DEVICE_MANUFACTURER               "device-manufacturer"
#define GST_TAG_DEVICE_MODEL                      "device-model"
#define GST_TAG_APPLICATION_NAME                  "application-name"
#define GST_TAG_APPLICATION_DATA          "application-data"
#define GST_TAG_IMAGE_ORIENTATION            "image-orientation"
#define GST_TAG_PUBLISHER                         "publisher"
#define GST_TAG_INTERPRETED_BY                    "interpreted-by"
#define GST_TAG_MIDI_BASE_NOTE                    "midi-base-note"
#define GST_TAG_PRIVATE_DATA                         "private-data"
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTagList, gst_tag_list_unref)
#endif
G_END_DECLS

#endif