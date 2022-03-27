#ifndef __G_IO_MODULE_PRIV_H__
#define __G_IO_MODULE_PRIV_H__

#include "giomodule.h"

G_BEGIN_DECLS
void _g_io_modules_ensure_extension_points_registered(void);
void _g_io_modules_ensure_loaded(void);
G_END_DECLS

#endif