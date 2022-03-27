#ifndef __GST_URI_H__
#define __GST_URI_H__

#include <glib/glib.h>
#include <glib/glib-object.h>
#include "gstconfig.h"
#include "gstminiobject.h"
#include "gstelementfactory.h"

G_BEGIN_DECLS
GQuark gst_uri_error_quark (void);
#define GST_URI_ERROR gst_uri_error_quark ()
typedef enum {
  GST_URI_ERROR_UNSUPPORTED_PROTOCOL,
  GST_URI_ERROR_BAD_URI,
  GST_URI_ERROR_BAD_STATE,
  GST_URI_ERROR_BAD_REFERENCE
} GstURIError;
typedef enum {
  GST_URI_UNKNOWN,
  GST_URI_SINK,
  GST_URI_SRC
} GstURIType;
typedef struct _GstElement GstElement;
#define GST_URI_TYPE_IS_VALID(type) ((type) == GST_URI_SRC || (type) == GST_URI_SINK)
#define GST_TYPE_URI_HANDLER               (gst_uri_handler_get_type ())
#define GST_URI_HANDLER(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_URI_HANDLER, GstURIHandler))
#define GST_IS_URI_HANDLER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_URI_HANDLER))
#define GST_URI_HANDLER_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GST_TYPE_URI_HANDLER, GstURIHandlerInterface))
typedef struct _GstURIHandler GstURIHandler;
typedef struct _GstURIHandlerInterface GstURIHandlerInterface;
struct _GstURIHandlerInterface {
  GTypeInterface parent;
  GstURIType             (* get_type)           (GType type);
  const gchar * const *  (* get_protocols)      (GType type);
  gchar *                (* get_uri)            (GstURIHandler * handler);
  gboolean               (* set_uri)            (GstURIHandler * handler, const gchar   * uri, GError       ** error);
};
gboolean        gst_uri_protocol_is_valid       (const gchar * protocol);
gboolean        gst_uri_protocol_is_supported   (const GstURIType type, const gchar *protocol);
gboolean        gst_uri_is_valid                (const gchar * uri);
gchar *         gst_uri_get_protocol            (const gchar * uri) G_GNUC_MALLOC;
gboolean        gst_uri_has_protocol            (const gchar * uri,
                                                 const gchar * protocol);
gchar *         gst_uri_get_location            (const gchar * uri) G_GNUC_MALLOC;
gchar *         gst_uri_construct               (const gchar * protocol,
                                                 const gchar * location) G_GNUC_MALLOC;

gchar *         gst_filename_to_uri             (const gchar * filename,
                                                 GError     ** error) G_GNUC_MALLOC;

GstElement *    gst_element_make_from_uri       (const GstURIType type,
                                                 const gchar *    uri,
                                                 const gchar *    elementname,
                                                 GError      **   error) G_GNUC_MALLOC;

/* accessing the interface */
GType                 gst_uri_handler_get_type (void);

GstURIType            gst_uri_handler_get_uri_type  (GstURIHandler * handler);
const gchar * const * gst_uri_handler_get_protocols (GstURIHandler * handler);
gchar *               gst_uri_handler_get_uri       (GstURIHandler * handler) G_GNUC_MALLOC;
gboolean              gst_uri_handler_set_uri       (GstURIHandler * handler,
                                                     const gchar   * uri,
                                                     GError       ** error);

/*
 * GstUri Type macros.
 */
#define GST_TYPE_URI        (gst_uri_get_type ())
#define GST_IS_URI(obj)	    (GST_IS_MINI_OBJECT_TYPE (obj, GST_TYPE_URI))
#define GST_URI_CAST(obj)   ((GstUri *)(obj))
#define GST_URI_CONST_CAST(obj) ((const GstUri *)(obj))
#define GST_URI(obj)        (GST_URI_CAST(obj))

/**
 * GstUri:
 *
 * This is a private structure that holds the various parts of a parsed URI.
 */
struct _GstUri;
typedef struct _GstUri GstUri;

/**
 * GST_URI_NO_PORT:
 *
 * Value for #GstUri<!-- -->.port to indicate no port number.
 */
#define GST_URI_NO_PORT 0

/* used by GST_TYPE_URI */
GType gst_uri_get_type (void);

/*
 * Method definitions.
 */

GstUri * gst_uri_new                   (const gchar * scheme,
                                        const gchar * userinfo,
                                        const gchar * host,
                                        guint port,
                                        const gchar * path,
                                        const gchar * query,
                                        const gchar * fragment) G_GNUC_MALLOC;
GstUri * gst_uri_new_with_base         (GstUri * base,
                                        const gchar * scheme,
                                        const gchar * userinfo,
                                        const gchar * host,
                                        guint port,
                                        const gchar * path,
                                        const gchar * query,
                                        const gchar * fragment) G_GNUC_MALLOC;
GstUri * gst_uri_from_string           (const gchar * uri) G_GNUC_MALLOC;
GstUri * gst_uri_from_string_with_base (GstUri * base,
                                        const gchar * uri) G_GNUC_MALLOC;
gboolean gst_uri_equal                 (const GstUri * first,
                                        const GstUri * second);
GstUri * gst_uri_join                  (GstUri * base_uri,
                                        GstUri * ref_uri);
gchar *  gst_uri_join_strings          (const gchar * base_uri,
                                        const gchar * ref_uri) G_GNUC_MALLOC;
gboolean gst_uri_is_writable           (const GstUri * uri);
GstUri * gst_uri_make_writable         (GstUri * uri);
gchar *  gst_uri_to_string             (const GstUri * uri) G_GNUC_MALLOC;
gboolean gst_uri_is_normalized         (const GstUri * uri);
gboolean gst_uri_normalize             (GstUri * uri);
const gchar * gst_uri_get_scheme       (const GstUri * uri);
gboolean gst_uri_set_scheme            (GstUri * uri, const gchar * scheme);
const gchar * gst_uri_get_userinfo     (const GstUri * uri);
gboolean gst_uri_set_userinfo          (GstUri * uri, const gchar * userinfo);
const gchar * gst_uri_get_host         (const GstUri * uri);
gboolean gst_uri_set_host              (GstUri * uri, const gchar * host);
guint gst_uri_get_port                 (const GstUri * uri);
gboolean gst_uri_set_port              (GstUri * uri, guint port);
gchar * gst_uri_get_path               (const GstUri * uri);
gboolean gst_uri_set_path              (GstUri * uri, const gchar * path);
gchar * gst_uri_get_path_string        (const GstUri * uri);
gboolean gst_uri_set_path_string       (GstUri * uri, const gchar * path);
GList * gst_uri_get_path_segments      (const GstUri * uri);
gboolean gst_uri_set_path_segments     (GstUri * uri, GList * path_segments);
gboolean gst_uri_append_path           (GstUri * uri,
                                        const gchar * relative_path);
gboolean gst_uri_append_path_segment   (GstUri * uri,
                                        const gchar * path_segment);
gchar * gst_uri_get_query_string       (const GstUri * uri);
gboolean gst_uri_set_query_string      (GstUri * uri, const gchar * query);
GHashTable * gst_uri_get_query_table   (const GstUri * uri);
gboolean gst_uri_set_query_table       (GstUri * uri,
                                        GHashTable * query_table);
gboolean gst_uri_set_query_value       (GstUri * uri, const gchar * query_key,
                                        const gchar * query_value);
gboolean gst_uri_remove_query_key      (GstUri * uri, const gchar * query_key);
gboolean gst_uri_query_has_key         (const GstUri * uri,
                                        const gchar * query_key);
const gchar * gst_uri_get_query_value  (const GstUri * uri,
                                        const gchar * query_key);
GList * gst_uri_get_query_keys         (const GstUri * uri);
const gchar * gst_uri_get_fragment     (const GstUri * uri);
gboolean gst_uri_set_fragment          (GstUri * uri, const gchar * fragment);

/**
 * gst_uri_copy:
 * @uri: This #GstUri object.
 *
 * Create a new #GstUri object with the same data as this #GstUri object.
 * If @uri is %NULL then returns %NULL.
 *
 * Returns: (transfer full): A new #GstUri object which is a copy of this
 *          #GstUri or %NULL.
 */
static inline GstUri *
gst_uri_copy (const GstUri * uri)
{
  return GST_URI_CAST (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (uri)));
}

/**
 * gst_uri_ref:
 * @uri: (transfer none): This #GstUri object.
 *
 * Add a reference to this #GstUri object. See gst_mini_object_ref() for further
 * info.
 *
 * Returns: This object with the reference count incremented.
 */
static inline GstUri *
gst_uri_ref (GstUri * uri)
{
  return GST_URI_CAST (gst_mini_object_ref (GST_MINI_OBJECT_CAST (uri)));
}

/**
 * gst_uri_unref:
 * @uri: (transfer full): This #GstUri object.
 *
 * Decrement the reference count to this #GstUri object.
 *
 * If the reference count drops to 0 then finalize this object.
 *
 * See gst_mini_object_unref() for further info.
 */
static inline void
gst_uri_unref (GstUri * uri)
{
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (uri));
}

#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstUri, gst_uri_unref)
#endif

G_END_DECLS

#endif /* __GST_URI_H__ */
