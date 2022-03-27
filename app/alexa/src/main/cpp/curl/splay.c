#include "curl_setup.h"
#include "splay.h"

#define compare(i,j) Curl_splaycomparekeys((i),(j))

struct Curl_tree *Curl_splay(struct curltime i, struct Curl_tree *t) {
    struct Curl_tree N, *l, *r, *y;
    if(t == NULL) return t;
    N.smaller = N.larger = NULL;
    l = r = &N;
    for( ; ; ) {
        long comp = compare(i, t->key);
        if(comp < 0) {
            if(t->smaller == NULL) break;
            if(compare(i, t->smaller->key) < 0) {
                y = t->smaller;
                t->smaller = y->larger;
                y->larger = t;
                t = y;
                if(t->smaller == NULL) break;
            }
            r->smaller = t;
            r = t;
            t = t->smaller;
        } else if(comp > 0) {
            if(t->larger == NULL) break;
            if(compare(i, t->larger->key) > 0) {
                y = t->larger;
                t->larger = y->smaller;
                y->smaller = t;
                t = y;
                if(t->larger == NULL) break;
            }
            l->larger = t;
            l = t;
            t = t->larger;
        } else break;
    }
    l->larger = t->smaller;
    r->smaller = t->larger;
    t->smaller = N.larger;
    t->larger = N.smaller;
    return t;
}
struct Curl_tree *Curl_splayinsert(struct curltime i, struct Curl_tree *t, struct Curl_tree *node) {
    static const struct curltime KEY_NOTUSED = {(time_t)-1, (unsigned int)-1 };
    if(node == NULL) return t;
    if(t != NULL) {
        t = Curl_splay(i, t);
        if(compare(i, t->key) == 0) {
            node->key = KEY_NOTUSED;
            node->samen = t;
            node->samep = t->samep;
            t->samep->samen = node;
            t->samep = node;
            return t;
        }
    }
    if(t == NULL) node->smaller = node->larger = NULL;
    else if(compare(i, t->key) < 0) {
        node->smaller = t->smaller;
        node->larger = t;
        t->smaller = NULL;
    } else {
        node->larger = t->larger;
        node->smaller = t;
        t->larger = NULL;
    }
    node->key = i;
    node->samen = node;
    node->samep = node;
    return node;
}
struct Curl_tree *Curl_splaygetbest(struct curltime i, struct Curl_tree *t, struct Curl_tree **removed) {
    static struct curltime tv_zero = {0, 0};
    struct Curl_tree *x;
    if(!t) {
        *removed = NULL;
        return NULL;
    }
    t = Curl_splay(tv_zero, t);
    if(compare(i, t->key) < 0) {
        *removed = NULL;
        return t;
    }
    x = t->samen;
    if(x != t) {
        x->key = t->key;
        x->larger = t->larger;
        x->smaller = t->smaller;
        x->samep = t->samep;
        t->samep->samen = x;
        *removed = t;
        return x;
    }
    x = t->larger;
    *removed = t;
    return x;
}
int Curl_splayremovebyaddr(struct Curl_tree *t, struct Curl_tree *removenode, struct Curl_tree **newroot) {
    static const struct curltime KEY_NOTUSED = {(time_t)-1, (unsigned int)-1 };
    struct Curl_tree *x;
    if(!t || !removenode) return 1;
    if(compare(KEY_NOTUSED, removenode->key) == 0) {
        if(removenode->samen == removenode) return 3;
        removenode->samep->samen = removenode->samen;
        removenode->samen->samep = removenode->samep;
        removenode->samen = removenode;
        *newroot = t;
        return 0;
    }
    t = Curl_splay(removenode->key, t);
    if(t != removenode) return 2;
    x = t->samen;
    if(x != t) {
        x->key = t->key;
        x->larger = t->larger;
        x->smaller = t->smaller;
        x->samep = t->samep;
        t->samep->samen = x;
    } else {
        if(t->smaller == NULL) x = t->larger;
        else {
            x = Curl_splay(removenode->key, t->smaller);
            x->larger = t->larger;
        }
    }
    *newroot = x;
    return 0;
}