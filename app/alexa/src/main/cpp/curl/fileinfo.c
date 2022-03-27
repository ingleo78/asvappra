#include "curl_setup.h"
#ifndef CURL_DISABLE_FTP
#include "strdup.h"
#include "fileinfo.h"
#include "curl_memory.h"
#include "memdebug.h"

struct fileinfo *Curl_fileinfo_alloc(void) {
    return calloc(1, sizeof(struct fileinfo));
}
void Curl_fileinfo_cleanup(struct fileinfo *finfo) {
    if(!finfo) return;
    Curl_safefree(finfo->info.b_data);
    free(finfo);
}
#endif