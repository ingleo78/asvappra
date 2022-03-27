#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_IO_ERROR_H__
#define __G_IO_ERROR_H__

#include "gioenums.h"

G_BEGIN_DECLS
#define G_IO_ERROR g_io_error_quark()
GQuark g_io_error_quark(void);
GIOErrorEnum g_io_error_from_errno(gint err_no);
#ifdef G_OS_WIN32
GIOErrorEnum g_io_error_from_win32_error(gint error_code);
#endif
G_END_DECLS

#endif
