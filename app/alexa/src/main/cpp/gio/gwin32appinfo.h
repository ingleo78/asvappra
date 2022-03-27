#ifndef __G_WIN32_APP_INFO_H__
#define __G_WIN32_APP_INFO_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_WIN32_APP_INFO  (g_win32_app_info_get_type())
#define G_WIN32_APP_INFO(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_WIN32_APP_INFO, GWin32AppInfo))
#define G_WIN32_APP_INFO_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WIN32_APP_INFO, GWin32AppInfoClass))
#define G_IS_WIN32_APP_INFO(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_WIN32_APP_INFO))
#define G_IS_WIN32_APP_INFO_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_WIN32_APP_INFO))
#define G_WIN32_APP_INFO_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_WIN32_APP_INFO, GWin32AppInfoClass))
typedef struct _GWin32AppInfo GWin32AppInfo;
typedef struct _GWin32AppInfoClass GWin32AppInfoClass;
struct _GWin32AppInfoClass {
  GObjectClass parent_class;
};
GType g_win32_app_info_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif