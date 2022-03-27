#ifndef __GIOTRACE_H__
#define __GIOTRACE_H__

#ifndef SIZEOF_CHAR
#error "config.h must be included prior to gio_trace.h"
#endif
#if defined(HAVE_DTRACE) && !defined(__clang_analyzer__)
#include "gio_probes.h"
#define TRACE(probe) probe
#else
#define TRACE(probe)
#endif
#endif