#ifndef __G_MAIN_INTERNAL_H__
#define __G_MAIN_INTERNAL_H__

#if defined (GLIB_COMPILATION)
//#error "This is a private header"
#endif

#include "gmain.h"

G_BEGIN_DECLS
GSource *_g_main_create_unix_signal_watch(int signum);
G_END_DECLS

#endif