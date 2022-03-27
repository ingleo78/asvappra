#include "../curl_setup.h"

#ifdef USE_WOLFSSH
#include <limits.h>
#include <wolfssh/ssh.h>
#include <wolfssh/wolfsftp.h>
#include "../urldata.h"
#include "../connect.h"
#include "../sendf.h"
#include "../progress.h"
#include "../curl_path.h"
#include "../strtoofft.h"
#include "../transfer.h"
#include "../speedcheck.h"
#include "../select.h"
#include "../multiif.h"
#include "../warnless.h"
#include "../curl_printf.h"
#include "../curl_memory.h"
#include "../memdebug.h"

static CURLcode wssh_connect(struct connectdata *conn, bool *done);
static CURLcode wssh_multi_statemach(struct connectdata *conn, bool *done);
static CURLcode wssh_do(struct connectdata *conn, bool *done);
#if 0
static CURLcode wscp_done(struct connectdata *conn, CURLcode, bool premature);
static CURLcode wscp_doing(struct connectdata *conn, bool *dophase_done);
static CURLcode wscp_disconnect(struct connectdata *conn, bool dead_connection);
#endif
static CURLcode wsftp_done(struct connectdata *conn, CURLcode, bool premature);
static CURLcode wsftp_doing(struct connectdata *conn, bool *dophase_done);
static CURLcode wsftp_disconnect(struct connectdata *conn, bool dead);
static int wssh_getsock(struct connectdata *conn, curl_socket_t *sock);
static int wssh_perform_getsock(const struct connectdata *conn, curl_socket_t *sock);
static CURLcode wssh_setup_connection(struct connectdata *conn);
#if 0
const struct Curl_handler Curl_handler_scp = {
    "SCP",
    wssh_setup_connection,
    wssh_do,
    wscp_done,
    ZERO_NULL,
    wssh_connect,
    wssh_multi_statemach,
    wscp_doing,
    wssh_getsock,
    wssh_getsock,
    ZERO_NULL,
    wssh_perform_getsock,
    wscp_disconnect,
    ZERO_NULL,
    ZERO_NULL,
    PORT_SSH,
    CURLPROTO_SCP,
    PROTOPT_DIRLOCK | PROTOPT_CLOSEACTION | PROTOPT_NOURLQUERY
};
#endif
const struct Curl_handler Curl_handler_sftp = {
    "SFTP",
    wssh_setup_connection,
    wssh_do,
    wsftp_done,
    ZERO_NULL,
    wssh_connect,
    wssh_multi_statemach,
    wsftp_doing,
    wssh_getsock,
    wssh_getsock,
    ZERO_NULL,
    wssh_perform_getsock,
    wsftp_disconnect,
    ZERO_NULL,
    ZERO_NULL,
    PORT_SSH,
    CURLPROTO_SFTP,
    PROTOPT_DIRLOCK | PROTOPT_CLOSEACTION | PROTOPT_NOURLQUERY
};
static void state(struct connectdata *conn, sshstate nowstate) {
    struct ssh_conn *sshc = &conn->proto.sshc;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
    static const char * const names[] = {
        "SSH_STOP",
        "SSH_INIT",
        "SSH_S_STARTUP",
        "SSH_HOSTKEY",
        "SSH_AUTHLIST",
        "SSH_AUTH_PKEY_INIT",
        "SSH_AUTH_PKEY",
        "SSH_AUTH_PASS_INIT",
        "SSH_AUTH_PASS",
        "SSH_AUTH_AGENT_INIT",
        "SSH_AUTH_AGENT_LIST",
        "SSH_AUTH_AGENT",
        "SSH_AUTH_HOST_INIT",
        "SSH_AUTH_HOST",
        "SSH_AUTH_KEY_INIT",
        "SSH_AUTH_KEY",
        "SSH_AUTH_GSSAPI",
        "SSH_AUTH_DONE",
        "SSH_SFTP_INIT",
        "SSH_SFTP_REALPATH",
        "SSH_SFTP_QUOTE_INIT",
        "SSH_SFTP_POSTQUOTE_INIT",
        "SSH_SFTP_QUOTE",
        "SSH_SFTP_NEXT_QUOTE",
        "SSH_SFTP_QUOTE_STAT",
        "SSH_SFTP_QUOTE_SETSTAT",
        "SSH_SFTP_QUOTE_SYMLINK",
        "SSH_SFTP_QUOTE_MKDIR",
        "SSH_SFTP_QUOTE_RENAME",
        "SSH_SFTP_QUOTE_RMDIR",
        "SSH_SFTP_QUOTE_UNLINK",
        "SSH_SFTP_QUOTE_STATVFS",
        "SSH_SFTP_GETINFO",
        "SSH_SFTP_FILETIME",
        "SSH_SFTP_TRANS_INIT",
        "SSH_SFTP_UPLOAD_INIT",
        "SSH_SFTP_CREATE_DIRS_INIT",
        "SSH_SFTP_CREATE_DIRS",
        "SSH_SFTP_CREATE_DIRS_MKDIR",
        "SSH_SFTP_READDIR_INIT",
        "SSH_SFTP_READDIR",
        "SSH_SFTP_READDIR_LINK",
        "SSH_SFTP_READDIR_BOTTOM",
        "SSH_SFTP_READDIR_DONE",
        "SSH_SFTP_DOWNLOAD_INIT",
        "SSH_SFTP_DOWNLOAD_STAT",
        "SSH_SFTP_CLOSE",
        "SSH_SFTP_SHUTDOWN",
        "SSH_SCP_TRANS_INIT",
        "SSH_SCP_UPLOAD_INIT",
        "SSH_SCP_DOWNLOAD_INIT",
        "SSH_SCP_DOWNLOAD",
        "SSH_SCP_DONE",
        "SSH_SCP_SEND_EOF",
        "SSH_SCP_WAIT_EOF",
        "SSH_SCP_WAIT_CLOSE",
        "SSH_SCP_CHANNEL_FREE",
        "SSH_SESSION_DISCONNECT",
        "SSH_SESSION_FREE",
        "QUIT"
    };
    DEBUGASSERT(sizeof(names)/sizeof(names[0]) == SSH_LAST);
    if(sshc->state != nowstate) infof(conn->data, "wolfssh %p state change from %s to %s\n", (void*)sshc, names[sshc->state], names[nowstate]);
#endif
    sshc->state = nowstate;
}
static ssize_t wscp_send(struct connectdata *conn, int sockindex, const void *mem, size_t len, CURLcode *err) {
    ssize_t nwrite = 0;
    (void)conn;
    (void)sockindex;
    (void)mem;
    (void)len;
    (void)err;
    return nwrite;
}
static ssize_t wscp_recv(struct connectdata *conn, int sockindex, char *mem, size_t len, CURLcode *err) {
    ssize_t nread = 0;
    (void)conn;
    (void)sockindex;
    (void)mem;
    (void)len;
    (void)err;
    return nread;
}
static ssize_t wsftp_send(struct connectdata *conn, int sockindex, const void *mem, size_t len, CURLcode *err) {
    struct ssh_conn *sshc = &conn->proto.sshc;
    word32 offset[2];
    int rc;
    (void)sockindex;
    offset[0] = (word32)sshc->offset&0xFFFFFFFF;
    offset[1] = (word32)(sshc->offset>>32)&0xFFFFFFFF;
    rc = wolfSSH_SFTP_SendWritePacket(sshc->ssh_session, sshc->handle, sshc->handleSz, &offset[0], (byte*)mem, (word32)len);
    if(rc == WS_FATAL_ERROR) rc = wolfSSH_get_error(sshc->ssh_session);
    if(rc == WS_WANT_READ) {
        conn->waitfor = KEEP_RECV;
        *err = CURLE_AGAIN;
        return -1;
    } else if(rc == WS_WANT_WRITE) {
        conn->waitfor = KEEP_SEND;
        *err = CURLE_AGAIN;
        return -1;
    }
    if(rc < 0) {
        failf(conn->data, "wolfSSH_SFTP_SendWritePacket returned %d\n", rc);
        return -1;
    }
    DEBUGASSERT(rc == (int)len);
    infof(conn->data, "sent %zd bytes SFTP from offset %zd\n", len, sshc->offset);
    sshc->offset += len;
    return (ssize_t)rc;
}
static ssize_t wsftp_recv(struct connectdata *conn, int sockindex, char *mem, size_t len, CURLcode *err) {
    int rc;
    struct ssh_conn *sshc = &conn->proto.sshc;
    word32 offset[2];
    (void)sockindex;
    offset[0] = (word32)sshc->offset&0xFFFFFFFF;
    offset[1] = (word32)(sshc->offset>>32)&0xFFFFFFFF;
    rc = wolfSSH_SFTP_SendReadPacket(sshc->ssh_session, sshc->handle, sshc->handleSz, &offset[0], (byte*)mem, (word32)len);
    if(rc == WS_FATAL_ERROR) rc = wolfSSH_get_error(sshc->ssh_session);
    if(rc == WS_WANT_READ) {
        conn->waitfor = KEEP_RECV;
        *err = CURLE_AGAIN;
        return -1;
    } else if(rc == WS_WANT_WRITE) {
        conn->waitfor = KEEP_SEND;
        *err = CURLE_AGAIN;
        return -1;
    }
    DEBUGASSERT(rc <= (int)len);
    if(rc < 0) {
        failf(conn->data, "wolfSSH_SFTP_SendReadPacket returned %d\n", rc);
        return -1;
    }
    sshc->offset += len;
    return (ssize_t)rc;
}
static CURLcode wssh_setup_connection(struct connectdata *conn) {
    struct SSHPROTO *ssh;
    conn->data->req.protop = ssh = calloc(1, sizeof(struct SSHPROTO));
    if(!ssh) return CURLE_OUT_OF_MEMORY;
    return CURLE_OK;
}
static Curl_recv wscp_recv, wsftp_recv;
static Curl_send wscp_send, wsftp_send;
static int userauth(byte authtype, WS_UserAuthData* authdata, void *ctx) {
    struct connectdata *conn = ctx;
    DEBUGF(infof(conn->data, "wolfssh callback: type %s\n", authtype == WOLFSSH_USERAUTH_PASSWORD ? "PASSWORD" : "PUBLICCKEY"));
    if(authtype == WOLFSSH_USERAUTH_PASSWORD) {
        authdata->sf.password.password = (byte *)conn->passwd;
        authdata->sf.password.passwordSz = (word32) strlen(conn->passwd);
    }
    return 0;
}
static CURLcode wssh_connect(struct connectdata *conn, bool *done) {
    struct Curl_easy *data = conn->data;
    struct ssh_conn *sshc;
    curl_socket_t sock = conn->sock[FIRSTSOCKET];
    int rc;
    if(!data->req.protop) wssh_setup_connection(conn);
    connkeep(conn, "SSH default");
    if(conn->handler->protocol & CURLPROTO_SCP) {
        conn->recv[FIRSTSOCKET] = wscp_recv;
        conn->send[FIRSTSOCKET] = wscp_send;
    } else {
        conn->recv[FIRSTSOCKET] = wsftp_recv;
        conn->send[FIRSTSOCKET] = wsftp_send;
    }
    sshc = &conn->proto.sshc;
    sshc->ctx = wolfSSH_CTX_new(WOLFSSH_ENDPOINT_CLIENT, NULL);
    if(!sshc->ctx) {
        failf(data, "No wolfSSH context");
        goto error;
    }
    sshc->ssh_session = wolfSSH_new(sshc->ctx);
    if(sshc->ssh_session == NULL) {
        failf(data, "No wolfSSH session");
        goto error;
    }
    rc = wolfSSH_SetUsername(sshc->ssh_session, conn->user);
    if(rc != WS_SUCCESS) {
        failf(data, "wolfSSH failed to set user name");
        goto error;
    }
    wolfSSH_SetUserAuth(sshc->ctx, userauth);
    wolfSSH_SetUserAuthCtx(sshc->ssh_session, conn);
    rc = wolfSSH_set_fd(sshc->ssh_session, (int)sock);
    if(rc) {
        failf(data, "wolfSSH failed to set socket");
        goto error;
    }
#if 0
    wolfSSH_Debugging_ON();
#endif
    *done = TRUE;
    if(conn->handler->protocol & CURLPROTO_SCP) state(conn, SSH_INIT);
    else state(conn, SSH_SFTP_INIT);
    return wssh_multi_statemach(conn, done);
    error:
    wolfSSH_free(sshc->ssh_session);
    wolfSSH_CTX_free(sshc->ctx);
    return CURLE_FAILED_INIT;
}
static CURLcode wssh_statemach_act(struct connectdata *conn, bool *block) {
    CURLcode result = CURLE_OK;
    struct ssh_conn *sshc = &conn->proto.sshc;
    struct Curl_easy *data = conn->data;
    struct SSHPROTO *sftp_scp = data->req.protop;
    WS_SFTPNAME *name;
    int rc = 0;
    *block = FALSE;
    do {
        switch(sshc->state) {
            case SSH_INIT: state(conn, SSH_S_STARTUP);
            case SSH_S_STARTUP:
                rc = wolfSSH_connect(sshc->ssh_session);
                if(rc != WS_SUCCESS) rc = wolfSSH_get_error(sshc->ssh_session);
                if(rc == WS_WANT_READ) {
                    *block = TRUE;
                    conn->waitfor = KEEP_RECV;
                    return CURLE_OK;
                } else if(rc == WS_WANT_WRITE) {
                    *block = TRUE;
                    conn->waitfor = KEEP_SEND;
                    return CURLE_OK;
                } else if(rc != WS_SUCCESS) {
                    state(conn, SSH_STOP);
                    return CURLE_SSH;
                }
                infof(data, "wolfssh connected!\n");
                state(conn, SSH_STOP);
                break;
            case SSH_STOP: break;
            case SSH_SFTP_INIT:
                rc = wolfSSH_SFTP_connect(sshc->ssh_session);
                if(rc != WS_SUCCESS) rc = wolfSSH_get_error(sshc->ssh_session);
                if(rc == WS_WANT_READ) {
                    *block = TRUE;
                    conn->waitfor = KEEP_RECV;
                    return CURLE_OK;
                } else if(rc == WS_WANT_WRITE) {
                    *block = TRUE;
                    conn->waitfor = KEEP_SEND;
                    return CURLE_OK;
                } else if(rc == WS_SUCCESS) {
                    infof(data, "wolfssh SFTP connected!\n");
                    state(conn, SSH_SFTP_REALPATH);
                } else {
                    failf(data, "wolfssh SFTP connect error %d", rc);
                    return CURLE_SSH;
                }
                break;
            case SSH_SFTP_REALPATH:
                name = wolfSSH_SFTP_RealPath(sshc->ssh_session, (char *)".");
                rc = wolfSSH_get_error(sshc->ssh_session);
                if(rc == WS_WANT_READ) {
                    *block = TRUE;
                    conn->waitfor = KEEP_RECV;
                    return CURLE_OK;
                } else if(rc == WS_WANT_WRITE) {
                    *block = TRUE;
                    conn->waitfor = KEEP_SEND;
                    return CURLE_OK;
                } else if(name && (rc == WS_SUCCESS)) {
                    sshc->homedir = malloc(name->fSz + 1);
                    if(!sshc->homedir) sshc->actualcode = CURLE_OUT_OF_MEMORY;
                    else {
                        memcpy(sshc->homedir, name->fName, name->fSz);
                        sshc->homedir[name->fSz] = 0;
                        infof(data, "wolfssh SFTP realpath succeeded!\n");
                    }
                    wolfSSH_SFTPNAME_list_free(name);
                    state(conn, SSH_STOP);
                    return CURLE_OK;
                }
                failf(data, "wolfssh SFTP realpath %d", rc);
                return CURLE_SSH;
            case SSH_SFTP_QUOTE_INIT:
                result = Curl_getworkingpath(conn, sshc->homedir, &sftp_scp->path);
                if(result) {
                    sshc->actualcode = result;
                    state(conn, SSH_STOP);
                    break;
                }
                if(data->set.quote) {
                    infof(data, "Sending quote commands\n");
                    sshc->quote_item = data->set.quote;
                    state(conn, SSH_SFTP_QUOTE);
                } else state(conn, SSH_SFTP_GETINFO);
                break;
            case SSH_SFTP_GETINFO:
                if(data->set.get_filetime) state(conn, SSH_SFTP_FILETIME);
                else state(conn, SSH_SFTP_TRANS_INIT);
                break;
            case SSH_SFTP_TRANS_INIT:
                if(data->set.upload) state(conn, SSH_SFTP_UPLOAD_INIT);
                else {
                    if(sftp_scp->path[strlen(sftp_scp->path)-1] == '/') state(conn, SSH_SFTP_READDIR_INIT);
                    else state(conn, SSH_SFTP_DOWNLOAD_INIT);
                }
                break;
            case SSH_SFTP_UPLOAD_INIT: {
                    word32 flags;
                    WS_SFTP_FILEATRB createattrs;
                    if(data->state.resume_from) {
                        WS_SFTP_FILEATRB attrs;
                        if(data->state.resume_from < 0) {
                            rc = wolfSSH_SFTP_STAT(sshc->ssh_session, sftp_scp->path, &attrs);
                            if(rc != WS_SUCCESS) break;
                            if(rc) data->state.resume_from = 0;
                            else {
                                curl_off_t size = ((curl_off_t)attrs.sz[1] << 32) | attrs.sz[0];
                                if(size < 0) {
                                    failf(data, "Bad file size (%" CURL_FORMAT_CURL_OFF_T ")", size);
                                    return CURLE_BAD_DOWNLOAD_RESUME;
                                }
                                data->state.resume_from = size;
                            }
                        }
                    }
                    if(data->set.ftp_append) flags = WOLFSSH_FXF_WRITE|WOLFSSH_FXF_CREAT|WOLFSSH_FXF_APPEND;
                    else if(data->state.resume_from > 0) flags = WOLFSSH_FXF_WRITE|WOLFSSH_FXF_APPEND;
                    else flags = WOLFSSH_FXF_WRITE|WOLFSSH_FXF_CREAT|WOLFSSH_FXF_TRUNC;
                    memset(&createattrs, 0, sizeof(createattrs));
                    createattrs.per = (word32)data->set.new_file_perms;
                    sshc->handleSz = sizeof(sshc->handle);
                    rc = wolfSSH_SFTP_Open(sshc->ssh_session, sftp_scp->path, flags, &createattrs, sshc->handle, &sshc->handleSz);
                    if(rc == WS_FATAL_ERROR) rc = wolfSSH_get_error(sshc->ssh_session);
                    if(rc == WS_WANT_READ) {
                        *block = TRUE;
                        conn->waitfor = KEEP_RECV;
                        return CURLE_OK;
                    } else if(rc == WS_WANT_WRITE) {
                        *block = TRUE;
                        conn->waitfor = KEEP_SEND;
                        return CURLE_OK;
                    } else if(rc == WS_SUCCESS) infof(data, "wolfssh SFTP open succeeded!\n");
                    else {
                        failf(data, "wolfssh SFTP upload open failed: %d", rc);
                        return CURLE_SSH;
                    }
                    state(conn, SSH_SFTP_DOWNLOAD_STAT);
                    if(data->state.resume_from > 0) {
                        int seekerr = CURL_SEEKFUNC_OK;
                        if(conn->seek_func) {
                            Curl_set_in_callback(data, true);
                            seekerr = conn->seek_func(conn->seek_client, data->state.resume_from, SEEK_SET);
                            Curl_set_in_callback(data, false);
                        }
                        if(seekerr != CURL_SEEKFUNC_OK) {
                            curl_off_t passed = 0;
                            if(seekerr != CURL_SEEKFUNC_CANTSEEK) {
                            failf(data, "Could not seek stream");
                            return CURLE_FTP_COULDNT_USE_REST;
                            }
                            do {
                                size_t readthisamountnow = (data->state.resume_from - passed > data->set.buffer_size) ? (size_t)data->set.buffer_size :
                                                            curlx_sotouz(data->state.resume_from - passed);
                                size_t actuallyread;
                                Curl_set_in_callback(data, true);
                                actuallyread = data->state.fread_func(data->state.buffer, 1, readthisamountnow, data->state.in);
                                Curl_set_in_callback(data, false);
                                passed += actuallyread;
                                if((actuallyread == 0) || (actuallyread > readthisamountnow)) {
                                    failf(data, "Failed to read data");
                                    return CURLE_FTP_COULDNT_USE_REST;
                                }
                            } while(passed < data->state.resume_from);
                        }
                        if(data->state.infilesize > 0) {
                            data->state.infilesize -= data->state.resume_from;
                            data->req.size = data->state.infilesize;
                            Curl_pgrsSetUploadSize(data, data->state.infilesize);
                        }
                        sshc->offset += data->state.resume_from;
                    }
                    if(data->state.infilesize > 0) {
                        data->req.size = data->state.infilesize;
                        Curl_pgrsSetUploadSize(data, data->state.infilesize);
                    }
                    Curl_setup_transfer(data, -1, -1, FALSE, FIRSTSOCKET);
                    conn->sockfd = conn->writesockfd;
                    if(result) {
                        state(conn, SSH_SFTP_CLOSE);
                        sshc->actualcode = result;
                    } else {
                        sshc->orig_waitfor = data->req.keepon;
                        conn->cselect_bits = CURL_CSELECT_OUT;
                        Curl_expire(data, 0, EXPIRE_RUN_NOW);
                        state(conn, SSH_STOP);
                    }
                    break;
                }
            case SSH_SFTP_DOWNLOAD_INIT:
                sshc->handleSz = sizeof(sshc->handle);
                rc = wolfSSH_SFTP_Open(sshc->ssh_session, sftp_scp->path, WOLFSSH_FXF_READ, NULL, sshc->handle, &sshc->handleSz);
                if(rc == WS_FATAL_ERROR) rc = wolfSSH_get_error(sshc->ssh_session);
                if(rc == WS_WANT_READ) {
                    *block = TRUE;
                    conn->waitfor = KEEP_RECV;
                    return CURLE_OK;
                } else if(rc == WS_WANT_WRITE) {
                    *block = TRUE;
                    conn->waitfor = KEEP_SEND;
                    return CURLE_OK;
                } else if(rc == WS_SUCCESS) {
                    infof(data, "wolfssh SFTP open succeeded!\n");
                    state(conn, SSH_SFTP_DOWNLOAD_STAT);
                    return CURLE_OK;
                }
                failf(data, "wolfssh SFTP open failed: %d", rc);
                return CURLE_SSH;
            case SSH_SFTP_DOWNLOAD_STAT: {
                    WS_SFTP_FILEATRB attrs;
                    curl_off_t size;
                    rc = wolfSSH_SFTP_STAT(sshc->ssh_session, sftp_scp->path, &attrs);
                    if(rc == WS_FATAL_ERROR) rc = wolfSSH_get_error(sshc->ssh_session);
                    if(rc == WS_WANT_READ) {
                        *block = TRUE;
                        conn->waitfor = KEEP_RECV;
                        return CURLE_OK;
                    } else if(rc == WS_WANT_WRITE) {
                        *block = TRUE;
                        conn->waitfor = KEEP_SEND;
                        return CURLE_OK;
                    } else if(rc == WS_SUCCESS) infof(data, "wolfssh STAT succeeded!\n");
                    else {
                        failf(data, "wolfssh SFTP open failed: %d", rc);
                        data->req.size = -1;
                        data->req.maxdownload = -1;
                        Curl_pgrsSetDownloadSize(data, -1);
                        return CURLE_SSH;
                    }
                    size = ((curl_off_t)attrs.sz[1] <<32) | attrs.sz[0];
                    data->req.size = size;
                    data->req.maxdownload = size;
                    Curl_pgrsSetDownloadSize(data, size);
                    infof(data, "SFTP download %" CURL_FORMAT_CURL_OFF_T " bytes\n", size);
                    if(conn->data->state.use_range || data->state.resume_from) {
                        infof(data, "wolfSSH cannot do range/seek on SFTP\n");
                        return CURLE_BAD_DOWNLOAD_RESUME;
                    }
                    if(data->req.size == 0) {
                        Curl_setup_transfer(data, -1, -1, FALSE, -1);
                        infof(data, "File already completely downloaded\n");
                        state(conn, SSH_STOP);
                        break;
                    }
                    Curl_setup_transfer(data, FIRSTSOCKET, data->req.size, FALSE, -1);
                    conn->writesockfd = conn->sockfd;
                    conn->cselect_bits = CURL_CSELECT_IN;
                    if(result) {
                        state(conn, SSH_SFTP_CLOSE);
                        sshc->actualcode = result;
                    } else state(conn, SSH_STOP);
                    break;
                }
            case SSH_SFTP_CLOSE:
                if(sshc->handleSz)
                rc = wolfSSH_SFTP_Close(sshc->ssh_session, sshc->handle, sshc->handleSz);
                else rc = WS_SUCCESS;
                if(rc == WS_WANT_READ) {
                    *block = TRUE;
                    conn->waitfor = KEEP_RECV;
                    return CURLE_OK;
                } else if(rc == WS_WANT_WRITE) {
                    *block = TRUE;
                    conn->waitfor = KEEP_SEND;
                    return CURLE_OK;
                } else if(rc == WS_SUCCESS) {
                    state(conn, SSH_STOP);
                    return CURLE_OK;
                }
                failf(data, "wolfssh SFTP CLOSE failed: %d", rc);
                return CURLE_SSH;
            case SSH_SFTP_READDIR_INIT:
                Curl_pgrsSetDownloadSize(data, -1);
                if(data->set.opt_no_body) {
                    state(conn, SSH_STOP);
                    break;
                }
                state(conn, SSH_SFTP_READDIR);
            case SSH_SFTP_READDIR:
                name = wolfSSH_SFTP_LS(sshc->ssh_session, sftp_scp->path);
                if(!name) rc = wolfSSH_get_error(sshc->ssh_session);
                else rc = WS_SUCCESS;
                if(rc == WS_WANT_READ) {
                    *block = TRUE;
                    conn->waitfor = KEEP_RECV;
                    return CURLE_OK;
                } else if(rc == WS_WANT_WRITE) {
                    *block = TRUE;
                    conn->waitfor = KEEP_SEND;
                    return CURLE_OK;
                } else if(name && (rc == WS_SUCCESS)) {
                    WS_SFTPNAME *origname = name;
                    result = CURLE_OK;
                    while(name) {
                        char *line = aprintf("%s\n", data->set.ftp_list_only ? name->fName : name->lName);
                        if(line == NULL) {
                            state(conn, SSH_SFTP_CLOSE);
                            sshc->actualcode = CURLE_OUT_OF_MEMORY;
                            break;
                        }
                        result = Curl_client_write(conn, CLIENTWRITE_BODY, line, strlen(line));
                        free(line);
                        if(result) {
                            sshc->actualcode = result;
                            break;
                        }
                        name = name->next;
                    }
                    wolfSSH_SFTPNAME_list_free(origname);
                    state(conn, SSH_STOP);
                    return result;
                }
                failf(data, "wolfssh SFTP ls failed: %d", rc);
                return CURLE_SSH;
            case SSH_SFTP_SHUTDOWN:
                Curl_safefree(sshc->homedir);
                wolfSSH_free(sshc->ssh_session);
                wolfSSH_CTX_free(sshc->ctx);
                state(conn, SSH_STOP);
                return CURLE_OK;
            default: break;
        }
    } while(!rc && (sshc->state != SSH_STOP));
    return result;
}
static CURLcode wssh_multi_statemach(struct connectdata *conn, bool *done) {
    struct ssh_conn *sshc = &conn->proto.sshc;
    CURLcode result = CURLE_OK;
    bool block;
    do {
        result = wssh_statemach_act(conn, &block);
        *done = (sshc->state == SSH_STOP) ? TRUE : FALSE;
        if(*done) DEBUGF(infof(conn->data, "wssh_statemach_act says DONE\n"));
    } while(!result && !*done && !block);
    return result;
}
static CURLcode wscp_perform(struct connectdata *conn, bool *connected, bool *dophase_done) {
    (void)conn;
    (void)connected;
    (void)dophase_done;
    return CURLE_OK;
}
static CURLcode wsftp_perform(struct connectdata *conn, bool *connected, bool *dophase_done) {
    CURLcode result = CURLE_OK;
    DEBUGF(infof(conn->data, "DO phase starts\n"));
    *dophase_done = FALSE;
    state(conn, SSH_SFTP_QUOTE_INIT);
    result = wssh_multi_statemach(conn, dophase_done);
    *connected = conn->bits.tcpconnect[FIRSTSOCKET];
    if(*dophase_done) DEBUGF(infof(conn->data, "DO phase is complete\n"));
    return result;
}
static CURLcode wssh_do(struct connectdata *conn, bool *done) {
    CURLcode result;
    bool connected = 0;
    struct Curl_easy *data = conn->data;
    struct ssh_conn *sshc = &conn->proto.sshc;
    *done = FALSE;
    data->req.size = -1;
    sshc->actualcode = CURLE_OK;
    sshc->secondCreateDirs = 0;
    Curl_pgrsSetUploadCounter(data, 0);
    Curl_pgrsSetDownloadCounter(data, 0);
    Curl_pgrsSetUploadSize(data, -1);
    Curl_pgrsSetDownloadSize(data, -1);
    if(conn->handler->protocol & CURLPROTO_SCP) result = wscp_perform(conn, &connected,  done);
    else result = wsftp_perform(conn, &connected,  done);
    return result;
}
static CURLcode wssh_block_statemach(struct connectdata *conn, bool disconnect) {
    struct ssh_conn *sshc = &conn->proto.sshc;
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    while((sshc->state != SSH_STOP) && !result) {
        bool block;
        timediff_t left = 1000;
        struct curltime now = Curl_now();
        result = wssh_statemach_act(conn, &block);
        if(result) break;
        if(!disconnect) {
            if(Curl_pgrsUpdate(conn)) return CURLE_ABORTED_BY_CALLBACK;
            result = Curl_speedcheck(data, now);
            if(result) break;
            left = Curl_timeleft(data, NULL, FALSE);
            if(left < 0) {
                failf(data, "Operation timed out");
                return CURLE_OPERATION_TIMEDOUT;
            }
        }
        if(!result) {
            int dir = conn->waitfor;
            curl_socket_t sock = conn->sock[FIRSTSOCKET];
            curl_socket_t fd_read = CURL_SOCKET_BAD;
            curl_socket_t fd_write = CURL_SOCKET_BAD;
            if(dir == KEEP_RECV) fd_read = sock;
            else if(dir == KEEP_SEND) fd_write = sock;
            (void)Curl_socket_check(fd_read, CURL_SOCKET_BAD, fd_write, left>1000?1000:left);
        }
    }
    return result;
}
static CURLcode wssh_done(struct connectdata *conn, CURLcode status) {
    CURLcode result = CURLE_OK;
    struct SSHPROTO *sftp_scp = conn->data->req.protop;
    if(!status) result = wssh_block_statemach(conn, FALSE);
    else result = status;
    if(sftp_scp) Curl_safefree(sftp_scp->path);
    if(Curl_pgrsDone(conn)) return CURLE_ABORTED_BY_CALLBACK;
    conn->data->req.keepon = 0;
    return result;
}
#if 0
static CURLcode wscp_done(struct connectdata *conn, CURLcode code, bool premature) {
    CURLcode result = CURLE_OK;
    (void)conn;
    (void)code;
    (void)premature;
    return result;
}
static CURLcode wscp_doing(struct connectdata *conn, bool *dophase_done) {
    CURLcode result = CURLE_OK;
    (void)conn;
    (void)dophase_done;
    return result;
}
static CURLcode wscp_disconnect(struct connectdata *conn, bool dead_connection) {
    CURLcode result = CURLE_OK;
    (void)conn;
    (void)dead_connection;
    return result;
}
#endif
static CURLcode wsftp_done(struct connectdata *conn, CURLcode code, bool premature) {
    (void)premature;
    state(conn, SSH_SFTP_CLOSE);
    return wssh_done(conn, code);
}
static CURLcode wsftp_doing(struct connectdata *conn, bool *dophase_done) {
    CURLcode result = wssh_multi_statemach(conn, dophase_done);
    if(*dophase_done) DEBUGF(infof(conn->data, "DO phase is complete\n"));
    return result;
}
static CURLcode wsftp_disconnect(struct connectdata *conn, bool dead) {
    CURLcode result = CURLE_OK;
    (void)dead;
    DEBUGF(infof(conn->data, "SSH DISCONNECT starts now\n"));
    if(conn->proto.sshc.ssh_session) {
        state(conn, SSH_SFTP_SHUTDOWN);
        result = wssh_block_statemach(conn, TRUE);
    }
    DEBUGF(infof(conn->data, "SSH DISCONNECT is done\n"));
    return result;
}
static int wssh_getsock(struct connectdata *conn, curl_socket_t *sock) {
    return wssh_perform_getsock(conn, sock);
}
static int wssh_perform_getsock(const struct connectdata *conn, curl_socket_t *sock) {
    int bitmap = GETSOCK_BLANK;
    int dir = conn->waitfor;
    sock[0] = conn->sock[FIRSTSOCKET];
    if(dir == KEEP_RECV) bitmap |= GETSOCK_READSOCK(FIRSTSOCKET);
    else if(dir == KEEP_SEND) bitmap |= GETSOCK_WRITESOCK(FIRSTSOCKET);
    return bitmap;
}
size_t Curl_ssh_version(char *buffer, size_t buflen) {
    return msnprintf(buffer, buflen, "wolfssh/%s", LIBWOLFSSH_VERSION_STRING);
}
CURLcode Curl_ssh_init(void) {
    if(WS_SUCCESS != wolfSSH_Init()) {
        DEBUGF(fprintf(stderr, "Error: wolfSSH_Init failed\n"));
        return CURLE_FAILED_INIT;
    }
    return CURLE_OK;
}
void Curl_ssh_cleanup(void) {}
#endif