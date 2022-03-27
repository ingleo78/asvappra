#include <assert.h>
#include "curl_setup.h"
#include "curl.h"
#include "slist.h"
#include "curl_memory.h"
#include "memdebug.h"

static struct curl_slist *slist_get_last(struct curl_slist *list) {
    struct curl_slist *item;
    if(!list) return NULL;
    item = list;
    while(item->next) item = item->next;
    return item;
}
struct curl_slist *Curl_slist_append_nodup(struct curl_slist *list, char *data) {
    struct curl_slist     *last;
    struct curl_slist     *new_item;
    DEBUGASSERT(data);
    new_item = malloc(sizeof(struct curl_slist));
    if(!new_item) return NULL;
    new_item->next = NULL;
    new_item->data = data;
    if(!list) return new_item;
    last = slist_get_last(list);
    last->next = new_item;
    return list;
}
struct curl_slist *curl_slist_append(struct curl_slist *list, const char *data) {
    char *dupdata = strdup(data);
    if(!dupdata) return NULL;
    list = Curl_slist_append_nodup(list, dupdata);
    if(!list) free(dupdata);
    return list;
}
struct curl_slist *Curl_slist_duplicate(struct curl_slist *inlist) {
    struct curl_slist *outlist = NULL;
    struct curl_slist *tmp;
    while(inlist) {
        tmp = curl_slist_append(outlist, inlist->data);
        if(!tmp) {
            curl_slist_free_all(outlist);
            return NULL;
        }
        outlist = tmp;
        inlist = inlist->next;
    }
    return outlist;
}
void curl_slist_free_all(struct curl_slist *list) {
    struct curl_slist     *next;
    struct curl_slist     *item;
    if(!list) return;
    item = list;
    do {
        next = item->next;
        Curl_safefree(item->data);
        free(item);
        item = next;
    } while(next);
}
