#include <stdbool.h>
#include "curl_setup.h"

#ifndef CURL_DISABLE_FILE
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "strtoofft.h"
#include "urldata.h"
#include "curl.h"
#include "progress.h"
#include "sendf.h"
#include "escape.h"
#include "file.h"
#include "speedcheck.h"
#include "getinfo.h"
#include "transfer.h"
#include "url.h"
#include "parsedate.h"
#include "warnless.h"
#include "curl_range.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#if defined(WIN32) || defined(MSDOS) || defined(__EMX__) || defined(__SYMBIAN32__)
#define DOS_FILESYSTEM 1
#endif
#ifdef OPEN_NEEDS_ARG3
#define open_readonly(p,f) open((p),(f),(0))
#else
#define open_readonly(p,f) open((p),(f))
#endif

static CURLcode file_do(struct connectdata *, bool *done);
static CURLcode file_done(struct connectdata *conn, CURLcode status, bool premature);
static CURLcode file_connect(struct connectdata *conn, bool *done);
static CURLcode file_disconnect(struct connectdata *conn, bool dead_connection);
static CURLcode file_setup_connection(struct connectdata *conn);
const struct Curl_handler Curl_handler_file = {
    "FILE",
    file_setup_connection,
    file_do,
    file_done,
    ZERO_NULL,
    file_connect,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    file_disconnect,
    ZERO_NULL,
    ZERO_NULL,
    0,
    CURLPROTO_FILE,
    PROTOPT_NONETWORK | PROTOPT_NOURLQUERY
};
static CURLcode file_setup_connection(struct connectdata *conn) {
    conn->data->req.protop = calloc(1, sizeof(struct FILEPROTO));
    if(!conn->data->req.protop) return CURLE_OUT_OF_MEMORY;
    return CURLE_OK;
}
static CURLcode file_connect(struct connectdata *conn, bool *done) {
    struct Curl_easy *data = conn->data;
    char *real_path;
    struct FILEPROTO *file = data->req.protop;
    int fd;
#ifdef DOS_FILESYSTEM
    size_t i;
    char *actual_path;
#endif
    size_t real_path_len;
    CURLcode result = Curl_urldecode(data, data->state.up.path, 0, &real_path, &real_path_len, REJECT_ZERO);
    if (result) return result;
#ifdef DOS_FILESYSTEM
    actual_path = real_path;
    if((actual_path[0] == '/') && actual_path[1] && (actual_path[2] == ':' || actual_path[2] == '|')) {
        actual_path[2] = ':';
        actual_path++;
        real_path_len--;
    }
    for(i = 0; i < real_path_len; ++i)
        if(actual_path[i] == '/') actual_path[i] = '\\';
        else if(!actual_path[i]) {
            Curl_safefree(real_path);
            return CURLE_URL_MALFORMAT;
        }
    fd = open_readonly(actual_path, O_RDONLY|O_BINARY);
    file->path = actual_path;
#else
    if(memchr(real_path, 0, real_path_len)) {
        Curl_safefree(real_path);
        return CURLE_URL_MALFORMAT;
    }
    fd = open_readonly(real_path, O_RDONLY);
    file->path = real_path;
#endif
    file->freepath = real_path;
    file->fd = fd;
    if(!data->set.upload && (fd == -1)) {
        failf(data, "Couldn't open file %s", data->state.up.path);
        file_done(conn, CURLE_FILE_COULDNT_READ_FILE, FALSE);
        return CURLE_FILE_COULDNT_READ_FILE;
    }
    *done = TRUE;
    return CURLE_OK;
}
static CURLcode file_done(struct connectdata *conn, CURLcode status, bool premature) {
    struct FILEPROTO *file = conn->data->req.protop;
    (void)status;
    (void)premature;
    if(file) {
        Curl_safefree(file->freepath);
        file->path = NULL;
        if(file->fd != -1) close(file->fd);
        file->fd = -1;
    }
    return CURLE_OK;
}
static CURLcode file_disconnect(struct connectdata *conn, bool dead_connection) {
    struct FILEPROTO *file = conn->data->req.protop;
    (void)dead_connection;
    if(file) {
        Curl_safefree(file->freepath);
        file->path = NULL;
        if(file->fd != -1) close(file->fd);
        file->fd = -1;
    }
    return CURLE_OK;
}
#ifdef DOS_FILESYSTEM
    #define DIRSEP '\\'
#else
    #define DIRSEP '/'
#endif
static CURLcode file_upload(struct connectdata *conn) {
    struct FILEPROTO *file = conn->data->req.protop;
    const char *dir = strchr(file->path, DIRSEP);
    int fd;
    int mode;
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    char *buf = data->state.buffer;
    curl_off_t bytecount = 0;
    struct_stat file_stat;
    const char *buf2;
    conn->data->req.upload_fromhere = buf;
    if (!dir) return CURLE_FILE_COULDNT_READ_FILE;
    if (!dir[1]) return CURLE_FILE_COULDNT_READ_FILE;
#ifdef O_BINARY
    #define MODE_DEFAULT O_WRONLY|O_CREAT|O_BINARY
#else
    #define MODE_DEFAULT O_WRONLY|O_CREAT
#endif
    if (data->state.resume_from) mode = MODE_DEFAULT|O_APPEND;
    else mode = MODE_DEFAULT|O_TRUNC;
    fd = open(file->path, mode, conn->data->set.new_file_perms);
    if (fd < 0) {
        failf(data, "Can't open %s for writing", file->path);
        return CURLE_WRITE_ERROR;
    }
    if (-1 != data->state.infilesize) Curl_pgrsSetUploadSize(data, data->state.infilesize);
    if (data->state.resume_from < 0) {
        if (fstat(fd, &file_stat)) {
            close(fd);
            failf(data, "Can't get the size of %s", file->path);
            return CURLE_WRITE_ERROR;
        }
        data->state.resume_from = (curl_off_t)file_stat.st_size;
    }
    while(!result) {
        size_t nread;
        size_t nwrite;
        size_t readcount;
        result = Curl_fillreadbuffer(conn, data->set.buffer_size, &readcount);
        if (result) break;
        if (!readcount) break;
        nread = readcount;
        if(data->state.resume_from) {
            if((curl_off_t)nread <= data->state.resume_from) {
                data->state.resume_from -= nread;
                nread = 0;
                buf2 = buf;
            } else {
                buf2 = buf + data->state.resume_from;
                nread -= (size_t)data->state.resume_from;
                data->state.resume_from = 0;
            }
        } else buf2 = buf;
        nwrite = write(fd, buf2, nread);
        if (nwrite != nread) {
            result = CURLE_SEND_ERROR;
            break;
        }
        bytecount += nread;
        Curl_pgrsSetUploadCounter(data, bytecount);
        if (Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
        else result = Curl_speedcheck(data, Curl_now());
    }
    if (!result && Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
    close(fd);
    return result;
}
static CURLcode file_do(struct connectdata *conn, bool *done) {
    CURLcode result = CURLE_OK;
    struct_stat statbuf;
    curl_off_t expected_size = 0;
    bool size_known;
    bool fstated = FALSE;
    struct Curl_easy *data = conn->data;
    char *buf = data->state.buffer;
    curl_off_t bytecount = 0;
    int fd;
    struct FILEPROTO *file;
    *done = TRUE;
    Curl_pgrsStartNow(data);
    if(data->set.upload) return file_upload(conn);
    file = conn->data->req.protop;
    fd = file->fd;
    if(-1 != fstat(fd, &statbuf)) {
        expected_size = statbuf.st_size;
        data->info.filetime = statbuf.st_mtime;
        fstated = TRUE;
    }
    if (fstated && !data->state.range && data->set.timecondition) {
        if (!Curl_meets_timecondition(data, data->info.filetime)) {
            *done = TRUE;
            return CURLE_OK;
        }
    }
    if (fstated) {
        time_t filetime;
        struct tm buffer;
        const struct tm *tm = &buffer;
        char header[80];
        msnprintf(header, sizeof(header),"Content-Length: %" CURL_FORMAT_CURL_OFF_T "\r\n", expected_size);
        result = Curl_client_write(conn, CLIENTWRITE_HEADER, header, 0);
        if (result) return result;
        result = Curl_client_write(conn, CLIENTWRITE_HEADER,(char *)"Accept-ranges: bytes\r\n", 0);
        if (result) return result;
        filetime = (time_t)statbuf.st_mtime;
        result = Curl_gmtime(filetime, &buffer);
        if (result) return result;
        msnprintf(header, sizeof(header), "Last-Modified: %s, %02d %s %4d %02d:%02d:%02d GMT\r\n%s", Curl_wkday[tm->tm_wday?tm->tm_wday-1:6], tm->tm_mday,
                  Curl_month[tm->tm_mon], tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec, data->set.opt_no_body ? "": "\r\n");
        result = Curl_client_write(conn, CLIENTWRITE_HEADER, header, 0);
        if (result) return result;
        Curl_pgrsSetDownloadSize(data, expected_size);
        if (data->set.opt_no_body) return result;
    }
    result = Curl_range(conn);
    if (result) return result;
    if (data->state.resume_from < 0) {
        if(!fstated) {
            failf(data, "Can't get the size of file.");
            return CURLE_READ_ERROR;
        }
        data->state.resume_from += (curl_off_t)statbuf.st_size;
    }
    if (data->state.resume_from <= expected_size) expected_size -= data->state.resume_from;
    else {
        failf(data, "failed to resume file:// transfer");
        return CURLE_BAD_DOWNLOAD_RESUME;
    }
    if (data->req.maxdownload > 0) expected_size = data->req.maxdownload;
    if (!fstated || (expected_size == 0)) size_known = FALSE;
    else size_known = TRUE;
    if (fstated) Curl_pgrsSetDownloadSize(data, expected_size);
    if (data->state.resume_from) {
        if(data->state.resume_from != lseek(fd, data->state.resume_from, SEEK_SET)) return CURLE_BAD_DOWNLOAD_RESUME;
    }
    Curl_pgrsTime(data, TIMER_STARTTRANSFER);
    while(!result) {
        ssize_t nread;
        size_t bytestoread;
        if (size_known) bytestoread = (expected_size < data->set.buffer_size) ? curlx_sotouz(expected_size) : (size_t)data->set.buffer_size;
        else bytestoread = data->set.buffer_size-1;
        nread = read(fd, buf, bytestoread);
        if(nread > 0) buf[nread] = 0;
        if(nread <= 0 || (size_known && (expected_size == 0))) break;
        bytecount += nread;
        if(size_known) expected_size -= nread;
        result = Curl_client_write(conn, CLIENTWRITE_BODY, buf, nread);
        if(result) return result;
        Curl_pgrsSetDownloadCounter(data, bytecount);
        if(Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
        else result = Curl_speedcheck(data, Curl_now());
    }
    if(Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
    return result;
}
#endif