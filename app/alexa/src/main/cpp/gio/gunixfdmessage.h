#ifndef __G_UNIX_FD_MESSAGE_H__
#define __G_UNIX_FD_MESSAGE_H__

#include "gsocketcontrolmessage.h"
#include "gunixfdlist.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_FD_MESSAGE  (g_unix_fd_message_get_type())
#define G_UNIX_FD_MESSAGE(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst),  G_TYPE_UNIX_FD_MESSAGE, GUnixFDMessage))
#define G_UNIX_FD_MESSAGE_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_UNIX_FD_MESSAGE, GUnixFDMessageClass))
#define G_IS_UNIX_FD_MESSAGE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_UNIX_FD_MESSAGE))
#define G_IS_UNIX_FD_MESSAGE_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_UNIX_FD_MESSAGE))
#define G_UNIX_FD_MESSAGE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_UNIX_FD_MESSAGE, GUnixFDMessageClass))
typedef struct _GUnixFDMessagePrivate GUnixFDMessagePrivate;
typedef struct _GUnixFDMessageClass GUnixFDMessageClass;
typedef struct _GUnixFDMessage GUnixFDMessage;
struct _GUnixFDMessageClass {
  GSocketControlMessageClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
};
struct _GUnixFDMessage {
  GSocketControlMessage parent_instance;
  GUnixFDMessagePrivate *priv;
};
GType g_unix_fd_message_get_type(void) G_GNUC_CONST;
GSocketControlMessage *g_unix_fd_message_new_with_fd_list(GUnixFDList *fd_list);
GSocketControlMessage *g_unix_fd_message_new(void);
GUnixFDList *g_unix_fd_message_get_fd_list(GUnixFDMessage *message);
gint *g_unix_fd_message_steal_fds(GUnixFDMessage *message, gint *length);
gboolean g_unix_fd_message_append_fd(GUnixFDMessage *message, gint fd, GError **error);
G_END_DECLS

#endif