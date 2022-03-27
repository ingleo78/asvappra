#include "curl_setup.h"

#if !defined(CURL_DISABLE_HTTP) && defined(USE_NTLM) && defined(NTLM_WB_ENABLED)
#define DEBUG_ME 0
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include "urldata.h"
#include "sendf.h"
#include "select.h"
#include "vauth/ntlm.h"
#include "curl_ntlm_core.h"
#include "curl_ntlm_wb.h"
#include "url.h"
#include "strerror.h"
#include "strdup.h"
#include "strcase.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#if DEBUG_ME
#define DEBUG_OUT(x) x
#else
#define DEBUG_OUT(x) Curl_nop_stmt
#endif
#if defined(HAVE_CLOSESOCKET)
#define sclose_nolog(x)  closesocket((x))
#elif defined(HAVE_CLOSESOCKET_CAMEL)
#define sclose_nolog(x)  CloseSocket((x))
#else
#define sclose_nolog(x)  close((x))
#endif
static void ntlm_wb_cleanup(struct ntlmdata *ntlm)  {
    if (ntlm->ntlm_auth_hlpr_socket != CURL_SOCKET_BAD) {
        sclose(ntlm->ntlm_auth_hlpr_socket);
        ntlm->ntlm_auth_hlpr_socket = CURL_SOCKET_BAD;
    }
    if (ntlm->ntlm_auth_hlpr_pid) {
        for (int i = 0; i < 4; i++) {
            pid_t ret = waitpid(ntlm->ntlm_auth_hlpr_pid, NULL, WNOHANG);
            if(ret == ntlm->ntlm_auth_hlpr_pid || errno == ECHILD) break;
            switch(i) {
                case 0: kill(ntlm->ntlm_auth_hlpr_pid, SIGTERM); break;
                case 1: Curl_wait_ms(1); break;
                case 2: kill(ntlm->ntlm_auth_hlpr_pid, SIGKILL); break;
                case 3: break;
            }
        }
        ntlm->ntlm_auth_hlpr_pid = 0;
    }
    Curl_safefree(ntlm->challenge);
    Curl_safefree(ntlm->response);
}
static CURLcode ntlm_wb_init(struct Curl_easy *data, struct ntlmdata *ntlm, const char *userp) {
    curl_socket_t sockfds[2];
    pid_t child_pid;
    const char *username;
    char *slash, *domain = NULL;
    const char *ntlm_auth = NULL;
    char *ntlm_auth_alloc = NULL;
#if defined(HAVE_GETPWUID_R) && defined(HAVE_GETEUID)
    struct passwd pw, *pw_res;
    char pwbuf[1024];
#endif
    char buffer[STRERROR_LEN];
#if defined(CURL_DISABLE_VERBOSE_STRINGS)
    (void) data;
#endif
    if (ntlm->ntlm_auth_hlpr_socket != CURL_SOCKET_BAD || ntlm->ntlm_auth_hlpr_pid) return CURLE_OK;
    username = userp;
    if (!username || !username[0]) {
        username = getenv("NTLMUSER");
        if(!username || !username[0]) username = getenv("LOGNAME");
        if(!username || !username[0]) username = getenv("USER");
    #if defined(HAVE_GETPWUID_R) && defined(HAVE_GETEUID)
        if ((!username || !username[0]) && !getpwuid_r(geteuid(), &pw, pwbuf, sizeof(pwbuf), &pw_res) && pw_res) username = pw.pw_name;
    #endif
        if (!username || !username[0]) username = userp;
    }
    slash = strpbrk(username, "\\/");
    if(slash) {
        domain = strdup(username);
        if (!domain) return CURLE_OUT_OF_MEMORY;
        slash = domain + (slash - username);
        *slash = '\0';
        username = username + (slash - domain) + 1;
    }
    #ifdef DEBUGBUILD
    ntlm_auth_alloc = curl_getenv("CURL_NTLM_WB_FILE");
    if(ntlm_auth_alloc) ntlm_auth = ntlm_auth_alloc;
    else ntlm_auth = NTLM_WB_FILE;
    #endif
    if (access(ntlm_auth, X_OK) != 0) {
        failf(data, "Could not access ntlm_auth: %s errno %d: %s", ntlm_auth, errno, Curl_strerror(errno, buffer, sizeof(buffer)));
        goto done;
    }
    if (Curl_socketpair(AF_UNIX, SOCK_STREAM, 0, sockfds)) {
        failf(data, "Could not open socket pair. errno %d: %s", errno, Curl_strerror(errno, buffer, sizeof(buffer)));
        goto done;
    }
    child_pid = fork();
    if (child_pid == -1) {
        sclose(sockfds[0]);
        sclose(sockfds[1]);
        failf(data, "Could not fork. errno %d: %s", errno, Curl_strerror(errno, buffer, sizeof(buffer)));
        goto done;
    } else if(!child_pid) {
        sclose_nolog(sockfds[0]);
        if (dup2(sockfds[1], STDIN_FILENO) == -1) {
            failf(data, "Could not redirect child stdin. errno %d: %s", errno, Curl_strerror(errno, buffer, sizeof(buffer)));
            exit(1);
        }
        if (dup2(sockfds[1], STDOUT_FILENO) == -1) {
            failf(data, "Could not redirect child stdout. errno %d: %s", errno, Curl_strerror(errno, buffer, sizeof(buffer)));
            exit(1);
        }
        if (domain) execl(ntlm_auth, ntlm_auth, "--helper-protocol", "ntlmssp-client-1", "--use-cached-creds", "--username", username, "--domain", domain, NULL);
        else execl(ntlm_auth, ntlm_auth, "--helper-protocol", "ntlmssp-client-1", "--use-cached-creds", "--username", username, NULL);
        sclose_nolog(sockfds[1]);
        failf(data, "Could not execl(). errno %d: %s", errno, Curl_strerror(errno, buffer, sizeof(buffer)));
        exit(1);
    }
    sclose(sockfds[1]);
    ntlm->ntlm_auth_hlpr_socket = sockfds[0];
    ntlm->ntlm_auth_hlpr_pid = child_pid;
    free(domain);
    free(ntlm_auth_alloc);
    return CURLE_OK;
    done:
    free(domain);
    free(ntlm_auth_alloc);
    return CURLE_REMOTE_ACCESS_DENIED;
}
#define MAX_NTLM_WB_RESPONSE 100000
static CURLcode ntlm_wb_response(struct Curl_easy *data, struct ntlmdata *ntlm, const char *input, curlntlm state) {
    size_t len_in = strlen(input), len_out = 0;
    struct dynbuf b;
    char *ptr = NULL;
    unsigned char *buf = (unsigned char *)data->state.buffer;
    Curl_dyn_init(&b, MAX_NTLM_WB_RESPONSE);
    while(len_in > 0) {
        ssize_t written = swrite(ntlm->ntlm_auth_hlpr_socket, input, len_in);
        if (written == -1) {
            if(errno == EINTR) continue;
            goto done;
        }
        input += written;
        len_in -= written;
    }
    while(1) {
        ssize_t size = sread(ntlm->ntlm_auth_hlpr_socket, buf, data->set.buffer_size);
        if (size == -1) {
            if(errno == EINTR) continue;
            goto done;
        } else if(size == 0) goto done;
        if(Curl_dyn_addn(&b, buf, size)) goto done;
        len_out = Curl_dyn_len(&b);
        ptr = Curl_dyn_ptr(&b);
        if (len_out && ptr[len_out - 1] == '\n') {
            ptr[len_out - 1] = '\0';
            break;
        }
    }
    if (state == NTLMSTATE_TYPE1 && len_out == 3 && ptr[0] == 'P' && ptr[1] == 'W') goto done;
    if (len_out < 4) goto done;
    if (state == NTLMSTATE_TYPE1 && (ptr[0]!='Y' || ptr[1]!='R' || ptr[2]!=' ')) goto done;
    if (state == NTLMSTATE_TYPE2 && (ptr[0]!='K' || ptr[1]!='K' || ptr[2]!=' ') && (ptr[0]!='A' || ptr[1]!='F' || ptr[2]!=' ')) goto done;
    ntlm->response = strdup(ptr + 3);
    Curl_dyn_free(&b);
    if (!ntlm->response) return CURLE_OUT_OF_MEMORY;
    return CURLE_OK;
    done:
    Curl_dyn_free(&b);
    return CURLE_REMOTE_ACCESS_DENIED;
}
CURLcode Curl_input_ntlm_wb(struct connectdata *conn, bool proxy, const char *header) {
    struct ntlmdata *ntlm = proxy ? &conn->proxyntlm : &conn->ntlm;
    curlntlm *state = proxy ? &conn->proxy_ntlm_state : &conn->http_ntlm_state;
    if (!checkprefix("NTLM", header)) return CURLE_BAD_CONTENT_ENCODING;
    header += strlen("NTLM");
    while(*header && ISSPACE(*header)) header++;
    if (*header) {
        ntlm->challenge = strdup(header);
        if (!ntlm->challenge) return CURLE_OUT_OF_MEMORY;
        *state = NTLMSTATE_TYPE2;
    } else {
        if (*state == NTLMSTATE_LAST) {
            infof(conn->data, "NTLM auth restarted\n");
            Curl_http_auth_cleanup_ntlm_wb(conn);
        } else if(*state == NTLMSTATE_TYPE3) {
            infof(conn->data, "NTLM handshake rejected\n");
            Curl_http_auth_cleanup_ntlm_wb(conn);
            *state = NTLMSTATE_NONE;
            return CURLE_REMOTE_ACCESS_DENIED;
        } else if(*state >= NTLMSTATE_TYPE1) {
            infof(conn->data, "NTLM handshake failure (internal error)\n");
            return CURLE_REMOTE_ACCESS_DENIED;
        }
        *state = NTLMSTATE_TYPE1;
    }
    return CURLE_OK;
}
CURLcode Curl_output_ntlm_wb(struct connectdata *conn, bool proxy) {
    char **allocuserpwd;
    const char *userp;
    struct ntlmdata *ntlm;
    curlntlm *state;
    struct auth *authp;
    struct Curl_easy *data = conn->data;
    CURLcode res = CURLE_OK;
    DEBUGASSERT(conn);
    DEBUGASSERT(conn->data);
    if (proxy) {
    #ifndef CURL_DISABLE_PROXY
        allocuserpwd = &data->state.aptr.proxyuserpwd;
        userp = conn->http_proxy.user;
        ntlm = &conn->proxyntlm;
        state = &conn->proxy_ntlm_state;
        authp = &conn->data->state.authproxy;
    #else
        return CURLE_NOT_BUILT_IN;
    #endif
    } else {
        allocuserpwd = &data->state.aptr.userpwd;
        userp = conn->user;
        ntlm = &conn->ntlm;
        state = &conn->http_ntlm_state;
        authp = &conn->data->state.authhost;
    }
    authp->done = FALSE;
    if (!userp) userp = "";
    switch(*state) {
        case NTLMSTATE_TYPE2: {
                char *input = aprintf("TT %s\n", ntlm->challenge);
                if (!input) return CURLE_OUT_OF_MEMORY;
                res = ntlm_wb_response(conn->data, ntlm, input, *state);
                free(input);
                if (res) return res;
                free(*allocuserpwd);
                *allocuserpwd = aprintf("%sAuthorization: NTLM %s\r\n", proxy ? "Proxy-" : "", ntlm->response);
                DEBUG_OUT(fprintf(stderr, "**** %s\n ", *allocuserpwd));
                *state = NTLMSTATE_TYPE3; /* we sent a type-3 */
                authp->done = TRUE;
                Curl_http_auth_cleanup_ntlm_wb(conn);
                if (!*allocuserpwd) return CURLE_OUT_OF_MEMORY;
                break;
            }
        case NTLMSTATE_TYPE3: *state = NTLMSTATE_LAST;
        case NTLMSTATE_LAST:
            Curl_safefree(*allocuserpwd);
            authp->done = TRUE;
            break;
        default:
            res = ntlm_wb_init(conn->data, ntlm, userp);
            if (res) return res;
            res = ntlm_wb_response(conn->data, ntlm, "YR\n", *state);
            if (res) return res;
            free(*allocuserpwd);
            *allocuserpwd = aprintf("%sAuthorization: NTLM %s\r\n", proxy ? "Proxy-" : "", ntlm->response);
            DEBUG_OUT(fprintf(stderr, "**** Header %s\n ", *allocuserpwd));
            Curl_safefree(ntlm->response);
            if (!*allocuserpwd) return CURLE_OUT_OF_MEMORY;
    }
    return CURLE_OK;
}
void Curl_http_auth_cleanup_ntlm_wb(struct connectdata *conn) {
    ntlm_wb_cleanup(&conn->ntlm);
    ntlm_wb_cleanup(&conn->proxyntlm);
}
#endif