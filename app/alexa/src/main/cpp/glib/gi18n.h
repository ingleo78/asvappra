#ifndef __G_I18N_H__
#define __G_I18N_H__

#include <string.h>
#include "glibintl.h"

#define _(String) gettext(String)
#define Q_(String) g_dpgettext(NULL, String, 0)
#define N_(String) (String)
#define C_(Context,String) g_dpgettext(NULL, Context "\004" String, strlen (Context) + 1)
#define NC_(Context, String) (String)

#endif