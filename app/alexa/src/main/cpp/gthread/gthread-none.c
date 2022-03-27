#include "../glib/gthread.h"

static GThreadFunctions g_thread_functions_for_glib_use_default;
static guint64 (*g_thread_gettime_impl) (void) = NULL;
 
#define G_MUTEX_SIZE 0
