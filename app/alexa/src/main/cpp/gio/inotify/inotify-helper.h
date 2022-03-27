#ifndef __INOTIFY_HELPER_H
#define __INOTIFY_HELPER_H

#include "inotify-sub.h"

gboolean _ih_startup(void);
gboolean _ih_sub_add(inotify_sub *sub);
gboolean _ih_sub_cancel(inotify_sub *sub);

#endif