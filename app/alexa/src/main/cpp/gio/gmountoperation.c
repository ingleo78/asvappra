#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gmountoperation.h"
#include "gioenumtypes.h"

G_DEFINE_TYPE(GMountOperation, g_mount_operation, G_TYPE_OBJECT);
enum {
  ASK_PASSWORD,
  ASK_QUESTION,
  REPLY,
  ABORTED,
  SHOW_PROCESSES,
  LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };
struct _GMountOperationPrivate {
  char *password;
  char *user;
  char *domain;
  gboolean anonymous;
  GPasswordSave password_save;
  int choice;
};
enum {
  PROP_0,
  PROP_USERNAME,
  PROP_PASSWORD,
  PROP_ANONYMOUS,
  PROP_DOMAIN,
  PROP_PASSWORD_SAVE,
  PROP_CHOICE
};
static void g_mount_operation_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GMountOperation *operation;
  operation = G_MOUNT_OPERATION(object);
  switch(prop_id) {
      case PROP_USERNAME: g_mount_operation_set_username(operation, g_value_get_string(value)); break;
      case PROP_PASSWORD: g_mount_operation_set_password(operation, g_value_get_string(value)); break;
      case PROP_ANONYMOUS: g_mount_operation_set_anonymous(operation, g_value_get_boolean(value)); break;
      case PROP_DOMAIN: g_mount_operation_set_domain(operation, g_value_get_string(value)); break;
      case PROP_PASSWORD_SAVE: g_mount_operation_set_password_save(operation, g_value_get_enum(value)); break;
      case PROP_CHOICE: g_mount_operation_set_choice(operation, g_value_get_int(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_mount_operation_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GMountOperation *operation;
  GMountOperationPrivate *priv;
  operation = G_MOUNT_OPERATION(object);
  priv = operation->priv;
  switch(prop_id) {
      case PROP_USERNAME: g_value_set_string(value, priv->user); break;
      case PROP_PASSWORD: g_value_set_string(value, priv->password); break;
      case PROP_ANONYMOUS: g_value_set_boolean(value, priv->anonymous); break;
      case PROP_DOMAIN: g_value_set_string(value, priv->domain); break;
      case PROP_PASSWORD_SAVE: g_value_set_enum(value, priv->password_save); break;
      case PROP_CHOICE: g_value_set_int(value, priv->choice); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_mount_operation_finalize(GObject *object) {
  GMountOperation *operation;
  GMountOperationPrivate *priv;
  operation = G_MOUNT_OPERATION(object);
  priv = operation->priv;
  g_free(priv->password);
  g_free(priv->user);
  g_free(priv->domain);
  G_OBJECT_CLASS(g_mount_operation_parent_class)->finalize(object);
}
static gboolean reply_non_handled_in_idle(gpointer data) {
  GMountOperation *op = data;
  g_mount_operation_reply(op, G_MOUNT_OPERATION_UNHANDLED);
  return FALSE;
}
static void ask_password(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags) {
  g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, reply_non_handled_in_idle, g_object_ref(op), g_object_unref);
}
static void ask_question(GMountOperation *op, const char *message, const char *choices[]) {
  g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, reply_non_handled_in_idle, g_object_ref(op), g_object_unref);
}
static void show_processes (GMountOperation *op, const gchar *message, GArray *processes, const gchar *choices[]) {
  g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, reply_non_handled_in_idle, g_object_ref(op), g_object_unref);
}
static void g_mount_operation_class_init(GMountOperationClass *klass) {
  GObjectClass *object_class;
  g_type_class_add_private (klass, sizeof (GMountOperationPrivate));
  object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = g_mount_operation_finalize;
  object_class->get_property = g_mount_operation_get_property;
  object_class->set_property = g_mount_operation_set_property;
  klass->ask_password = ask_password;
  klass->ask_question = ask_question;
  klass->show_processes = show_processes;
  signals[ASK_PASSWORD] = g_signal_new(I_("ask-password"), G_TYPE_FROM_CLASS(object_class), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GMountOperationClass, ask_password),
                                       NULL, NULL, NULL, G_TYPE_NONE, 4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                       G_TYPE_ASK_PASSWORD_FLAGS);
  signals[ASK_QUESTION] = g_signal_new(I_("ask-question"), G_TYPE_FROM_CLASS(object_class), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GMountOperationClass, ask_question),
                                       NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRV);
  signals[REPLY] = g_signal_new(I_("reply"), G_TYPE_FROM_CLASS(object_class), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GMountOperationClass, reply), NULL, NULL,
		                        g_cclosure_marshal_VOID__ENUM, G_TYPE_NONE, 1, G_TYPE_MOUNT_OPERATION_RESULT);
  signals[ABORTED] = g_signal_new(I_("aborted"), G_TYPE_FROM_CLASS(object_class), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GMountOperationClass, aborted), NULL,
		                          NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  signals[SHOW_PROCESSES] = g_signal_new(I_("show-processes"), G_TYPE_FROM_CLASS(object_class), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GMountOperationClass, show_processes),
		                                 NULL, NULL, NULL, G_TYPE_NONE, 3, G_TYPE_STRING, G_TYPE_ARRAY, G_TYPE_STRV);
  g_object_class_install_property(object_class,PROP_USERNAME,g_param_spec_string ("username", P_("Username"), P_("The user name"),
                                  NULL,G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
  g_object_class_install_property(object_class,PROP_PASSWORD,g_param_spec_string("password", P_("Password"), P_("The password"),
                                  NULL,G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
  g_object_class_install_property(object_class,PROP_ANONYMOUS,g_param_spec_boolean ("anonymous", P_("Anonymous"),
                                  P_("Whether to use an anonymous user"),FALSE,G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                         G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
  g_object_class_install_property(object_class,PROP_DOMAIN,g_param_spec_string("domain", P_("Domain"), P_("The domain of the "
                                  "mount operation"),NULL,G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
  g_object_class_install_property(object_class, PROP_PASSWORD_SAVE, g_param_spec_enum("password-save", P_("Password save"), P_("How passwords should "
                                  "be saved"), G_TYPE_PASSWORD_SAVE, G_PASSWORD_SAVE_NEVER, G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                                  G_PARAM_STATIC_BLURB));
  g_object_class_install_property(object_class,PROP_CHOICE,g_param_spec_int ("choice", P_("Choice"), P_("The users choice"),
                                  0, G_MAXINT,0,G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}
static void g_mount_operation_init(GMountOperation *operation) {
  operation->priv = G_TYPE_INSTANCE_GET_PRIVATE(operation, G_TYPE_MOUNT_OPERATION, GMountOperationPrivate);
}
GMountOperation *g_mount_operation_new(void) {
  return g_object_new(G_TYPE_MOUNT_OPERATION, NULL);
}
const char *g_mount_operation_get_username(GMountOperation *op) {
  g_return_val_if_fail(G_IS_MOUNT_OPERATION(op), NULL);
  return op->priv->user;
}
void g_mount_operation_set_username(GMountOperation *op, const char *username) {
  g_return_if_fail(G_IS_MOUNT_OPERATION(op));
  g_free (op->priv->user);
  op->priv->user = g_strdup(username);
  g_object_notify(G_OBJECT(op), "username");
}
const char *g_mount_operation_get_password(GMountOperation *op) {
  g_return_val_if_fail (G_IS_MOUNT_OPERATION (op), NULL);
  return op->priv->password;
}
void g_mount_operation_set_password(GMountOperation *op, const char *password) {
  g_return_if_fail(G_IS_MOUNT_OPERATION(op));
  g_free (op->priv->password);
  op->priv->password = g_strdup(password);
  g_object_notify(G_OBJECT(op), "password");
}
gboolean g_mount_operation_get_anonymous(GMountOperation *op) {
  g_return_val_if_fail(G_IS_MOUNT_OPERATION(op), FALSE);
  return op->priv->anonymous;
}
void g_mount_operation_set_anonymous(GMountOperation *op, gboolean anonymous) {
  GMountOperationPrivate *priv;
  g_return_if_fail(G_IS_MOUNT_OPERATION(op));
  priv = op->priv;
  if (priv->anonymous != anonymous) {
      priv->anonymous = anonymous;
      g_object_notify(G_OBJECT(op), "anonymous");
  }
}
const char *g_mount_operation_get_domain(GMountOperation *op) {
  g_return_val_if_fail(G_IS_MOUNT_OPERATION(op), NULL);
  return op->priv->domain;
}
void g_mount_operation_set_domain(GMountOperation *op, const char *domain) {
  g_return_if_fail(G_IS_MOUNT_OPERATION(op));
  g_free(op->priv->domain);
  op->priv->domain = g_strdup(domain);
  g_object_notify(G_OBJECT(op), "domain");
}
GPasswordSave g_mount_operation_get_password_save(GMountOperation *op) {
  g_return_val_if_fail(G_IS_MOUNT_OPERATION(op), G_PASSWORD_SAVE_NEVER);
  return op->priv->password_save;
}
void g_mount_operation_set_password_save(GMountOperation *op, GPasswordSave save) {
  GMountOperationPrivate *priv;
  g_return_if_fail(G_IS_MOUNT_OPERATION(op));
  priv = op->priv;
  if (priv->password_save != save) {
      priv->password_save = save;
      g_object_notify(G_OBJECT(op), "password-save");
  }
}
int g_mount_operation_get_choice(GMountOperation *op) {
  g_return_val_if_fail(G_IS_MOUNT_OPERATION(op), 0);
  return op->priv->choice;
}
void g_mount_operation_set_choice(GMountOperation *op, int choice) {
  GMountOperationPrivate *priv;
  g_return_if_fail(G_IS_MOUNT_OPERATION(op));
  priv = op->priv;
  if (priv->choice != choice) {
      priv->choice = choice;
      g_object_notify(G_OBJECT(op), "choice");
  }
}
void g_mount_operation_reply(GMountOperation *op, GMountOperationResult result) {
  g_return_if_fail(G_IS_MOUNT_OPERATION(op));
  g_signal_emit(op, signals[REPLY], 0, result);
}