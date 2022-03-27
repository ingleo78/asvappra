#ifndef __FAM_HELPER_H__
#define __FAM_HELPER_H__

#include "../../glib/glib.h"

typedef struct _fam_sub fam_sub;
gboolean  _fam_sub_startup(void);
void _fam_sub_shutdown(void);
fam_sub* _fam_sub_add(const gchar* pathname, gboolean directory, gpointer user_data);
gboolean _fam_sub_cancel(fam_sub* sub);
void _fam_sub_free(fam_sub* sub);

#endif
