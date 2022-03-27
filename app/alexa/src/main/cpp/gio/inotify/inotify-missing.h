#ifndef __INOTIFY_MISSING_H
#define __INOTIFY_MISSING_H

#include "../../glib/giochannel.h"
#include "inotify-sub.h"

void _im_startup(void (*missing_cb)(inotify_sub *sub));
void _im_add(inotify_sub *sub);
void _im_rm(inotify_sub *sub);
void _im_diag_dump(GIOChannel *ioc);

#endif
