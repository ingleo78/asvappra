#ifndef HEADER_CURL_SPLAY_H
#define HEADER_CURL_SPLAY_H

#include "curl_setup.h"
#include "timeval.h"

struct Curl_tree {
    struct Curl_tree *smaller;
    struct Curl_tree *larger;
    struct Curl_tree *samen;
    struct Curl_tree *samep;
    struct curltime key;
    void *payload;
};
struct Curl_tree *Curl_splay(struct curltime i, struct Curl_tree *t);
struct Curl_tree *Curl_splayinsert(struct curltime key, struct Curl_tree *t, struct Curl_tree *newnode);
#if 0
struct Curl_tree *Curl_splayremove(struct curltime key, struct Curl_tree *t, struct Curl_tree **removed);
#endif
struct Curl_tree *Curl_splaygetbest(struct curltime key, struct Curl_tree *t, struct Curl_tree **removed);
int Curl_splayremovebyaddr(struct Curl_tree *t, struct Curl_tree *removenode, struct Curl_tree **newroot);
#define Curl_splaycomparekeys(i,j) ( ((i.tv_sec)  < (j.tv_sec)) ? -1 : \
                                   ( ((i.tv_sec)  > (j.tv_sec)) ?  1 : \
                                   ( ((i.tv_usec) < (j.tv_usec)) ? -1 : \
                                   ( ((i.tv_usec) > (j.tv_usec)) ?  1 : 0))))
#endif