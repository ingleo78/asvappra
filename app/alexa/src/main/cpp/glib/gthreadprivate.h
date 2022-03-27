#ifndef __G_THREADPRIVATE_H__
#define __G_THREADPRIVATE_H__

#include "gmacros.h"

G_BEGIN_DECLS
#if GLIB_SIZEOF_SYSTEM_THREAD == SIZEOF_VOID_P
#define g_system_thread_equal_simple(thread1, thread2) ((thread1).dummy_pointer == (thread2).dummy_pointer)
#define g_system_thread_assign(dest, src) ((dest).dummy_pointer = (src).dummy_pointer)
#else
# define g_system_thread_equal_simple(thread1, thread2) (memcmp (&(thread1), &(thread2), GLIB_SIZEOF_SYSTEM_THREAD) == 0)
# define g_system_thread_assign(dest, src) (memcpy (&(dest), &(src), GLIB_SIZEOF_SYSTEM_THREAD))
#endif
#define g_system_thread_equal(thread1, thread2)	(g_thread_functions_for_glib_use.thread_equal ? \
    g_thread_functions_for_glib_use.thread_equal(&(thread1), &(thread2)) : g_system_thread_equal_simple((thread1), (thread2)))
void g_thread_init_glib(void);
G_GNUC_INTERNAL void _g_mem_thread_init_noprivate_nomessage(void);
G_GNUC_INTERNAL void _g_slice_thread_init_nomessage(void);
G_GNUC_INTERNAL void _g_messages_thread_init_nomessage(void);
G_GNUC_INTERNAL void _g_convert_thread_init(void);
G_GNUC_INTERNAL void _g_rand_thread_init(void);
G_GNUC_INTERNAL void _g_main_thread_init(void);
G_GNUC_INTERNAL void _g_atomic_thread_init(void);
G_GNUC_INTERNAL void _g_utils_thread_init(void);
G_GNUC_INTERNAL void _g_futex_thread_init(void);
#ifdef G_OS_WIN32
G_GNUC_INTERNAL void _g_win32_thread_init(void);
#endif
G_END_DECLS

#endif