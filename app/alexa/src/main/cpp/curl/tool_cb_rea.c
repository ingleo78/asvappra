#include <errno.h>
#include <unistd.h>
#include "tool_setup.h"
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "tool_cfgable.h"
#include "tool_cb_rea.h"
#include "tool_operate.h"
#include "memdebug.h"

size_t tool_read_cb(void *buffer, size_t sz, size_t nmemb, void *userdata) {
    ssize_t rc;
    struct InStruct *in = userdata;
    rc = read(in->fd, buffer, sz*nmemb);
    if(rc < 0) {
        if(errno == EAGAIN) {
            errno = 0;
            in->config->readbusy = TRUE;
            return CURL_READFUNC_PAUSE;
        }
        rc = 0;
    }
    in->config->readbusy = FALSE;
    return (size_t)rc;
}
int tool_readbusy_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    struct per_transfer *per = clientp;
    struct OperationConfig *config = per->config;
    (void)dltotal;
    (void)dlnow;
    (void)ultotal;
    (void)ulnow;
    if(config->readbusy) {
        config->readbusy = FALSE;
        curl_easy_pause(per->curl, CURLPAUSE_CONT);
    }
    return per->noprogress? 0 : CURL_PROGRESSFUNC_CONTINUE;
}