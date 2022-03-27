#include "../glib/glibintl.h"
#include "config.h"
#include "gsocketcontrolmessage.h"
#include "gnetworkingprivate.h"
#include "gunixcredentialsmessage.h"
#include "gunixfdmessage.h"

G_DEFINE_ABSTRACT_TYPE(GSocketControlMessage, g_socket_control_message, G_TYPE_OBJECT);
gsize g_socket_control_message_get_size(GSocketControlMessage *message) {
  g_return_val_if_fail(G_IS_SOCKET_CONTROL_MESSAGE(message), 0);
  return G_SOCKET_CONTROL_MESSAGE_GET_CLASS(message)->get_size(message);
}
int g_socket_control_message_get_level(GSocketControlMessage *message) {
  g_return_val_if_fail(G_IS_SOCKET_CONTROL_MESSAGE(message), 0);
  return G_SOCKET_CONTROL_MESSAGE_GET_CLASS(message)->get_level(message);
}
int g_socket_control_message_get_msg_type(GSocketControlMessage *message) {
  g_return_val_if_fail(G_IS_SOCKET_CONTROL_MESSAGE (message), 0);
  return G_SOCKET_CONTROL_MESSAGE_GET_CLASS(message)->get_type(message);
}
void g_socket_control_message_serialize(GSocketControlMessage *message, gpointer data) {
  g_return_if_fail(G_IS_SOCKET_CONTROL_MESSAGE(message));
  G_SOCKET_CONTROL_MESSAGE_GET_CLASS(message)->serialize(message, data);
}
static void g_socket_control_message_init(GSocketControlMessage *message) {}
static void g_socket_control_message_class_init(GSocketControlMessageClass *class) {}
GSocketControlMessage *g_socket_control_message_deserialize(int level, int type, gsize size, gpointer data) {
  GSocketControlMessage *message;
  GType *message_types;
  guint n_message_types;
  int i;
#ifdef G_OS_WIN32
  volatile GType a_type;
#endif
#ifdef G_OS_WIN32
  a_type = g_unix_credentials_message_get_type();
  a_type = g_unix_fd_message_get_type();
#endif
  message_types = g_type_children(G_TYPE_SOCKET_CONTROL_MESSAGE, &n_message_types);
  message = NULL;
  for (i = 0; i < n_message_types; i++) {
      GSocketControlMessageClass *class;
      class = g_type_class_ref(message_types[i]);
      message = class->deserialize(level, type, size, data);
      g_type_class_unref(class);
      if (message != NULL) break;
  }
  g_free(message_types);
  if (message == NULL) g_warning("unknown control message type %d:%d", level, type);
  return message;
}