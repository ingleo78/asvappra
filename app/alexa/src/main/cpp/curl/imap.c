#include <stdbool.h>
#include <assert.h>
#include "curl_setup.h"
#include "curl_ctype.h"

#ifndef CURL_DISABLE_IMAP

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_UTSNAME_H
#include <sys/utsname.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef __VMS
#include <in.h>
#include <inet.h>
#endif

#if (defined(NETWARE) && defined(__NOVELL_LIBC__))
#undef in_addr_t
#define in_addr_t unsigned long
#endif

#include "curl.h"
#include "urldata.h"
#include "sendf.h"
#include "hostip.h"
#include "progress.h"
#include "transfer.h"
#include "escape.h"
#include "http.h" /* for HTTP proxy tunnel stuff */
#include "socks.h"
#include "imap.h"
#include "mime.h"
#include "strtoofft.h"
#include "strcase.h"
#include "vtls/vtls.h"
#include "connect.h"
#include "strerror.h"
#include "select.h"
#include "multiif.h"
#include "url.h"
#include "strcase.h"
#include "curl_sasl.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static CURLcode imap_regular_transfer(struct connectdata *conn, bool *done);
static CURLcode imap_do(struct connectdata *conn, bool *done);
static CURLcode imap_done(struct connectdata *conn, CURLcode status, bool premature);
static CURLcode imap_connect(struct connectdata *conn, bool *done);
static CURLcode imap_disconnect(struct connectdata *conn, bool dead);
static CURLcode imap_multi_statemach(struct connectdata *conn, bool *done);
static int imap_getsock(struct connectdata *conn, curl_socket_t *socks);
static CURLcode imap_doing(struct connectdata *conn, bool *dophase_done);
static CURLcode imap_setup_connection(struct connectdata *conn);
static char *imap_atom(const char *str, bool escape_only);
static CURLcode imap_sendf(struct connectdata *conn, const char *fmt, ...);
static CURLcode imap_parse_url_options(struct connectdata *conn);
static CURLcode imap_parse_url_path(struct connectdata *conn);
static CURLcode imap_parse_custom_request(struct connectdata *conn);
static CURLcode imap_perform_authenticate(struct connectdata *conn, const char *mech, const char *initresp);
static CURLcode imap_continue_authenticate(struct connectdata *conn, const char *resp);
static void imap_get_message(char *buffer, char **outptr);
const struct Curl_handler Curl_handler_imap = {
    "IMAP",
    imap_setup_connection,
    imap_do,
    imap_done,
    ZERO_NULL,
    imap_connect,
    imap_multi_statemach,
    imap_doing,
    imap_getsock,
    imap_getsock,
    ZERO_NULL,
    ZERO_NULL,
    imap_disconnect,
    ZERO_NULL,
    ZERO_NULL,
    PORT_IMAP,
    CURLPROTO_IMAP,
    PROTOPT_CLOSEACTION | PROTOPT_URLOPTIONS
};
#ifdef USE_SSL
const struct Curl_handler Curl_handler_imaps = {
    "IMAPS",
    imap_setup_connection,
    imap_do,
    imap_done,
    ZERO_NULL,
    imap_connect,
    imap_multi_statemach,
    imap_doing,
    imap_getsock,
    imap_getsock,
    ZERO_NULL,
    ZERO_NULL,
    imap_disconnect,
    ZERO_NULL,
    ZERO_NULL,
    PORT_IMAPS,
    CURLPROTO_IMAPS,
    PROTOPT_CLOSEACTION | PROTOPT_SSL | PROTOPT_URLOPTIONS
};
#endif
#define IMAP_RESP_OK       1
#define IMAP_RESP_NOT_OK   2
#define IMAP_RESP_PREAUTH  3
static const struct SASLproto saslimap = {
    "imap",
    '+',
    IMAP_RESP_OK,
    0,
    imap_perform_authenticate,
    imap_continue_authenticate,
    imap_get_message
};
#ifdef USE_SSL
static void imap_to_imaps(struct connectdata *conn) {
    conn->handler = &Curl_handler_imaps;
    conn->bits.tls_upgraded = TRUE;
}
#else
#define imap_to_imaps(x) Curl_nop_stmt
#endif
static bool imap_matchresp(const char *line, size_t len, const char *cmd) {
    const char *end = line + len;
    size_t cmd_len = strlen(cmd);
    line += 2;
    if(line < end && ISDIGIT(*line)) {
        do {
            line++;
        } while(line < end && ISDIGIT(*line));
        if(line == end || *line != ' ') return FALSE;
        line++;
    }
    if(line + cmd_len <= end && strncasecompare(line, cmd, cmd_len) && (line[cmd_len] == ' ' || line + cmd_len + 2 == end)) return TRUE;
    return FALSE;
}
static bool imap_endofresp(struct connectdata *conn, char *line, size_t len, int *resp) {
    struct IMAP *imap = conn->data->req.protop;
    struct imap_conn *imapc = &conn->proto.imapc;
    const char *id = imapc->resptag;
    size_t id_len = strlen(id);
    if(len >= id_len + 1 && !memcmp(id, line, id_len) && line[id_len] == ' ') {
        line += id_len + 1;
        len -= id_len + 1;
        if(len >= 2 && !memcmp(line, "OK", 2)) *resp = IMAP_RESP_OK;
        else if(len >= 7 && !memcmp(line, "PREAUTH", 7)) *resp = IMAP_RESP_PREAUTH;
        else *resp = IMAP_RESP_NOT_OK;
        return TRUE;
    }
    if(len >= 2 && !memcmp("* ", line, 2)) {
        switch(imapc->state) {
            case IMAP_CAPABILITY:
                if(!imap_matchresp(line, len, "CAPABILITY")) return FALSE;
                break;
            case IMAP_LIST:
                if((!imap->custom && !imap_matchresp(line, len, "LIST")) || (imap->custom && !imap_matchresp(line, len, imap->custom) &&
                   (!strcasecompare(imap->custom, "STORE") || !imap_matchresp(line, len, "FETCH")) && !strcasecompare(imap->custom, "SELECT") &&
                   !strcasecompare(imap->custom, "EXAMINE") && !strcasecompare(imap->custom, "SEARCH") &&
                   !strcasecompare(imap->custom, "EXPUNGE") && !strcasecompare(imap->custom, "LSUB") &&
                   !strcasecompare(imap->custom, "UID") && !strcasecompare(imap->custom, "NOOP"))) {
                    return FALSE;
                }
                break;
            case IMAP_SELECT: break;
            case IMAP_FETCH:
                if(!imap_matchresp(line, len, "FETCH")) return FALSE;
                break;
            case IMAP_SEARCH:
                if(!imap_matchresp(line, len, "SEARCH")) return FALSE;
                break;
            default: return FALSE;
        }
        *resp = '*';
        return TRUE;
    }
    if(imap && !imap->custom && ((len == 3 && line[0] == '+') || (len >= 2 && !memcmp("+ ", line, 2)))) {
        switch(imapc->state) {
            case IMAP_AUTHENTICATE: case IMAP_APPEND: *resp = '+'; break;
            default:
                failf(conn->data, "Unexpected continuation response");
                *resp = -1;
                break;
        }
        return TRUE;
    }
    return FALSE;
}
static void imap_get_message(char *buffer, char **outptr) {
    size_t len = strlen(buffer);
    char *message = NULL;
    if(len > 2) {
        len -= 2;
        for(message = buffer + 2; *message == ' ' || *message == '\t'; message++, len--);
        for(; len--;)
            if(message[len] != '\r' && message[len] != '\n' && message[len] != ' ' && message[len] != '\t') break;
        if(++len) message[len] = '\0';
    } else message = &buffer[len];
    *outptr = message;
}
static void state(struct connectdata *conn, imapstate newstate) {
    struct imap_conn *imapc = &conn->proto.imapc;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
    static const char * const names[]={
        "STOP",
        "SERVERGREET",
        "CAPABILITY",
        "STARTTLS",
        "UPGRADETLS",
        "AUTHENTICATE",
        "LOGIN",
        "LIST",
        "SELECT",
        "FETCH",
        "FETCH_FINAL",
        "APPEND",
        "APPEND_FINAL",
        "SEARCH",
        "LOGOUT"
    };
    if(imapc->state != newstate) infof(conn->data, "IMAP %p state change from %s to %s\n", (void*)imapc, names[imapc->state], names[newstate]);
#endif
    imapc->state = newstate;
}
static CURLcode imap_perform_capability(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct imap_conn *imapc = &conn->proto.imapc;
    imapc->sasl.authmechs = SASL_AUTH_NONE;
    imapc->sasl.authused = SASL_AUTH_NONE;
    imapc->tls_supported = FALSE;
    result = imap_sendf(conn, "CAPABILITY");
    if(!result) state(conn, IMAP_CAPABILITY);
    return result;
}
static CURLcode imap_perform_starttls(struct connectdata *conn) {
    CURLcode result = imap_sendf(conn, "STARTTLS");
    if(!result) state(conn, IMAP_STARTTLS);
    return result;
}
static CURLcode imap_perform_upgrade_tls(struct connectdata *conn) {
    struct imap_conn *imapc = &conn->proto.imapc;
    CURLcode result = Curl_ssl_connect_nonblocking(conn, FIRSTSOCKET,&imapc->ssldone);
    if(!result) {
        if(imapc->state != IMAP_UPGRADETLS) state(conn, IMAP_UPGRADETLS);
        if(imapc->ssldone) {
            imap_to_imaps(conn);
            result = imap_perform_capability(conn);
        }
    }
    return result;
}
static CURLcode imap_perform_login(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    char *user;
    char *passwd;
    if(!conn->bits.user_passwd) {
        state(conn, IMAP_STOP);
        return result;
    }
    user = imap_atom(conn->user, false);
    passwd = imap_atom(conn->passwd, false);
    result = imap_sendf(conn, "LOGIN %s %s", user ? user : "", passwd ? passwd : "");
    free(user);
    free(passwd);
    if(!result) state(conn, IMAP_LOGIN);
    return result;
}
static CURLcode imap_perform_authenticate(struct connectdata *conn, const char *mech, const char *initresp) {
    CURLcode result = CURLE_OK;
    if(initresp) result = imap_sendf(conn, "AUTHENTICATE %s %s", mech, initresp);
    else result = imap_sendf(conn, "AUTHENTICATE %s", mech);
    return result;
}
static CURLcode imap_continue_authenticate(struct connectdata *conn, const char *resp) {
    struct imap_conn *imapc = &conn->proto.imapc;
    return Curl_pp_sendf(&imapc->pp, "%s", resp);
}
static CURLcode imap_perform_authentication(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct imap_conn *imapc = &conn->proto.imapc;
    saslprogress progress;
    if(imapc->preauth || !Curl_sasl_can_authenticate(&imapc->sasl, conn)) {
        state(conn, IMAP_STOP);
        return result;
    }
    result = Curl_sasl_start(&imapc->sasl, conn, imapc->ir_supported, &progress);
    if(!result) {
        if(progress == SASL_INPROGRESS) state(conn, IMAP_AUTHENTICATE);
        else if(!imapc->login_disabled && (imapc->preftype & IMAP_TYPE_CLEARTEXT)) result = imap_perform_login(conn);
        else {
            infof(conn->data, "No known authentication mechanisms supported!\n");
            result = CURLE_LOGIN_DENIED;
        }
    }
    return result;
}
static CURLcode imap_perform_list(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap = data->req.protop;
    if(imap->custom) result = imap_sendf(conn, "%s%s", imap->custom, imap->custom_params ? imap->custom_params : "");
    else {
        char *mailbox = imap->mailbox ? imap_atom(imap->mailbox, true) : strdup("");
        if(!mailbox) return CURLE_OUT_OF_MEMORY;
        result = imap_sendf(conn, "LIST \"%s\" *", mailbox);
        free(mailbox);
    }
    if(!result) state(conn, IMAP_LIST);
    return result;
}
static CURLcode imap_perform_select(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap = data->req.protop;
    struct imap_conn *imapc = &conn->proto.imapc;
    char *mailbox;
    Curl_safefree(imapc->mailbox);
    Curl_safefree(imapc->mailbox_uidvalidity);
    if(!imap->mailbox) {
        failf(conn->data, "Cannot SELECT without a mailbox.");
        return CURLE_URL_MALFORMAT;
    }
    mailbox = imap_atom(imap->mailbox, false);
    if(!mailbox) return CURLE_OUT_OF_MEMORY;
    result = imap_sendf(conn, "SELECT %s", mailbox);
    free(mailbox);
    if(!result) state(conn, IMAP_SELECT);
    return result;
}
static CURLcode imap_perform_fetch(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct IMAP *imap = conn->data->req.protop;
    if(imap->uid) {
        if(imap->partial) result = imap_sendf(conn, "UID FETCH %s BODY[%s]<%s>", imap->uid, imap->section ? imap->section : "", imap->partial);
        else result = imap_sendf(conn, "UID FETCH %s BODY[%s]", imap->uid, imap->section ? imap->section : "");
    } else if(imap->mindex) {
        if(imap->partial) result = imap_sendf(conn, "FETCH %s BODY[%s]<%s>", imap->mindex, imap->section ? imap->section : "", imap->partial);
        else result = imap_sendf(conn, "FETCH %s BODY[%s]", imap->mindex, imap->section ? imap->section : "");
    } else {
        failf(conn->data, "Cannot FETCH without a UID.");
        return CURLE_URL_MALFORMAT;
    }
    if(!result) state(conn, IMAP_FETCH);
    return result;
}
static CURLcode imap_perform_append(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap = data->req.protop;
    char *mailbox;
    if(!imap->mailbox) {
        failf(data, "Cannot APPEND without a mailbox.");
        return CURLE_URL_MALFORMAT;
    }
    if(data->set.mimepost.kind != MIMEKIND_NONE) {
        data->set.mimepost.flags &= ~MIME_BODY_ONLY;
        curl_mime_headers(&data->set.mimepost, data->set.headers, 0);
        result = Curl_mime_prepare_headers(&data->set.mimepost, NULL,NULL, MIMESTRATEGY_MAIL);
        if(!result)
            if(!Curl_checkheaders(conn, "Mime-Version")) result = Curl_mime_add_header(&data->set.mimepost.curlheaders,"Mime-Version: 1.0");
        if(!result) result = Curl_mime_rewind(&data->set.mimepost);
        if(result) return result;
        data->state.infilesize = Curl_mime_size(&data->set.mimepost);
        data->state.fread_func = (curl_read_callback) Curl_mime_read;
        data->state.in = (void *) &data->set.mimepost;
    }
    if(data->state.infilesize < 0) {
        failf(data, "Cannot APPEND with unknown input file size\n");
        return CURLE_UPLOAD_FAILED;
    }
    mailbox = imap_atom(imap->mailbox, false);
    if(!mailbox) return CURLE_OUT_OF_MEMORY;
    result = imap_sendf(conn, "APPEND %s (\\Seen) {%" CURL_FORMAT_CURL_OFF_T "}", mailbox, data->state.infilesize);
    free(mailbox);
    if(!result) state(conn, IMAP_APPEND);
    return result;
}
static CURLcode imap_perform_search(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct IMAP *imap = conn->data->req.protop;
    if(!imap->query) {
        failf(conn->data, "Cannot SEARCH without a query string.");
        return CURLE_URL_MALFORMAT;
    }
    result = imap_sendf(conn, "SEARCH %s", imap->query);
    if(!result) state(conn, IMAP_SEARCH);
    return result;
}
static CURLcode imap_perform_logout(struct connectdata *conn) {
    CURLcode result = imap_sendf(conn, "LOGOUT");
    if(!result) state(conn, IMAP_LOGOUT);
    return result;
}
static CURLcode imap_state_servergreet_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    struct Curl_easy *data = conn->data;
    (void)instate;
    if(imapcode == IMAP_RESP_PREAUTH) {
        struct imap_conn *imapc = &conn->proto.imapc;
        imapc->preauth = TRUE;
        infof(data, "PREAUTH connection, already authenticated!\n");
    } else if(imapcode != IMAP_RESP_OK) {
        failf(data, "Got unexpected imap-server response");
        return CURLE_WEIRD_SERVER_REPLY;
    }
    return imap_perform_capability(conn);
}
static CURLcode imap_state_capability_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct imap_conn *imapc = &conn->proto.imapc;
    const char *line = data->state.buffer;
    (void)instate;
    if(imapcode == '*') {
        line += 2;
        for( ; ; ) {
            size_t wordlen;
            while(*line && (*line == ' ' || *line == '\t' || *line == '\r' || *line == '\n')) line++;
            if(!*line) break;
            for(wordlen = 0; line[wordlen] && line[wordlen] != ' ' && line[wordlen] != '\t' && line[wordlen] != '\r' && line[wordlen] != '\n'; wordlen++);
            if(wordlen == 8 && !memcmp(line, "STARTTLS", 8)) imapc->tls_supported = TRUE;
            else if(wordlen == 13 && !memcmp(line, "LOGINDISABLED", 13)) imapc->login_disabled = TRUE;
            else if(wordlen == 7 && !memcmp(line, "SASL-IR", 7)) imapc->ir_supported = TRUE;
            else if(wordlen > 5 && !memcmp(line, "AUTH=", 5)) {
                size_t llen;
                unsigned int mechbit;
                line += 5;
                wordlen -= 5;
                mechbit = Curl_sasl_decode_mech(line, wordlen, &llen);
                if(mechbit && llen == wordlen) imapc->sasl.authmechs |= mechbit;
            }
            line += wordlen;
        }
    } else if(imapcode == IMAP_RESP_OK) {
        if(data->set.use_ssl && !conn->ssl[FIRSTSOCKET].use) {
            if(imapc->tls_supported) result = imap_perform_starttls(conn);
            else if(data->set.use_ssl == CURLUSESSL_TRY) result = imap_perform_authentication(conn);
            else {
                failf(data, "STARTTLS not supported.");
                result = CURLE_USE_SSL_FAILED;
            }
        } else result = imap_perform_authentication(conn);
    } else result = imap_perform_authentication(conn);
    return result;
}
static CURLcode imap_state_starttls_resp(struct connectdata *conn, int imapcode, imapstate instate) {
  CURLcode result = CURLE_OK;
  struct Curl_easy *data = conn->data;
  (void)instate;
  if(imapcode != IMAP_RESP_OK) {
    if(data->set.use_ssl != CURLUSESSL_TRY) {
      failf(data, "STARTTLS denied");
      result = CURLE_USE_SSL_FAILED;
    } else result = imap_perform_authentication(conn);
  } else result = imap_perform_upgrade_tls(conn);
  return result;
}
static CURLcode imap_state_auth_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct imap_conn *imapc = &conn->proto.imapc;
    saslprogress progress;
    (void)instate;
    result = Curl_sasl_continue(&imapc->sasl, conn, imapcode, &progress);
    if(!result)
        switch(progress) {
            case SASL_DONE: state(conn, IMAP_STOP); break;
            case SASL_IDLE:
                if((!imapc->login_disabled) && (imapc->preftype & IMAP_TYPE_CLEARTEXT)) result = imap_perform_login(conn);
                else {
                failf(data, "Authentication cancelled");
                result = CURLE_LOGIN_DENIED;
                }
                break;
        }
    return result;
}
static CURLcode imap_state_login_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    (void)instate;
    if(imapcode != IMAP_RESP_OK) {
        failf(data, "Access denied. %c", imapcode);
        result = CURLE_LOGIN_DENIED;
    } else state(conn, IMAP_STOP);
    return result;
}
static CURLcode imap_state_listsearch_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    char *line = conn->data->state.buffer;
    size_t len = strlen(line);
    (void)instate;
    if(imapcode == '*') {
        line[len] = '\n';
        result = Curl_client_write(conn, CLIENTWRITE_BODY, line, len + 1);
        line[len] = '\0';
    } else if(imapcode != IMAP_RESP_OK) result = CURLE_QUOTE_ERROR;
    else state(conn, IMAP_STOP);
    return result;
}
static CURLcode imap_state_select_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap = conn->data->req.protop;
    struct imap_conn *imapc = &conn->proto.imapc;
    const char *line = data->state.buffer;
    (void)instate;
    if(imapcode == '*') {
        char tmp[20];
        if(sscanf(line + 2, "OK [UIDVALIDITY %19[0123456789]]", tmp) == 1) {
            Curl_safefree(imapc->mailbox_uidvalidity);
            imapc->mailbox_uidvalidity = strdup(tmp);
        }
    } else if(imapcode == IMAP_RESP_OK) {
        if(imap->uidvalidity && imapc->mailbox_uidvalidity && !strcasecompare(imap->uidvalidity, imapc->mailbox_uidvalidity)) {
            failf(conn->data, "Mailbox UIDVALIDITY has changed");
            result = CURLE_REMOTE_FILE_NOT_FOUND;
        } else {
            imapc->mailbox = strdup(imap->mailbox);
            if(imap->custom) result = imap_perform_list(conn);
            else if(imap->query) result = imap_perform_search(conn);
            else result = imap_perform_fetch(conn);
        }
    } else {
        failf(data, "Select failed");
        result = CURLE_LOGIN_DENIED;
    }
    return result;
}
static CURLcode imap_state_fetch_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct imap_conn *imapc = &conn->proto.imapc;
    struct pingpong *pp = &imapc->pp;
    const char *ptr = data->state.buffer;
    bool parsed = FALSE;
    curl_off_t size = 0;
    (void)instate;
    if(imapcode != '*') {
        Curl_pgrsSetDownloadSize(data, -1);
        state(conn, IMAP_STOP);
        return CURLE_REMOTE_FILE_NOT_FOUND;
    }
    while(*ptr && (*ptr != '{')) ptr++;
    if(*ptr == '{') {
        char *endptr;
        if(!curlx_strtoofft(ptr + 1, &endptr, 10, &size)) {
            if(endptr - ptr > 1 && endptr[0] == '}' && endptr[1] == '\r' && endptr[2] == '\0') parsed = TRUE;
        }
    }
    if(parsed) {
        infof(data, "Found %" CURL_FORMAT_CURL_OFF_T " bytes to download\n", size);
        Curl_pgrsSetDownloadSize(data, size);
        if(pp->cache) {
            size_t chunk = pp->cache_size;
            if(chunk > (size_t)size) chunk = (size_t)size;
            if(!chunk) {
                state(conn, IMAP_STOP);
                return CURLE_OK;
            }
            result = Curl_client_write(conn, CLIENTWRITE_BODY, pp->cache, chunk);
            if(result) return result;
            data->req.bytecount += chunk;
            infof(data, "Written %zu bytes, %" CURL_FORMAT_CURL_OFF_TU " bytes are left for transfer\n", chunk, size - chunk);
            if(pp->cache_size > chunk) {
                memmove(pp->cache, pp->cache + chunk, pp->cache_size - chunk);
                pp->cache_size -= chunk;
            } else {
                Curl_safefree(pp->cache);
                pp->cache_size = 0;
            }
        }
        if(data->req.bytecount == size) Curl_setup_transfer(data, -1, -1, FALSE, -1);
        else {
            data->req.maxdownload = size;
            Curl_setup_transfer(data, FIRSTSOCKET, size, FALSE, -1);
        }
    } else {
        failf(pp->conn->data, "Failed to parse FETCH response.");
        result = CURLE_WEIRD_SERVER_REPLY;
    }
    state(conn, IMAP_STOP);
    return result;
}
static CURLcode imap_state_fetch_final_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    (void)instate;
    if(imapcode != IMAP_RESP_OK) result = CURLE_WEIRD_SERVER_REPLY;
    else state(conn, IMAP_STOP);
    return result;
}
static CURLcode imap_state_append_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    (void)instate;
    if(imapcode != '+') result = CURLE_UPLOAD_FAILED;
    else {
        Curl_pgrsSetUploadSize(data, data->state.infilesize);
        Curl_setup_transfer(data, -1, -1, FALSE, FIRSTSOCKET);
        state(conn, IMAP_STOP);
    }
    return result;
}
static CURLcode imap_state_append_final_resp(struct connectdata *conn, int imapcode, imapstate instate) {
    CURLcode result = CURLE_OK;
    (void)instate;
    if(imapcode != IMAP_RESP_OK) result = CURLE_UPLOAD_FAILED;
    else state(conn, IMAP_STOP);
    return result;
}
static CURLcode imap_statemach_act(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    curl_socket_t sock = conn->sock[FIRSTSOCKET];
    int imapcode;
    struct imap_conn *imapc = &conn->proto.imapc;
    struct pingpong *pp = &imapc->pp;
    size_t nread = 0;
    if(imapc->state == IMAP_UPGRADETLS) return imap_perform_upgrade_tls(conn);
    if(pp->sendleft) return Curl_pp_flushsend(pp);
    do {
        result = Curl_pp_readresp(sock, pp, &imapcode, &nread);
        if(result) return result;
        if(imapcode == -1) return CURLE_WEIRD_SERVER_REPLY;
        if(!imapcode) break;
        switch(imapc->state) {
            case IMAP_SERVERGREET: result = imap_state_servergreet_resp(conn, imapcode, imapc->state); break;
            case IMAP_CAPABILITY: result = imap_state_capability_resp(conn, imapcode, imapc->state); break;
            case IMAP_STARTTLS: result = imap_state_starttls_resp(conn, imapcode, imapc->state); break;
            case IMAP_AUTHENTICATE: result = imap_state_auth_resp(conn, imapcode, imapc->state); break;
            case IMAP_LOGIN: result = imap_state_login_resp(conn, imapcode, imapc->state); break;
            case IMAP_LIST: case IMAP_SEARCH: result = imap_state_listsearch_resp(conn, imapcode, imapc->state); break;
            case IMAP_SELECT: result = imap_state_select_resp(conn, imapcode, imapc->state); break;
            case IMAP_FETCH: result = imap_state_fetch_resp(conn, imapcode, imapc->state); break;
            case IMAP_FETCH_FINAL: result = imap_state_fetch_final_resp(conn, imapcode, imapc->state); break;
            case IMAP_APPEND: result = imap_state_append_resp(conn, imapcode, imapc->state); break;
            case IMAP_APPEND_FINAL: result = imap_state_append_final_resp(conn, imapcode, imapc->state); break;
            case IMAP_LOGOUT: default: state(conn, IMAP_STOP); break;
        }
    } while(!result && imapc->state != IMAP_STOP && Curl_pp_moredata(pp));
    return result;
}
static CURLcode imap_multi_statemach(struct connectdata *conn, bool *done) {
    CURLcode result = CURLE_OK;
    struct imap_conn *imapc = &conn->proto.imapc;
    if((conn->handler->flags & PROTOPT_SSL) && !imapc->ssldone) {
        result = Curl_ssl_connect_nonblocking(conn, FIRSTSOCKET, &imapc->ssldone);
        if(result || !imapc->ssldone) return result;
    }
    result = Curl_pp_statemach(&imapc->pp, FALSE, FALSE);
    *done = (imapc->state == IMAP_STOP) ? TRUE : FALSE;
    return result;
}
static CURLcode imap_block_statemach(struct connectdata *conn, bool disconnecting) {
    CURLcode result = CURLE_OK;
    struct imap_conn *imapc = &conn->proto.imapc;
    while(imapc->state != IMAP_STOP && !result) result = Curl_pp_statemach(&imapc->pp, TRUE, disconnecting);
    return result;
}
static CURLcode imap_init(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap;
    imap = data->req.protop = calloc(sizeof(struct IMAP), 1);
    if(!imap) result = CURLE_OUT_OF_MEMORY;
    return result;
}
static int imap_getsock(struct connectdata *conn, curl_socket_t *socks) {
    return Curl_pp_getsock(&conn->proto.imapc.pp, socks);
}
static CURLcode imap_connect(struct connectdata *conn, bool *done) {
    CURLcode result = CURLE_OK;
    struct imap_conn *imapc = &conn->proto.imapc;
    struct pingpong *pp = &imapc->pp;
    *done = FALSE;
    connkeep(conn, "IMAP default");
    pp->response_time = RESP_TIMEOUT;
    pp->statemach_act = imap_statemach_act;
    pp->endofresp = imap_endofresp;
    pp->conn = conn;
    imapc->preftype = IMAP_TYPE_ANY;
    Curl_sasl_init(&imapc->sasl, &saslimap);
    Curl_pp_init(pp);
    result = imap_parse_url_options(conn);
    if(result) return result;
    state(conn, IMAP_SERVERGREET);
    strcpy(imapc->resptag, "*");
    result = imap_multi_statemach(conn, done);
    return result;
}
static CURLcode imap_done(struct connectdata *conn, CURLcode status, bool premature) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap = data->req.protop;
    (void)premature;
    if(!imap) return CURLE_OK;
    if(status) {
    connclose(conn, "IMAP done with bad status");
    result = status;
    } else if(!data->set.connect_only && !imap->custom && (imap->uid || imap->mindex || data->set.upload || data->set.mimepost.kind != MIMEKIND_NONE)) {
        if(!data->set.upload && data->set.mimepost.kind == MIMEKIND_NONE) state(conn, IMAP_FETCH_FINAL);
        else {
            result = Curl_pp_sendf(&conn->proto.imapc.pp, "%s", "");
            if(!result) state(conn, IMAP_APPEND_FINAL);
        }
        if(!result) result = imap_block_statemach(conn, FALSE);
    }
    Curl_safefree(imap->mailbox);
    Curl_safefree(imap->uidvalidity);
    Curl_safefree(imap->uid);
    Curl_safefree(imap->mindex);
    Curl_safefree(imap->section);
    Curl_safefree(imap->partial);
    Curl_safefree(imap->query);
    Curl_safefree(imap->custom);
    Curl_safefree(imap->custom_params);
    imap->transfer = FTPTRANSFER_BODY;
    return result;
}
static CURLcode imap_perform(struct connectdata *conn, bool *connected, bool *dophase_done) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap = data->req.protop;
    struct imap_conn *imapc = &conn->proto.imapc;
    bool selected = FALSE;
    DEBUGF(infof(conn->data, "DO phase starts\n"));
    if(conn->data->set.opt_no_body) imap->transfer = FTPTRANSFER_INFO;
    *dophase_done = FALSE;
    if(imap->mailbox && imapc->mailbox && strcasecompare(imap->mailbox, imapc->mailbox) && (!imap->uidvalidity || !imapc->mailbox_uidvalidity ||
       strcasecompare(imap->uidvalidity, imapc->mailbox_uidvalidity))) {
        selected = TRUE;
    }
    if(conn->data->set.upload || data->set.mimepost.kind != MIMEKIND_NONE) result = imap_perform_append(conn);
    else if(imap->custom && (selected || !imap->mailbox))  result = imap_perform_list(conn);
    else if(!imap->custom && selected && (imap->uid || imap->mindex))  result = imap_perform_fetch(conn);
    else if(!imap->custom && selected && imap->query) result = imap_perform_search(conn);
    else if(imap->mailbox && !selected && (imap->custom || imap->uid || imap->mindex || imap->query))  result = imap_perform_select(conn);
    else result = imap_perform_list(conn);
    if(result) return result;
    result = imap_multi_statemach(conn, dophase_done);
    *connected = conn->bits.tcpconnect[FIRSTSOCKET];
    if(*dophase_done) DEBUGF(infof(conn->data, "DO phase is complete\n"));
    return result;
}
static CURLcode imap_do(struct connectdata *conn, bool *done) {
    CURLcode result = CURLE_OK;
    *done = FALSE;
    result = imap_parse_url_path(conn);
    if(result) return result;
    result = imap_parse_custom_request(conn);
    if(result) return result;
    result = imap_regular_transfer(conn, done);
    return result;
}
static CURLcode imap_disconnect(struct connectdata *conn, bool dead_connection) {
    struct imap_conn *imapc = &conn->proto.imapc;
    if(!dead_connection && imapc->pp.conn && imapc->pp.conn->bits.protoconnstart)
        if(!imap_perform_logout(conn)) (void)imap_block_statemach(conn, TRUE);
    Curl_pp_disconnect(&imapc->pp);
    Curl_sasl_cleanup(conn, imapc->sasl.authused);
    Curl_safefree(imapc->mailbox);
    Curl_safefree(imapc->mailbox_uidvalidity);
    return CURLE_OK;
}
static CURLcode imap_dophase_done(struct connectdata *conn, bool connected) {
    struct IMAP *imap = conn->data->req.protop;
    (void)connected;
    if(imap->transfer != FTPTRANSFER_BODY) Curl_setup_transfer(conn->data, -1, -1, FALSE, -1);
    return CURLE_OK;
}
static CURLcode imap_doing(struct connectdata *conn, bool *dophase_done) {
    CURLcode result = imap_multi_statemach(conn, dophase_done);
    if(result) DEBUGF(infof(conn->data, "DO phase failed\n"));
    else if(*dophase_done) {
        result = imap_dophase_done(conn, FALSE /* not connected */);
        DEBUGF(infof(conn->data, "DO phase is complete\n"));
    }
    return result;
}
static CURLcode imap_regular_transfer(struct connectdata *conn, bool *dophase_done) {
    CURLcode result = CURLE_OK;
    bool connected = FALSE;
    struct Curl_easy *data = conn->data;
    data->req.size = -1;
    Curl_pgrsSetUploadCounter(data, 0);
    Curl_pgrsSetDownloadCounter(data, 0);
    Curl_pgrsSetUploadSize(data, -1);
    Curl_pgrsSetDownloadSize(data, -1);
    result = imap_perform(conn, &connected, dophase_done);
    if(!result && *dophase_done) result = imap_dophase_done(conn, connected);
    return result;
}
static CURLcode imap_setup_connection(struct connectdata *conn) {
    CURLcode result = imap_init(conn);
    if(result) return result;
    conn->bits.tls_upgraded = FALSE;
    return CURLE_OK;
}
static CURLcode imap_sendf(struct connectdata *conn, const char *fmt, ...) {
    CURLcode result = CURLE_OK;
    struct imap_conn *imapc = &conn->proto.imapc;
    char *taggedfmt;
    va_list ap;
    DEBUGASSERT(fmt);
    imapc->cmdid = (imapc->cmdid + 1) % 1000;
    msnprintf(imapc->resptag, sizeof(imapc->resptag), "%c%03d", 'A' + curlx_sltosi(conn->connection_id % 26), imapc->cmdid);
    taggedfmt = aprintf("%s %s", imapc->resptag, fmt);
    if(!taggedfmt) return CURLE_OUT_OF_MEMORY;
    va_start(ap, fmt);
    result = Curl_pp_vsendf(&imapc->pp, taggedfmt, ap);
    va_end(ap);
    free(taggedfmt);
    return result;
}
static char *imap_atom(const char *str, bool escape_only) {
    const char atom_specials[] = "(){ %*]";
    const char *p1;
    char *p2;
    size_t backsp_count = 0;
    size_t quote_count = 0;
    bool others_exists = FALSE;
    size_t newlen = 0;
    char *newstr = NULL;
    if(!str) return NULL;
    p1 = str;
    while(*p1) {
        if(*p1 == '\\') backsp_count++;
        else if(*p1 == '"') quote_count++;
        else if(!escape_only) {
            const char *p3 = atom_specials;
            while(*p3 && !others_exists) {
                if(*p1 == *p3) others_exists = TRUE;
                p3++;
            }
        }
        p1++;
    }
    if(!backsp_count && !quote_count && !others_exists) return strdup(str);
    newlen = strlen(str) + backsp_count + quote_count + (escape_only ? 0 : 2);
    newstr = (char *) malloc((newlen + 1) * sizeof(char));
    if(!newstr) return NULL;
    p2 = newstr;
    if(!escape_only) {
        newstr[0] = '"';
        newstr[newlen - 1] = '"';
        p2++;
    }
    p1 = str;
    while(*p1) {
        if(*p1 == '\\' || *p1 == '"') {
            *p2 = '\\';
            p2++;
        }
        *p2 = *p1;
        p1++;
        p2++;
    }
    newstr[newlen] = '\0';
    return newstr;
}
static bool imap_is_bchar(char ch) {
    switch(ch) {
        case ':': case '@': case '/': case '&': case '=':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case '-': case '.': case '_': case '~':
        case '!': case '$': case '\'': case '(': case ')': case '*':
        case '+': case ',': case '%':
            return true;
        default:
            return false;
    }
}
static CURLcode imap_parse_url_options(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct imap_conn *imapc = &conn->proto.imapc;
    const char *ptr = conn->options;
    imapc->sasl.resetprefs = TRUE;
    while(!result && ptr && *ptr) {
        const char *key = ptr;
        const char *value;
        while(*ptr && *ptr != '=') ptr++;
        value = ptr + 1;
        while(*ptr && *ptr != ';') ptr++;
        if(strncasecompare(key, "AUTH=", 5)) result = Curl_sasl_parse_url_auth_option(&imapc->sasl, value, ptr - value);
        else result = CURLE_URL_MALFORMAT;
        if(*ptr == ';') ptr++;
    }
    switch(imapc->sasl.prefmech) {
        case SASL_AUTH_NONE: imapc->preftype = IMAP_TYPE_NONE; break;
        case SASL_AUTH_DEFAULT: imapc->preftype = IMAP_TYPE_ANY; break;
        default: imapc->preftype = IMAP_TYPE_SASL;
    }
    return result;
}
static CURLcode imap_parse_url_path(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap = data->req.protop;
    const char *begin = &data->state.up.path[1];
    const char *ptr = begin;
    while(imap_is_bchar(*ptr)) ptr++;
    if(ptr != begin) {
        const char *end = ptr;
        if(end > begin && end[-1] == '/') end--;
        result = Curl_urldecode(data, begin, end - begin, &imap->mailbox, NULL,REJECT_CTRL);
        if(result) return result;
    } else imap->mailbox = NULL;
    while(*ptr == ';') {
        char *name;
        char *value;
        size_t valuelen;
        begin = ++ptr;
        while(*ptr && *ptr != '=') ptr++;
        if(!*ptr) return CURLE_URL_MALFORMAT;
        result = Curl_urldecode(data, begin, ptr - begin, &name, NULL,REJECT_CTRL);
        if(result) return result;
        begin = ++ptr;
        while(imap_is_bchar(*ptr)) ptr++;
        result = Curl_urldecode(data, begin, ptr - begin, &value, &valuelen,REJECT_CTRL);
        if(result) {
            free(name);
            return result;
        }
        DEBUGF(infof(conn->data, "IMAP URL parameter '%s' = '%s'\n", name, value));
        if(strcasecompare(name, "UIDVALIDITY") && !imap->uidvalidity) {
            if(valuelen > 0 && value[valuelen - 1] == '/') value[valuelen - 1] = '\0';
            imap->uidvalidity = value;
            value = NULL;
        } else if(strcasecompare(name, "UID") && !imap->uid) {
            if(valuelen > 0 && value[valuelen - 1] == '/') value[valuelen - 1] = '\0';
            imap->uid = value;
            value = NULL;
        } else if(strcasecompare(name, "MAILINDEX") && !imap->mindex) {
            if(valuelen > 0 && value[valuelen - 1] == '/') value[valuelen - 1] = '\0';
            imap->mindex = value;
            value = NULL;
        } else if(strcasecompare(name, "SECTION") && !imap->section) {
            if(valuelen > 0 && value[valuelen - 1] == '/') value[valuelen - 1] = '\0';
            imap->section = value;
            value = NULL;
        } else if(strcasecompare(name, "PARTIAL") && !imap->partial) {
            if(valuelen > 0 && value[valuelen - 1] == '/') value[valuelen - 1] = '\0';
            imap->partial = value;
            value = NULL;
        } else {
            free(name);
            free(value);
            return CURLE_URL_MALFORMAT;
        }
        free(name);
        free(value);
    }
    if(imap->mailbox && !imap->uid && !imap->mindex) (void)curl_url_get(data->state.uh, CURLUPART_QUERY, &imap->query,CURLU_URLDECODE);
    if(*ptr) return CURLE_URL_MALFORMAT;
    return CURLE_OK;
}
static CURLcode imap_parse_custom_request(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct IMAP *imap = data->req.protop;
    const char *custom = data->set.str[STRING_CUSTOMREQUEST];
    if(custom) {
        result = Curl_urldecode(data, custom, 0, &imap->custom, NULL, REJECT_CTRL);
        if(!result) {
            const char *params = imap->custom;
            while(*params && *params != ' ') params++;
            if(*params) {
            imap->custom_params = strdup(params);
            imap->custom[params - imap->custom] = '\0';
            if(!imap->custom_params) result = CURLE_OUT_OF_MEMORY;
            }
        }
    }
    return result;
}
#endif