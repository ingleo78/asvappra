#include "gmessages.h"
#include "glibconfig.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "gerror.h"
#include "gmacros.h"
#include "gtypes.h"

GError* g_error_new_valist(GQuark domain, gint code, const char *format, va_list args) {
  GError *error;
  error = g_slice_new(GError);
  error->domain = domain;
  error->code = code;
  error->message = g_strdup_vprintf(format, args);
  return error;
}
GError* g_error_new(GQuark domain, gint code, const char *format, ...) {
  GError* error;
  va_list args;
  g_return_val_if_fail(format != NULL, NULL);
  g_return_val_if_fail(domain != 0, NULL);
  va_start(args, format);
  error = g_error_new_valist(domain, code, format, args);
  va_end(args);
  return error;
}
GError* g_error_new_literal(GQuark domain, gint code, const char *message) {
  GError* err;
  g_return_val_if_fail(message != NULL, NULL);
  g_return_val_if_fail(domain != 0, NULL);
  err = g_slice_new(GError);
  err->domain = domain;
  err->code = code;
  err->message = g_strdup(message);
  return err;
}
void g_error_free(GError *error) {
  g_return_if_fail(error != NULL);
  g_free(error->message);
  g_slice_free(GError, error);
}
GError* g_error_copy(const GError *error) {
  GError *copy;
  g_return_val_if_fail(error != NULL, NULL);
  copy = g_slice_new(GError);
  *copy = *error;
  copy->message = g_strdup(error->message);
  return copy;
}
gboolean g_error_matches(const GError *error, GQuark domain, gint code) {
  return error && error->domain == domain && error->code == code;
}
#define ERROR_OVERWRITTEN_WARNING "GError set over the top of a previous GError or uninitialized memory.\n" \
               "This indicates a bug in someone's code. You must ensure an error is NULL before it's set.\n" \
               "The overwriting error message was: %s"
void g_set_error(GError **err, GQuark domain, gint code, const char *format, ...) {
  GError *new;
  va_list args;
  if (err == NULL) return;
  va_start(args, format);
  new = g_error_new_valist(domain, code, format, args);
  va_end (args);
  if (*err == NULL) *err = new;
  else g_warning(ERROR_OVERWRITTEN_WARNING, new->message);
}
void g_set_error_literal(GError **err, GQuark domain, gint code, const char *message) {
  GError *new;
  if (err == NULL) return;
  new = g_error_new_literal(domain, code, message);
  if (*err == NULL) *err = new;
  else g_warning(ERROR_OVERWRITTEN_WARNING, new->message);
}
void g_propagate_error(GError **dest, GError *src) {
  g_return_if_fail(src != NULL);
  if (dest == NULL) {
      if (src) g_error_free(src);
      return;
  } else {
      if (*dest != NULL) g_warning(ERROR_OVERWRITTEN_WARNING, src->message);
      else *dest = src;
  }
}
void g_clear_error(GError **err) {
  if (err && *err) {
      g_error_free(*err);
      *err = NULL;
  }
}
static void g_error_add_prefix(char **string, const char *format, va_list ap) {
  char *oldstring;
  char *prefix;
  prefix = g_strdup_vprintf(format, ap);
  oldstring = *string;
  *string = g_strconcat(prefix, oldstring, NULL);
  g_free(oldstring);
  g_free(prefix);
}
void g_prefix_error(GError **err, const char *format, ...) {
  if (err && *err) {
      va_list ap;
      va_start(ap, format);
      g_error_add_prefix(&(*err)->message, format, ap);
      va_end(ap);
  }
}
void g_propagate_prefixed_error(GError **dest, GError *src, const char *format, ...) {
  g_propagate_error(dest, src);
  if (dest && *dest) {
      va_list ap;
      va_start(ap, format);
      g_error_add_prefix(&(*dest)->message, format, ap);
      va_end(ap);
  }
}