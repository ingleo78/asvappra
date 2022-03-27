#ifndef __G_DATASETPRIVATE_H__
#define __G_DATASETPRIVATE_H__

#include "gatomic.h"

G_BEGIN_DECLS
#define G_DATALIST_GET_FLAGS(datalist)	((gsize) g_atomic_pointer_get (datalist) & G_DATALIST_FLAGS_MASK)
G_END_DECLS

#endif