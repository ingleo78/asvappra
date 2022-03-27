#ifndef HEADER_CURL_SSH_H
#define HEADER_CURL_SSH_H

#include "../curl_setup.h"

#if defined(HAVE_LIBSSH2_H)
#include <libssh2.h>
#include <libssh2_sftp.h>
#elif defined(HAVE_LIBSSH_LIBSSH_H)
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#elif defined(USE_WOLFSSH)
#include <wolfssh/ssh.h>
#include <wolfssh/wolfsftp.h>
#endif

typedef enum {
    SSH_NO_STATE = -1,
    SSH_STOP = 0,
    SSH_INIT,
    SSH_S_STARTUP,
    SSH_HOSTKEY,
    SSH_AUTHLIST,
    SSH_AUTH_PKEY_INIT,
    SSH_AUTH_PKEY,
    SSH_AUTH_PASS_INIT,
    SSH_AUTH_PASS,
    SSH_AUTH_AGENT_LIST,
    SSH_AUTH_AGENT,
    SSH_AUTH_HOST_INIT,
    SSH_AUTH_HOST,
    SSH_AUTH_KEY_INIT,
    SSH_AUTH_KEY,
    SSH_AUTH_GSSAPI,
    SSH_AUTH_DONE,
    SSH_SFTP_INIT,
    SSH_SFTP_REALPATH,
    SSH_SFTP_QUOTE_INIT,
    SSH_SFTP_POSTQUOTE_INIT,
    SSH_SFTP_QUOTE,
    SSH_SFTP_NEXT_QUOTE,
    SSH_SFTP_QUOTE_STAT,
    SSH_SFTP_QUOTE_SETSTAT,
    SSH_SFTP_QUOTE_SYMLINK,
    SSH_SFTP_QUOTE_MKDIR,
    SSH_SFTP_QUOTE_RENAME,
    SSH_SFTP_QUOTE_RMDIR,
    SSH_SFTP_QUOTE_UNLINK,
    SSH_SFTP_QUOTE_STATVFS,
    SSH_SFTP_GETINFO,
    SSH_SFTP_FILETIME,
    SSH_SFTP_TRANS_INIT,
    SSH_SFTP_UPLOAD_INIT,
    SSH_SFTP_CREATE_DIRS_INIT,
    SSH_SFTP_CREATE_DIRS,
    SSH_SFTP_CREATE_DIRS_MKDIR,
    SSH_SFTP_READDIR_INIT,
    SSH_SFTP_READDIR,
    SSH_SFTP_READDIR_LINK,
    SSH_SFTP_READDIR_BOTTOM,
    SSH_SFTP_READDIR_DONE,
    SSH_SFTP_DOWNLOAD_INIT,
    SSH_SFTP_DOWNLOAD_STAT,
    SSH_SFTP_CLOSE,
    SSH_SFTP_SHUTDOWN,
    SSH_SCP_TRANS_INIT,
    SSH_SCP_UPLOAD_INIT,
    SSH_SCP_DOWNLOAD_INIT,
    SSH_SCP_DOWNLOAD,
    SSH_SCP_DONE,
    SSH_SCP_SEND_EOF,
    SSH_SCP_WAIT_EOF,
    SSH_SCP_WAIT_CLOSE,
    SSH_SCP_CHANNEL_FREE,
    SSH_SESSION_DISCONNECT,
    SSH_SESSION_FREE,
    SSH_QUIT,
    SSH_LAST
} sshstate;
struct SSHPROTO {
    char *path;
};
struct ssh_conn {
    const char *authlist;
    const char *passphrase;
    char *rsa_pub;
    char *rsa;
    bool authed;
    sshstate state;
    sshstate nextstate;
    CURLcode actualcode;
    struct curl_slist *quote_item;
    char *quote_path1;
    char *quote_path2;
    bool acceptfail;
    char *homedir;
    char *readdir_line;
    int secondCreateDirs;
    char *slash_pos;
    int orig_waitfor;
#if defined(USE_LIBSSH)
    char *readdir_linkPath;
    size_t readdir_len, readdir_totalLen, readdir_currLen;
    unsigned kbd_state;
    ssh_key privkey;
    ssh_key pubkey;
    int auth_methods;
    ssh_session ssh_session;
    ssh_scp scp_session;
    sftp_session sftp_session;
    sftp_file sftp_file;
    sftp_dir sftp_dir;
    unsigned sftp_recv_state;
    int sftp_file_index;
    sftp_attributes readdir_attrs;
    sftp_attributes readdir_link_attrs;
    sftp_attributes quote_attrs;
    const char *readdir_filename;
    const char *readdir_longentry;
    char *readdir_tmp;
#elif defined(USE_LIBSSH2)
    struct dynbuf readdir_link;
    struct dynbuf readdir;
    char *readdir_filename;
    char *readdir_longentry;
    LIBSSH2_SFTP_ATTRIBUTES quote_attrs;
    LIBSSH2_SFTP_ATTRIBUTES readdir_attrs;
    LIBSSH2_SESSION *ssh_session;
    LIBSSH2_CHANNEL *ssh_channel;
    LIBSSH2_SFTP *sftp_session;
    LIBSSH2_SFTP_HANDLE *sftp_handle;
#ifdef HAVE_LIBSSH2_AGENT_API
    LIBSSH2_AGENT *ssh_agent;
    struct libssh2_agent_publickey *sshagent_identity, *sshagent_prev_identity;
#endif
#ifdef HAVE_LIBSSH2_KNOWNHOST_API
    LIBSSH2_KNOWNHOSTS *kh;
#endif
#elif defined(USE_WOLFSSH)
    WOLFSS* USE_LIBSSH */H *ssh_session;
    WOLFSSH_CTX *ctx;
    word32 handleSz;
    byte handle[WOLFSSH_MAX_HANDLE];
    curl_off_t offset;
#endif
};
#if defined(USE_LIBSSH)
#define CURL_LIBSSH_VERSION ssh_version(0)
#elif defined(USE_LIBSSH2)
#if !defined(LIBSSH2_VERSION_NUM) || (LIBSSH2_VERSION_NUM < 0x001000)
#error "SCP/SFTP protocols require libssh2 0.16 or later"
#endif
#if LIBSSH2_VERSION_NUM >= 0x010000
#define HAVE_LIBSSH2_SFTP_SEEK64 1
#endif
#if LIBSSH2_VERSION_NUM >= 0x010100
#define HAVE_LIBSSH2_VERSION 1
#endif
#if LIBSSH2_VERSION_NUM >= 0x010205
#define HAVE_LIBSSH2_INIT 1
#define HAVE_LIBSSH2_EXIT 1
#endif
#if LIBSSH2_VERSION_NUM >= 0x010206
#define HAVE_LIBSSH2_KNOWNHOST_CHECKP 1
#define HAVE_LIBSSH2_SCP_SEND64 1
#endif
#if LIBSSH2_VERSION_NUM >= 0x010208
#define HAVE_LIBSSH2_SESSION_HANDSHAKE 1
#endif
#ifdef HAVE_LIBSSH2_VERSION
#define CURL_LIBSSH2_VERSION libssh2_version(0)
#else
#define CURL_LIBSSH2_VERSION LIBSSH2_VERSION
#endif
#endif
#ifdef USE_SSH
extern const struct Curl_handler Curl_handler_scp;
extern const struct Curl_handler Curl_handler_sftp;
CURLcode Curl_ssh_init(void);
void Curl_ssh_cleanup(void);
size_t Curl_ssh_version(char *buffer, size_t buflen);
#else
#define Curl_ssh_cleanup()
#endif
#endif