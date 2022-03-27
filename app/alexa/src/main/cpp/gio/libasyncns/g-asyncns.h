#ifndef __G_ASYNCNS_H__

#include "../config.h"
#define _GNU_SOURCE
#undef HAVE_PTHREAD
#if HAVE_ARPA_NAMESER_COMPAT_H
#include <arpa/nameser_compat.h>
#endif
#include "asyncns.h"

#endif