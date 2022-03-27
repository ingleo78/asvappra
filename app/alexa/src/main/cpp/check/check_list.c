#include <stdlib.h>
#include <string.h>
#include "libcompat.h"
#include "check_list.h"
#include "check_error.h"

enum {
    LINIT = 1,
    LGROW = 2
};
struct List {
    unsigned int n_elts;
    unsigned int max_elts;
    int current;
    int last;
    void **data;
};
static void maybe_grow(List * lp) {
    if(lp->n_elts >= lp->max_elts) {
        lp->max_elts *= LGROW;
        lp->data = (void **)erealloc(lp->data, lp->max_elts * sizeof(lp->data[0]));
    }
}
List *check_list_create(void) {
    List *lp;
    lp = (List*)emalloc(sizeof(List));
    lp->n_elts = 0;
    lp->max_elts = LINIT;
    lp->data = (void**)emalloc(sizeof(lp->data[0]) * LINIT);
    lp->current = lp->last = -1;
    return lp;
}
void check_list_add_front(List * lp, void *val) {
    if(lp == NULL) return;
    maybe_grow(lp);
    memmove(lp->data + 1, lp->data, lp->n_elts * sizeof lp->data[0]);
    lp->last++;
    lp->n_elts++;
    lp->current = 0;
    lp->data[lp->current] = val;
}
void check_list_add_end(List *lp, void *val) {
    if(lp == NULL) return;
    maybe_grow(lp);
    lp->last++;
    lp->n_elts++;
    lp->current = lp->last;
    lp->data[lp->current] = val;
}
int check_list_at_end(List * lp) {
    if(lp->current == -1) return 1;
    return (lp->current > lp->last);
}
void check_list_front(List * lp) {
    if(lp->current == -1) return;
    lp->current = 0;
}
void check_list_free(List * lp) {
    if(lp == NULL) return;
    free(lp->data);
    free(lp);
}
void *check_list_val(List * lp) {
    if(lp == NULL) return NULL;
    if(lp->current == -1 || lp->current > lp->last) return NULL;
    return lp->data[lp->current];
}
void check_list_advance(List * lp) {
    if(lp == NULL) return;
    if(check_list_at_end(lp)) return;
    lp->current++;
}
void check_list_apply(List * lp, void (*fp) (void *)) {
    if(lp == NULL || fp == NULL) return;
    for(check_list_front(lp); !check_list_at_end(lp); check_list_advance(lp)) fp(check_list_val(lp));
}
int check_list_contains(List * lp, void *val) {
    for(check_list_front(lp); !check_list_at_end(lp); check_list_advance(lp)) {
        if(check_list_val(lp) == val) return 1;
    }
    return 0;
}