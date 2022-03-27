#ifndef __G_UNIX_CREDENTIALS_MESSAGE_H__
#define __G_UNIX_CREDENTIALS_MESSAGE_H__

#include "../gobject/gobject.h"
#include "giotypes.h"
#include "gsocketcontrolmessage.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_CREDENTIALS_MESSAGE  (g_unix_credentials_message_get_type())
#define G_UNIX_CREDENTIALS_MESSAGE(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_CREDENTIALS_MESSAGE, GUnixCredentialsMessage))
#define G_UNIX_CREDENTIALS_MESSAGE_CLASS(c)  (G_TYPE_CHECK_CLASS_CAST((c), G_TYPE_UNIX_CREDENTIALS_MESSAGE, GUnixCredentialsMessageClass))
#define G_IS_UNIX_CREDENTIALS_MESSAGE(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_CREDENTIALS_MESSAGE))
#define G_IS_UNIX_CREDENTIALS_MESSAGE_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), G_TYPE_UNIX_CREDENTIALS_MESSAGE))
#define G_UNIX_CREDENTIALS_MESSAGE_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_UNIX_CREDENTIALS_MESSAGE, GUnixCredentialsMessageClass))
typedef struct _GUnixCredentialsMessagePrivate  GUnixCredentialsMessagePrivate;
typedef struct _GUnixCredentialsMessageClass    GUnixCredentialsMessageClass;
struct _GUnixCredentialsMessageClass {
  GSocketControlMessageClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
};
struct _GUnixCredentialsMessage {
  GSocketControlMessage parent_instance;
  GUnixCredentialsMessagePrivate *priv;
};
GType g_unix_credentials_message_get_type(void) G_GNUC_CONST;
GSocketControlMessage *g_unix_credentials_message_new(void);
GSocketControlMessage *g_unix_credentials_message_new_with_credentials(GCredentials *credentials);
GCredentials *g_unix_credentials_message_get_credentials(GUnixCredentialsMessage *message);
gboolean g_unix_credentials_message_is_supported(void);
G_END_DECLS

#endif