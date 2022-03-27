#include <netinet/in.h>
#include <linux/tcp.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include "urldata.h"
#include "sendf.h"
#include "connect.h"
#include "vtls/vtls.h"
#include "vssh/ssh.h"
#include "easyif.h"
#include "multiif.h"
#include "non-ascii.h"
#include "strerror.h"
#include "select.h"
#include "strdup.h"
#include "http2.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#ifdef CURL_DO_LINEEND_CONV
static size_t convert_lineends(struct Curl_easy *data, char *startPtr, size_t size) {
    char *inPtr, *outPtr;
    if((startPtr == NULL) || (size < 1)) return size;
    if(data->state.prev_block_had_trailing_cr) {
        if(*startPtr == '\n') {
            memmove(startPtr, startPtr + 1, size-1);
            size--;
            data->state.crlf_conversions++;
        }
        data->state.prev_block_had_trailing_cr = FALSE;
    }
    inPtr = outPtr = memchr(startPtr, '\r', size);
    if(inPtr) {
        while(inPtr < (startPtr + size-1)) {
            if(memcmp(inPtr, "\r\n", 2) == 0) {
                inPtr++;
                *outPtr = *inPtr;
                data->state.crlf_conversions++;
            } else {
                if(*inPtr == '\r') *outPtr = '\n';
                else *outPtr = *inPtr;
            }
            outPtr++;
            inPtr++;
        }
        if(inPtr < startPtr + size) {
            if(*inPtr == '\r') {
                *outPtr = '\n';
                data->state.prev_block_had_trailing_cr = TRUE;
            } else *outPtr = *inPtr;
            outPtr++;
        }
        if(outPtr < startPtr + size)*outPtr = '\0';
        return (outPtr - startPtr);
    }
    return size;
}
#endif
#ifdef USE_RECV_BEFORE_SEND_WORKAROUND
bool Curl_recv_has_postponed_data(struct connectdata *conn, int sockindex)
{
  struct postponed_data * const psnd = &(conn->postponed[sockindex]);
  return psnd->buffer && psnd->allocated_size &&
         psnd->recv_size > psnd->recv_processed;
}

static void pre_receive_plain(struct connectdata *conn, int num)
{
  const curl_socket_t sockfd = conn->sock[num];
  struct postponed_data * const psnd = &(conn->postponed[num]);
  size_t bytestorecv = psnd->allocated_size - psnd->recv_size;
  /* WinSock will destroy unread received data if send() is
     failed.
     To avoid lossage of received data, recv() must be
     performed before every send() if any incoming data is
     available. However, skip this, if buffer is already full. */
  if((conn->handler->protocol&PROTO_FAMILY_HTTP) != 0 &&
     conn->recv[num] == Curl_recv_plain &&
     (!psnd->buffer || bytestorecv)) {
    const int readymask = Curl_socket_check(sockfd, CURL_SOCKET_BAD,
                                            CURL_SOCKET_BAD, 0);
    if(readymask != -1 && (readymask & CURL_CSELECT_IN) != 0) {
      /* Have some incoming data */
      if(!psnd->buffer) {
        /* Use buffer double default size for intermediate buffer */
        psnd->allocated_size = 2 * conn->data->set.buffer_size;
        psnd->buffer = malloc(psnd->allocated_size);
        psnd->recv_size = 0;
        psnd->recv_processed = 0;
#ifdef DEBUGBUILD
        psnd->bindsock = sockfd; /* Used only for DEBUGASSERT */
#endif /* DEBUGBUILD */
        bytestorecv = psnd->allocated_size;
      }
      if(psnd->buffer) {
        ssize_t recvedbytes;
        DEBUGASSERT(psnd->bindsock == sockfd);
        recvedbytes = sread(sockfd, psnd->buffer + psnd->recv_size,
                            bytestorecv);
        if(recvedbytes > 0)
          psnd->recv_size += recvedbytes;
      }
      else
        psnd->allocated_size = 0;
    }
  }
}

static ssize_t get_pre_recved(struct connectdata *conn, int num, char *buf,
                              size_t len)
{
  struct postponed_data * const psnd = &(conn->postponed[num]);
  size_t copysize;
  if(!psnd->buffer)
    return 0;

  DEBUGASSERT(psnd->allocated_size > 0);
  DEBUGASSERT(psnd->recv_size <= psnd->allocated_size);
  DEBUGASSERT(psnd->recv_processed <= psnd->recv_size);
  /* Check and process data that already received and storied in internal
     intermediate buffer */
  if(psnd->recv_size > psnd->recv_processed) {
    DEBUGASSERT(psnd->bindsock == conn->sock[num]);
    copysize = CURLMIN(len, psnd->recv_size - psnd->recv_processed);
    memcpy(buf, psnd->buffer + psnd->recv_processed, copysize);
    psnd->recv_processed += copysize;
  }
  else
    copysize = 0; /* buffer was allocated, but nothing was received */

  /* Free intermediate buffer if it has no unprocessed data */
  if(psnd->recv_processed == psnd->recv_size) {
    free(psnd->buffer);
    psnd->buffer = NULL;
    psnd->allocated_size = 0;
    psnd->recv_size = 0;
    psnd->recv_processed = 0;
#ifdef DEBUGBUILD
    psnd->bindsock = CURL_SOCKET_BAD;
#endif /* DEBUGBUILD */
  }
  return (ssize_t)copysize;
}
#else
bool Curl_recv_has_postponed_data(struct connectdata *conn, int sockindex) {
    (void)conn;
    (void)sockindex;
    return false;
}
#define pre_receive_plain(c,n) do {} while(0)
#define get_pre_recved(c,n,b,l) 0
#endif
void Curl_infof(struct Curl_easy *data, const char *fmt, ...) {
    if(data && data->set.verbose) {
        va_list ap;
        size_t len;
        char print_buffer[2048 + 1];
        va_start(ap, fmt);
        len = mvsnprintf(print_buffer, sizeof(print_buffer), fmt, ap);
        if(len >= sizeof(print_buffer)) {
            len = strlen(fmt);
            if(fmt[--len] == '\n') msnprintf(print_buffer + (sizeof(print_buffer) - 5), 5, "...\n");
            else msnprintf(print_buffer + (sizeof(print_buffer) - 4), 4, "...");
        }
        va_end(ap);
        len = strlen(print_buffer);
        Curl_debug(data, CURLINFO_TEXT, print_buffer, len);
    }
}
void Curl_failf(struct Curl_easy *data, const char *fmt, ...) {
    if(data->set.verbose || data->set.errorbuffer) {
        va_list ap;
        size_t len;
        char error[CURL_ERROR_SIZE + 2];
        va_start(ap, fmt);
        (void)mvsnprintf(error, CURL_ERROR_SIZE, fmt, ap);
        len = strlen(error);
        if(data->set.errorbuffer && !data->state.errorbuf) {
            strcpy(data->set.errorbuffer, error);
            data->state.errorbuf = TRUE;
        }
        if(data->set.verbose) {
            error[len] = '\n';
            error[++len] = '\0';
            Curl_debug(data, CURLINFO_TEXT, error, len);
        }
        va_end(ap);
    }
}
CURLcode Curl_sendf(curl_socket_t sockfd, struct connectdata *conn, const char *fmt, ...) {
    struct Curl_easy *data = conn->data;
    ssize_t bytes_written;
    size_t write_len;
    CURLcode result = CURLE_OK;
    char *s;
    char *sptr;
    va_list ap;
    va_start(ap, fmt);
    s = vaprintf(fmt, ap);
    va_end(ap);
    if(!s) return CURLE_OUT_OF_MEMORY;
    bytes_written = 0;
    write_len = strlen(s);
    sptr = s;
    for(;;) {
        result = Curl_write(conn, sockfd, sptr, write_len, &bytes_written);
        if(result) break;
        if(data->set.verbose) Curl_debug(data, CURLINFO_DATA_OUT, sptr, (size_t)bytes_written);
        if((size_t)bytes_written != write_len) {
            write_len -= bytes_written;
            sptr += bytes_written;
        } else break;
    }
    free(s);
    return result;
}
CURLcode Curl_write(struct connectdata *conn, curl_socket_t sockfd, const void *mem, size_t len, ssize_t *written) {
    ssize_t bytes_written;
    CURLcode result = CURLE_OK;
    int num = (sockfd == conn->sock[SECONDARYSOCKET]);
    bytes_written = conn->send[num](conn, num, mem, len, &result);
    *written = bytes_written;
    if(bytes_written >= 0) return CURLE_OK;
    switch(result) {
        case CURLE_AGAIN: *written = 0; return CURLE_OK;
        case CURLE_OK: return CURLE_SEND_ERROR;
        default: return result;
    }
}
ssize_t Curl_send_plain(struct connectdata *conn, int num, const void *mem, size_t len, CURLcode *code) {
    curl_socket_t sockfd = conn->sock[num];
    ssize_t bytes_written;
    pre_receive_plain(conn, num);
#if defined(MSG_FASTOPEN) && !defined(TCP_FASTOPEN_CONNECT)
    if(conn->bits.tcp_fastopen) {
      bytes_written = sendto(sockfd, mem, len, MSG_FASTOPEN, conn->ip_addr->ai_addr, conn->ip_addr->ai_addrlen);
      conn->bits.tcp_fastopen = FALSE;
    } else
#endif
    bytes_written = swrite(sockfd, mem, len);
    *code = CURLE_OK;
    if(-1 == bytes_written) {
        int err = SOCKERRNO;
        if(
        #ifdef WSAEWOULDBLOCK
           (WSAEWOULDBLOCK == err)
        #else
           (EWOULDBLOCK == err) || (EAGAIN == err) || (EINTR == err) || (EINPROGRESS == err)
        #endif
        ) {
            bytes_written = 0;
            *code = CURLE_AGAIN;
        } else {
            char buffer[STRERROR_LEN];
            failf(conn->data, "Send failure: %s", Curl_strerror(err, buffer, sizeof(buffer)));
            conn->data->state.os_errno = err;
            *code = CURLE_SEND_ERROR;
        }
    }
    return bytes_written;
}
CURLcode Curl_write_plain(struct connectdata *conn, curl_socket_t sockfd, const void *mem, size_t len, ssize_t *written) {
    ssize_t bytes_written;
    CURLcode result;
    int num = (sockfd == conn->sock[SECONDARYSOCKET]);
    bytes_written = Curl_send_plain(conn, num, mem, len, &result);
    *written = bytes_written;
    return result;
}
ssize_t Curl_recv_plain(struct connectdata *conn, int num, char *buf, size_t len, CURLcode *code) {
    curl_socket_t sockfd = conn->sock[num];
    ssize_t nread;
    nread = get_pre_recved(conn, num, buf, len);
    if(nread > 0) {
        *code = CURLE_OK;
        return nread;
    }
    nread = sread(sockfd, buf, len);
    *code = CURLE_OK;
    if(-1 == nread) {
        int err = SOCKERRNO;
        if(
        #ifdef WSAEWOULDBLOCK
           (WSAEWOULDBLOCK == err)
        #else
           (EWOULDBLOCK == err) || (EAGAIN == err) || (EINTR == err)
        #endif
        ) *code = CURLE_AGAIN;
        else {
            char buffer[STRERROR_LEN];
            failf(conn->data, "Recv failure: %s", Curl_strerror(err, buffer, sizeof(buffer)));
            conn->data->state.os_errno = err;
            *code = CURLE_RECV_ERROR;
        }
    }
    return nread;
}
static CURLcode pausewrite(struct Curl_easy *data, int type, const char *ptr, size_t len) {
    struct SingleRequest *k = &data->req;
    struct UrlState *s = &data->state;
    unsigned int i;
    bool newtype = TRUE;
    if(s->tempcount) {
        for(i = 0; i< s->tempcount; i++) {
            if(s->tempwrite[i].type == type) {
                newtype = FALSE;
                break;
            }
        }
        DEBUGASSERT(i < 3);
    } else i = 0;
    if(newtype) {
        Curl_dyn_init(&s->tempwrite[i].b, DYN_PAUSE_BUFFER);
        s->tempwrite[i].type = type;
        if(newtype) s->tempcount++;
    }
    if(Curl_dyn_addn(&s->tempwrite[i].b, (unsigned char *)ptr, len)) return CURLE_OUT_OF_MEMORY;
    k->keepon |= KEEP_RECV_PAUSE;
    return CURLE_OK;
}
static CURLcode chop_write(struct connectdata *conn, int type, char *optr, size_t olen) {
    struct Curl_easy *data = conn->data;
    curl_write_callback writeheader = NULL;
    curl_write_callback writebody = NULL;
    char *ptr = optr;
    size_t len = olen;
    if(!len) return CURLE_OK;
    if(data->req.keepon & KEEP_RECV_PAUSE) return pausewrite(data, type, ptr, len);
    if(type & CLIENTWRITE_BODY) writebody = data->set.fwrite_func;
    if((type & CLIENTWRITE_HEADER) && (data->set.fwrite_header || data->set.writeheader)) {
        writeheader = data->set.fwrite_header? data->set.fwrite_header: data->set.fwrite_func;
    }
    while(len) {
        size_t chunklen = len <= CURL_MAX_WRITE_SIZE? len: CURL_MAX_WRITE_SIZE;
        if(writebody) {
            size_t wrote;
            Curl_set_in_callback(data, true);
            wrote = writebody(ptr, 1, chunklen, data->set.out);
            Curl_set_in_callback(data, false);
            if(CURL_WRITEFUNC_PAUSE == wrote) {
                if(conn->handler->flags & PROTOPT_NONETWORK) {
                    failf(data, "Write callback asked for PAUSE when not supported!");
                    return CURLE_WRITE_ERROR;
                }
                return pausewrite(data, type, ptr, len);
            }
            if(wrote != chunklen) {
                failf(data, "Failure writing output to destination");
                return CURLE_WRITE_ERROR;
            }
        }
        ptr += chunklen;
        len -= chunklen;
    }
    if(writeheader) {
        size_t wrote;
        ptr = optr;
        len = olen;
        Curl_set_in_callback(data, true);
        wrote = writeheader(ptr, 1, len, data->set.writeheader);
        Curl_set_in_callback(data, false);
        if(CURL_WRITEFUNC_PAUSE == wrote) return pausewrite(data, CLIENTWRITE_HEADER, ptr, len);
        if(wrote != len) {
            failf(data, "Failed writing header");
            return CURLE_WRITE_ERROR;
        }
    }
    return CURLE_OK;
}
CURLcode Curl_client_write(struct connectdata *conn, int type, char *ptr, size_t len) {
    struct Curl_easy *data = conn->data;
    if(0 == len) len = strlen(ptr);
    DEBUGASSERT(type <= 3);
    if((type & CLIENTWRITE_BODY) &&
    (conn->handler->protocol & PROTO_FAMILY_FTP) &&
    conn->proto.ftpc.transfertype == 'A') {
        CURLcode result = Curl_convert_from_network(data, ptr, len);
        if(result) return result;
    #ifdef CURL_DO_LINEEND_CONV
        len = convert_lineends(data, ptr, len);
    #endif
    }
    return chop_write(conn, type, ptr, len);
}
CURLcode Curl_read_plain(curl_socket_t sockfd, char *buf, size_t bytesfromsocket, ssize_t *n) {
    ssize_t nread = sread(sockfd, buf, bytesfromsocket);
    if(-1 == nread) {
    const int err = SOCKERRNO;
    const bool return_error =
    #ifdef USE_WINSOCK
        WSAEWOULDBLOCK == err
    #else
        EWOULDBLOCK == err || EAGAIN == err || EINTR == err
    #endif
        ;
    *n = 0;
    if(return_error) return CURLE_AGAIN;
    return CURLE_RECV_ERROR;
    }
    *n = nread;
    return CURLE_OK;
}
CURLcode Curl_read(struct connectdata *conn, curl_socket_t sockfd, char *buf, size_t sizerequested, ssize_t *n) {
    CURLcode result = CURLE_RECV_ERROR;
    ssize_t nread = 0;
    size_t bytesfromsocket = 0;
    char *buffertofill = NULL;
    struct Curl_easy *data = conn->data;
    int num = (sockfd == conn->sock[SECONDARYSOCKET]);
    *n = 0;
    bytesfromsocket = CURLMIN(sizerequested, (size_t)data->set.buffer_size);
    buffertofill = buf;
    nread = conn->recv[num](conn, num, buffertofill, bytesfromsocket, &result);
    if(nread < 0) return result;
    *n += nread;
    return CURLE_OK;
}
int Curl_debug(struct Curl_easy *data, curl_infotype type, char *ptr, size_t size) {
    static const char s_infotype[CURLINFO_END][3] = { "* ", "< ", "> ", "{ ", "} ", "{ ", "} " };
    int rc = 0;
#ifdef CURL_DOES_CONVERSIONS
    char *buf = NULL;
    size_t conv_size = 0;
    switch(type) {
        case CURLINFO_HEADER_OUT:
            buf = Curl_memdup(ptr, size);
            if(!buf) return 1;
            conv_size = size;
            if(size > 4) {
                size_t i;
                for(i = 0; i < size-4; i++) {
                    if(memcmp(&buf[i], "\x0d\x0a\x0d\x0a", 4) == 0) {
                        conv_size = i + 4;
                        break;
                    }
                }
            }
            Curl_convert_from_network(data, buf, conv_size);
            ptr = buf;
            break;
    }
#endif
    if(data->set.fdebug) {
        Curl_set_in_callback(data, true);
        rc = (*data->set.fdebug)(data, type, ptr, size, data->set.debugdata);
        Curl_set_in_callback(data, false);
    } else {
        switch(type) {
            case CURLINFO_TEXT: case CURLINFO_HEADER_OUT: case CURLINFO_HEADER_IN:
                fwrite(s_infotype[type], 2, 1, data->set.err);
                fwrite(ptr, size, 1, data->set.err);
            #ifdef CURL_DOES_CONVERSIONS
                if(size != conv_size) fwrite("\n", 1, 1, data->set.err);
            #endif
                break;
        }
    }
#ifdef CURL_DOES_CONVERSIONS
    free(buf);
#endif
    return rc;
}
