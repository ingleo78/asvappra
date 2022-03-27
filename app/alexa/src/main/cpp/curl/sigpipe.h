#ifndef HEADER_CURL_SIGPIPE_H
#define HEADER_CURL_SIGPIPE_H

#include "curl_setup.h"

#if defined(HAVE_SIGNAL_H) && defined(HAVE_SIGACTION) && (defined(USE_OPENSSL) || defined(USE_MBEDTLS))
#include <signal.h>

struct sigpipe_ignore {
    struct sigaction old_pipe_act;
    bool no_signal;
};
#define SIGPIPE_VARIABLE(x) struct sigpipe_ignore x
static void sigpipe_ignore(struct Curl_easy *data, struct sigpipe_ignore *ig) {
    ig->no_signal = data->set.no_signal;
    if(!data->set.no_signal) {
        struct sigaction action;
        memset(&ig->old_pipe_act, 0, sizeof(struct sigaction));
        sigaction(SIGPIPE, NULL, &ig->old_pipe_act);
        action = ig->old_pipe_act;
        action.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &action, NULL);
    }
}
static void sigpipe_restore(struct sigpipe_ignore *ig) {
    if(!ig->no_signal) sigaction(SIGPIPE, &ig->old_pipe_act, NULL);
}
#else
#define sigpipe_ignore(x,y) Curl_nop_stmt
#define sigpipe_restore(x)  Curl_nop_stmt
#define SIGPIPE_VARIABLE(x)
#endif

#endif