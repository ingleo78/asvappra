#ifndef HEADER_CURL_FILEINFO_H
#define HEADER_CURL_FILEINFO_H

#include "curl.h"
#include "llist.h"

struct fileinfo {
    struct curl_fileinfo info;
    struct curl_llist_element list;
};
struct fileinfo *Curl_fileinfo_alloc(void);
void Curl_fileinfo_cleanup(struct fileinfo *finfo);

#endif