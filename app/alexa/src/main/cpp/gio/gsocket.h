#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SOCKET_H__
#define __G_SOCKET_H__

#include "../gobject/gobject.h"
#include "../glib/gmain.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET  (g_socket_get_type())
#define G_SOCKET(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SOCKET, GSocket))
#define G_SOCKET_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SOCKET, GSocketClass))
#define G_IS_SOCKET(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SOCKET))
#define G_IS_SOCKET_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SOCKET))
#define G_SOCKET_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SOCKET, GSocketClass))
#define FD_SETSIZE 64
#define SD_RECEIVE 0
#define SD_SEND 1
#define SD_BOTH 2
#define FD_READ 1
#define FD_WRITE 2
#define FD_OOB 4
#define FD_ACCEPT 8
#define FD_CONNECT 16
#define FD_CLOSE 32
typedef struct _GSocketPrivate GSocketPrivate;
typedef struct _GSocketClass GSocketClass;
struct _GSocketClass {
  GObjectClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
  void (*_g_reserved6)(void);
  void (*_g_reserved7)(void);
  void (*_g_reserved8)(void);
  void (*_g_reserved9)(void);
  void (*_g_reserved10)(void);
};
struct _GSocket {
  GObject parent_instance;
  GSocketPrivate *priv;
};
GType g_socket_get_type(void) G_GNUC_CONST;
GSocket *g_socket_new(GSocketFamily family, GSocketType type, GSocketProtocol protocol, GError **error);
GSocket *g_socket_new_from_fd(gint fd, GError **error);
int g_socket_get_fd(GSocket *socket);
GSocketFamily g_socket_get_family(GSocket *socket);
GSocketType g_socket_get_socket_type(GSocket *socket);
GSocketProtocol g_socket_get_protocol(GSocket *socket);
GSocketAddress *g_socket_get_local_address(GSocket *socket, GError **error);
GSocketAddress *g_socket_get_remote_address(GSocket *socket, GError **error);
void g_socket_set_blocking(GSocket *socket, gboolean blocking);
gboolean g_socket_get_blocking(GSocket *socket);
void g_socket_set_keepalive(GSocket *socket, gboolean keepalive);
gboolean g_socket_get_keepalive(GSocket *socket);
gint g_socket_get_listen_backlog(GSocket *socket);
void g_socket_set_listen_backlog(GSocket *socket, gint backlog);
guint g_socket_get_timeout(GSocket *socket);
void g_socket_set_timeout(GSocket *socket, guint timeout);
gboolean g_socket_is_connected(GSocket *socket);
gboolean g_socket_bind(GSocket *socket, GSocketAddress *address, gboolean allow_reuse, GError **error);
gboolean g_socket_connect(GSocket *socket, GSocketAddress *address, GCancellable *cancellable, GError **error);
gboolean g_socket_check_connect_result(GSocket *socket, GError **error);
GIOCondition g_socket_condition_check(GSocket *socket, GIOCondition condition);
gboolean g_socket_condition_wait(GSocket *socket, GIOCondition condition, GCancellable *cancellable, GError **error);
GSocket *g_socket_accept(GSocket *socket, GCancellable *cancellable, GError **error);
gboolean g_socket_listen(GSocket *socket, GError **error);
gssize g_socket_receive(GSocket *socket, gchar *buffer, gsize size, GCancellable *cancellable, GError **error);
gssize g_socket_receive_from(GSocket *socket, GSocketAddress **address, gchar *buffer, gsize size, GCancellable *cancellable, GError **error);
gssize g_socket_send(GSocket *socket, const gchar *buffer, gsize size, GCancellable *cancellable, GError **error);
gssize g_socket_send_to(GSocket *socket, GSocketAddress *address, const gchar *buffer, gsize size, GCancellable *cancellable, GError **error);
gssize g_socket_receive_message(GSocket *socket, GSocketAddress **address, GInputVector *vectors, gint num_vectors, GSocketControlMessage ***messages,
							    gint *num_messages, gint *flags, GCancellable *cancellable, GError **error);
gssize g_socket_send_message(GSocket *socket, GSocketAddress *address, GOutputVector *vectors, gint num_vectors, GSocketControlMessage **messages, gint num_messages,
							 gint flags, GCancellable *cancellable, GError **error);
gboolean g_socket_close(GSocket *socket, GError **error);
gboolean g_socket_shutdown(GSocket *socket, gboolean shutdown_read, gboolean shutdown_write, GError **error);
gboolean g_socket_is_closed(GSocket *socket);
GSource *g_socket_create_source(GSocket *socket, GIOCondition condition, GCancellable *cancellable);
gboolean g_socket_speaks_ipv4(GSocket *socket);
GCredentials *g_socket_get_credentials(GSocket *socket, GError **error);
gssize g_socket_receive_with_blocking(GSocket *socket, gchar *buffer, gsize size, gboolean blocking, GCancellable *cancellable, GError **error);
gssize g_socket_send_with_blocking(GSocket *socket, const gchar *buffer, gsize size, gboolean blocking, GCancellable *cancellable, GError **error);
G_END_DECLS

#endif