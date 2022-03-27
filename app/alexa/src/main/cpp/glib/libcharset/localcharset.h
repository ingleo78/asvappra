#ifndef _LOCALCHARSET_H
#define _LOCALCHARSET_H

#ifdef __cplusplus
extern "C" {
#endif
extern const char *_g_locale_charset_raw(void);
extern const char *_g_locale_charset_unalias(const char *codeset);
extern const char *_g_locale_get_charset_aliases(void);
#ifdef __cplusplus
}
#endif
#endif