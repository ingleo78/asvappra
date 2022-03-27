#include "vquic.h"

#ifdef ENABLE_QUIC
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "../urldata.h"
#include "../dynbuf.h"
#include "../curl_printf.h"
#include "vquic.h"

#ifdef O_BINARY
#define QLOGMODE O_WRONLY|O_CREAT|O_BINARY
#else
#define QLOGMODE O_WRONLY|O_CREAT
#endif

CURLcode Curl_qlogdir(struct Curl_easy *data, unsigned char *scid, size_t scidlen, int *qlogfdp) {
    const char *qlog_dir = getenv("QLOGDIR");
    *qlogfdp = -1;
    if(qlog_dir) {
        struct dynbuf fname;
        CURLcode result;
        unsigned int i;
        Curl_dyn_init(&fname, DYN_QLOG_NAME);
        result = Curl_dyn_add(&fname, qlog_dir);
        if(!result) result = Curl_dyn_add(&fname, "/");
        for(i = 0; (i < scidlen) && !result; i++) {
            char hex[3];
            msnprintf(hex, 3, "%02x", scid[i]);
            result = Curl_dyn_add(&fname, hex);
        }
        if(!result) result = Curl_dyn_add(&fname, ".qlog");
        if(!result) {
            int qlogfd = open(Curl_dyn_ptr(&fname), QLOGMODE, data->set.new_file_perms);
            if(qlogfd != -1) *qlogfdp = qlogfd;
        }
        Curl_dyn_free(&fname);
        if(result) return result;
    }
    return CURLE_OK;
}
#endif
