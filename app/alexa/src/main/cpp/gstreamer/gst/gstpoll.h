#ifndef __GST_POLL_H__
#define __GST_POLL_H__

#include <glib/glib.h>
#include <glib/glib-object.h>
#include "gstclock.h"

G_BEGIN_DECLS
typedef struct _GstPoll GstPoll;
typedef struct {
  int fd;
  gint idx;
} GstPollFD;
#define GST_POLL_FD_INIT  { -1, -1 }
GstPoll*        gst_poll_new              (gboolean controllable) G_GNUC_MALLOC;
GstPoll*        gst_poll_new_timer        (void) G_GNUC_MALLOC;
void            gst_poll_free             (GstPoll *set);
void            gst_poll_get_read_gpollfd (GstPoll *set, GPollFD *fd);
void            gst_poll_fd_init          (GstPollFD *fd);
gboolean        gst_poll_add_fd           (GstPoll *set, GstPollFD *fd);
gboolean        gst_poll_remove_fd        (GstPoll *set, GstPollFD *fd);
gboolean        gst_poll_fd_ctl_write     (GstPoll *set, GstPollFD *fd, gboolean active);
gboolean        gst_poll_fd_ctl_read      (GstPoll *set, GstPollFD *fd, gboolean active);
void            gst_poll_fd_ignored       (GstPoll *set, GstPollFD *fd);
gboolean        gst_poll_fd_has_closed    (const GstPoll *set, GstPollFD *fd);
gboolean        gst_poll_fd_has_error     (const GstPoll *set, GstPollFD *fd);
gboolean        gst_poll_fd_can_read      (const GstPoll *set, GstPollFD *fd);
gboolean        gst_poll_fd_can_write     (const GstPoll *set, GstPollFD *fd);
gint            gst_poll_wait             (GstPoll *set, GstClockTime timeout);
gboolean        gst_poll_set_controllable (GstPoll *set, gboolean controllable);
void            gst_poll_restart          (GstPoll *set);
void            gst_poll_set_flushing     (GstPoll *set, gboolean flushing);
gboolean        gst_poll_write_control    (GstPoll *set);
gboolean        gst_poll_read_control     (GstPoll *set);
G_END_DECLS

#endif