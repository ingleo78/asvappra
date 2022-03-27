#ifndef __G_WAKEUP_H__
#define __G_WAKEUP_H__

typedef struct _GWakeup GWakeup;
GWakeup *g_wakeup_new(void);
void g_wakeup_free(GWakeup *wakeup);
void g_wakeup_get_pollfd(GWakeup *wakeup, GPollFD *poll_fd);
void g_wakeup_signal(GWakeup *wakeup);
void g_wakeup_acknowledge(GWakeup *wakeup);

#endif