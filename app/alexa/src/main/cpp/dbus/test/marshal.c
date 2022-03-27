#include <string.h>
#include "../../glib/glib.h"
#include "../config.h"
#include "../dbus.h"
#include "test-utils-glib.h"

typedef struct {
  DBusError e;
} Fixture;
static void assert_no_error(const DBusError *e) {
  if (G_UNLIKELY(dbus_error_is_set(e))) g_error("expected success but got error: %s: %s", e->name, e->message);
}
static void setup(Fixture *f, gconstpointer arg G_GNUC_UNUSED) {
  dbus_error_init(&f->e);
}
static guint32 get_uint32(const gchar *blob, gsize offset, char endian) {
  if (endian == 'l') return blob[offset] | (blob[offset + 1] << 8) | (blob[offset + 2] << 16) | (blob[offset + 3] << 24);
  else if (endian == 'B') return (blob[offset] << 24) | (blob[offset + 1] << 16) | (blob[offset + 2] << 8) | blob[offset + 3];
  else { g_assert_not_reached(); }
}
#define BLOB_LENGTH  (sizeof(le_blob) - 1)
#define OFFSET_BODY_LENGTH  (4)
#define OFFSET_SERIAL  (8)
const gchar le_blob[] = "l\2\2\1\4\0\0\0\x78\x56\x34\x12\x0f\0\0\0\5\1u\0\x12\xef\xcd\xab\x08\1g\0\1u\0\0\xef\xbe\xad\xde";
const gchar be_blob[] = "B\2\2\1\0\0\0\4\x12\x34\x56\x78\0\0\0\x0f\5\1u\0\xab\xcd\xef\x12\x08\1g\0\1u\0\0\xde\xad\xbe\xef";
static void
test_endian(Fixture *f, gconstpointer arg) {
  const gchar *blob = arg;
  char *output;
  DBusMessage *m;
  int len;
  dbus_uint32_t u;
  dbus_bool_t ok;
  g_assert_cmpuint((guint)sizeof(le_blob), ==, (guint)sizeof(be_blob));
  g_assert_cmpuint(get_uint32(blob, OFFSET_BODY_LENGTH, blob[0]), ==, 4);
  g_assert_cmpuint(get_uint32(blob, OFFSET_SERIAL, blob[0]), ==,0x12345678u);
  len = dbus_message_demarshal_bytes_needed(blob, sizeof(le_blob));
  g_assert_cmpint(len, ==, BLOB_LENGTH);
  m = dbus_message_demarshal(blob, sizeof(le_blob), &f->e);
  assert_no_error(&f->e);
  g_assert(m != NULL);
  g_assert_cmpuint(dbus_message_get_serial(m), ==, 0x12345678u);
  g_assert_cmpuint(dbus_message_get_reply_serial(m), ==, 0xabcdef12u);
  g_assert_cmpstr(dbus_message_get_signature(m), ==, "u");
  u = 0xdecafbadu;
  ok = dbus_message_append_args(m, DBUS_TYPE_UINT32, &u, DBUS_TYPE_INVALID);
  g_assert(ok);
  dbus_message_marshal(m, &output, &len);
  g_assert(output[0] == 'l' || output[0] == 'B');
  g_assert_cmpint(output[1], ==, blob[1]);
  g_assert_cmpint(output[2], ==, blob[2]);
  g_assert_cmpint(output[3], ==, blob[3]);
  g_assert_cmpuint(get_uint32(output, OFFSET_BODY_LENGTH, output[0]), ==, 8);
  g_assert_cmpuint(get_uint32(output, OFFSET_SERIAL, output[0]), ==,0x12345678u);
  g_assert_cmpint(len, ==, BLOB_LENGTH + 4);
}
static void test_needed(Fixture *f, gconstpointer arg) {
  const gchar *blob = arg;
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, 0), ==, 0);
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, 15), ==, 0);
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, 16), ==, BLOB_LENGTH);
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, 31), ==, BLOB_LENGTH);
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, 32), ==, BLOB_LENGTH);
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, 33), ==, BLOB_LENGTH);
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, BLOB_LENGTH - 1), ==, BLOB_LENGTH);
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, BLOB_LENGTH), ==, BLOB_LENGTH);
  g_assert_cmpint(dbus_message_demarshal_bytes_needed(blob, sizeof(be_blob)), ==, BLOB_LENGTH);
}
static void teardown(Fixture *f, gconstpointer arg G_GNUC_UNUSED) {
  dbus_error_free (&f->e);
}
int main(int argc, char **argv) {
  int ret;
  char *aligned_le_blob;
  char *aligned_be_blob;
  test_init (&argc, &argv);
  aligned_le_blob = g_malloc(sizeof (le_blob));
  memcpy(aligned_le_blob, le_blob, sizeof(le_blob));
  aligned_be_blob = g_malloc(sizeof(be_blob));
  memcpy(aligned_be_blob, be_blob, sizeof(be_blob));
  g_test_add("/demarshal/le", Fixture, aligned_le_blob, setup, test_endian, teardown);
  g_test_add("/demarshal/be", Fixture, aligned_be_blob, setup, test_endian, teardown);
  g_test_add("/demarshal/needed/le", Fixture, aligned_le_blob, setup, test_needed, teardown);
  g_test_add("/demarshal/needed/be", Fixture, aligned_be_blob, setup, test_needed, teardown);
  ret = g_test_run();
  g_free(aligned_le_blob);
  g_free(aligned_be_blob);
  return ret;
}