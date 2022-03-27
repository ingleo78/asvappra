#ifndef HEADER_CURL_SLIST_H
#define HEADER_CURL_SLIST_H

struct curl_slist *Curl_slist_duplicate(struct curl_slist *inlist);
struct curl_slist *Curl_slist_append_nodup(struct curl_slist *list, char *data);

#endif