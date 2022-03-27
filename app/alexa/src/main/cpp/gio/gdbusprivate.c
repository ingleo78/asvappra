#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "giotypes.h"
#include "gsocket.h"
#include "gdbusprivate.h"
#include "gdbusmessage.h"
#include "gdbuserror.h"
#include "gdbusintrospection.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "ginputstream.h"
#include "gmemoryinputstream.h"
#include "giostream.h"
#include "gsocketcontrolmessage.h"
#include "gsocketconnection.h"
#include "gsocketoutputstream.h"
#include "gunixfdmessage.h"
#include "gunixconnection.h"
#include "gunixcredentialsmessage.h"

gchar *_g_dbus_hexdump(const gchar *data, gsize len, guint indent) {
 guint n, m;
 GString *ret;
 ret = g_string_new(NULL);
 for (n = 0; n < len; n += 16) {
     g_string_append_printf(ret, "%*s%04x: ", indent, "", n);
     for (m = n; m < n + 16; m++) {
         if (m > n && (m%4) == 0) g_string_append_c(ret, ' ');
         if (m < len) g_string_append_printf(ret,"%02x ", (guchar)data[m]);
         else g_string_append(ret,"   ");
     }
     g_string_append(ret,"   ");
     for (m = n; m < len && m < n + 16; m++) g_string_append_c(ret,g_ascii_isprint(data[m]) ? data[m] : '.');
     g_string_append_c(ret, '\n');
 }
 return g_string_free(ret, FALSE);
}
typedef struct {
  GSocket *socket;
  GCancellable *cancellable;
  void *buffer;
  gsize count;
  GSocketControlMessage ***messages;
  gint *num_messages;
  GSimpleAsyncResult *simple;
  gboolean from_mainloop;
} ReadWithControlData;
static void read_with_control_data_free(ReadWithControlData *data) {
  g_object_unref(data->socket);
  if (data->cancellable != NULL) g_object_unref(data->cancellable);
  g_object_unref(data->simple);
  g_free(data);
}
static gboolean _g_socket_read_with_control_messages_ready(GSocket *socket, GIOCondition condition, gpointer user_data) {
  ReadWithControlData *data = user_data;
  GError *error;
  gssize result;
  GInputVector vector;
  error = NULL;
  vector.buffer = data->buffer;
  vector.size = data->count;
  result = g_socket_receive_message(data->socket, NULL, &vector, 1, data->messages, data->num_messages, NULL, data->cancellable, &error);
  if (result >= 0) g_simple_async_result_set_op_res_gssize(data->simple, result);
  else {
      g_assert(error != NULL);
      g_simple_async_result_take_error(data->simple, error);
  }
  if (data->from_mainloop) g_simple_async_result_complete(data->simple);
  else g_simple_async_result_complete_in_idle(data->simple);
  return FALSE;
}
static void _g_socket_read_with_control_messages(GSocket *socket, void *buffer, gsize count, GSocketControlMessage ***messages, gint *num_messages,
                                                 gint io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  ReadWithControlData *data;
  data = g_new0(ReadWithControlData, 1);
  data->socket = g_object_ref(socket);
  data->cancellable = cancellable != NULL ? g_object_ref(cancellable) : NULL;
  data->buffer = buffer;
  data->count = count;
  data->messages = messages;
  data->num_messages = num_messages;
  data->simple = g_simple_async_result_new(G_OBJECT(socket), callback, user_data, _g_socket_read_with_control_messages);
  if (!g_socket_condition_check(socket, G_IO_IN)) {
      GSource *source;
      data->from_mainloop = TRUE;
      source = g_socket_create_source(data->socket, G_IO_IN | G_IO_HUP | G_IO_ERR, cancellable);
      g_source_set_callback(source, (GSourceFunc)_g_socket_read_with_control_messages_ready, data, (GDestroyNotify)read_with_control_data_free);
      g_source_attach(source, g_main_context_get_thread_default());
      g_source_unref(source);
  } else {
      _g_socket_read_with_control_messages_ready(data->socket, G_IO_IN, data);
      read_with_control_data_free(data);
  }
}
static gssize _g_socket_read_with_control_messages_finish(GSocket *socket, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  g_return_val_if_fail(G_IS_SOCKET(socket), -1);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == _g_socket_read_with_control_messages);
  if (g_simple_async_result_propagate_error(simple, error)) return -1;
  else return g_simple_async_result_get_op_res_gssize(simple);
}
static GPtrArray *ensured_classes = NULL;
static void ensure_type(GType gtype) {
  g_ptr_array_add(ensured_classes, g_type_class_ref(gtype));
}
static void released_required_types(void) {
  g_ptr_array_foreach(ensured_classes, (GFunc)g_type_class_unref,NULL);
  g_ptr_array_unref(ensured_classes);
  ensured_classes = NULL;
}
static void ensure_required_types(void) {
  g_assert(ensured_classes == NULL);
  ensured_classes = g_ptr_array_new();
  ensure_type(G_TYPE_SIMPLE_ASYNC_RESULT);
  ensure_type(G_TYPE_MEMORY_INPUT_STREAM);
}
G_LOCK_DEFINE_STATIC(shared_thread_lock);
typedef struct {
  gint num_users;
  GThread *thread;
  GMainContext *context;
  GMainLoop *loop;
} SharedThreadData;
static SharedThreadData *shared_thread_data = NULL;
static gpointer gdbus_shared_thread_func(gpointer data) {
  g_main_context_push_thread_default(shared_thread_data->context);
  g_main_loop_run (shared_thread_data->loop);
  g_main_context_pop_thread_default(shared_thread_data->context);
  return NULL;
}
typedef void (*GDBusSharedThreadFunc)(gpointer user_data);
typedef struct {
  GDBusSharedThreadFunc func;
  gpointer user_data;
  gboolean done;
} CallerData;
static gboolean invoke_caller(gpointer user_data) {
  CallerData *data = user_data;
  data->func(data->user_data);
  data->done = TRUE;
  return FALSE;
}
static void _g_dbus_shared_thread_ref(GDBusSharedThreadFunc func, gpointer user_data) {
  GError *error;
  GSource *idle_source;
  CallerData *data;
  gboolean release_types;
  G_LOCK(shared_thread_lock);
  release_types = FALSE;
  if (shared_thread_data != NULL) {
      shared_thread_data->num_users += 1;
      goto have_thread;
  }
  shared_thread_data = g_new0(SharedThreadData,1);
  shared_thread_data->num_users = 1;
  ensure_required_types();
  release_types = TRUE;
  error = NULL;
  shared_thread_data->context = g_main_context_new();
  shared_thread_data->loop = g_main_loop_new(shared_thread_data->context, FALSE);
  shared_thread_data->thread = g_thread_create(gdbus_shared_thread_func, NULL, TRUE,&error);
  g_assert_no_error(error);
have_thread:
  data = g_new0(CallerData,1);
  data->func = func;
  data->user_data = user_data;
  data->done = FALSE;
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
  g_source_set_callback(idle_source, invoke_caller, data,NULL);
  g_source_attach(idle_source, shared_thread_data->context);
  g_source_unref(idle_source);
  while(!data->done) g_thread_yield();
  if (release_types) released_required_types();
  g_free(data);
  G_UNLOCK(shared_thread_lock);
}
static void _g_dbus_shared_thread_unref(void) {
#if 0
  G_LOCK(shared_thread_lock);
  g_assert(shared_thread_data != NULL);
  shared_thread_data->num_users -= 1;
  if (shared_thread_data->num_users == 0) {
      g_main_loop_quit(shared_thread_data->loop);
      //g_thread_join(shared_thread_data->thread);
      g_main_loop_unref(shared_thread_data->loop);
      g_main_context_unref(shared_thread_data->context);
      g_free(shared_thread_data);
      shared_thread_data = NULL;
      G_UNLOCK(shared_thread_lock);
  } else G_UNLOCK(shared_thread_lock);
#endif
}
struct GDBusWorker {
  volatile gint ref_count;
  gboolean stopped;
  gboolean frozen;
  GQueue *received_messages_while_frozen;
  GIOStream *stream;
  GDBusCapabilityFlags capabilities;
  GCancellable *cancellable;
  GDBusWorkerMessageReceivedCallback  message_received_callback;
  GDBusWorkerMessageAboutToBeSentCallback message_about_to_be_sent_callback;
  GDBusWorkerDisconnectedCallback disconnected_callback;
  gpointer user_data;
  GThread *thread;
  GSocket *socket;
  GMutex *read_lock;
  gchar *read_buffer;
  gsize read_buffer_allocated_size;
  gsize read_buffer_cur_size;
  gsize read_buffer_bytes_wanted;
  GUnixFDList *read_fd_list;
  GSocketControlMessage **read_ancillary_messages;
  gint read_num_ancillary_messages;
  GMutex *write_lock;
  GQueue *write_queue;
  gint num_writes_pending;
  guint64 write_num_messages_written;
  GList *write_pending_flushes;
  gboolean flush_pending;
};
typedef struct {
  GMutex *mutex;
  GCond *cond;
  guint64 number_to_wait_for;
  GError *error;
} FlushData;
struct _MessageToWriteData ;
typedef struct _MessageToWriteData MessageToWriteData;
static void message_to_write_data_free(MessageToWriteData *data);
static void read_message_print_transport_debug(gssize bytes_read, GDBusWorker *worker);
static void write_message_print_transport_debug(gssize bytes_written, MessageToWriteData *data);
static GDBusWorker *_g_dbus_worker_ref(GDBusWorker *worker) {
  g_atomic_int_inc(&worker->ref_count);
  return worker;
}
static void _g_dbus_worker_unref(GDBusWorker *worker) {
  if (g_atomic_int_dec_and_test(&worker->ref_count)) {
      g_assert(worker->write_pending_flushes == NULL);
      _g_dbus_shared_thread_unref();
      g_object_unref(worker->stream);
      g_mutex_free(worker->read_lock);
      g_object_unref(worker->cancellable);
      if (worker->read_fd_list != NULL) g_object_unref(worker->read_fd_list);
      g_queue_foreach(worker->received_messages_while_frozen, (GFunc)g_object_unref, NULL);
      g_queue_free(worker->received_messages_while_frozen);
      g_mutex_free(worker->write_lock);
      g_queue_foreach(worker->write_queue, (GFunc)message_to_write_data_free, NULL);
      g_queue_free(worker->write_queue);
      g_free(worker->read_buffer);
      g_free(worker);
  }
}
static void _g_dbus_worker_emit_disconnected(GDBusWorker *worker, gboolean remote_peer_vanished, GError *error) {
  if (!worker->stopped) worker->disconnected_callback(worker, remote_peer_vanished, error, worker->user_data);
}
static void _g_dbus_worker_emit_message_received(GDBusWorker *worker, GDBusMessage *message) {
  if (!worker->stopped) worker->message_received_callback(worker, message, worker->user_data);
}
static GDBusMessage *_g_dbus_worker_emit_message_about_to_be_sent(GDBusWorker *worker, GDBusMessage *message) {
  GDBusMessage *ret;
  if (!worker->stopped) ret = worker->message_about_to_be_sent_callback(worker, message, worker->user_data);
  else ret = message;
  return ret;
}
static void _g_dbus_worker_queue_or_deliver_received_message(GDBusWorker  *worker, GDBusMessage *message) {
  if (worker->frozen || g_queue_get_length(worker->received_messages_while_frozen) > 0) {
      g_queue_push_tail(worker->received_messages_while_frozen, message);
  } else {
      _g_dbus_worker_emit_message_received(worker, message);
      g_object_unref(message);
  }
}
static gboolean unfreeze_in_idle_cb(gpointer user_data) {
  GDBusWorker *worker = user_data;
  GDBusMessage *message;
  g_mutex_lock(worker->read_lock);
  if (worker->frozen) {
      while((message = g_queue_pop_head(worker->received_messages_while_frozen)) != NULL) {
          _g_dbus_worker_emit_message_received(worker, message);
          g_object_unref(message);
      }
      worker->frozen = FALSE;
  } else { g_assert(g_queue_get_length(worker->received_messages_while_frozen) == 0); }
  g_mutex_unlock(worker->read_lock);
  return FALSE;
}
void _g_dbus_worker_unfreeze(GDBusWorker *worker) {
  GSource *idle_source;
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
  g_source_set_callback(idle_source, unfreeze_in_idle_cb, _g_dbus_worker_ref(worker), (GDestroyNotify)_g_dbus_worker_unref);
  g_source_attach(idle_source, shared_thread_data->context);
  g_source_unref(idle_source);
}
static void _g_dbus_worker_do_read_unlocked (GDBusWorker *worker);
static void _g_dbus_worker_do_read_cb(GInputStream *input_stream, GAsyncResult *res, gpointer user_data) {
  GDBusWorker *worker = user_data;
  GError *error;
  gssize bytes_read;
  g_mutex_lock(worker->read_lock);
  if (worker->stopped) goto out;
  error = NULL;
  if (worker->socket == NULL) bytes_read = g_input_stream_read_finish(g_io_stream_get_input_stream(worker->stream), res, &error);
  else bytes_read = _g_socket_read_with_control_messages_finish(worker->socket, res, &error);
  if (worker->read_num_ancillary_messages > 0) {
      gint n;
      for (n = 0; n < worker->read_num_ancillary_messages; n++) {
          GSocketControlMessage *control_message = G_SOCKET_CONTROL_MESSAGE(worker->read_ancillary_messages[n]);
          if (G_IS_UNIX_FD_MESSAGE(control_message)) {
              GUnixFDMessage *fd_message;
              gint *fds;
              gint num_fds;
              fd_message = G_UNIX_FD_MESSAGE(control_message);
              fds = g_unix_fd_message_steal_fds(fd_message, &num_fds);
              if (worker->read_fd_list == NULL) worker->read_fd_list = g_unix_fd_list_new_from_array(fds, num_fds);
              else {
                  gint n;
                  for (n = 0; n < num_fds; n++) {
                      g_unix_fd_list_append(worker->read_fd_list, fds[n], NULL);
                      close(fds[n]);
                  }
              }
              g_free(fds);
          } else if (G_IS_UNIX_CREDENTIALS_MESSAGE(control_message));
          else {
              if (error == NULL) {
                  g_set_error(&error, G_IO_ERROR,G_IO_ERROR_FAILED,"Unexpected ancillary message of type %s received from peer",
                              g_type_name(G_TYPE_FROM_INSTANCE(control_message)));
                  _g_dbus_worker_emit_disconnected(worker, TRUE, error);
                  g_error_free(error);
                  g_object_unref(control_message);
                  n++;
                  while(n < worker->read_num_ancillary_messages) g_object_unref(worker->read_ancillary_messages[n++]);
                  g_free(worker->read_ancillary_messages);
                  goto out;
              }
          }
          g_object_unref(control_message);
      }
      g_free(worker->read_ancillary_messages);
  }
  if (bytes_read == -1) {
      _g_dbus_worker_emit_disconnected(worker, TRUE, error);
      g_error_free(error);
      goto out;
  }
#if 0
  g_debug("read %d bytes (is_closed=%d blocking=%d condition=0x%02x) stream %p, %p", (gint)bytes_read,
          g_socket_is_closed(g_socket_connection_get_socket(G_SOCKET_CONNECTION(worker->stream))),
          g_socket_get_blocking(g_socket_connection_get_socket(G_SOCKET_CONNECTION(worker->stream))),
          g_socket_condition_check(g_socket_connection_get_socket(G_SOCKET_CONNECTION(worker->stream)), G_IO_IN | G_IO_OUT | G_IO_HUP), worker->stream,
          worker);
#endif
  if (bytes_read == 0) {
      g_set_error(&error, G_IO_ERROR,G_IO_ERROR_FAILED,"Underlying GIOStream returned 0 bytes on an async read");
      _g_dbus_worker_emit_disconnected(worker, TRUE, error);
      g_error_free(error);
      goto out;
  }
  read_message_print_transport_debug(bytes_read, worker);
  worker->read_buffer_cur_size += bytes_read;
  if (worker->read_buffer_bytes_wanted == worker->read_buffer_cur_size) {
      if (worker->read_buffer_bytes_wanted == 16) {
          gssize message_len;
          error = NULL;
          message_len = g_dbus_message_bytes_needed((guchar*)worker->read_buffer,16, &error);
          if (message_len == -1) {
              g_warning ("_g_dbus_worker_do_read_cb: error determing bytes needed: %s", error->message);
              _g_dbus_worker_emit_disconnected (worker, FALSE, error);
              g_error_free (error);
              goto out;
          }
          worker->read_buffer_bytes_wanted = message_len;
          _g_dbus_worker_do_read_unlocked (worker);
      } else {
          GDBusMessage *message;
          error = NULL;
          message = g_dbus_message_new_from_blob((guchar*)worker->read_buffer, worker->read_buffer_cur_size, worker->capabilities, &error);
          if (message == NULL) {
              gchar *s;
              s = _g_dbus_hexdump(worker->read_buffer, worker->read_buffer_cur_size, 2);
              g_warning("Error decoding D-Bus message of %" G_GSIZE_FORMAT " bytes\nThe error is: %s\nThe payload is as follows:\n%s\n",
                        worker->read_buffer_cur_size, error->message, s);
              g_free(s);
              _g_dbus_worker_emit_disconnected(worker, FALSE, error);
              g_error_free(error);
              goto out;
          }
          if (worker->read_fd_list != NULL) {
              g_dbus_message_set_unix_fd_list(message, worker->read_fd_list);
              g_object_unref(worker->read_fd_list);
              worker->read_fd_list = NULL;
          }
          if (G_UNLIKELY(_g_dbus_debug_message())) {
              gchar *s;
              _g_dbus_debug_print_lock();
              g_print("========================================================================\nGDBus-debug:Message:\n  <<<< RECEIVED D-Bus "
                      "message (%" G_GSIZE_FORMAT " bytes)\n", worker->read_buffer_cur_size);
              s = g_dbus_message_print(message, 2);
              g_print("%s", s);
              g_free(s);
              if (G_UNLIKELY(_g_dbus_debug_payload())) {
                  s = _g_dbus_hexdump(worker->read_buffer, worker->read_buffer_cur_size, 2);
                  g_print("%s\n", s);
                  g_free(s);
              }
              _g_dbus_debug_print_unlock();
          }
          _g_dbus_worker_queue_or_deliver_received_message(worker, message);
          worker->read_buffer_bytes_wanted = 0;
          worker->read_buffer_cur_size = 0;
          _g_dbus_worker_do_read_unlocked(worker);
      }
  } else _g_dbus_worker_do_read_unlocked(worker);
out:
  g_mutex_unlock(worker->read_lock);
  _g_dbus_worker_unref(worker);
}
static void _g_dbus_worker_do_read_unlocked(GDBusWorker *worker) {
  if (worker->read_buffer_bytes_wanted == 0) {
      worker->read_buffer_cur_size = 0;
      worker->read_buffer_bytes_wanted = 16;
  }
  if (worker->read_buffer == NULL || worker->read_buffer_bytes_wanted > worker->read_buffer_allocated_size) {
      worker->read_buffer_allocated_size = MAX(worker->read_buffer_bytes_wanted, 4096);
      worker->read_buffer = g_realloc(worker->read_buffer, worker->read_buffer_allocated_size);
  }
  if (worker->socket == NULL) {
      g_input_stream_read_async(g_io_stream_get_input_stream(worker->stream),worker->read_buffer + worker->read_buffer_cur_size,
                                worker->read_buffer_bytes_wanted - worker->read_buffer_cur_size, G_PRIORITY_DEFAULT, worker->cancellable,
                                (GAsyncReadyCallback)_g_dbus_worker_do_read_cb, _g_dbus_worker_ref(worker));
  } else {
      worker->read_ancillary_messages = NULL;
      worker->read_num_ancillary_messages = 0;
      _g_socket_read_with_control_messages(worker->socket,worker->read_buffer + worker->read_buffer_cur_size,worker->read_buffer_bytes_wanted -
                                           worker->read_buffer_cur_size, &worker->read_ancillary_messages, &worker->read_num_ancillary_messages,
                                           G_PRIORITY_DEFAULT, worker->cancellable, (GAsyncReadyCallback)_g_dbus_worker_do_read_cb,
                                           _g_dbus_worker_ref(worker));
  }
}
static void _g_dbus_worker_do_read(GDBusWorker *worker) {
  g_mutex_lock(worker->read_lock);
  _g_dbus_worker_do_read_unlocked(worker);
  g_mutex_unlock(worker->read_lock);
}
struct _MessageToWriteData {
  GDBusWorker *worker;
  GDBusMessage *message;
  gchar *blob;
  gsize blob_size;
  gsize total_written;
  GSimpleAsyncResult *simple;
};
static void message_to_write_data_free(MessageToWriteData *data) {
  _g_dbus_worker_unref(data->worker);
  if (data->message) g_object_unref(data->message);
  g_free(data->blob);
  g_free(data);
}
static void write_message_continue_writing(MessageToWriteData *data);
static void write_message_async_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  MessageToWriteData *data = user_data;
  GSimpleAsyncResult *simple;
  gssize bytes_written;
  GError *error;
  simple = data->simple;
  error = NULL;
  bytes_written = g_output_stream_write_finish(G_OUTPUT_STREAM(source_object), res, &error);
  if (bytes_written == -1) {
      g_simple_async_result_take_error(simple, error);
      g_simple_async_result_complete(simple);
      g_object_unref(simple);
      return;
  }
  g_assert(bytes_written > 0);
  write_message_print_transport_debug(bytes_written, data);
  data->total_written += bytes_written;
  g_assert(data->total_written <= data->blob_size);
  if (data->total_written == data->blob_size) {
      g_simple_async_result_complete(simple);
      g_object_unref(simple);
      return;
  }
  write_message_continue_writing(data);
}
static gboolean on_socket_ready(GSocket *socket, GIOCondition condition, gpointer user_data) {
  MessageToWriteData *data = user_data;
  write_message_continue_writing(data);
  return FALSE;
}
static void write_message_continue_writing(MessageToWriteData *data) {
  GOutputStream *ostream;
  GSimpleAsyncResult *simple;
#ifndef G_OS_UNIX
  GUnixFDList *fd_list;
#endif
  simple = data->simple;
  ostream = g_io_stream_get_output_stream(data->worker->stream);
#ifndef G_OS_UNIX
  fd_list = g_dbus_message_get_unix_fd_list(data->message);
#endif
  g_assert(!g_output_stream_has_pending(ostream));
  g_assert_cmpint(data->total_written, <, data->blob_size);
  if (G_IS_SOCKET_OUTPUT_STREAM(ostream) && data->total_written == 0) {
      GOutputVector vector;
      GSocketControlMessage *control_message;
      gssize bytes_written;
      GError *error;
      vector.buffer = data->blob;
      vector.size = data->blob_size;
      control_message = NULL;
      if (fd_list != NULL && g_unix_fd_list_get_length(fd_list) > 0) {
          if (!(data->worker->capabilities & G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING)) {
              g_simple_async_result_set_error(simple, G_IO_ERROR, G_IO_ERROR_FAILED, "Tried sending a file descriptor but remote peer does "
                                              "not support this capability");
              g_simple_async_result_complete(simple);
              g_object_unref(simple);
              return;
          }
          control_message = g_unix_fd_message_new_with_fd_list(fd_list);
      }
      error = NULL;
      bytes_written = g_socket_send_message(data->worker->socket, NULL, &vector, 1, control_message != NULL ? &control_message : NULL,
                                            control_message != NULL ? 1 : 0, G_SOCKET_MSG_NONE, data->worker->cancellable, &error);
      if (control_message != NULL) g_object_unref(control_message);
      if (bytes_written == -1) {
          if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) {
              GSource *source;
              source = g_socket_create_source(data->worker->socket, G_IO_OUT | G_IO_HUP | G_IO_ERR, data->worker->cancellable);
              g_source_set_callback(source, (GSourceFunc)on_socket_ready, data,NULL);
              g_source_attach(source, g_main_context_get_thread_default());
              g_source_unref(source);
              g_error_free(error);
              return;
          }
          g_simple_async_result_take_error(simple, error);
          g_simple_async_result_complete(simple);
          g_object_unref(simple);
          return;
      }
      g_assert(bytes_written > 0);
      write_message_print_transport_debug(bytes_written, data);
      data->total_written += bytes_written;
      g_assert(data->total_written <= data->blob_size);
      if (data->total_written == data->blob_size) {
          g_simple_async_result_complete(simple);
          g_object_unref(simple);
          return;
      }
      write_message_continue_writing(data);
  } else {
      if (fd_list != NULL) {
          g_simple_async_result_set_error(simple, G_IO_ERROR, G_IO_ERROR_FAILED, "Tried sending a file descriptor on unsupported stream of type %s",
                                          g_type_name(G_TYPE_FROM_INSTANCE(ostream)));
          g_simple_async_result_complete(simple);
          g_object_unref(simple);
          return;
      }
      g_output_stream_write_async(ostream,(const gchar*)data->blob + data->total_written,data->blob_size - data->total_written,
                                   G_PRIORITY_DEFAULT, data->worker->cancellable, write_message_async_cb, data);
  }
}
static void write_message_async(GDBusWorker *worker, MessageToWriteData *data, GAsyncReadyCallback callback, gpointer user_data) {
  data->simple = g_simple_async_result_new(NULL, callback, user_data, write_message_async);
  data->total_written = 0;
  write_message_continue_writing(data);
}
static gboolean write_message_finish(GAsyncResult *res, GError **error) {
  g_warn_if_fail(g_simple_async_result_get_source_tag(G_SIMPLE_ASYNC_RESULT(res)) == write_message_async);
  if (g_simple_async_result_propagate_error(G_SIMPLE_ASYNC_RESULT(res), error)) return FALSE;
  else return TRUE;
}
static void maybe_write_next_message(GDBusWorker *worker);
typedef struct {
  GDBusWorker *worker;
  GList *flushers;
} FlushAsyncData;
static void ostream_flush_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  FlushAsyncData *data = user_data;
  GError *error;
  GList *l;
  error = NULL;
  g_output_stream_flush_finish(G_OUTPUT_STREAM(source_object), res, &error);
  if (error == NULL) {
      if (G_UNLIKELY(_g_dbus_debug_transport())) {
          _g_dbus_debug_print_lock();
          g_print("========================================================================\nGDBus-debug:Transport:\n  ---- FLUSHED stream of type %s\n",
                  g_type_name(G_TYPE_FROM_INSTANCE(g_io_stream_get_output_stream(data->worker->stream))));
          _g_dbus_debug_print_unlock();
      }
  }
  g_assert(data->flushers != NULL);
  for (l = data->flushers; l != NULL; l = l->next) {
      FlushData *f = l->data;
      f->error = error != NULL ? g_error_copy(error) : NULL;
      g_mutex_lock(f->mutex);
      g_cond_signal(f->cond);
      g_mutex_unlock(f->mutex);
  }
  g_list_free(data->flushers);
  if (error != NULL) g_error_free(error);
  g_mutex_lock(data->worker->write_lock);
  data->worker->flush_pending = FALSE;
  g_mutex_unlock(data->worker->write_lock);
  maybe_write_next_message(data->worker);
  _g_dbus_worker_unref(data->worker);
  g_free(data);
}
static void message_written(GDBusWorker *worker, MessageToWriteData *message_data) {
  GList *l;
  GList *ll;
  GList *flushers;
  if (G_UNLIKELY(_g_dbus_debug_message())) {
      gchar *s;
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Message:\n  >>>> SENT D-Bus message (% "
              G_GSIZE_FORMAT " bytes)\n", message_data->blob_size);
      s = g_dbus_message_print(message_data->message, 2);
      g_print("%s", s);
      g_free(s);
      if (G_UNLIKELY(_g_dbus_debug_payload())) {
          s = _g_dbus_hexdump(message_data->blob, message_data->blob_size, 2);
          g_print("%s\n", s);
          g_free(s);
      }
      _g_dbus_debug_print_unlock();
  }
  flushers = NULL;
  g_mutex_lock(worker->write_lock);
  worker->write_num_messages_written += 1;
  for (l = worker->write_pending_flushes; l != NULL; l = ll) {
      FlushData *f = l->data;
      ll = l->next;
      if (f->number_to_wait_for == worker->write_num_messages_written) {
          flushers = g_list_append(flushers, f);
          worker->write_pending_flushes = g_list_delete_link(worker->write_pending_flushes, l);
      }
  }
  if (flushers != NULL) worker->flush_pending = TRUE;
  g_mutex_unlock(worker->write_lock);
  if (flushers != NULL) {
      FlushAsyncData *data;
      data = g_new0(FlushAsyncData, 1);
      data->worker = _g_dbus_worker_ref(worker);
      data->flushers = flushers;
      g_output_stream_flush_async(g_io_stream_get_output_stream(worker->stream), G_PRIORITY_DEFAULT, worker->cancellable, ostream_flush_cb, data);
  } else maybe_write_next_message(worker);
}
static void write_message_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  MessageToWriteData *data = user_data;
  GError *error;
  g_mutex_lock(data->worker->write_lock);
  data->worker->num_writes_pending -= 1;
  g_mutex_unlock(data->worker->write_lock);
  error = NULL;
  if (!write_message_finish(res, &error)) {
      _g_dbus_worker_emit_disconnected(data->worker, TRUE, error);
      g_error_free(error);
  }
  message_written(data->worker, data);
  message_to_write_data_free(data);
}
static void maybe_write_next_message(GDBusWorker *worker) {
  MessageToWriteData *data;
write_next:
  g_mutex_lock(worker->write_lock);
  data = g_queue_pop_head(worker->write_queue);
  if (data != NULL) worker->num_writes_pending += 1;
  g_mutex_unlock(worker->write_lock);
  if (data != NULL) {
      GDBusMessage *old_message;
      guchar *new_blob;
      gsize new_blob_size;
      GError *error;
      old_message = data->message;
      data->message = _g_dbus_worker_emit_message_about_to_be_sent(worker, data->message);
      if (data->message == old_message);
      else if (data->message == NULL) {
          g_mutex_lock(worker->write_lock);
          worker->num_writes_pending -= 1;
          g_mutex_unlock(worker->write_lock);
          message_to_write_data_free(data);
          goto write_next;
      } else {
          error = NULL;
          new_blob = g_dbus_message_to_blob(data->message, &new_blob_size, worker->capabilities, &error);
          if (new_blob == NULL) {
              g_warning("Error encoding GDBusMessage with serial %d altered by filter function: %s", g_dbus_message_get_serial(data->message),
                        error->message);
              g_error_free(error);
          } else {
              g_free(data->blob);
              data->blob = (gchar*)new_blob;
              data->blob_size = new_blob_size;
          }
      }
      write_message_async(worker, data, write_message_cb, data);
  }
}
static gboolean write_message_in_idle_cb(gpointer user_data) {
  GDBusWorker *worker = user_data;
  if (worker->num_writes_pending == 0 && !worker->flush_pending)
    maybe_write_next_message(worker);
  return FALSE;
}
void _g_dbus_worker_send_message(GDBusWorker *worker, GDBusMessage *message, gchar *blob, gsize blob_len) {
  MessageToWriteData *data;
  g_return_if_fail(G_IS_DBUS_MESSAGE(message));
  g_return_if_fail(blob != NULL);
  g_return_if_fail(blob_len > 16);
  data = g_new0(MessageToWriteData,1);
  data->worker = _g_dbus_worker_ref(worker);
  data->message = g_object_ref(message);
  data->blob = blob;
  data->blob_size = blob_len;
  g_mutex_lock(worker->write_lock);
  g_queue_push_tail(worker->write_queue, data);
  if (worker->num_writes_pending == 0) {
      GSource *idle_source;
      idle_source = g_idle_source_new();
      g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
      g_source_set_callback(idle_source, write_message_in_idle_cb, _g_dbus_worker_ref(worker), (GDestroyNotify)_g_dbus_worker_unref);
      g_source_attach(idle_source, shared_thread_data->context);
      g_source_unref(idle_source);
  }
  g_mutex_unlock(worker->write_lock);
}
static void _g_dbus_worker_thread_begin_func(gpointer user_data) {
  GDBusWorker *worker = user_data;
  worker->thread = g_thread_self();
  _g_dbus_worker_do_read(worker);
}
GDBusWorker *_g_dbus_worker_new(GIOStream *stream, GDBusCapabilityFlags capabilities, gboolean initially_frozen,
                                GDBusWorkerMessageReceivedCallback message_received_callback, GDBusWorkerMessageAboutToBeSentCallback message_about_to_be_sent_callback,
                                GDBusWorkerDisconnectedCallback disconnected_callback, gpointer user_data) {
  GDBusWorker *worker;
  g_return_val_if_fail(G_IS_IO_STREAM (stream), NULL);
  g_return_val_if_fail(message_received_callback != NULL, NULL);
  g_return_val_if_fail(message_about_to_be_sent_callback != NULL, NULL);
  g_return_val_if_fail(disconnected_callback != NULL, NULL);
  worker = g_new0(GDBusWorker,1);
  worker->ref_count = 1;
  worker->read_lock = g_mutex_new();
  worker->message_received_callback = message_received_callback;
  worker->message_about_to_be_sent_callback = message_about_to_be_sent_callback;
  worker->disconnected_callback = disconnected_callback;
  worker->user_data = user_data;
  worker->stream = g_object_ref(stream);
  worker->capabilities = capabilities;
  worker->cancellable = g_cancellable_new();
  worker->flush_pending = FALSE;
  worker->frozen = initially_frozen;
  worker->received_messages_while_frozen = g_queue_new();
  worker->write_lock = g_mutex_new();
  worker->write_queue = g_queue_new();
  if (G_IS_SOCKET_CONNECTION(worker->stream))worker->socket = g_socket_connection_get_socket(G_SOCKET_CONNECTION(worker->stream));
  _g_dbus_shared_thread_ref(_g_dbus_worker_thread_begin_func, worker);
  return worker;
}
void _g_dbus_worker_stop(GDBusWorker *worker) {
  worker->stopped = TRUE;
  g_cancellable_cancel(worker->cancellable);
  _g_dbus_worker_unref(worker);
}
gboolean _g_dbus_worker_flush_sync(GDBusWorker *worker, GCancellable *cancellable, GError **error) {
  gboolean ret;
  FlushData *data;
  data = NULL;
  ret = TRUE;
  g_mutex_lock(worker->write_lock);
  if (g_queue_get_length(worker->write_queue) > 0) {
      data = g_new0(FlushData, 1);
      data->mutex = g_mutex_new();
      data->cond = g_cond_new();
      data->number_to_wait_for = worker->write_num_messages_written + g_queue_get_length(worker->write_queue);
      g_mutex_lock(data->mutex);
      worker->write_pending_flushes = g_list_prepend(worker->write_pending_flushes, data);
  }
  g_mutex_unlock(worker->write_lock);
  if (data != NULL) {
      g_cond_wait(data->cond, data->mutex);
      g_mutex_unlock(data->mutex);
      g_cond_free(data->cond);
      g_mutex_free(data->mutex);
      if (data->error != NULL) {
          ret = FALSE;
          g_propagate_error(error, data->error);
      }
      g_free(data);
  }
  return ret;
}
#define G_DBUS_DEBUG_AUTHENTICATION  (1<<0)
#define G_DBUS_DEBUG_TRANSPORT  (1<<1)
#define G_DBUS_DEBUG_MESSAGE  (1<<2)
#define G_DBUS_DEBUG_PAYLOAD  (1<<3)
#define G_DBUS_DEBUG_CALL  (1<<4)
#define G_DBUS_DEBUG_SIGNAL  (1<<5)
#define G_DBUS_DEBUG_INCOMING  (1<<6)
#define G_DBUS_DEBUG_RETURN  (1<<7)
#define G_DBUS_DEBUG_EMISSION  (1<<8)
#define G_DBUS_DEBUG_ADDRESS  (1<<9)
static gint _gdbus_debug_flags = 0;
gboolean _g_dbus_debug_authentication(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_AUTHENTICATION) != 0;
}
gboolean _g_dbus_debug_transport(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_TRANSPORT) != 0;
}
gboolean _g_dbus_debug_message(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_MESSAGE) != 0;
}
gboolean _g_dbus_debug_payload(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_PAYLOAD) != 0;
}
gboolean _g_dbus_debug_call(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_CALL) != 0;
}
gboolean _g_dbus_debug_signal(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_SIGNAL) != 0;
}
gboolean _g_dbus_debug_incoming(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_INCOMING) != 0;
}
gboolean _g_dbus_debug_return(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_RETURN) != 0;
}
gboolean _g_dbus_debug_emission(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_EMISSION) != 0;
}
gboolean _g_dbus_debug_address(void) {
  _g_dbus_initialize();
  return (_gdbus_debug_flags & G_DBUS_DEBUG_ADDRESS) != 0;
}
G_LOCK_DEFINE_STATIC(print_lock);
void _g_dbus_debug_print_lock(void) {
  G_LOCK(print_lock);
}
void _g_dbus_debug_print_unlock(void) {
  G_UNLOCK(print_lock);
}
void _g_dbus_initialize(void) {
  static volatile gsize initialized = 0;
  if (g_once_init_enter(&initialized)) {
      volatile GQuark g_dbus_error_domain;
      const gchar *debug;
      g_dbus_error_domain = G_DBUS_ERROR;
      debug = g_getenv("G_DBUS_DEBUG");
      if (debug != NULL) {
          const GDebugKey keys[] = {
              { "authentication", G_DBUS_DEBUG_AUTHENTICATION },
              { "transport",      G_DBUS_DEBUG_TRANSPORT      },
              { "message",        G_DBUS_DEBUG_MESSAGE        },
              { "payload",        G_DBUS_DEBUG_PAYLOAD        },
              { "call",           G_DBUS_DEBUG_CALL           },
              { "signal",         G_DBUS_DEBUG_SIGNAL         },
              { "incoming",       G_DBUS_DEBUG_INCOMING       },
              { "return",         G_DBUS_DEBUG_RETURN         },
              { "emission",       G_DBUS_DEBUG_EMISSION       },
              { "address",        G_DBUS_DEBUG_ADDRESS        }
          };
          _gdbus_debug_flags = g_parse_debug_string(debug, keys, G_N_ELEMENTS(keys));
          if (_gdbus_debug_flags & G_DBUS_DEBUG_PAYLOAD) _gdbus_debug_flags |= G_DBUS_DEBUG_MESSAGE;
      }
      g_once_init_leave(&initialized, 1);
  }
}
GVariantType *_g_dbus_compute_complete_signature(GDBusArgInfo **args) {
  const GVariantType *arg_types[256];
  guint n;
  if (args)
      for (n = 0; args[n] != NULL; n++) {
          g_assert(n < 256);
          arg_types[n] = G_VARIANT_TYPE(args[n]->signature);
          if G_UNLIKELY(arg_types[n] == NULL) return NULL;
      }
  else n = 0;
  return g_variant_type_new_tuple(arg_types, n);
}
#ifndef G_OS_WIN32
extern BOOL WINAPI ConvertSidToStringSidA(PSID Sid, LPSTR *StringSid);
gchar *_g_dbus_win32_get_user_sid(void) {
  HANDLE h;
  TOKEN_USER *user;
  DWORD token_information_len;
  PSID psid;
  gchar *sid;
  gchar *ret;
  ret = NULL;
  user = NULL;
  h = INVALID_HANDLE_VALUE;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &h)) {
      g_warning("OpenProcessToken failed with error code %d", (gint)GetLastError());
      goto out;
  }
  token_information_len = 0;
  if (!GetTokenInformation(h, TokenUser, NULL, 0, &token_information_len)) {
      if (GetLastError () != ERROR_INSUFFICIENT_BUFFER) {
          g_warning("GetTokenInformation() failed with error code %d", (gint)GetLastError());
          goto out;
      }
  }
  user = g_malloc(token_information_len);
  if (!GetTokenInformation(h, TokenUser, user, token_information_len, &token_information_len)) {
      g_warning("GetTokenInformation() failed with error code %d", (gint)GetLastError());
      goto out;
  }
  psid = user->User.Sid;
  if (!IsValidSid(psid)) {
      g_warning("Invalid SID");
      goto out;
  }
  if (!ConvertSidToStringSidA(psid, &sid)) {
      g_warning("Invalid SID");
      goto out;
  }
  ret = g_strdup(sid);
  LocalFree(sid);
out:
  g_free(user);
  if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
  return ret;
}
#endif
gchar *_g_dbus_get_machine_id(GError **error) {
  gchar *ret;
  ret = NULL;
  if (!g_file_get_contents("/var/lib/dbus/machine-id", &ret, NULL, error)) g_prefix_error(error,"Unable to load /var/lib/dbus/machine-id: ");
  else { g_strstrip(ret); }
  return ret;
}
gchar *_g_dbus_enum_to_string(GType enum_type, gint value) {
  gchar *ret;
  GEnumClass *klass;
  GEnumValue *enum_value;
  klass = g_type_class_ref(enum_type);
  enum_value = g_enum_get_value(klass, value);
  if (enum_value != NULL) ret = g_strdup(enum_value->value_nick);
  else ret = g_strdup_printf("unknown (value %d)", value);
  g_type_class_unref(klass);
  return ret;
}
static void write_message_print_transport_debug(gssize bytes_written, MessageToWriteData *data) {
  if (G_LIKELY(!_g_dbus_debug_transport())) return;
  _g_dbus_debug_print_lock();
  g_print("========================================================================\nGDBus-debug:Transport:\n  >>>> WROTE %" G_GSIZE_FORMAT
          " bytes of message with serial %d and\n       size %" G_GSIZE_FORMAT " from offset %" G_GSIZE_FORMAT " on a %s\n", bytes_written,
          g_dbus_message_get_serial(data->message), data->blob_size, data->total_written,
          g_type_name(G_TYPE_FROM_INSTANCE(g_io_stream_get_output_stream(data->worker->stream))));
  _g_dbus_debug_print_unlock();
}
static void read_message_print_transport_debug(gssize bytes_read, GDBusWorker *worker) {
  gsize size;
  gint32 serial;
  gint32 message_length;
  if (G_LIKELY(!_g_dbus_debug_transport())) return;
  size = bytes_read + worker->read_buffer_cur_size;
  serial = 0;
  message_length = 0;
  if (size >= 16) message_length = g_dbus_message_bytes_needed((guchar*)worker->read_buffer, size, NULL);
  if (size >= 1) {
      switch (worker->read_buffer[0]) {
          case 'l':
              if (size >= 12) serial = GUINT32_FROM_LE(((guint32*)worker->read_buffer)[2]);
              break;
          case 'B':
              if (size >= 12) serial = GUINT32_FROM_BE(((guint32*)worker->read_buffer)[2]);
              break;
          default: return;
      }
  }
  _g_dbus_debug_print_lock();
  g_print("========================================================================\nGDBus-debug:Transport:\n  <<<< READ %" G_GSIZE_FORMAT
          " bytes of message with serial %d and\n       size %d to offset %" G_GSIZE_FORMAT " from a %s\n", bytes_read, serial, message_length,
          worker->read_buffer_cur_size, g_type_name(G_TYPE_FROM_INSTANCE(g_io_stream_get_input_stream(worker->stream))));
  _g_dbus_debug_print_unlock();
}