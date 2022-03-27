#include "tool_setup.h"

#ifndef CURL_DISABLE_LIBCURL_OPTION
#include "slist_wc.h"
#include "memdebug.h"

struct slist_wc *slist_wc_append(struct slist_wc *list, const char *data) {
    struct curl_slist *new_item = curl_slist_append(NULL, data);
    if (!new_item) return NULL;
    if (!list) {
        list = malloc(sizeof(struct slist_wc));
        if (!list) {
            curl_slist_free_all(new_item);
            return NULL;
        }
        list->first = new_item;
        list->last = new_item;
        return list;
    }
    list->last->next = new_item;
    list->last = list->last->next;
    return list;
}
void slist_wc_free_all(struct slist_wc *list) {
    if(!list) return;
    curl_slist_free_all(list->first);
    free(list);
}
#endif