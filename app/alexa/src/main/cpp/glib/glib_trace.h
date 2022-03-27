#ifndef __GLIBTRACE_H__
#define __GLIBTRACE_H__

#include "../gio/config.h"

#ifndef SIZEOF_CHAR
#error "config.h must be included prior to glib_trace.h"
#endif

#ifdef HAVE_DTRACE
#include "glib_probes.h"
#define TRACE(probe) probe
#else
#define TRACE(probe)
#endif

#endif