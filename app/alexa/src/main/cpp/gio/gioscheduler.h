#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_IO_SCHEDULER_H__
#define __G_IO_SCHEDULER_H__

#include "giotypes.h"

G_BEGIN_DECLS
void g_io_scheduler_push_job(GIOSchedulerJobFunc job_func, gpointer user_data, GDestroyNotify notify, gint io_priority, GCancellable *cancellable);
void g_io_scheduler_cancel_all_jobs(void);
gboolean g_io_scheduler_job_send_to_mainloop(GIOSchedulerJob *job, GSourceFunc func, gpointer user_data, GDestroyNotify notify);
void g_io_scheduler_job_send_to_mainloop_async(GIOSchedulerJob *job, GSourceFunc func, gpointer user_data, GDestroyNotify notify);
G_END_DECLS

#endif