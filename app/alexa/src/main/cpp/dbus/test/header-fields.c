#include "../../glib/glib.h"
#include "../../gio/gdbuserror.h"
#include "../config.h"
#include "../dbus.h"
#include "../dbus-internals.h"
#include "../dbus-marshal-recursive.h"
#include "../dbus-message-private.h"
#include "../dbus-string.h"
#include "../dbus-test-tap.h"
#include "test-utils-glib.h"

typedef struct {
  const gchar *mode;
  TestMainContext *ctx;
  DBusConnection *left_conn;
  DBusConnection *right_conn;
  GPid daemon_pid;
  gchar *address;
  GQueue held_messages;
  gboolean skip;
} Fixture;
static DBusHandlerResult hold_filter(DBusConnection *connection, DBusMessage *message, void *user_data) {
  Fixture *f = user_data;
  if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_METHOD_CALL) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  g_queue_push_tail(&f->held_messages, dbus_message_ref(message));
  return DBUS_HANDLER_RESULT_HANDLED;
}
static void setup_dbus_daemon(Fixture *f, gconstpointer context G_GNUC_UNUSED) {
  f->address = test_get_dbus_daemon(NULL, TEST_USER_ME, NULL, &f->daemon_pid);
  if (f->address == NULL) {
      f->skip = TRUE;
      return;
  }
}
static void teardown_dbus_daemon (Fixture *f, gconstpointer context G_GNUC_UNUSED) {
  if (f->daemon_pid != 0) {
      test_kill_pid(f->daemon_pid);
      g_spawn_close_pid(f->daemon_pid);
      f->daemon_pid = 0;
  }
  g_free(f->address);
}
#define BYTE_ORDER_OFFSET  0
#define VERSION_OFFSET  3
#define FIELDS_ARRAY_LENGTH_OFFSET  12
#define FIRST_FIELD_OFFSET  16
#define FIELDS_ARRAY_SIGNATURE_OFFSET  6
#define NOT_A_HEADER_FIELD  123
_DBUS_STRING_DEFINE_STATIC(_dbus_header_signature_str, DBUS_HEADER_SIGNATURE);
static void string_overwrite_n(DBusString *str, int start, const void *bytes, int len) {
  unsigned char *data = _dbus_string_get_udata_len(str, start, len);
  g_assert(data != NULL);
  memcpy(data, bytes, len);
}
static void lsteal_reply_cb(DBusPendingCall *pc, void *data) {
  DBusMessage **message_p = data;
  g_assert(message_p != NULL);
  g_assert(*message_p == NULL);
  *message_p = dbus_pending_call_steal_reply(pc);
  g_assert(*message_p != NULL);
}
static dbus_bool_t test_weird_header_field(void *user_data, dbus_bool_t have_memory) {
  Fixture *f = user_data;
  const char *body = "hello";
  const char *new_body = NULL;
  DBusError error = DBUS_ERROR_INIT;
  DBusMessage *original = NULL;
  DBusMessage *modified = NULL;
  DBusMessage *filtered = NULL;
  DBusMessage *relayed = NULL;
  DBusMessage *reply = NULL;
  DBusPendingCall *pc = NULL;
  char *blob = NULL;
  int blob_len;
  DBusString modified_blob = _DBUS_STRING_INIT_INVALID;
  unsigned char weird_header[8] = { NOT_A_HEADER_FIELD, 1, 'u', '\0', '\x12', '\x34', '\x56', '\x78' };
  int bytes_needed;
  DBusTypeReader reader;
  DBusTypeReader array;
  gboolean added_hold_filter = FALSE;
  GError *gerror = NULL;
  if (f->skip) return TRUE;
  g_assert_cmpint(_dbus_get_malloc_blocks_outstanding(), ==, 0);
  f->ctx = test_main_context_try_get();
  if (f->ctx == NULL) {
      //g_assert_false(have_memory);
      goto out;
  }
  f->left_conn = test_try_connect_to_bus(f->ctx, f->address, &gerror);
  if (f->left_conn == NULL) {
      g_assert_error(gerror, G_DBUS_ERROR, G_DBUS_ERROR_NO_MEMORY);
      //g_assert_false(have_memory);
      goto out;
  }
  f->right_conn = test_try_connect_to_bus(f->ctx, f->address, &gerror);
  if (f->right_conn == NULL) {
      g_assert_error(gerror, G_DBUS_ERROR, G_DBUS_ERROR_NO_MEMORY);
      //g_assert_false(have_memory);
      goto out;
  }
  if (!dbus_connection_add_filter(f->right_conn, hold_filter, f, NULL)) {
      //g_assert_false(have_memory);
      goto out;
  }
  added_hold_filter = TRUE;
  original = dbus_message_new_method_call(dbus_bus_get_unique_name(f->right_conn),"/com/example/Path","com.example.Interface","Method");
  if (original == NULL || !dbus_message_append_args(original, DBUS_TYPE_STRING, &body, DBUS_TYPE_INVALID)) {
      //g_assert_false(have_memory);
      goto out;
  }
  dbus_message_set_serial(original, 42);
  if (!dbus_message_marshal(original, &blob, &blob_len)) {
      //g_assert_false(have_memory);
      goto out;
  }
  if (!_dbus_string_init_preallocated(&modified_blob, blob_len + 8) || !_dbus_string_append_len(&modified_blob, blob, blob_len)) {
      //g_assert_false(have_memory);
      goto out;
  }
  g_assert_cmpint(blob_len, >, FIRST_FIELD_OFFSET);
  g_assert_cmpint(blob[VERSION_OFFSET], ==, 1);
  g_assert_cmpint(blob[BYTE_ORDER_OFFSET], ==, DBUS_COMPILER_BYTE_ORDER);
  if (f->mode == NULL);
  else if (g_str_equal(f->mode, "change") || g_str_equal(f->mode, "multi")) {
      _dbus_type_reader_init(&reader, DBUS_COMPILER_BYTE_ORDER, &_dbus_header_signature_str, FIELDS_ARRAY_SIGNATURE_OFFSET, &modified_blob, FIELDS_ARRAY_LENGTH_OFFSET);
      _dbus_type_reader_recurse(&reader, &array);
      while(_dbus_type_reader_get_current_type(&array) != DBUS_TYPE_INVALID) {
          DBusTypeReader sub;
          unsigned char field_code;
          _dbus_type_reader_recurse(&array, &sub);
          g_assert_cmpint(_dbus_type_reader_get_current_type(&sub), ==, DBUS_TYPE_BYTE);
          _dbus_type_reader_read_basic(&sub, &field_code);
          if (field_code == DBUS_HEADER_FIELD_INTERFACE) {
              //_dbus_string_set_byte(&modified_blob,_dbus_type_reader_get_value_pos&sub), NOT_A_HEADER_FIELD);
              break;
          }
          _dbus_type_reader_next(&array);
      }
      if (g_str_equal(f->mode, "multi")) {
          dbus_uint32_t header_fields_length;
          unsigned int i;
          memcpy(&header_fields_length, &blob[FIELDS_ARRAY_LENGTH_OFFSET], 4);
          for (i = 1; i <= 2; i++) {
              weird_header[0] = NOT_A_HEADER_FIELD - i;
              if (!_dbus_string_insert_8_aligned(&modified_blob, FIRST_FIELD_OFFSET, weird_header)) {
                  //g_assert_false(have_memory);
                  goto out;
              }
              header_fields_length += 8;
          }
          header_fields_length = _DBUS_ALIGN_VALUE(header_fields_length, 8);
          g_assert_cmpint(header_fields_length % 8, ==, 0);
          for (i = 1; i <= 2; i++) {
              weird_header[0] = NOT_A_HEADER_FIELD + i;
              if (!_dbus_string_insert_8_aligned(&modified_blob,(FIRST_FIELD_OFFSET + header_fields_length), weird_header)) {
                  //g_assert_false(have_memory);
                  goto out;
              }
              header_fields_length += 8;
          }
          string_overwrite_n(&modified_blob, FIELDS_ARRAY_LENGTH_OFFSET, &header_fields_length, 4);
      }
  } else if (g_str_equal(f->mode, "prepend")) {
      dbus_uint32_t header_fields_length;
      memcpy(&header_fields_length, &blob[FIELDS_ARRAY_LENGTH_OFFSET], 4);
      if (!_dbus_string_insert_8_aligned(&modified_blob, FIRST_FIELD_OFFSET, weird_header)) {
          //g_assert_false(have_memory);
          goto out;
      }
      header_fields_length += 8;
      string_overwrite_n(&modified_blob, FIELDS_ARRAY_LENGTH_OFFSET, &header_fields_length, 4);
  } else if (g_str_equal(f->mode, "append")) {
      dbus_uint32_t header_fields_length;
      memcpy(&header_fields_length, &blob[FIELDS_ARRAY_LENGTH_OFFSET], 4);
      header_fields_length = _DBUS_ALIGN_VALUE(header_fields_length, 8);
      g_assert_cmpint(header_fields_length % 8, ==, 0);
      if (!_dbus_string_insert_8_aligned(&modified_blob,(FIRST_FIELD_OFFSET + header_fields_length), weird_header)) {
          //g_assert_false(have_memory);
          goto out;
      }
      header_fields_length += 8;
      string_overwrite_n(&modified_blob, FIELDS_ARRAY_LENGTH_OFFSET, &header_fields_length, 4);
  } else { g_assert_not_reached(); }
  bytes_needed = dbus_message_demarshal_bytes_needed(_dbus_string_get_const_data(&modified_blob), _dbus_string_get_length(&modified_blob));
  if (f->mode == NULL || g_str_equal(f->mode, "change")) { g_assert_cmpint(bytes_needed, ==, blob_len); }
  else if (g_str_equal(f->mode, "multi")) { g_assert_cmpint(bytes_needed, ==, blob_len + 32); }
  else { g_assert_cmpint(bytes_needed, ==, blob_len + 8); }
  g_assert_cmpint(_dbus_string_get_length (&modified_blob), ==, bytes_needed);
  modified = dbus_message_demarshal(_dbus_string_get_const_data(&modified_blob), _dbus_string_get_length(&modified_blob), &error);
  if (modified == NULL) {
      g_assert_cmpstr(error.name, ==, DBUS_ERROR_NO_MEMORY);
      //g_assert_false(have_memory);
      goto out;
  }
  g_assert_cmpint(dbus_message_get_type(modified), ==,dbus_message_get_type(original));
  g_assert_cmpstr(dbus_message_get_path(modified), ==,dbus_message_get_path(original));
  g_assert_cmpstr(dbus_message_get_member(modified), ==,dbus_message_get_member(original));
  g_assert_cmpstr(dbus_message_get_error_name(modified), ==,dbus_message_get_error_name(original));
  g_assert_cmpstr(dbus_message_get_destination(modified), ==,dbus_message_get_destination(original));
  g_assert_cmpstr(dbus_message_get_sender(modified), ==,dbus_message_get_sender(original));
  g_assert_cmpstr(dbus_message_get_signature(modified), ==,dbus_message_get_signature(original));
  g_assert_cmpint(dbus_message_get_no_reply(modified), ==,dbus_message_get_no_reply(original));
  g_assert_cmpint(dbus_message_get_serial(modified), ==,dbus_message_get_serial(original));
  g_assert_cmpint(dbus_message_get_reply_serial(modified), ==,dbus_message_get_reply_serial(original));
  g_assert_cmpint(dbus_message_get_auto_start(modified), ==,dbus_message_get_auto_start(original));
  g_assert_cmpint(dbus_message_get_allow_interactive_authorization(modified), ==,dbus_message_get_allow_interactive_authorization(original));
  if (dbus_message_get_args(modified, &error, DBUS_TYPE_STRING, &new_body, DBUS_TYPE_INVALID)) { g_assert_cmpstr(new_body, ==, body); }
  else {
      g_assert_cmpstr(error.name, ==, DBUS_ERROR_NO_MEMORY);
      //g_assert_false(have_memory);
      goto out;
  }
  if (f->mode != NULL && (g_str_equal(f->mode, "change") || g_str_equal(f->mode, "multi"))) {
      g_assert_cmpstr(dbus_message_get_interface(modified), ==, NULL);
  } else { g_assert_cmpstr(dbus_message_get_interface(modified), ==,dbus_message_get_interface(original)); }
  filtered = dbus_message_demarshal(_dbus_string_get_const_data(&modified_blob), _dbus_string_get_length(&modified_blob), &error);
  if (filtered == NULL) {
      g_assert_cmpstr(error.name, ==, DBUS_ERROR_NO_MEMORY);
      //g_assert_false(have_memory);
      goto out;
  }
  if (!_dbus_message_remove_unknown_fields(filtered)) {
      //g_assert_false(have_memory);
      goto out;
  }
  g_assert_cmpint(dbus_message_get_type(filtered), ==, dbus_message_get_type(modified));
  g_assert_cmpstr(dbus_message_get_path(filtered), ==,dbus_message_get_path(modified));
  g_assert_cmpstr(dbus_message_get_member(filtered), ==,dbus_message_get_member(modified));
  g_assert_cmpstr(dbus_message_get_error_name(filtered), ==,dbus_message_get_error_name(modified));
  g_assert_cmpstr(dbus_message_get_destination(filtered), ==,dbus_message_get_destination(modified));
  g_assert_cmpstr(dbus_message_get_sender(filtered), ==,dbus_message_get_sender(modified));
  g_assert_cmpstr(dbus_message_get_signature(filtered), ==,dbus_message_get_signature(modified));
  g_assert_cmpint(dbus_message_get_no_reply(filtered), ==,dbus_message_get_no_reply(modified));
  g_assert_cmpint(dbus_message_get_serial(filtered), ==,dbus_message_get_serial(modified));
  g_assert_cmpint(dbus_message_get_reply_serial(filtered), ==,dbus_message_get_reply_serial(modified));
  g_assert_cmpint(dbus_message_get_auto_start(filtered), ==,dbus_message_get_auto_start(modified));
  g_assert_cmpint(dbus_message_get_allow_interactive_authorization(modified), ==,dbus_message_get_allow_interactive_authorization(original));
  if (dbus_message_get_args(filtered, &error, DBUS_TYPE_STRING, &new_body, DBUS_TYPE_INVALID)) { g_assert_cmpstr(new_body, ==, body); }
  else {
      g_assert_cmpstr(error.name, ==, DBUS_ERROR_NO_MEMORY);
      //g_assert_false(have_memory);
      goto out;
  }
  _dbus_type_reader_init(&reader, DBUS_COMPILER_BYTE_ORDER, &_dbus_header_signature_str, FIELDS_ARRAY_SIGNATURE_OFFSET, &filtered->header.data, FIELDS_ARRAY_LENGTH_OFFSET);
  _dbus_type_reader_recurse(&reader, &array);
  while(_dbus_type_reader_get_current_type(&array) != DBUS_TYPE_INVALID) {
      DBusTypeReader sub;
      unsigned char field_code;
      _dbus_type_reader_recurse(&array, &sub);
      g_assert_cmpint(_dbus_type_reader_get_current_type(&sub), ==, DBUS_TYPE_BYTE);
      _dbus_type_reader_read_basic(&sub, &field_code);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD - 2);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD - 1);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD + 1);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD + 2);
      _dbus_type_reader_next(&array);
  }
  if (!dbus_connection_send_with_reply(f->left_conn, modified, &pc, -1)) {
      //g_assert_false(have_memory);
      goto out;
  }
  /*if (dbus_pending_call_get_completed(pc)) steal_reply_cb(pc, &reply);
  else if (!dbus_pending_call_set_notify(pc, steal_reply_cb, &reply, NULL)) {
      dbus_pending_call_cancel(pc);
      g_assert_false(have_memory);
      goto out;
  }*/
  while(g_queue_get_length(&f->held_messages) < 1 && reply == NULL) test_main_context_iterate(f->ctx, TRUE);
  if (reply != NULL) {
      dbus_set_error_from_message(&error, reply);
      g_assert_cmpstr(error.name, ==, DBUS_ERROR_NO_MEMORY);
      //g_assert_false(have_memory);
      goto out;
  }
  relayed = g_queue_pop_head(&f->held_messages);
  g_assert_cmpint(g_queue_get_length(&f->held_messages), ==, 0);
  g_assert_cmpint(dbus_message_get_type(relayed), ==,dbus_message_get_type(modified));
  g_assert_cmpstr(dbus_message_get_path(relayed), ==,dbus_message_get_path(modified));
  g_assert_cmpstr(dbus_message_get_member(relayed), ==,dbus_message_get_member(modified));
  g_assert_cmpstr(dbus_message_get_error_name(relayed), ==,dbus_message_get_error_name(modified));
  g_assert_cmpstr(dbus_message_get_destination(relayed), ==,dbus_message_get_destination(modified));
  g_assert_cmpstr(dbus_message_get_sender(relayed), ==,dbus_bus_get_unique_name(f->left_conn));
  g_assert_cmpstr(dbus_message_get_signature(relayed), ==,dbus_message_get_signature(modified));
  g_assert_cmpint(dbus_message_get_no_reply(relayed), ==,dbus_message_get_no_reply(modified));
  g_assert_cmpint(dbus_message_get_serial(relayed), !=, 0);
  g_assert_cmpint(dbus_message_get_reply_serial(relayed), ==,dbus_message_get_reply_serial(modified));
  g_assert_cmpint(dbus_message_get_auto_start(relayed), ==,dbus_message_get_auto_start(modified));
  g_assert_cmpint(dbus_message_get_allow_interactive_authorization(modified), ==,dbus_message_get_allow_interactive_authorization(original));
  if (dbus_message_get_args(relayed, &error, DBUS_TYPE_STRING, &new_body, DBUS_TYPE_INVALID)) {
      g_assert_cmpstr(new_body, ==, body);
  } else {
      g_assert_cmpstr(error.name, ==, DBUS_ERROR_NO_MEMORY);
      //g_assert_false(have_memory);
      goto out;
  }
  _dbus_type_reader_init(&reader, DBUS_COMPILER_BYTE_ORDER, &_dbus_header_signature_str, FIELDS_ARRAY_SIGNATURE_OFFSET, &relayed->header.data, FIELDS_ARRAY_LENGTH_OFFSET);
  _dbus_type_reader_recurse(&reader, &array);
  while(_dbus_type_reader_get_current_type(&array) != DBUS_TYPE_INVALID) {
      DBusTypeReader sub;
      unsigned char field_code;
      _dbus_type_reader_recurse(&array, &sub);
      g_assert_cmpint(_dbus_type_reader_get_current_type(&sub), ==, DBUS_TYPE_BYTE);
      _dbus_type_reader_read_basic(&sub, &field_code);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD - 2);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD - 1);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD + 1);
      g_assert_cmpuint(field_code, !=, NOT_A_HEADER_FIELD + 2);
      _dbus_type_reader_next(&array);
  }
out:
  g_clear_error(&gerror);
  _dbus_string_free(&modified_blob);
  dbus_free(blob);
  if (pc != NULL) dbus_pending_call_cancel(pc);
  dbus_clear_pending_call(&pc);
  dbus_clear_message(&reply);
  dbus_clear_message(&relayed);
  dbus_clear_message(&filtered);
  dbus_clear_message(&modified);
  dbus_clear_message(&original);
  dbus_error_free(&error);
  if (f->left_conn != NULL) {
      dbus_connection_close(f->left_conn);
      test_connection_shutdown(f->ctx, f->left_conn);
  }
  if (f->right_conn != NULL) {
      if (added_hold_filter) dbus_connection_remove_filter(f->right_conn, hold_filter, f);
      g_queue_foreach(&f->held_messages, (GFunc)dbus_message_unref, NULL);
      g_queue_clear(&f->held_messages);
      dbus_connection_close(f->right_conn);
      test_connection_shutdown(f->ctx, f->right_conn);
  }
  dbus_clear_connection(&f->left_conn);
  dbus_clear_connection(&f->right_conn);
  //g_clear_pointer(&f->ctx, test_main_context_unref);
  dbus_shutdown();
  g_assert_cmpint(_dbus_get_malloc_blocks_outstanding(), ==, 0);
  return TRUE;//!g_test_failed ();
}
typedef struct {
  const gchar *name;
  DBusTestMemoryFunction function;
  const gchar *mode;
} OOMTestCase;
static void test_oom_wrapper(Fixture *f, gconstpointer data) {
  const OOMTestCase *test = data;
  f->mode = test->mode;
  if (g_test_slow()) test_timeout_reset(30);
  else test_timeout_reset(1);
  if (!_dbus_test_oom_handling(test->name, test->function, f)) {
      g_test_message("OOM test failed");
      //g_test_fail();
  }
}
static GQueue *test_cases_to_free = NULL;
static void add_oom_test(const gchar *name, DBusTestMemoryFunction function, const gchar *mode) {
  OOMTestCase *test_case = g_new0 (OOMTestCase, 1);
  test_case->name = name;
  test_case->function = function;
  test_case->mode = mode;
  g_test_add(name, Fixture, test_case, setup_dbus_daemon, test_oom_wrapper, teardown_dbus_daemon);
  g_queue_push_tail(test_cases_to_free, test_case);
}
int main(int argc, char **argv) {
  int ret;
  test_init(&argc, &argv);
  if (g_getenv("DBUS_TEST_MALLOC_FAILURES") == NULL) {
      if (!g_test_slow()) g_setenv("DBUS_TEST_MALLOC_FAILURES", "2", TRUE);
  }
  test_cases_to_free = g_queue_new();
  add_oom_test("/message/weird-header-field/none", test_weird_header_field,NULL);
  add_oom_test("/message/weird-header-field/append", test_weird_header_field,"append");
  add_oom_test("/message/weird-header-field/change", test_weird_header_field,"change");
  add_oom_test("/message/weird-header-field/prepend", test_weird_header_field,"prepend");
  add_oom_test("/message/weird-header-field/multi", test_weird_header_field,"multi");
  ret = g_test_run();
  //g_queue_free_full(test_cases_to_free, g_free);
  return ret;
}