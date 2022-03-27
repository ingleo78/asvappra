#ifndef HEADER_CURL_TOOL_EASYSRC_H
#define HEADER_CURL_TOOL_EASYSRC_H

#include "tool_setup.h"
#ifndef CURL_DISABLE_LIBCURL_OPTION

extern struct slist_wc *easysrc_decl;
extern struct slist_wc *easysrc_data;
extern struct slist_wc *easysrc_code;
extern struct slist_wc *easysrc_toohard;
extern struct slist_wc *easysrc_clean;
extern int easysrc_mime_count;
extern int easysrc_slist_count;
extern CURLcode easysrc_init(void);
extern CURLcode easysrc_add(struct slist_wc **plist, const char *bupf);
extern CURLcode easysrc_addf(struct slist_wc **plist, const char *fmt, ...);
extern CURLcode easysrc_perform(void);
extern CURLcode easysrc_cleanup(void);
void dumpeasysrc(struct GlobalConfig *config);
#endif
#endif