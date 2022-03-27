#ifndef __G_TASK_H__
#define __G_TASK_H__

#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#include "giotypes.h"

G_BEGIN_DECLS
typedef struct _GTask GTask;
#define G_TYPE_TASK  (g_task_get_type ())
#define G_TASK(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_TASK, GTask))
#define G_TASK_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_TASK, GTaskClass))
#define G_IS_TASK(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_TASK))
#define G_IS_TASK_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_TASK))
#define G_TASK_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_TASK, GTaskClass))
typedef struct _GTaskClass GTaskClass;
GType g_task_get_type(void) G_GNUC_CONST;
GTask *g_task_new(gpointer source_object, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data);
void g_task_report_error(gpointer source_object, GAsyncReadyCallback callback, gpointer callback_data, gpointer source_tag, GError *error);
void g_task_report_new_error(gpointer source_object, GAsyncReadyCallback callback, gpointer callback_data, gpointer source_tag, GQuark domain, gint code,
                             const char *format, ...) G_GNUC_PRINTF(7, 8);
void g_task_set_task_data(GTask *task, gpointer task_data, GDestroyNotify task_data_destroy);
void g_task_set_priority(GTask *task, gint priority);
void g_task_set_check_cancellable(GTask *task, gboolean check_cancellable);
void g_task_set_source_tag(GTask *task, gpointer source_tag);
void g_task_set_name(GTask *task, const gchar *name);
gpointer g_task_get_source_object(GTask *task);
gpointer g_task_get_task_data(GTask *task);
gint g_task_get_priority(GTask *task);
GMainContext *g_task_get_context(GTask *task);
GCancellable *g_task_get_cancellable(GTask *task);
gboolean g_task_get_check_cancellable(GTask *task);
gpointer g_task_get_source_tag(GTask *task);
const gchar *g_task_get_name(GTask *task);
gboolean g_task_is_valid(gpointer result, gpointer source_object);
typedef void (*GTaskThreadFunc)(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable);
void g_task_run_in_thread(GTask *task, GTaskThreadFunc task_func);
void g_task_run_in_thread_sync(GTask *task, GTaskThreadFunc task_func);
gboolean g_task_set_return_on_cancel(GTask *task, gboolean return_on_cancel);
gboolean g_task_get_return_on_cancel(GTask *task);
void g_task_attach_source(GTask *task, GSource *source, GSourceFunc callback);
void g_task_return_pointer(GTask *task, gpointer result, GDestroyNotify result_destroy);
void g_task_return_boolean(GTask *task, gboolean result);
void g_task_return_int(GTask *task, gssize result);
void g_task_return_error(GTask *task, GError *error);
void g_task_return_new_error(GTask *task, GQuark domain, gint code, const char *format, ...) G_GNUC_PRINTF (4, 5);
void g_task_return_value(GTask *task, GValue *result);
gboolean g_task_return_error_if_cancelled(GTask *task);
gpointer g_task_propagate_pointer(GTask *task, GError **error);
gboolean g_task_propagate_boolean(GTask *task, GError **error);
gssize g_task_propagate_int(GTask *task, GError **error);
gboolean g_task_propagate_value(GTask *task, GValue *value, GError **error);
gboolean g_task_had_error(GTask *task);
gboolean g_task_get_completed(GTask *task);
G_END_DECLS
#endif