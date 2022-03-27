#ifndef __GLIBINTL_H__
#define __GLIBINTL_H__

//#include <libintl.h>
#include "gmacros.h"
#include "gtypes.h"

G_CONST_RETURN gchar *glib_gettext(const gchar *str) G_GNUC_FORMAT(1);
#ifdef ENABLE_NLS
#define _(String) glib_gettext(String)
#define P_(String) glib_gettext(String)
#define C_(Context,String) g_dpgettext (NULL, Context "\004" String, strlen (Context) + 1)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else
#define _(String) (String)
#define N_(String) (String)
#define P_(String) (String)
#define C_(Context,String) (String)
#define textdomain(String) ((String) ? (String) : "messages")
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define dngettext(Domain,String1,String2,N) ((N) == 1 ? (String1) : (String2))
#define bindtextdomain(Domain,Directory) (Domain) 
#define bind_textdomain_codeset(Domain,Codeset)
#endif
#define I_(string) g_intern_static_string (string)
#endif