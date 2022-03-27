#include <stdbool.h>
#include <assert.h>
#include "curl_setup.h"

#ifndef CURL_DISABLE_GOPHER
#include "urldata.h"
#include "curl.h"
#include "transfer.h"
#include "sendf.h"
#include "connect.h"
#include "progress.h"
#include "gopher.h"
#include "select.h"
#include "strdup.h"
#include "url.h"
#include "escape.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static CURLcode gopher_do(struct connectdata *conn, bool *done);
const struct Curl_handler Curl_handler_gopher = {
    "GOPHER",
    ZERO_NULL,
    gopher_do,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    PORT_GOPHER,
    CURLPROTO_GOPHER,
    PROTOPT_NONE
};
static CURLcode gopher_do(struct connectdata *conn, bool *done) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
    char *gopherpath;
    char *path = data->state.up.path;
    char *query = data->state.up.query;
    char *sel = NULL;
    char *sel_org = NULL;
    timediff_t timeout_ms;
    ssize_t amount, k;
    size_t len;
    int what;
    *done = TRUE;
    DEBUGASSERT(path);
    if (query) gopherpath = aprintf("%s?%s", path, query);
    else gopherpath = strdup(path);
    if (!gopherpath) return CURLE_OUT_OF_MEMORY;
    if (strlen(gopherpath) <= 2) {
        sel = (char *)"";
        len = strlen(sel);
        free(gopherpath);
    } else {
        char *newp;
        newp = gopherpath;
        newp += 2;
        result = Curl_urldecode(data, newp, 0, &sel, &len, REJECT_ZERO);
        free(gopherpath);
        if (result) return result;
        sel_org = sel;
    }
    k = curlx_uztosz(len);
    for ( ; ; ) {
        result = Curl_write(conn, sockfd, sel, k, &amount);
        if (!result) {
            result = Curl_client_write(conn, CLIENTWRITE_HEADER, sel, amount);
            if (result) break;
            k -= amount;
            sel += amount;
            if (k < 1) break;
        } else break;
        timeout_ms = Curl_timeleft(conn->data, NULL, FALSE);
        if (timeout_ms < 0) {
            result = CURLE_OPERATION_TIMEDOUT;
            break;
        }
        if (!timeout_ms) timeout_ms = TIMEDIFF_T_MAX;
        what = SOCKET_WRITABLE(sockfd, timeout_ms);
        if (what < 0) {
            result = CURLE_SEND_ERROR;
            break;
        } else if (!what) {
            result = CURLE_OPERATION_TIMEDOUT;
            break;
        }
    }
    free(sel_org);
    if(!result) result = Curl_sendf(sockfd, conn, "\r\n");
    if(result) {
        failf(data, "Failed sending Gopher request");
        return result;
    }
    result = Curl_client_write(conn, CLIENTWRITE_HEADER, (char *)"\r\n", 2);
    if(result) return result;
    Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
    return CURLE_OK;
}
#endif