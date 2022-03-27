#ifndef HEADER_CURL_FTP_H
#define HEADER_CURL_FTP_H

#include "pingpong.h"

#ifndef CURL_DISABLE_FTP
extern const struct Curl_handler Curl_handler_ftp;
#ifdef USE_SSL
extern const struct Curl_handler Curl_handler_ftps;
#endif
CURLcode Curl_ftpsend(struct connectdata *, const char *cmd);
CURLcode Curl_GetFTPResponse(size_t *nread, struct connectdata *conn, int *ftpcode);
#endif
typedef enum {
    FTP_STOP,
    FTP_WAIT220,
    FTP_AUTH,
    FTP_USER,
    FTP_PASS,
    FTP_ACCT,
    FTP_PBSZ,
    FTP_PROT,
    FTP_CCC,
    FTP_PWD,
    FTP_SYST,
    FTP_NAMEFMT,
    FTP_QUOTE,
    FTP_RETR_PREQUOTE,
    FTP_STOR_PREQUOTE,
    FTP_POSTQUOTE,
    FTP_CWD,
    FTP_MKD,
    FTP_MDTM,
    FTP_TYPE,
    FTP_LIST_TYPE,
    FTP_RETR_TYPE,
    FTP_STOR_TYPE,
    FTP_SIZE,
    FTP_RETR_SIZE,
    FTP_STOR_SIZE,
    FTP_REST,
    FTP_RETR_REST,
    FTP_PORT,
    FTP_PRET,
    FTP_PASV,
    FTP_LIST,
    FTP_RETR,
    FTP_STOR,
    FTP_QUIT,
    FTP_LAST
} ftpstate;
struct ftp_parselist_data;
struct ftp_wc {
    struct ftp_parselist_data *parser;
    struct {
        curl_write_callback write_function;
        FILE *file_descriptor;
    } backup;
};
typedef enum {
    FTPFILE_MULTICWD  = 1,
    FTPFILE_NOCWD     = 2,
    FTPFILE_SINGLECWD = 3
} curl_ftpfile;
struct FTP {
    char *path;
    char *pathalloc;
    curl_pp_transfer transfer;
    curl_off_t downloadsize;
};
struct ftp_conn {
    struct pingpong pp;
    char *entrypath;
    char **dirs;
    int dirdepth;
    char *file;
    bool dont_check;
    bool ctl_valid;
    bool cwddone;
    int cwdcount;
    bool cwdfail;
    bool wait_data_conn;
    char *prevpath;
    char transfertype;
    int count1;
    int count2;
    int count3;
    ftpstate state;
    ftpstate state_saved;
    curl_off_t retr_size_saved;
    char *server_os;
    curl_off_t known_filesize;
    char *newhost;
    unsigned short newport;
};
#define DEFAULT_ACCEPT_TIMEOUT   60000

#endif