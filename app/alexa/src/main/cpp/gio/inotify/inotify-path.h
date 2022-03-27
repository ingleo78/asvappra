#ifndef __INOTIFY_PATH_H
#define __INOTIFY_PATH_H

#include "inotify-kernel.h"
#include "inotify-sub.h"

gboolean _ip_startup(void (*event_cb)(ik_event_t *event, inotify_sub *sub));
gboolean _ip_start_watching(inotify_sub *sub);
gboolean _ip_stop_watching (inotify_sub *sub);
const char *_ip_get_path_for_wd(gint32 wd);

#endif
