#include <sys/types.h>
#include <errno.h>
#include "fen-node.h"

#ifndef _FEN_KERNEL_H_
#define _FEN_KERNEL_H_
#define CONCERNED_EVENTS (FILE_MODIFIED | FILE_ATTRIB | FILE_NOFOLLOW)
#define EXCEPTION_EVENTS (FILE_DELETE | FILE_RENAME_FROM)
#define HAS_EXCEPTION_EVENTS(e) ((e & EXCEPTION_EVENTS) != 0)
#define HAS_NO_EXCEPTION_EVENTS(e) ((e & EXCEPTION_EVENTS) == 0)
gint port_add(node_t* f);
void port_remove(node_t *f);
gboolean port_class_init();

#endif