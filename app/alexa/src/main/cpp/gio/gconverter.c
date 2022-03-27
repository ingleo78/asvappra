#include "../glib/glibintl.h"
#include "../glib/gerror.h"
#include "config.h"
#include "gconverter.h"

typedef GConverterIface GConverterInterface;
G_DEFINE_INTERFACE(GConverter, g_converter, G_TYPE_OBJECT)
static void g_converter_default_init(GConverterInterface *iface) {}
GConverterResult g_converter_convert(GConverter *converter, const void *inbuf, gsize inbuf_size, void *outbuf, gsize outbuf_size, GConverterFlags flags,
		     						 gsize *bytes_read, gsize *bytes_written, GError **error) {
  GConverterIface *iface;
  g_return_val_if_fail(G_IS_CONVERTER (converter), G_CONVERTER_ERROR);
  g_return_val_if_fail(outbuf_size > 0, G_CONVERTER_ERROR);
  *bytes_read = 0;
  *bytes_written = 0;
  iface = G_CONVERTER_GET_IFACE(converter);
  return (*iface->convert)(converter, inbuf, inbuf_size, outbuf, outbuf_size, flags, bytes_read, bytes_written, error);
}
void g_converter_reset(GConverter *converter) {
  GConverterIface *iface;
  g_return_if_fail(G_IS_CONVERTER(converter));
  iface = G_CONVERTER_GET_IFACE(converter);
  (*iface->reset)(converter);
}