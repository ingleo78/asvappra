#include "../glib/glibintl.h"
#include "config.h"
#include "gsocketaddressenumerator.h"
#include "gsimpleasyncresult.h"

G_DEFINE_ABSTRACT_TYPE (GSocketAddressEnumerator, g_socket_address_enumerator, G_TYPE_OBJECT);
static void g_socket_address_enumerator_real_next_async(GSocketAddressEnumerator *enumerator, GCancellable *cancellable, GAsyncReadyCallback callback,
								                        gpointer user_data);
static GSocketAddress *g_socket_address_enumerator_real_next_finish(GSocketAddressEnumerator *enumerator, GAsyncResult *result, GError **error);
static void
g_socket_address_enumerator_init (GSocketAddressEnumerator *enumerator) {}
static void g_socket_address_enumerator_class_init(GSocketAddressEnumeratorClass *enumerator_class) {
  enumerator_class->next_async = g_socket_address_enumerator_real_next_async;
  enumerator_class->next_finish = g_socket_address_enumerator_real_next_finish;
}
GSocketAddress *g_socket_address_enumerator_next(GSocketAddressEnumerator *enumerator, GCancellable *cancellable, GError **error) {
  GSocketAddressEnumeratorClass *klass;
  g_return_val_if_fail(G_IS_SOCKET_ADDRESS_ENUMERATOR(enumerator), NULL);
  klass = G_SOCKET_ADDRESS_ENUMERATOR_GET_CLASS(enumerator);
  return (*klass->next)(enumerator, cancellable, error);
}
static void g_socket_address_enumerator_real_next_async(GSocketAddressEnumerator *enumerator, GCancellable *cancellable, GAsyncReadyCallback callback,
					                                    gpointer user_data) {
  GSimpleAsyncResult *result;
  GSocketAddress *address;
  GError *error = NULL;
  result = g_simple_async_result_new(G_OBJECT(enumerator), callback, user_data, g_socket_address_enumerator_real_next_async);
  address = g_socket_address_enumerator_next(enumerator, cancellable, &error);
  if (address) g_simple_async_result_set_op_res_gpointer(result, address, NULL);
  else if (error) g_simple_async_result_take_error(result, error);
  g_simple_async_result_complete_in_idle(result);
  g_object_unref(result);
}
void g_socket_address_enumerator_next_async(GSocketAddressEnumerator *enumerator, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSocketAddressEnumeratorClass *klass;
  g_return_if_fail(G_IS_SOCKET_ADDRESS_ENUMERATOR(enumerator));
  klass = G_SOCKET_ADDRESS_ENUMERATOR_GET_CLASS(enumerator);
  (*klass->next_async)(enumerator, cancellable, callback, user_data);
}
static GSocketAddress *g_socket_address_enumerator_real_next_finish(GSocketAddressEnumerator *enumerator, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(result), NULL);
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_return_val_if_fail(g_simple_async_result_get_source_tag(simple) == g_socket_address_enumerator_real_next_async, NULL);
  if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  else return g_simple_async_result_get_op_res_gpointer(simple);
}
GSocketAddress *g_socket_address_enumerator_next_finish(GSocketAddressEnumerator *enumerator, GAsyncResult *result, GError **error) {
  GSocketAddressEnumeratorClass *klass;
  g_return_val_if_fail(G_IS_SOCKET_ADDRESS_ENUMERATOR(enumerator), NULL);
  klass = G_SOCKET_ADDRESS_ENUMERATOR_GET_CLASS(enumerator);
  return (*klass->next_finish)(enumerator, result, error);
}