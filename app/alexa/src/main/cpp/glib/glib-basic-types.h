#ifndef BASIC_TYPES
#define BASIC_TYPES

#include <stdint.h>

typedef int gint;
typedef gint gboolean;
typedef void* gpointer;
typedef const void* gconstpointer;
typedef char gchar;
typedef unsigned char guchar;
typedef unsigned int guint;
typedef short gshort;
typedef unsigned short gushort;
typedef long glong;
typedef unsigned long gulong;
typedef int8_t gint8;
typedef uint8_t guint8;
typedef int16_t gint16;
typedef uint16_t guint16;
typedef int32_t gint32;
typedef uint32_t guint32;
#define G_HAVE_GINT64
typedef int64_t gint64;
typedef uint64_t guint64;
//GLIB_VAR guint64                             ();
#define G_GINT64_CONSTANT(val)  (val##L)
#define G_GUINT64_CONSTANT(val) (val##UL)
typedef float gfloat;
typedef double gdouble;
#define gsize unsigned long
#define gssize signed long
typedef gint64 goffset;
#endif