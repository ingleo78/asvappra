#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_IOCHANNEL_H__
#define __G_IOCHANNEL_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gconvert.h"
#include "gmain.h"
#include "gstring.h"
#include "gunicode.h"
#include "gpoll.h"

G_BEGIN_DECLS
typedef struct _GIOChannel	GIOChannel;
typedef struct _GIOFuncs GIOFuncs;
#define G_IO_CHANNEL_ERROR g_io_channel_error_quark()
typedef enum {
  G_SEEK_CUR,
  G_SEEK_SET,
  G_SEEK_END
} GSeekType;
struct _GIOChannel {
  gint ref_count;
  GIOFuncs *funcs;
  gchar *encoding;
  GIConv read_cd;
  GIConv write_cd;
  gchar *line_term;
  guint line_term_len;
  gsize buf_size;
  GString *read_buf;
  GString *encoded_read_buf;
  GString *write_buf;
  gchar partial_write_buf[6];
  guint use_buffer     : 1;
  guint do_encode      : 1;
  guint close_on_unref : 1;
  guint is_readable    : 1;
  guint is_writeable   : 1;
  guint is_seekable    : 1;
  gpointer reserved1;	
  gpointer reserved2;	
};
typedef int (*GIOFunc)(GIOChannel *source, GIOCondition  condition, gpointer data);
struct _GIOFuncs {
  GIOStatus (*io_read)(GIOChannel *channel, gchar *buf, gsize count, gsize *bytes_read, GError **err);
  GIOStatus (*io_write)(GIOChannel *channel, const gchar *buf, gsize count, gsize *bytes_written, GError **err);
  GIOStatus (*io_seek)(GIOChannel *channel, gint64 offset, GSeekType type, GError **err);
  GIOStatus (*io_close)(GIOChannel *channel, GError **err);
  GSource* (*io_create_watch)(GIOChannel *channel, GIOCondition condition);
  void (*io_free)(GIOChannel *channel);
  GIOStatus (*io_set_flags)(GIOChannel *channel, GIOFlags flags, GError **err);
  GIOFlags (*io_get_flags)(GIOChannel *channel);
};
void g_io_channel_init(GIOChannel *channel);
GIOChannel *g_io_channel_ref(GIOChannel *channel);
void g_io_channel_unref(GIOChannel *channel);
#ifndef G_DISABLE_DEPRECATED
GIOError  g_io_channel_read(GIOChannel *channel, gchar *buf, gsize count, gsize *bytes_read);
GIOError g_io_channel_write(GIOChannel *channel, const gchar *buf, gsize count, gsize *bytes_written);
GIOError g_io_channel_seek(GIOChannel *channel, gint64 offset, GSeekType type);
void g_io_channel_close(GIOChannel *channel);
#endif
GIOStatus g_io_channel_shutdown(GIOChannel *channel, int flush, GError **err);
guint g_io_add_watch_full(GIOChannel *channel, gint priority, GIOCondition condition, GIOFunc func, gpointer user_data, GDestroyNotify notify);
GSource* g_io_create_watch(GIOChannel *channel, GIOCondition condition);
guint g_io_add_watch(GIOChannel *channel, GIOCondition condition, GIOFunc func, gpointer user_data);
void g_io_channel_set_buffer_size(GIOChannel *channel, gsize size);
gsize g_io_channel_get_buffer_size(GIOChannel *channel);
GIOCondition g_io_channel_get_buffer_condition(GIOChannel *channel);
GIOStatus g_io_channel_set_flags(GIOChannel *channel, GIOFlags flags, GError **error);
GIOFlags g_io_channel_get_flags(GIOChannel *channel);
void g_io_channel_set_line_term(GIOChannel *channel, const gchar *line_term, gint length);
G_CONST_RETURN gchar* g_io_channel_get_line_term(GIOChannel *channel, gint *length);
void g_io_channel_set_buffered(GIOChannel *channel, int buffered);
int g_io_channel_get_buffered(GIOChannel *channel);
GIOStatus g_io_channel_set_encoding(GIOChannel *channel, const gchar *encoding, GError **error);
G_CONST_RETURN gchar* g_io_channel_get_encoding(GIOChannel *channel);
void g_io_channel_set_close_on_unref(GIOChannel *channel, int do_close);
int g_io_channel_get_close_on_unref(GIOChannel *channel);
GIOStatus g_io_channel_flush(GIOChannel *channel, GError **error);
GIOStatus g_io_channel_read_line(GIOChannel *channel, gchar **str_return, gsize *length, gsize *terminator_pos, GError **error);
GIOStatus g_io_channel_read_line_string(GIOChannel *channel, GString *buffer, gsize *terminator_pos, GError **error);
GIOStatus g_io_channel_read_to_end(GIOChannel *channel, gchar **str_return, gsize *length, GError **error);
GIOStatus g_io_channel_read_chars(GIOChannel *channel, gchar *buf, gsize count, gsize *bytes_read, GError **error);
GIOStatus g_io_channel_read_unichar(GIOChannel *channel, gunichar *thechar, GError **error);
GIOStatus g_io_channel_write_chars(GIOChannel *channel, const gchar *buf, gssize count, gsize *bytes_written, GError **error);
GIOStatus g_io_channel_write_unichar(GIOChannel *channel, gunichar thechar, GError **error);
GIOStatus g_io_channel_seek_position(GIOChannel *channel, gint64 offset, GSeekType type, GError **error);
#ifdef G_OS_WIN32
#define g_io_channel_new_file g_io_channel_new_file_utf8
#endif
GIOChannel* g_io_channel_new_file(const gchar *filename, const gchar *mode, GError **error);
GQuark g_io_channel_error_quark(void);
GIOChannelError g_io_channel_error_from_errno(gint en);
GIOChannel* g_io_channel_unix_new(int fd);
gint g_io_channel_unix_get_fd(GIOChannel *channel);
GLIB_VAR GSourceFuncs g_io_watch_funcs;
#ifdef G_OS_WIN32
#define G_WIN32_MSG_HANDLE 19981206
void g_io_channel_win32_make_pollfd (GIOChannel *channel, GIOCondition condition, GPollFD *fd);
gint g_io_channel_win32_poll(GPollFD *fds, gint n_fds, gint timeout_);
#if GLIB_SIZEOF_VOID_P == 8
GIOChannel *g_io_channel_win32_new_messages(gsize hwnd);
#else
GIOChannel *g_io_channel_win32_new_messages(guint hwnd);
#endif
GIOChannel* g_io_channel_win32_new_fd(gint fd);
gint g_io_channel_win32_get_fd(GIOChannel *channel);
GIOChannel *g_io_channel_win32_new_socket(gint socket);
#endif
G_END_DECLS

#endif