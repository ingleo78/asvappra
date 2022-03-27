#include <stddef.h>
#include "curl_setup.h"
#include "curl.h"
#include "llist.h"
#include "curl_memory.h"
#include "memdebug.h"

void Curl_llist_init(struct curl_llist *l, curl_llist_dtor dtor) {
    l->size = 0;
    l->dtor = dtor;
    l->head = NULL;
    l->tail = NULL;
}
void Curl_llist_insert_next(struct curl_llist *list, struct curl_llist_element *e, const void *p, struct curl_llist_element *ne) {
    ne->ptr = (void *) p;
    if(list->size == 0) {
        list->head = ne;
        list->head->prev = NULL;
        list->head->next = NULL;
        list->tail = ne;
    } else {
        ne->next = e?e->next:list->head;
        ne->prev = e;
        if(!e) {
            list->head->prev = ne;
            list->head = ne;
        }
        else if(e->next) e->next->prev = ne;
        else list->tail = ne;
        if(e) e->next = ne;
    }
    ++list->size;
}
void Curl_llist_remove(struct curl_llist *list, struct curl_llist_element *e, void *user) {
    void *ptr;
    if(e == NULL || list->size == 0) return;
    if(e == list->head) {
        list->head = e->next;
        if(list->head == NULL) list->tail = NULL;
        else e->next->prev = NULL;
    } else {
        if(!e->prev) list->head = e->next;
        else e->prev->next = e->next;
        if(!e->next) list->tail = e->prev;
        else e->next->prev = e->prev;
    }
    ptr = e->ptr;
    e->ptr  = NULL;
    e->prev = NULL;
    e->next = NULL;
    --list->size;
    if(list->dtor) list->dtor(user, ptr);
}
void Curl_llist_destroy(struct curl_llist *list, void *user) {
    if(list) {
        while(list->size > 0) Curl_llist_remove(list, list->tail, user);
    }
}
size_t Curl_llist_count(struct curl_llist *list) { return list->size; }