#include "../glib/glibintl.h"
#include "config.h"
#include "gfileenumerator.h"
#include "gfile.h"
#include "gioscheduler.h"
#include "gasyncresult.h"
#include "gasynchelper.h"
#include "gsimpleasyncresult.h"
#include "gioerror.h"

G_DEFINE_TYPE(GFileEnumerator, g_file_enumerator, G_TYPE_OBJECT);
struct _GFileEnumeratorPrivate {
  GFile *container;
  guint closed : 1;
  guint pending : 1;
  GAsyncReadyCallback outstanding_callback;
  GError *outstanding_error;
};
enum {
  PROP_0,
  PROP_CONTAINER
};
static void g_file_enumerator_real_next_files_async(GFileEnumerator *enumerator, int num_files, int io_priority, GCancellable *cancellable,
							                        GAsyncReadyCallback callback, gpointer user_data);
static GList *g_file_enumerator_real_next_files_finish(GFileEnumerator *enumerator, GAsyncResult *res, GError **error);
static void g_file_enumerator_real_close_async(GFileEnumerator *enumerator, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
							                   gpointer user_data);
static gboolean g_file_enumerator_real_close_finish(GFileEnumerator *enumerator, GAsyncResult *res, GError **error);
static void g_file_enumerator_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  GFileEnumerator *enumerator;
  enumerator = G_FILE_ENUMERATOR(object);
  switch(property_id) {
      case PROP_CONTAINER: enumerator->priv->container = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec); break;
  }
}
static void g_file_enumerator_dispose(GObject *object) {
  GFileEnumerator *enumerator;
  enumerator = G_FILE_ENUMERATOR(object);
  if (enumerator->priv->container) {
      g_object_unref(enumerator->priv->container);
      enumerator->priv->container = NULL;
  }
  G_OBJECT_CLASS (g_file_enumerator_parent_class)->dispose (object);
}
static void g_file_enumerator_finalize(GObject *object) {
  GFileEnumerator *enumerator;
  enumerator = G_FILE_ENUMERATOR(object);
  if (!enumerator->priv->closed) g_file_enumerator_close(enumerator, NULL, NULL);
  G_OBJECT_CLASS(g_file_enumerator_parent_class)->finalize(object);
}
static void g_file_enumerator_class_init(GFileEnumeratorClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GFileEnumeratorPrivate));
  gobject_class->set_property = g_file_enumerator_set_property;
  gobject_class->dispose = g_file_enumerator_dispose;
  gobject_class->finalize = g_file_enumerator_finalize;
  klass->next_files_async = g_file_enumerator_real_next_files_async;
  klass->next_files_finish = g_file_enumerator_real_next_files_finish;
  klass->close_async = g_file_enumerator_real_close_async;
  klass->close_finish = g_file_enumerator_real_close_finish;
  g_object_class_install_property(gobject_class, PROP_CONTAINER, g_param_spec_object("container","Container", "The container that is being "
                                  "enumerated", G_TYPE_FILE, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
}
static void g_file_enumerator_init(GFileEnumerator *enumerator) {
  enumerator->priv = G_TYPE_INSTANCE_GET_PRIVATE(enumerator, G_TYPE_FILE_ENUMERATOR, GFileEnumeratorPrivate);
}
GFileInfo *g_file_enumerator_next_file(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error) {
  GFileEnumeratorClass *class;
  GFileInfo *info;
  g_return_val_if_fail(G_IS_FILE_ENUMERATOR(enumerator), NULL);
  g_return_val_if_fail(enumerator != NULL, NULL);
  if (enumerator->priv->closed) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_CLOSED,"Enumerator is closed");
      return NULL;
  }
  if (enumerator->priv->pending) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_PENDING,"File enumerator has outstanding operation");
      return NULL;
  }
  if (enumerator->priv->outstanding_error) {
      g_propagate_error(error, enumerator->priv->outstanding_error);
      enumerator->priv->outstanding_error = NULL;
      return NULL;
  }
  class = G_FILE_ENUMERATOR_GET_CLASS(enumerator);
  if (cancellable) g_cancellable_push_current(cancellable);
  enumerator->priv->pending = TRUE;
  info = (*class->next_file)(enumerator, cancellable, error);
  enumerator->priv->pending = FALSE;
  if (cancellable) g_cancellable_pop_current(cancellable);
  return info;
}
gboolean g_file_enumerator_close(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error) {
  GFileEnumeratorClass *class;
  g_return_val_if_fail(G_IS_FILE_ENUMERATOR (enumerator), FALSE);
  g_return_val_if_fail(enumerator != NULL, FALSE);
  class = G_FILE_ENUMERATOR_GET_CLASS(enumerator);
  if (enumerator->priv->closed) return TRUE;
  if (enumerator->priv->pending) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_PENDING,"File enumerator has outstanding operation");
      return FALSE;
  }
  if (cancellable) g_cancellable_push_current(cancellable);
  enumerator->priv->pending = TRUE;
  (* class->close_fn)(enumerator, cancellable, error);
  enumerator->priv->pending = FALSE;
  enumerator->priv->closed = TRUE;
  if (cancellable) g_cancellable_pop_current(cancellable);
  return TRUE;
}
static void next_async_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GFileEnumerator *enumerator = G_FILE_ENUMERATOR(source_object);
  enumerator->priv->pending = FALSE;
  if (enumerator->priv->outstanding_callback) (*enumerator->priv->outstanding_callback)(source_object, res, user_data);
  g_object_unref(enumerator);
}
void g_file_enumerator_next_files_async(GFileEnumerator *enumerator, int num_files, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
				                        gpointer user_data) {
  GFileEnumeratorClass *class;
  GSimpleAsyncResult *simple;
  g_return_if_fail(G_IS_FILE_ENUMERATOR(enumerator));
  g_return_if_fail(enumerator != NULL);
  g_return_if_fail(num_files >= 0);
  if (num_files == 0) {
      simple = g_simple_async_result_new(G_OBJECT(enumerator), callback, user_data, g_file_enumerator_next_files_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (enumerator->priv->closed) {
      g_simple_async_report_error_in_idle(G_OBJECT(enumerator), callback, user_data, G_IO_ERROR, G_IO_ERROR_CLOSED,"File enumerator is already closed");
      return;
  }
  if (enumerator->priv->pending) {
      g_simple_async_report_error_in_idle(G_OBJECT(enumerator), callback, user_data, G_IO_ERROR, G_IO_ERROR_PENDING,"File enumerator has outstanding operation");
      return;
  }
  class = G_FILE_ENUMERATOR_GET_CLASS(enumerator);
  enumerator->priv->pending = TRUE;
  enumerator->priv->outstanding_callback = callback;
  g_object_ref(enumerator);
  (*class->next_files_async)(enumerator, num_files, io_priority, cancellable, next_async_callback_wrapper, user_data);
}
GList *g_file_enumerator_next_files_finish(GFileEnumerator *enumerator, GAsyncResult *result, GError **error) {
  GFileEnumeratorClass *class;
  GSimpleAsyncResult *simple;
  g_return_val_if_fail(G_IS_FILE_ENUMERATOR(enumerator), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
      if (g_simple_async_result_get_source_tag(simple) == g_file_enumerator_next_files_async) return NULL;
  }
  class = G_FILE_ENUMERATOR_GET_CLASS(enumerator);
  return class->next_files_finish(enumerator, result, error);
}
static void close_async_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GFileEnumerator *enumerator = G_FILE_ENUMERATOR(source_object);
  enumerator->priv->pending = FALSE;
  enumerator->priv->closed = TRUE;
  if (enumerator->priv->outstanding_callback) (*enumerator->priv->outstanding_callback)(source_object, res, user_data);
  g_object_unref(enumerator);
}
void g_file_enumerator_close_async(GFileEnumerator *enumerator, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileEnumeratorClass *class;
  g_return_if_fail(G_IS_FILE_ENUMERATOR(enumerator));
  if (enumerator->priv->closed) {
      g_simple_async_report_error_in_idle(G_OBJECT(enumerator), callback, user_data, G_IO_ERROR,G_IO_ERROR_CLOSED,"File enumerator is already closed");
      return;
  }
  if (enumerator->priv->pending) {
      g_simple_async_report_error_in_idle(G_OBJECT(enumerator), callback, user_data, G_IO_ERROR,G_IO_ERROR_PENDING,"File enumerator has outstanding operation");
      return;
  }
  class = G_FILE_ENUMERATOR_GET_CLASS(enumerator);
  enumerator->priv->pending = TRUE;
  enumerator->priv->outstanding_callback = callback;
  g_object_ref(enumerator);
  (*class->close_async)(enumerator, io_priority, cancellable, close_async_callback_wrapper, user_data);
}
gboolean g_file_enumerator_close_finish(GFileEnumerator *enumerator, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GFileEnumeratorClass *class;
  g_return_val_if_fail(G_IS_FILE_ENUMERATOR(enumerator), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  class = G_FILE_ENUMERATOR_GET_CLASS(enumerator);
  return class->close_finish(enumerator, result, error);
}
gboolean g_file_enumerator_is_closed(GFileEnumerator *enumerator) {
  g_return_val_if_fail(G_IS_FILE_ENUMERATOR(enumerator), TRUE);
  return enumerator->priv->closed;
}
gboolean g_file_enumerator_has_pending(GFileEnumerator *enumerator) {
  g_return_val_if_fail(G_IS_FILE_ENUMERATOR(enumerator), TRUE);
  return enumerator->priv->pending;
}
void g_file_enumerator_set_pending(GFileEnumerator *enumerator, gboolean pending) {
  g_return_if_fail(G_IS_FILE_ENUMERATOR(enumerator));
  enumerator->priv->pending = pending;
}
GFile *g_file_enumerator_get_container(GFileEnumerator *enumerator) {
  g_return_val_if_fail(G_IS_FILE_ENUMERATOR(enumerator), NULL);
  return enumerator->priv->container;
}
typedef struct {
  int num_files;
  GList *files;
} NextAsyncOp;
static void next_async_op_free(NextAsyncOp *op) {
  g_list_foreach(op->files, (GFunc)g_object_unref, NULL);
  g_list_free(op->files);
  g_free(op);
}
static void next_files_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  NextAsyncOp *op;
  GFileEnumeratorClass *class;
  GError *error = NULL;
  GFileInfo *info;
  GFileEnumerator *enumerator;
  int i;
  enumerator = G_FILE_ENUMERATOR(object);
  op = g_simple_async_result_get_op_res_gpointer(res);
  class = G_FILE_ENUMERATOR_GET_CLASS(object);
  for (i = 0; i < op->num_files; i++) {
      if (g_cancellable_set_error_if_cancelled(cancellable, &error)) info = NULL;
      else info = class->next_file(enumerator, cancellable, &error);
      if (info == NULL) {
          if (error != NULL && i > 0) {
              if (error->domain == G_IO_ERROR && error->code == G_IO_ERROR_CANCELLED) g_error_free(error);
              else enumerator->priv->outstanding_error = error;
              error = NULL;
          }
          break;
      } else op->files = g_list_prepend(op->files, info);
  }
}
static void g_file_enumerator_real_next_files_async(GFileEnumerator *enumerator, int num_files, int io_priority, GCancellable *cancellable,
                                                    GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  NextAsyncOp *op;
  op = g_new0(NextAsyncOp, 1);
  op->num_files = num_files;
  op->files = NULL;
  res = g_simple_async_result_new(G_OBJECT(enumerator), callback, user_data, g_file_enumerator_real_next_files_async);
  g_simple_async_result_set_op_res_gpointer(res, op, (GDestroyNotify)next_async_op_free);
  g_simple_async_result_run_in_thread(res, next_files_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GList *g_file_enumerator_real_next_files_finish(GFileEnumerator *enumerator, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  NextAsyncOp *op;
  GList *res;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_enumerator_real_next_files_async);
  op = g_simple_async_result_get_op_res_gpointer (simple);
  res = op->files;
  op->files = NULL;
  return res;
}
static void close_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileEnumeratorClass *class;
  GError *error = NULL;
  gboolean result;
  class = G_FILE_ENUMERATOR_GET_CLASS (object);
  result = class->close_fn (G_FILE_ENUMERATOR (object), cancellable, &error);
  if (!result) g_simple_async_result_take_error (res, error);
}
static void g_file_enumerator_real_close_async(GFileEnumerator *enumerator, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
				                               gpointer user_data) {
  GSimpleAsyncResult *res;
  res = g_simple_async_result_new (G_OBJECT (enumerator), callback, user_data, g_file_enumerator_real_close_async);
  g_simple_async_result_set_handle_cancellation (res, FALSE);
  g_simple_async_result_run_in_thread (res, close_async_thread, io_priority, cancellable);
  g_object_unref (res);
}
static gboolean g_file_enumerator_real_close_finish(GFileEnumerator *enumerator, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_enumerator_real_close_async);
  return TRUE;
}