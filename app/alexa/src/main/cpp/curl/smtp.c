#include "curl_setup.h"
#ifndef CURL_DISABLE_SMTP
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
#include <stdbool.h>
#include <assert.h>
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
#include "http.h"
#include "mime.h"
#include "socks.h"
#include "smtp.h"
#include "strtoofft.h"
#include "strcase.h"
#include "vtls/vtls.h"
#include "connect.h"
#include "strerror.h"
#include "select.h"
#include "multiif.h"
#include "url.h"
#include "curl_gethostname.h"
#include "curl_sasl.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#include "curl_ctype.h"

static CURLcode smtp_regular_transfer(struct connectdata *conn, bool *done);
static CURLcode smtp_do(struct connectdata *conn, bool *done);
static CURLcode smtp_done(struct connectdata *conn, CURLcode status, bool premature);
static CURLcode smtp_connect(struct connectdata *conn, bool *done);
static CURLcode smtp_disconnect(struct connectdata *conn, bool dead);
static CURLcode smtp_multi_statemach(struct connectdata *conn, bool *done);
static int smtp_getsock(struct connectdata *conn, curl_socket_t *socks);
static CURLcode smtp_doing(struct connectdata *conn, bool *dophase_done);
static CURLcode smtp_setup_connection(struct connectdata *conn);
static CURLcode smtp_parse_url_options(struct connectdata *conn);
static CURLcode smtp_parse_url_path(struct connectdata *conn);
static CURLcode smtp_parse_custom_request(struct connectdata *conn);
static CURLcode smtp_parse_address(struct connectdata *conn, const char *fqma, char **address, struct hostname *host);
static CURLcode smtp_perform_auth(struct connectdata *conn, const char *mech, const char *initresp);
static CURLcode smtp_continue_auth(struct connectdata *conn, const char *resp);
static void smtp_get_message(char *buffer, char **outptr);
const struct Curl_handler Curl_handler_smtp = {
    "SMTP",
    smtp_setup_connection,
    smtp_do,
    smtp_done,
    ZERO_NULL,
    smtp_connect,
    smtp_multi_statemach,
    smtp_doing,
    smtp_getsock,
    smtp_getsock,
    ZERO_NULL,
    ZERO_NULL,
    smtp_disconnect,
    ZERO_NULL,
    ZERO_NULL,
    PORT_SMTP,
    CURLPROTO_SMTP,
    PROTOPT_CLOSEACTION | PROTOPT_NOURLQUERY | PROTOPT_URLOPTIONS
};
#ifdef USE_SSL
const struct Curl_handler Curl_handler_smtps = {
    "SMTPS",
    smtp_setup_connection,
    smtp_do,
    smtp_done,
    ZERO_NULL,
    smtp_connect,
    smtp_multi_statemach,
    smtp_doing,
    smtp_getsock,
    smtp_getsock,
    ZERO_NULL,
    ZERO_NULL,
    smtp_disconnect,
    ZERO_NULL,
    ZERO_NULL,
    PORT_SMTPS,
    CURLPROTO_SMTPS,
    PROTOPT_CLOSEACTION | PROTOPT_SSL | PROTOPT_NOURLQUERY | PROTOPT_URLOPTIONS
};
#endif
static const struct SASLproto saslsmtp = {
    "smtp",
    334,
    235,
    512 - 8,
    smtp_perform_auth,
    smtp_continue_auth,
    smtp_get_message
};
#ifdef USE_SSL
static void smtp_to_smtps(struct connectdata *conn) {
    conn->handler = &Curl_handler_smtps;
    conn->bits.tls_upgraded = TRUE;
}
#else
#define smtp_to_smtps(x) Curl_nop_stmt
#endif
static bool smtp_endofresp(struct connectdata *conn, char *line, size_t len, int *resp) {
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    bool result = FALSE;
    if(len < 4 || !ISDIGIT(line[0]) || !ISDIGIT(line[1]) || !ISDIGIT(line[2])) return FALSE;
    if(line[3] == ' ' || len == 5) {
        char tmpline[6];
        result = TRUE;
        memset(tmpline, '\0', sizeof(tmpline));
        memcpy(tmpline, line, (len == 5 ? 5 : 3));
        *resp = curlx_sltosi(strtol(tmpline, NULL, 10));
        if(*resp == 1) *resp = 0;
    } else if(line[3] == '-' && (smtpc->state == SMTP_EHLO || smtpc->state == SMTP_COMMAND)) {
        result = TRUE;
        *resp = 1;
    }
    return result;
}
static void smtp_get_message(char *buffer, char **outptr) {
    size_t len = strlen(buffer);
    char *message = NULL;
    if(len > 4) {
        len -= 4;
        for(message = buffer + 4; *message == ' ' || *message == '\t'; message++, len--);
        for(; len--;) if(message[len] != '\r' && message[len] != '\n' && message[len] != ' ' && message[len] != '\t') break;
        if(++len)  message[len] = '\0';
    } else message = &buffer[len];
    *outptr = message;
}
static void state(struct connectdata *conn, smtpstate newstate) {
    struct smtp_conn *smtpc = &conn->proto.smtpc;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
    static const char * const names[] = {
        "STOP",
        "SERVERGREET",
        "EHLO",
        "HELO",
        "STARTTLS",
        "UPGRADETLS",
        "AUTH",
        "COMMAND",
        "MAIL",
        "RCPT",
        "DATA",
        "POSTDATA",
        "QUIT"
    };
    if(smtpc->state != newstate) infof(conn->data, "SMTP %p state change from %s to %s\n", (void*)smtpc, names[smtpc->state], names[newstate]);
#endif
    smtpc->state = newstate;
}
static CURLcode smtp_perform_ehlo(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    smtpc->sasl.authmechs = SASL_AUTH_NONE;
    smtpc->sasl.authused = SASL_AUTH_NONE;
    smtpc->tls_supported = FALSE;
    smtpc->auth_supported = FALSE;
    result = Curl_pp_sendf(&smtpc->pp, "EHLO %s", smtpc->domain);
    if(!result) state(conn, SMTP_EHLO);
    return result;
}
static CURLcode smtp_perform_helo(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    smtpc->sasl.authused = SASL_AUTH_NONE;
    result = Curl_pp_sendf(&smtpc->pp, "HELO %s", smtpc->domain);
    if(!result) state(conn, SMTP_HELO);
    return result;
}
static CURLcode smtp_perform_starttls(struct connectdata *conn) {
    CURLcode result = Curl_pp_sendf(&conn->proto.smtpc.pp, "%s", "STARTTLS");
    if(!result) state(conn, SMTP_STARTTLS);
    return result;
}
static CURLcode smtp_perform_upgrade_tls(struct connectdata *conn) {
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    CURLcode result = Curl_ssl_connect_nonblocking(conn, FIRSTSOCKET,&smtpc->ssldone);
    if(!result) {
        if(smtpc->state != SMTP_UPGRADETLS) state(conn, SMTP_UPGRADETLS);
        if(smtpc->ssldone) {
            smtp_to_smtps(conn);
            result = smtp_perform_ehlo(conn);
        }
    }
    return result;
}
static CURLcode smtp_perform_auth(struct connectdata *conn, const char *mech, const char *initresp) {
    CURLcode result = CURLE_OK;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    if(initresp) result = Curl_pp_sendf(&smtpc->pp, "AUTH %s %s", mech, initresp);
    else result = Curl_pp_sendf(&smtpc->pp, "AUTH %s", mech);
    return result;
}
static CURLcode smtp_continue_auth(struct connectdata *conn, const char *resp) {
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    return Curl_pp_sendf(&smtpc->pp, "%s", resp);
}
static CURLcode smtp_perform_authentication(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    saslprogress progress;
    if(!smtpc->auth_supported || !Curl_sasl_can_authenticate(&smtpc->sasl, conn)) {
        state(conn, SMTP_STOP);
        return result;
    }
    result = Curl_sasl_start(&smtpc->sasl, conn, FALSE, &progress);
    if(!result) {
        if(progress == SASL_INPROGRESS) state(conn, SMTP_AUTH);
        else {
            infof(conn->data, "No known authentication mechanisms supported!\n");
            result = CURLE_LOGIN_DENIED;
        }
    }
    return result;
}
static CURLcode smtp_perform_command(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp = data->req.protop;
    if(smtp->rcpt) {
        bool utf8 = FALSE;
        if((!smtp->custom) || (!smtp->custom[0])) {
            char *address = NULL;
            struct hostname host = { NULL, NULL, NULL, NULL };
            result = smtp_parse_address(conn, smtp->rcpt->data, &address, &host);
            if(result) return result;
            utf8 = (conn->proto.smtpc.utf8_supported) && ((host.encalloc) || (!Curl_is_ASCII_name(address)) || (!Curl_is_ASCII_name(host.name)));
            result = Curl_pp_sendf(&conn->proto.smtpc.pp, "VRFY %s%s%s%s", address, host.name ? "@" : "", host.name ? host.name : "", utf8 ? " SMTPUTF8" : "");
            Curl_free_idnconverted_hostname(&host);
            free(address);
        } else {
            utf8 = (conn->proto.smtpc.utf8_supported) && (!strcmp(smtp->custom, "EXPN"));
            result = Curl_pp_sendf(&conn->proto.smtpc.pp, "%s %s%s", smtp->custom, smtp->rcpt->data, utf8 ? " SMTPUTF8" : "");
        }
    } else result = Curl_pp_sendf(&conn->proto.smtpc.pp, "%s", smtp->custom && smtp->custom[0] != '\0' ? smtp->custom : "HELP");
    if(!result) state(conn, SMTP_COMMAND);
    return result;
}
static CURLcode smtp_perform_mail(struct connectdata *conn) {
    char *from = NULL;
    char *auth = NULL;
    char *size = NULL;
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    bool utf8 = FALSE;
    if(data->set.str[STRING_MAIL_FROM]) {
        char *address = NULL;
        struct hostname host = { NULL, NULL, NULL, NULL };
        result = smtp_parse_address(conn, data->set.str[STRING_MAIL_FROM], &address, &host);
        if(result) return result;
        utf8 = (conn->proto.smtpc.utf8_supported) && ((host.encalloc) || (!Curl_is_ASCII_name(address)) || (!Curl_is_ASCII_name(host.name)));
        if(host.name) {
            from = aprintf("<%s@%s>", address, host.name);
            Curl_free_idnconverted_hostname(&host);
        } else from = aprintf("<%s>", address);
        free(address);
    } else from = strdup("<>");
    if(!from) return CURLE_OUT_OF_MEMORY;
    if(data->set.str[STRING_MAIL_AUTH] && conn->proto.smtpc.sasl.authused) {
        if(data->set.str[STRING_MAIL_AUTH][0] != '\0') {
            char *address = NULL;
            struct hostname host = { NULL, NULL, NULL, NULL };
            result = smtp_parse_address(conn, data->set.str[STRING_MAIL_AUTH], &address, &host);
            if(result) {
                free(from);
                return result;
            }
            if((!utf8) && (conn->proto.smtpc.utf8_supported) && ((host.encalloc) || (!Curl_is_ASCII_name(address)) || (!Curl_is_ASCII_name(host.name))))
                utf8 = TRUE;
            if(host.name) {
                auth = aprintf("<%s@%s>", address, host.name);
                Curl_free_idnconverted_hostname(&host);
            } else auth = aprintf("<%s>", address);
            free(address);
        } else auth = strdup("<>");
        if(!auth) {
            free(from);
            return CURLE_OUT_OF_MEMORY;
        }
    }
    if(data->set.mimepost.kind != MIMEKIND_NONE) {
        data->set.mimepost.flags &= ~MIME_BODY_ONLY;
        curl_mime_headers(&data->set.mimepost, data->set.headers, 0);
        result = Curl_mime_prepare_headers(&data->set.mimepost, NULL, NULL, MIMESTRATEGY_MAIL);
        if(!result)
            if(!Curl_checkheaders(conn, "Mime-Version")) result = Curl_mime_add_header(&data->set.mimepost.curlheaders, "Mime-Version: 1.0");
        if(!result) result = Curl_mime_rewind(&data->set.mimepost);
        if(result) {
            free(from);
            free(auth);
            return result;
        }
        data->state.infilesize = Curl_mime_size(&data->set.mimepost);
        data->state.fread_func = (curl_read_callback) Curl_mime_read;
        data->state.in = (void *) &data->set.mimepost;
    }
    if(conn->proto.smtpc.size_supported && data->state.infilesize > 0) {
      size = aprintf("%" CURL_FORMAT_CURL_OFF_T, data->state.infilesize);
      if(!size) {
          free(from);
          free(auth);
          return CURLE_OUT_OF_MEMORY;
      }
    }
    if(conn->proto.smtpc.utf8_supported && !utf8) {
        struct SMTP *smtp = data->req.protop;
        struct curl_slist *rcpt = smtp->rcpt;
        while(rcpt && !utf8) {
            if(!Curl_is_ASCII_name(rcpt->data)) utf8 = TRUE;
            rcpt = rcpt->next;
        }
    }
    result = Curl_pp_sendf(&conn->proto.smtpc.pp, "MAIL FROM:%s%s%s%s%s%s", from, auth ? " AUTH=" : "", auth ? auth : "", size ? " SIZE=" : "",
                           size ? size : "", utf8 ? " SMTPUTF8" : "");
    free(from);
    free(auth);
    free(size);
    if(!result) state(conn, SMTP_MAIL);
    return result;
}
static CURLcode smtp_perform_rcpt_to(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp = data->req.protop;
    char *address = NULL;
    struct hostname host = { NULL, NULL, NULL, NULL };
    result = smtp_parse_address(conn, smtp->rcpt->data, &address, &host);
    if(result) return result;
    if(host.name) result = Curl_pp_sendf(&conn->proto.smtpc.pp, "RCPT TO:<%s@%s>", address, host.name);
    else result = Curl_pp_sendf(&conn->proto.smtpc.pp, "RCPT TO:<%s>", address);
    Curl_free_idnconverted_hostname(&host);
    free(address);
    if(!result) state(conn, SMTP_RCPT);
    return result;
}
static CURLcode smtp_perform_quit(struct connectdata *conn) {
    CURLcode result = Curl_pp_sendf(&conn->proto.smtpc.pp, "%s", "QUIT");
    if(!result) state(conn, SMTP_QUIT);
    return result;
}
static CURLcode smtp_state_servergreet_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    (void)instate;
    if(smtpcode/100 != 2) {
        failf(data, "Got unexpected smtp-server response: %d", smtpcode);
        result = CURLE_WEIRD_SERVER_REPLY;
    } else result = smtp_perform_ehlo(conn);
    return result;
}
static CURLcode smtp_state_starttls_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
  CURLcode result = CURLE_OK;
  struct Curl_easy *data = conn->data;
  (void)instate;
  if(smtpcode != 220) {
    if(data->set.use_ssl != CURLUSESSL_TRY) {
      failf(data, "STARTTLS denied, code %d", smtpcode);
      result = CURLE_USE_SSL_FAILED;
    } else result = smtp_perform_authentication(conn);
  } else result = smtp_perform_upgrade_tls(conn);
  return result;
}
static CURLcode smtp_state_ehlo_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    const char *line = data->state.buffer;
    size_t len = strlen(line);
    (void)instate;
    if(smtpcode/100 != 2 && smtpcode != 1) {
        if(data->set.use_ssl <= CURLUSESSL_TRY || conn->ssl[FIRSTSOCKET].use) result = smtp_perform_helo(conn);
        else {
            failf(data, "Remote access denied: %d", smtpcode);
            result = CURLE_REMOTE_ACCESS_DENIED;
        }
    } else if(len >= 4) {
        line += 4;
        len -= 4;
        if(len >= 8 && !memcmp(line, "STARTTLS", 8)) smtpc->tls_supported = TRUE;
        else if(len >= 4 && !memcmp(line, "SIZE", 4)) smtpc->size_supported = TRUE;
        else if(len >= 8 && !memcmp(line, "SMTPUTF8", 8)) smtpc->utf8_supported = TRUE;
        else if(len >= 5 && !memcmp(line, "AUTH ", 5)) {
            smtpc->auth_supported = TRUE;
            line += 5;
            len -= 5;
            for(;;) {
                size_t llen;
                size_t wordlen;
                unsigned int mechbit;
                while(len && (*line == ' ' || *line == '\t' || *line == '\r' || *line == '\n')) {
                    line++;
                    len--;
                }
                if(!len) break;
                for(wordlen = 0; wordlen < len && line[wordlen] != ' ' && line[wordlen] != '\t' && line[wordlen] != '\r' && line[wordlen] != '\n';) wordlen++;
                mechbit = Curl_sasl_decode_mech(line, wordlen, &llen);
                if(mechbit && llen == wordlen) smtpc->sasl.authmechs |= mechbit;
                line += wordlen;
                len -= wordlen;
            }
        }
        if(smtpcode != 1) {
            if(data->set.use_ssl && !conn->ssl[FIRSTSOCKET].use) {
                if(smtpc->tls_supported)  result = smtp_perform_starttls(conn);
                else if(data->set.use_ssl == CURLUSESSL_TRY) result = smtp_perform_authentication(conn);
                else {
                    failf(data, "STARTTLS not supported.");
                    result = CURLE_USE_SSL_FAILED;
                }
            } else result = smtp_perform_authentication(conn);
        }
    } else {
        failf(data, "Unexpectedly short EHLO response");
        result = CURLE_WEIRD_SERVER_REPLY;
    }
    return result;
}
static CURLcode smtp_state_helo_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    (void)instate;
    if(smtpcode/100 != 2) {
        failf(data, "Remote access denied: %d", smtpcode);
        result = CURLE_REMOTE_ACCESS_DENIED;
    } else state(conn, SMTP_STOP);
    return result;
}
static CURLcode smtp_state_auth_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    saslprogress progress;
    (void)instate;
    result = Curl_sasl_continue(&smtpc->sasl, conn, smtpcode, &progress);
    if(!result)
        switch(progress) {
        case SASL_DONE: state(conn, SMTP_STOP);  break;
        case SASL_IDLE:
            failf(data, "Authentication cancelled");
            result = CURLE_LOGIN_DENIED;
            break;
        }
    return result;
}
static CURLcode smtp_state_command_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp = data->req.protop;
    char *line = data->state.buffer;
    size_t len = strlen(line);
    (void)instate;
    if((smtp->rcpt && smtpcode/100 != 2 && smtpcode != 553 && smtpcode != 1) || (!smtp->rcpt && smtpcode/100 != 2 && smtpcode != 1)) {
        failf(data, "Command failed: %d", smtpcode);
        result = CURLE_RECV_ERROR;
    } else {
        if(!data->set.opt_no_body) {
            line[len] = '\n';
            result = Curl_client_write(conn, CLIENTWRITE_BODY, line, len + 1);
            line[len] = '\0';
        }
        if(smtpcode != 1) {
            if(smtp->rcpt) {
                smtp->rcpt = smtp->rcpt->next;
                if(smtp->rcpt) result = smtp_perform_command(conn);
                else state(conn, SMTP_STOP);
            } else state(conn, SMTP_STOP);
        }
    }
    return result;
}
static CURLcode smtp_state_mail_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    (void)instate;
    if(smtpcode/100 != 2) {
        failf(data, "MAIL failed: %d", smtpcode);
        result = CURLE_SEND_ERROR;
    } else result = smtp_perform_rcpt_to(conn);
    return result;
}
static CURLcode smtp_state_rcpt_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp = data->req.protop;
    bool is_smtp_err = FALSE;
    bool is_smtp_blocking_err = FALSE;
    (void)instate;
    is_smtp_err = (smtpcode/100 != 2) ? TRUE : FALSE;
    is_smtp_blocking_err = (is_smtp_err && !data->set.mail_rcpt_allowfails) ? TRUE : FALSE;
    if(is_smtp_err) {
        smtp->rcpt_last_error = smtpcode;
        if(is_smtp_blocking_err) {
            failf(data, "RCPT failed: %d", smtpcode);
            result = CURLE_SEND_ERROR;
        }
    } else smtp->rcpt_had_ok = TRUE;
    if(!is_smtp_blocking_err) {
        smtp->rcpt = smtp->rcpt->next;
        if(smtp->rcpt) result = smtp_perform_rcpt_to(conn);
        else {
            if(!smtp->rcpt_had_ok) {
                failf(data, "RCPT failed: %d (last error)", smtp->rcpt_last_error);
                result = CURLE_SEND_ERROR;
            } else {
                result = Curl_pp_sendf(&conn->proto.smtpc.pp, "%s", "DATA");
                if(!result) state(conn, SMTP_DATA);
            }
        }
    }
    return result;
}
static CURLcode smtp_state_data_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    (void)instate;
    if(smtpcode != 354) {
        failf(data, "DATA failed: %d", smtpcode);
        result = CURLE_SEND_ERROR;
    } else {
        Curl_pgrsSetUploadSize(data, data->state.infilesize);
        Curl_setup_transfer(data, -1, -1, FALSE, FIRSTSOCKET);
        state(conn, SMTP_STOP);
    }
    return result;
}
static CURLcode smtp_state_postdata_resp(struct connectdata *conn, int smtpcode, smtpstate instate) {
    CURLcode result = CURLE_OK;
    (void)instate;
    if(smtpcode != 250) result = CURLE_RECV_ERROR;
    state(conn, SMTP_STOP);
    return result;
}
static CURLcode smtp_statemach_act(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    curl_socket_t sock = conn->sock[FIRSTSOCKET];
    struct Curl_easy *data = conn->data;
    int smtpcode;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    struct pingpong *pp = &smtpc->pp;
    size_t nread = 0;
    if(smtpc->state == SMTP_UPGRADETLS) return smtp_perform_upgrade_tls(conn);
    if(pp->sendleft) return Curl_pp_flushsend(pp);
    do {
        result = Curl_pp_readresp(sock, pp, &smtpcode, &nread);
        if(result) return result;
        if(smtpc->state != SMTP_QUIT && smtpcode != 1) data->info.httpcode = smtpcode;
        if(!smtpcode) break;
        switch(smtpc->state) {
            case SMTP_SERVERGREET: result = smtp_state_servergreet_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_EHLO: result = smtp_state_ehlo_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_HELO: result = smtp_state_helo_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_STARTTLS: result = smtp_state_starttls_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_AUTH: result = smtp_state_auth_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_COMMAND: result = smtp_state_command_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_MAIL: result = smtp_state_mail_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_RCPT: result = smtp_state_rcpt_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_DATA: result = smtp_state_data_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_POSTDATA: result = smtp_state_postdata_resp(conn, smtpcode, smtpc->state); break;
            case SMTP_QUIT: default: state(conn, SMTP_STOP); break;
        }
    } while(!result && smtpc->state != SMTP_STOP && Curl_pp_moredata(pp));
    return result;
}
static CURLcode smtp_multi_statemach(struct connectdata *conn, bool *done) {
    CURLcode result = CURLE_OK;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    if((conn->handler->flags & PROTOPT_SSL) && !smtpc->ssldone) {
        result = Curl_ssl_connect_nonblocking(conn, FIRSTSOCKET, &smtpc->ssldone);
        if(result || !smtpc->ssldone) return result;
    }
    result = Curl_pp_statemach(&smtpc->pp, FALSE, FALSE);
    *done = (smtpc->state == SMTP_STOP) ? TRUE : FALSE;
    return result;
}
static CURLcode smtp_block_statemach(struct connectdata *conn, bool disconnecting) {
    CURLcode result = CURLE_OK;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    while(smtpc->state != SMTP_STOP && !result) result = Curl_pp_statemach(&smtpc->pp, TRUE, disconnecting);
    return result;
}
static CURLcode smtp_init(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp;
    smtp = data->req.protop = calloc(sizeof(struct SMTP), 1);
    if(!smtp) result = CURLE_OUT_OF_MEMORY;
    return result;
}
static int smtp_getsock(struct connectdata *conn, curl_socket_t *socks) {
  return Curl_pp_getsock(&conn->proto.smtpc.pp, socks);
}
static CURLcode smtp_connect(struct connectdata *conn, bool *done) {
    CURLcode result = CURLE_OK;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    struct pingpong *pp = &smtpc->pp;
    *done = FALSE;
    connkeep(conn, "SMTP default");
    pp->response_time = RESP_TIMEOUT;
    pp->statemach_act = smtp_statemach_act;
    pp->endofresp = smtp_endofresp;
    pp->conn = conn;
    Curl_sasl_init(&smtpc->sasl, &saslsmtp);
    Curl_pp_init(pp);
    result = smtp_parse_url_options(conn);
    if(result) return result;
    result = smtp_parse_url_path(conn);
    if(result) return result;
    state(conn, SMTP_SERVERGREET);
    result = smtp_multi_statemach(conn, done);
    return result;
}
static CURLcode smtp_done(struct connectdata *conn, CURLcode status, bool premature) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp = data->req.protop;
    struct pingpong *pp = &conn->proto.smtpc.pp;
    char *eob;
    ssize_t len;
    ssize_t bytes_written;
    (void)premature;
    if(!smtp || !pp->conn) return CURLE_OK;
    Curl_safefree(smtp->custom);
    if(status) {
        connclose(conn, "SMTP done with bad status");
        result = status;
    } else if(!data->set.connect_only && data->set.mail_rcpt && (data->set.upload || data->set.mimepost.kind)) {
        if(smtp->trailing_crlf || !conn->data->state.infilesize) {
            eob = strdup(&SMTP_EOB[2]);
            len = SMTP_EOB_LEN - 2;
        } else {
            eob = strdup(SMTP_EOB);
            len = SMTP_EOB_LEN;
        }
        if(!eob) return CURLE_OUT_OF_MEMORY;
        result = Curl_write(conn, conn->writesockfd, eob, len, &bytes_written);
        if(result) {
            free(eob);
            return result;
        }
        if(bytes_written != len) {
            pp->sendthis = eob;
            pp->sendsize = len;
            pp->sendleft = len - bytes_written;
        } else {
            pp->response = Curl_now();
            free(eob);
        }
        state(conn, SMTP_POSTDATA);
        result = smtp_block_statemach(conn, FALSE);
    }
    smtp->transfer = FTPTRANSFER_BODY;
    return result;
}
static CURLcode smtp_perform(struct connectdata *conn, bool *connected, bool *dophase_done) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp = data->req.protop;
    DEBUGF(infof(conn->data, "DO phase starts\n"));
    if(data->set.opt_no_body) smtp->transfer = FTPTRANSFER_INFO;
    *dophase_done = FALSE;
    smtp->rcpt = data->set.mail_rcpt;
    smtp->rcpt_had_ok = FALSE;
    smtp->rcpt_last_error = 0;
    smtp->trailing_crlf = TRUE;
    smtp->eob = 2;
    if((data->set.upload || data->set.mimepost.kind) && data->set.mail_rcpt) result = smtp_perform_mail(conn);
    else result = smtp_perform_command(conn);
    if(result) return result;
    result = smtp_multi_statemach(conn, dophase_done);
    *connected = conn->bits.tcpconnect[FIRSTSOCKET];
    if(*dophase_done) DEBUGF(infof(conn->data, "DO phase is complete\n"));
    return result;
}
static CURLcode smtp_do(struct connectdata *conn, bool *done) {
    CURLcode result = CURLE_OK;
    *done = FALSE;
    result = smtp_parse_custom_request(conn);
    if(result) return result;
    result = smtp_regular_transfer(conn, done);
    return result;
}
static CURLcode smtp_disconnect(struct connectdata *conn, bool dead_connection) {
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    if(!dead_connection && smtpc->pp.conn && smtpc->pp.conn->bits.protoconnstart)
        if(!smtp_perform_quit(conn)) (void)smtp_block_statemach(conn, TRUE);
    Curl_pp_disconnect(&smtpc->pp);
    Curl_sasl_cleanup(conn, smtpc->sasl.authused);
    Curl_safefree(smtpc->domain);
    return CURLE_OK;
}
static CURLcode smtp_dophase_done(struct connectdata *conn, bool connected) {
    struct SMTP *smtp = conn->data->req.protop;
    (void)connected;
    if(smtp->transfer != FTPTRANSFER_BODY) Curl_setup_transfer(conn->data, -1, -1, FALSE, -1);
    return CURLE_OK;
}
static CURLcode smtp_doing(struct connectdata *conn, bool *dophase_done) {
    CURLcode result = smtp_multi_statemach(conn, dophase_done);
    if(result) DEBUGF(infof(conn->data, "DO phase failed\n"));
    else if(*dophase_done) {
        result = smtp_dophase_done(conn, FALSE);
        DEBUGF(infof(conn->data, "DO phase is complete\n"));
    }
    return result;
}
static CURLcode smtp_regular_transfer(struct connectdata *conn, bool *dophase_done) {
    CURLcode result = CURLE_OK;
    bool connected = FALSE;
    struct Curl_easy *data = conn->data;
    data->req.size = -1;
    Curl_pgrsSetUploadCounter(data, 0);
    Curl_pgrsSetDownloadCounter(data, 0);
    Curl_pgrsSetUploadSize(data, -1);
    Curl_pgrsSetDownloadSize(data, -1);
    result = smtp_perform(conn, &connected, dophase_done);
    if(!result && *dophase_done) result = smtp_dophase_done(conn, connected);
    return result;
}
static CURLcode smtp_setup_connection(struct connectdata *conn) {
    CURLcode result;
    conn->bits.tls_upgraded = FALSE;
    result = smtp_init(conn);
    if(result) return result;
    return CURLE_OK;
}
static CURLcode smtp_parse_url_options(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    const char *ptr = conn->options;
    smtpc->sasl.resetprefs = TRUE;
    while(!result && ptr && *ptr) {
        const char *key = ptr;
        const char *value;
        while(*ptr && *ptr != '=') ptr++;
        value = ptr + 1;
        while(*ptr && *ptr != ';') ptr++;
        if(strncasecompare(key, "AUTH=", 5)) result = Curl_sasl_parse_url_auth_option(&smtpc->sasl, value, ptr - value);
        else result = CURLE_URL_MALFORMAT;
        if(*ptr == ';') ptr++;
    }
    return result;
}
static CURLcode smtp_parse_url_path(struct connectdata *conn) {
    struct Curl_easy *data = conn->data;
    struct smtp_conn *smtpc = &conn->proto.smtpc;
    const char *path = &data->state.up.path[1];
    char localhost[HOSTNAME_MAX + 1];
    if(!*path) {
        if(!Curl_gethostname(localhost, sizeof(localhost))) path = localhost;
        else path = "localhost";
    }
    return Curl_urldecode(conn->data, path, 0, &smtpc->domain, NULL,REJECT_CTRL);
}
static CURLcode smtp_parse_custom_request(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp = data->req.protop;
    const char *custom = data->set.str[STRING_CUSTOMREQUEST];
    if(custom) result = Curl_urldecode(data, custom, 0, &smtp->custom, NULL, REJECT_CTRL);
    return result;
}
static CURLcode smtp_parse_address(struct connectdata *conn, const char *fqma, char **address, struct hostname *host) {
    CURLcode result = CURLE_OK;
    size_t length;
    char *dup = strdup(fqma[0] == '<' ? fqma + 1  : fqma);
    if(!dup) return CURLE_OUT_OF_MEMORY;
    length = strlen(dup);
    if(dup[length - 1] == '>') dup[length - 1] = '\0';
    host->name = strpbrk(dup, "@");
    if(host->name) {
        *host->name = '\0';
        host->name = host->name + 1;
        (void) Curl_idnconvert_hostname(conn, host);
    }
    *address = dup;
    return result;
}
CURLcode Curl_smtp_escape_eob(struct connectdata *conn, const ssize_t nread) {
    ssize_t i;
    ssize_t si;
    struct Curl_easy *data = conn->data;
    struct SMTP *smtp = data->req.protop;
    char *scratch = data->state.scratch;
    char *newscratch = NULL;
    char *oldscratch = NULL;
    size_t eob_sent;
    if(!scratch || data->set.crlf) {
        oldscratch = scratch;
        scratch = newscratch = malloc(2 * data->set.upload_buffer_size);
        if(!newscratch) {
            failf(data, "Failed to alloc scratch buffer!");
            return CURLE_OUT_OF_MEMORY;
        }
    }
    DEBUGASSERT(data->set.upload_buffer_size >= (size_t)nread);
    eob_sent = smtp->eob;
    for(i = 0, si = 0; i < nread; i++) {
        if(SMTP_EOB[smtp->eob] == data->req.upload_fromhere[i]) {
            smtp->eob++;
            if(2 == smtp->eob || SMTP_EOB_LEN == smtp->eob) smtp->trailing_crlf = TRUE;
            else smtp->trailing_crlf = FALSE;
        } else if(smtp->eob) {
            memcpy(&scratch[si], &SMTP_EOB[eob_sent], smtp->eob - eob_sent);
            si += smtp->eob - eob_sent;
            if(SMTP_EOB[0] == data->req.upload_fromhere[i]) smtp->eob = 1;
            else smtp->eob = 0;
            eob_sent = 0;
            smtp->trailing_crlf = FALSE;
        }
        if(SMTP_EOB_FIND_LEN == smtp->eob) {
            memcpy(&scratch[si], &SMTP_EOB_REPL[eob_sent],SMTP_EOB_REPL_LEN - eob_sent);
            si += SMTP_EOB_REPL_LEN - eob_sent;
            smtp->eob = 0;
            eob_sent = 0;
        } else if(!smtp->eob) scratch[si++] = data->req.upload_fromhere[i];
    }
    if(smtp->eob - eob_sent) {
        memcpy(&scratch[si], &SMTP_EOB[eob_sent], smtp->eob - eob_sent);
        si += smtp->eob - eob_sent;
    }
    if(si != nread) {
        data->req.upload_fromhere = scratch;
        data->state.scratch = scratch;
        free(oldscratch);
        data->req.upload_present = si;
    } else free(newscratch);
    return CURLE_OK;
}

#endif