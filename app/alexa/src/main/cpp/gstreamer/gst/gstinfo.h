#ifndef __GSTINFO_H__
#define __GSTINFO_H__

#include <glib/glib.h>
#include <glib/glib-object.h>
#include "gstconfig.h"

G_BEGIN_DECLS
typedef enum {
  GST_LEVEL_NONE = 0,
  GST_LEVEL_ERROR = 1,
  GST_LEVEL_WARNING = 2,
  GST_LEVEL_FIXME = 3,
  GST_LEVEL_INFO = 4,
  GST_LEVEL_DEBUG = 5,
  GST_LEVEL_LOG = 6,
  GST_LEVEL_TRACE = 7,
  GST_LEVEL_MEMDUMP = 9,
  GST_LEVEL_COUNT
} GstDebugLevel;
#ifndef GST_LEVEL_DEFAULT
#define GST_LEVEL_DEFAULT GST_LEVEL_NONE
#endif
#ifndef GST_LEVEL_MAX
#define GST_LEVEL_MAX GST_LEVEL_COUNT
#endif
typedef enum {
  GST_DEBUG_FG_BLACK		= 0x0000,
  GST_DEBUG_FG_RED		= 0x0001,
  GST_DEBUG_FG_GREEN		= 0x0002,
  GST_DEBUG_FG_YELLOW		= 0x0003,
  GST_DEBUG_FG_BLUE		= 0x0004,
  GST_DEBUG_FG_MAGENTA		= 0x0005,
  GST_DEBUG_FG_CYAN		= 0x0006,
  GST_DEBUG_FG_WHITE		= 0x0007,
  GST_DEBUG_BG_BLACK		= 0x0000,
  GST_DEBUG_BG_RED		= 0x0010,
  GST_DEBUG_BG_GREEN		= 0x0020,
  GST_DEBUG_BG_YELLOW		= 0x0030,
  GST_DEBUG_BG_BLUE		= 0x0040,
  GST_DEBUG_BG_MAGENTA		= 0x0050,
  GST_DEBUG_BG_CYAN		= 0x0060,
  GST_DEBUG_BG_WHITE		= 0x0070,
  GST_DEBUG_BOLD		= 0x0100,
  GST_DEBUG_UNDERLINE		= 0x0200
} GstDebugColorFlags;
typedef enum {
  GST_DEBUG_COLOR_MODE_OFF  = 0,
  GST_DEBUG_COLOR_MODE_ON   = 1,
  GST_DEBUG_COLOR_MODE_UNIX = 2
} GstDebugColorMode;
#define GST_DEBUG_FG_MASK	(0x000F)
#define GST_DEBUG_BG_MASK	(0x00F0)
#define GST_DEBUG_FORMAT_MASK	(0xFF00)
typedef struct _GstDebugCategory GstDebugCategory;
struct _GstDebugCategory {
  gint                  threshold;
  guint			color;
  const gchar *		name;
  const gchar *		description;
};
#define GST_STR_NULL(str) ((str) ? (str) : "(NULL)")
#define GST_DEBUG_PAD_NAME(pad) \
  (pad != NULL) ?  ((GST_OBJECT_PARENT(pad) != NULL) ? GST_STR_NULL (GST_OBJECT_NAME (GST_OBJECT_PARENT(pad))) : "''" ) : "''", \
  (pad != NULL) ? GST_STR_NULL (GST_OBJECT_NAME (pad)) : "''"
#ifndef GST_FUNCTION
#if defined (__STDC__) && defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define GST_FUNCTION     ((const char*) (__func__))
#elif defined (__GNUC__) || (defined (_MSC_VER) && _MSC_VER >= 1300)
#  define GST_FUNCTION     ((const char*) (__FUNCTION__))
#else
#  define GST_FUNCTION     ((const char*) ("???"))
#endif
#endif
#define GST_PTR_FORMAT     "p\aA"
#define GST_SEGMENT_FORMAT "p\aB"
typedef struct _GstDebugMessage GstDebugMessage;
typedef void (*GstLogFunction)(GstDebugCategory *category, GstDebugLevel level, const gchar *file, const gchar *function, gint line,
                               GObject *object, GstDebugMessage *message, gpointer user_data);
void gst_debug_log(GstDebugCategory *category, GstDebugLevel level, const gchar *file, const gchar *function, gint line, GObject *object,
                   const gchar *format, ...) G_GNUC_PRINTF (7, 8) G_GNUC_NO_INSTRUMENT;
void gst_debug_log_valist(GstDebugCategory *category, GstDebugLevel level, const gchar *file, const gchar *function, gint line,
                          GObject *object, const gchar *format, va_list args) G_GNUC_NO_INSTRUMENT;
GstDebugCategory *_gst_debug_category_new (const gchar *name, guint color, const gchar *description);
GstDebugCategory *_gst_debug_get_category (const gchar *name);
void _gst_debug_dump_mem(GstDebugCategory *cat, const gchar *file, const gchar *func, gint line, GObject *obj, const gchar *msg,
                         const guint8 *data, guint length);
typedef	void (* GstDebugFuncPtr)	(void);
void	_gst_debug_register_funcptr	(GstDebugFuncPtr	func, const gchar *		ptrname);
const gchar *	_gst_debug_nameof_funcptr	(GstDebugFuncPtr	func) G_GNUC_NO_INSTRUMENT;
const gchar   * gst_debug_message_get    (GstDebugMessage  * message);
void gst_debug_log_default(GstDebugCategory *category, GstDebugLevel level, const gchar *file, const gchar *function, gint line,
                           GObject *object, GstDebugMessage *message, gpointer user_data) G_GNUC_NO_INSTRUMENT;
const gchar *   gst_debug_level_get_name (GstDebugLevel level);
void            gst_debug_add_log_function            (GstLogFunction func, gpointer user_data, GDestroyNotify notify);
guint           gst_debug_remove_log_function         (GstLogFunction func);
guint           gst_debug_remove_log_function_by_data (gpointer       data);
void            gst_debug_set_active  (gboolean active);
gboolean        gst_debug_is_active   (void);
void            gst_debug_set_colored (gboolean colored);
void            gst_debug_set_color_mode   (GstDebugColorMode mode);
void            gst_debug_set_color_mode_from_string (const gchar * mode);
gboolean        gst_debug_is_colored  (void);
GstDebugColorMode gst_debug_get_color_mode (void);
void            gst_debug_set_default_threshold      (GstDebugLevel level);
GstDebugLevel   gst_debug_get_default_threshold      (void);
void            gst_debug_set_threshold_for_name     (const gchar * name, GstDebugLevel level);
void            gst_debug_set_threshold_from_string  (const gchar * list, gboolean reset);
void            gst_debug_unset_threshold_for_name   (const gchar * name);
void            gst_debug_category_free              (GstDebugCategory *	category);
void	            gst_debug_category_set_threshold     (GstDebugCategory *	category, GstDebugLevel		level);
void            gst_debug_category_reset_threshold   (GstDebugCategory *	category);
GstDebugLevel   gst_debug_category_get_threshold     (GstDebugCategory *	category);
const gchar *   gst_debug_category_get_name          (GstDebugCategory *	category);
guint           gst_debug_category_get_color         (GstDebugCategory *	category);
const gchar *   gst_debug_category_get_description   (GstDebugCategory *	category);
GSList *        gst_debug_get_all_categories	(void);
gchar * gst_debug_construct_term_color (guint colorinfo);
gint    gst_debug_construct_win_color  (guint colorinfo);
gint    gst_info_vasprintf              (gchar ** result, const gchar * format, va_list args) G_GNUC_PRINTF (2, 0);
gchar * gst_info_strdup_vprintf         (const gchar *format, va_list args) G_GNUC_PRINTF (1, 0);
gchar * gst_info_strdup_printf          (const gchar *format, ...) G_GNUC_PRINTF (1, 2);
#ifndef GST_DISABLE_GST_DEBUG
#define gst_debug_add_log_function(func,data,notify) \
G_STMT_START{                                        \
  if (func == gst_debug_log_default) gst_debug_add_log_function(NULL,data,notify);    \
  else gst_debug_add_log_function(func,data,notify);    \
}G_STMT_END
#define gst_debug_remove_log_function(func)   \
    (func == gst_debug_log_default) ? gst_debug_remove_log_function(NULL) : gst_debug_remove_log_function(func)
#define GST_DEBUG_CATEGORY(cat) GstDebugCategory *cat = NULL
#define GST_DEBUG_CATEGORY_EXTERN(cat) extern GstDebugCategory *cat
#define GST_DEBUG_CATEGORY_STATIC(cat) static GstDebugCategory *cat = NULL
#define GST_DEBUG_CATEGORY_INIT(cat,name,color,description) \
G_STMT_START{\
  if (cat == NULL) cat = _gst_debug_category_new (name,color,description);		\
}G_STMT_END
#ifdef GST_CAT_DEFAULT
#define GST_DEBUG_CATEGORY_GET(cat,name)  G_STMT_START{\
  cat = _gst_debug_get_category (name);			\
  if (!cat) cat = GST_CAT_DEFAULT;				\
}G_STMT_END
#else
#define GST_DEBUG_CATEGORY_GET(cat,name)  \
G_STMT_START{\
  cat = _gst_debug_get_category (name);			\
}G_STMT_END
#endif
GST_EXPORT GstDebugCategory *	GST_CAT_DEFAULT;
GST_EXPORT gboolean			_gst_debug_enabled;
GST_EXPORT GstDebugLevel            _gst_debug_min;
#ifdef G_HAVE_ISO_VARARGS
#define GST_CAT_LEVEL_LOG(cat,level,object,...) \
G_STMT_START{		\
  if (G_UNLIKELY (level <= GST_LEVEL_MAX && level <= _gst_debug_min)) {						\
    gst_debug_log ((cat), (level), __FILE__, GST_FUNCTION, __LINE__, (GObject *) (object), __VA_ARGS__);				\
  }									\
}G_STMT_END
#else
#ifdef G_HAVE_GNUC_VARARGS
#define GST_CAT_LEVEL_LOG(cat,level,object,args...) \
G_STMT_START{	\
  if (G_UNLIKELY (level <= GST_LEVEL_MAX && level <= _gst_debug_min)) {						\
    gst_debug_log ((cat), (level), __FILE__, GST_FUNCTION, __LINE__, (GObject *) (object), ##args );					\
  }									\
}G_STMT_END
#else
static inline void GST_CAT_LEVEL_LOG_valist (GstDebugCategory * cat, GstDebugLevel level, gpointer object, const char *format, va_list varargs) {
  if (G_UNLIKELY (level <= GST_LEVEL_MAX && level <= _gst_debug_min)) {
    gst_debug_log_valist (cat, level, "", "", 0, (GObject *) object, format, varargs);
  }
}
static inline void GST_CAT_LEVEL_LOG (GstDebugCategory * cat, GstDebugLevel level, gpointer object, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, level, object, format, varargs);
  va_end (varargs);
}
#endif
#endif
#define __GST_CAT_MEMDUMP_LOG(cat,object,msg,data,length) \
G_STMT_START{       \
    if (G_UNLIKELY (GST_LEVEL_MEMDUMP <= GST_LEVEL_MAX && GST_LEVEL_MEMDUMP <= _gst_debug_min)) {		      \
    _gst_debug_dump_mem ((cat), __FILE__, GST_FUNCTION, __LINE__, (GObject *) (object), (msg), (data), (length));                       \
  }                                                                           \
}G_STMT_END
#define GST_CAT_MEMDUMP_OBJECT(cat,obj,msg,data,length)  __GST_CAT_MEMDUMP_LOG(cat,obj,msg,data,length)
#define GST_CAT_MEMDUMP(cat,msg,data,length)  __GST_CAT_MEMDUMP_LOG(cat,NULL,msg,data,length)
#define GST_MEMDUMP_OBJECT(obj,msg,data,length)  __GST_CAT_MEMDUMP_LOG(GST_CAT_DEFAULT,obj,msg,data,length)
#define GST_MEMDUMP(msg,data,length)  __GST_CAT_MEMDUMP_LOG(GST_CAT_DEFAULT,NULL,msg,data,length)
#ifdef G_HAVE_ISO_VARARGS
#define GST_CAT_ERROR_OBJECT(cat,obj,...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_ERROR,   obj,  __VA_ARGS__)
#define GST_CAT_WARNING_OBJECT(cat,obj,...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_WARNING, obj,  __VA_ARGS__)
#define GST_CAT_INFO_OBJECT(cat,obj,...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_INFO,    obj,  __VA_ARGS__)
#define GST_CAT_DEBUG_OBJECT(cat,obj,...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_DEBUG,   obj,  __VA_ARGS__)
#define GST_CAT_LOG_OBJECT(cat,obj,...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_LOG,     obj,  __VA_ARGS__)
#define GST_CAT_FIXME_OBJECT(cat,obj,...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_FIXME,   obj,  __VA_ARGS__)
#define GST_CAT_TRACE_OBJECT(cat,obj,...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_TRACE,   obj,  __VA_ARGS__)
#define GST_CAT_ERROR(cat,...)			GST_CAT_LEVEL_LOG (cat, GST_LEVEL_ERROR,   NULL, __VA_ARGS__)
#define GST_CAT_WARNING(cat,...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_WARNING, NULL, __VA_ARGS__)
#define GST_CAT_INFO(cat,...)			GST_CAT_LEVEL_LOG (cat, GST_LEVEL_INFO,    NULL, __VA_ARGS__)
#define GST_CAT_DEBUG(cat,...)			GST_CAT_LEVEL_LOG (cat, GST_LEVEL_DEBUG,   NULL, __VA_ARGS__)
#define GST_CAT_LOG(cat,...)			GST_CAT_LEVEL_LOG (cat, GST_LEVEL_LOG,     NULL, __VA_ARGS__)
#define GST_CAT_FIXME(cat,...)			GST_CAT_LEVEL_LOG (cat, GST_LEVEL_FIXME,   NULL, __VA_ARGS__)
#define GST_CAT_TRACE(cat,...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_TRACE,   NULL, __VA_ARGS__)
#define GST_ERROR_OBJECT(obj,...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_ERROR,   obj,  __VA_ARGS__)
#define GST_WARNING_OBJECT(obj,...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_WARNING, obj,  __VA_ARGS__)
#define GST_INFO_OBJECT(obj,...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_INFO,    obj,  __VA_ARGS__)
#define GST_DEBUG_OBJECT(obj,...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_DEBUG,   obj,  __VA_ARGS__)
#define GST_LOG_OBJECT(obj,...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_LOG,     obj,  __VA_ARGS__)
#define GST_FIXME_OBJECT(obj,...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_FIXME,   obj,  __VA_ARGS__)
#define GST_TRACE_OBJECT(obj,...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_TRACE,   obj,  __VA_ARGS__)
#define GST_ERROR(...)			GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_ERROR,   NULL, __VA_ARGS__)
#define GST_WARNING(...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_WARNING, NULL, __VA_ARGS__)
#define GST_INFO(...)			GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_INFO,    NULL, __VA_ARGS__)
#define GST_DEBUG(...)			GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_DEBUG,   NULL, __VA_ARGS__)
#define GST_LOG(...)			GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_LOG,     NULL, __VA_ARGS__)
#define GST_FIXME(...)			GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_FIXME,   NULL, __VA_ARGS__)
#define GST_TRACE(...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_TRACE,   NULL, __VA_ARGS__)
#else
#ifdef G_HAVE_GNUC_VARARGS
#define GST_CAT_ERROR_OBJECT(cat,obj,args...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_ERROR,   obj,  ##args )
#define GST_CAT_WARNING_OBJECT(cat,obj,args...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_WARNING, obj,  ##args )
#define GST_CAT_INFO_OBJECT(cat,obj,args...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_INFO,    obj,  ##args )
#define GST_CAT_DEBUG_OBJECT(cat,obj,args...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_DEBUG,   obj,  ##args )
#define GST_CAT_LOG_OBJECT(cat,obj,args...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_LOG,     obj,  ##args )
#define GST_CAT_FIXME_OBJECT(cat,obj,args...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_FIXME,   obj,  ##args )
#define GST_CAT_TRACE_OBJECT(cat,obj,args...)	GST_CAT_LEVEL_LOG (cat, GST_LEVEL_TRACE,   obj,  ##args )
#define GST_CAT_ERROR(cat,args...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_ERROR,   NULL, ##args )
#define GST_CAT_WARNING(cat,args...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_WARNING, NULL, ##args )
#define GST_CAT_INFO(cat,args...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_INFO,    NULL, ##args )
#define GST_CAT_DEBUG(cat,args...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_DEBUG,   NULL, ##args )
#define GST_CAT_LOG(cat,args...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_LOG,     NULL, ##args )
#define GST_CAT_FIXME(cat,args...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_FIXME,   NULL, ##args )
#define GST_CAT_TRACE(cat,args...)		GST_CAT_LEVEL_LOG (cat, GST_LEVEL_TRACE,   NULL, ##args )
#define GST_ERROR_OBJECT(obj,args...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_ERROR,   obj,  ##args )
#define GST_WARNING_OBJECT(obj,args...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_WARNING, obj,  ##args )
#define GST_INFO_OBJECT(obj,args...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_INFO,    obj,  ##args )
#define GST_DEBUG_OBJECT(obj,args...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_DEBUG,   obj,  ##args )
#define GST_LOG_OBJECT(obj,args...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_LOG,     obj,  ##args )
#define GST_FIXME_OBJECT(obj,args...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_FIXME,   obj,  ##args )
#define GST_TRACE_OBJECT(obj,args...)	GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_TRACE,   obj,  ##args )
#define GST_ERROR(args...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_ERROR,   NULL, ##args )
#define GST_WARNING(args...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_WARNING, NULL, ##args )
#define GST_INFO(args...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_INFO,    NULL, ##args )
#define GST_DEBUG(args...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_DEBUG,   NULL, ##args )
#define GST_LOG(args...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_LOG,     NULL, ##args )
#define GST_FIXME(args...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_FIXME,   NULL, ##args )
#define GST_TRACE(args...)		GST_CAT_LEVEL_LOG (GST_CAT_DEFAULT, GST_LEVEL_TRACE,   NULL, ##args )
#else
static inline void GST_CAT_ERROR_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_ERROR, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_WARNING_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_WARNING, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_INFO_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_INFO, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_DEBUG_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_DEBUG, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_LOG_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_LOG, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_FIXME_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_FIXME, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_TRACE_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_TRACE, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_ERROR (GstDebugCategory * cat, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_ERROR, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_WARNING (GstDebugCategory * cat, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_WARNING, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_INFO (GstDebugCategory * cat, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_INFO, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_DEBUG (GstDebugCategory * cat, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_DEBUG, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_LOG (GstDebugCategory * cat, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_LOG, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_FIXME (GstDebugCategory * cat, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_FIXME, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_CAT_TRACE (GstDebugCategory * cat, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (cat, GST_LEVEL_TRACE, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_ERROR_OBJECT (gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_ERROR, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_WARNING_OBJECT (gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_WARNING, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_INFO_OBJECT (gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_INFO, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_DEBUG_OBJECT (gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_DEBUG, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_LOG_OBJECT (gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_LOG, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_FIXME_OBJECT (gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_FIXME, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_TRACE_OBJECT (gpointer obj, const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_TRACE, obj, format, varargs);
  va_end (varargs);
}
static inline void GST_ERROR (const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_ERROR, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_WARNING (const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_WARNING, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_INFO (const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_INFO, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_DEBUG (const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_DEBUG, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_LOG (const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_LOG, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_FIXME (const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_FIXME, NULL, format, varargs);
  va_end (varargs);
}
static inline void GST_TRACE (const char *format, ...) {
  va_list varargs;
  va_start (varargs, format);
  GST_CAT_LEVEL_LOG_valist (GST_CAT_DEFAULT, GST_LEVEL_TRACE, NULL, format, varargs);
  va_end (varargs);
}
#endif
#endif
#define GST_DEBUG_REGISTER_FUNCPTR(ptr)  _gst_debug_register_funcptr((GstDebugFuncPtr)(ptr), #ptr)
#define GST_DEBUG_FUNCPTR(ptr)  (_gst_debug_register_funcptr((GstDebugFuncPtr)(ptr), #ptr) , ptr)
#define GST_DEBUG_FUNCPTR_NAME(ptr)  _gst_debug_nameof_funcptr((GstDebugFuncPtr)ptr)
#else
#ifndef GST_INFO_C
#if defined(__GNUC__) && __GNUC__ >= 3
#  pragma GCC poison gst_debug_log
#  pragma GCC poison gst_debug_log_valist
#  pragma GCC poison _gst_debug_category_new
#endif
#define _gst_debug_min GST_LEVEL_NONE
#define gst_debug_set_default_threshold(level)		G_STMT_START{ }G_STMT_END
#define gst_debug_get_default_threshold()		(GST_LEVEL_NONE)
#define gst_debug_level_get_name(level)				("NONE")
#define gst_debug_message_get(message)  			("")
#define gst_debug_add_log_function(func,data,notify)    G_STMT_START{ }G_STMT_END
#define gst_debug_set_active(active)			G_STMT_START{ }G_STMT_END
#define gst_debug_is_active()				(FALSE)
#define gst_debug_set_colored(colored)			G_STMT_START{ }G_STMT_END
#define gst_debug_set_color_mode(mode)			G_STMT_START{ }G_STMT_END
#define gst_debug_set_color_mode_from_string(mode)	G_STMT_START{ }G_STMT_END
#define gst_debug_is_colored()				(FALSE)
#define gst_debug_get_color_mode()			(GST_DEBUG_COLOR_MODE_OFF)
#define gst_debug_set_default_threshold(level)		G_STMT_START{ }G_STMT_END
#define gst_debug_get_default_threshold()		(GST_LEVEL_NONE)
#define gst_debug_set_threshold_for_name(name,level)	G_STMT_START{ }G_STMT_END
#define gst_debug_unset_threshold_for_name(name)	G_STMT_START{ }G_STMT_END
#define GST_DEBUG_CATEGORY(var)				void _gst_debug_dummy_##var (void)
#define GST_DEBUG_CATEGORY_EXTERN(var)			void _gst_debug_dummy_extern_##var (void)
#define GST_DEBUG_CATEGORY_STATIC(var)			void _gst_debug_dummy_static_##var (void)
#define GST_DEBUG_CATEGORY_INIT(var,name,color,desc)	G_STMT_START{ }G_STMT_END
#define GST_DEBUG_CATEGORY_GET(var,name)		G_STMT_START{ }G_STMT_END
#define gst_debug_category_free(category)		G_STMT_START{ }G_STMT_END
#define gst_debug_category_set_threshold(category,level) G_STMT_START{ }G_STMT_END
#define gst_debug_category_reset_threshold(category)	G_STMT_START{ }G_STMT_END
#define gst_debug_category_get_threshold(category)	(GST_LEVEL_NONE)
#define gst_debug_category_get_name(cat)		("")
#define gst_debug_category_get_color(cat)		(0)
#define gst_debug_category_get_description(cat)		("")
#define gst_debug_get_all_categories()			(NULL)
#define gst_debug_construct_term_color(colorinfo)	(g_strdup ("00"))
#define gst_debug_construct_win_color(colorinfo)	(0)
#endif
#ifdef G_HAVE_ISO_VARARGS
#define GST_CAT_LEVEL_LOG(cat,level,...)		G_STMT_START{ }G_STMT_END
#define GST_CAT_ERROR_OBJECT(...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_WARNING_OBJECT(...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_INFO_OBJECT(...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_DEBUG_OBJECT(...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_LOG_OBJECT(...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_FIXME_OBJECT(...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_TRACE_OBJECT(...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_ERROR(...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_WARNING(...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_INFO(...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_DEBUG(...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_LOG(...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_FIXME(...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_TRACE(...)				G_STMT_START{ }G_STMT_END
#define GST_ERROR_OBJECT(...)				G_STMT_START{ }G_STMT_END
#define GST_WARNING_OBJECT(...)				G_STMT_START{ }G_STMT_END
#define GST_INFO_OBJECT(...)				G_STMT_START{ }G_STMT_END
#define GST_DEBUG_OBJECT(...)				G_STMT_START{ }G_STMT_END
#define GST_LOG_OBJECT(...)				G_STMT_START{ }G_STMT_END
#define GST_FIXME_OBJECT(...)				G_STMT_START{ }G_STMT_END
#define GST_TRACE_OBJECT(...)				G_STMT_START{ }G_STMT_END
#define GST_ERROR(...)					G_STMT_START{ }G_STMT_END
#define GST_WARNING(...)				G_STMT_START{ }G_STMT_END
#define GST_INFO(...)					G_STMT_START{ }G_STMT_END
#define GST_DEBUG(...)					G_STMT_START{ }G_STMT_END
#define GST_LOG(...)					G_STMT_START{ }G_STMT_END
#define GST_FIXME(...)					G_STMT_START{ }G_STMT_END
#define GST_TRACE(...)					G_STMT_START{ }G_STMT_END
#else
#ifdef G_HAVE_GNUC_VARARGS
#define GST_CAT_LEVEL_LOG(cat,level,args...)		G_STMT_START{ }G_STMT_END
#define GST_CAT_ERROR_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_WARNING_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_INFO_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_DEBUG_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_LOG_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_FIXME_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_TRACE_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_ERROR(args...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_WARNING(args...)			G_STMT_START{ }G_STMT_END
#define GST_CAT_INFO(args...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_DEBUG(args...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_LOG(args...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_FIXME(args...)				G_STMT_START{ }G_STMT_END
#define GST_CAT_TRACE(args...)				G_STMT_START{ }G_STMT_END
#define GST_ERROR_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_WARNING_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_INFO_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_DEBUG_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_LOG_OBJECT(args...)				G_STMT_START{ }G_STMT_END
#define GST_FIXME_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_TRACE_OBJECT(args...)			G_STMT_START{ }G_STMT_END
#define GST_ERROR(args...)				G_STMT_START{ }G_STMT_END
#define GST_WARNING(args...)				G_STMT_START{ }G_STMT_END
#define GST_INFO(args...)				G_STMT_START{ }G_STMT_END
#define GST_DEBUG(args...)				G_STMT_START{ }G_STMT_END
#define GST_LOG(args...)				G_STMT_START{ }G_STMT_END
#define GST_FIXME(args...)				G_STMT_START{ }G_STMT_END
#define GST_TRACE(args...)				G_STMT_START{ }G_STMT_END
#else
static inline void
GST_CAT_LEVEL_LOG_valist (GstDebugCategory * cat, GstDebugLevel level, gpointer object, const char *format, va_list varargs) {}
static inline void GST_CAT_ERROR_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {}
static inline void GST_CAT_WARNING_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {}
static inline void GST_CAT_INFO_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {}
static inline void GST_CAT_DEBUG_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {}
static inline void GST_CAT_LOG_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {}
static inline void GST_CAT_FIXME_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {}
static inline void GST_CAT_TRACE_OBJECT (GstDebugCategory * cat, gpointer obj, const char *format, ...) {}
static inline void GST_CAT_ERROR (GstDebugCategory * cat, const char *format, ...) {}
static inline void GST_CAT_WARNING (GstDebugCategory * cat, const char *format, ...) {}
static inline void GST_CAT_INFO (GstDebugCategory * cat, const char *format, ...) {}
static inline void GST_CAT_DEBUG (GstDebugCategory * cat, const char *format, ...) {}
static inline void GST_CAT_LOG (GstDebugCategory * cat, const char *format, ...) {}
static inline void GST_CAT_FIXME (GstDebugCategory * cat, const char *format, ...) {}
static inline void GST_CAT_TRACE (GstDebugCategory * cat, const char *format, ...) {}
static inline void GST_ERROR_OBJECT (gpointer obj, const char *format, ...) {}
static inline void GST_WARNING_OBJECT (gpointer obj, const char *format, ...) {}
static inline void GST_INFO_OBJECT (gpointer obj, const char *format, ...) {}
static inline void GST_DEBUG_OBJECT (gpointer obj, const char *format, ...) {}
static inline void GST_LOG_OBJECT (gpointer obj, const char *format, ...) {}
static inline void GST_FIXME_OBJECT (gpointer obj, const char *format, ...) {}
static inline void GST_TRACE_OBJECT (gpointer obj, const char *format, ...) {}
static inline void GST_ERROR (const char *format, ...) {}
static inline void GST_WARNING (const char *format, ...) {}
static inline void GST_INFO (const char *format, ...) {}
static inline void GST_DEBUG (const char *format, ...) {}
static inline void GST_LOG (const char *format, ...) {}
static inline void GST_FIXME (const char *format, ...) {}
static inline void GST_TRACE (const char *format, ...) {}
#endif
#endif
#define GST_DEBUG_REGISTER_FUNCPTR(ptr) G_STMT_START{ }G_STMT_END
#define GST_DEBUG_FUNCPTR(ptr) (ptr)
#define GST_DEBUG_FUNCPTR_NAME(ptr) (g_strdup_printf ("%p", ptr))
#define GST_CAT_MEMDUMP_OBJECT(cat,obj,msg,data,length) G_STMT_START{ }G_STMT_END
#define GST_CAT_MEMDUMP(cat,msg,data,length)            G_STMT_START{ }G_STMT_END
#define GST_MEMDUMP_OBJECT(obj,msg,data,length)         G_STMT_START{ }G_STMT_END
#define GST_MEMDUMP(msg,data,length)                    G_STMT_START{ }G_STMT_END
#endif
void gst_debug_print_stack_trace (void);
G_END_DECLS

#endif