#include <stdbool.h>
#include "curl_setup.h"

#ifndef CURL_DISABLE_DICT
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
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#elif defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include "urldata.h"
#include "curl.h"
#include "transfer.h"
#include "sendf.h"
#include "escape.h"
#include "progress.h"
#include "dict.h"
#include "strcase.h"
#include "curl_memory.h"
#include "memdebug.h"
#define TRUE 1
#define FALSE 0
#define ZERO_NULL 0
static CURLcode dict_do(struct connectdata *conn, bool *done);
const struct Curl_handler Curl_handler_dict = {
    "DICT",
    ZERO_NULL,
    dict_do,
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
    PORT_DICT,
    CURLPROTO_DICT,
    PROTOPT_NONE | PROTOPT_NOURLQUERY
};
static char *unescape_word(struct Curl_easy *data, const char *inputbuff) {
    char *newp = NULL;
    char *dictp;
    size_t len;
    CURLcode result = Curl_urldecode(data, inputbuff, 0, &newp, &len, REJECT_NADA);
    if (!newp || result) return NULL;
    dictp = malloc(len*2 + 1);
    if (dictp) {
        char *ptr;
        char ch;
        int olen = 0;
        for (ptr = newp;
            (ch = *ptr) != 0;
            ptr++) {
          if ((ch <= 32) || (ch == 127) || (ch == '\'') || (ch == '\"') || (ch == '\\')) dictp[olen++] = '\\';
          dictp[olen++] = ch;
        }
        dictp[olen] = 0;
    }
    free(newp);
    return dictp;
}
static CURLcode dict_do(struct connectdata *conn, bool *done) {
    char *word;
    char *eword;
    char *ppath;
    char *database = NULL;
    char *strategy = NULL;
    char *nthdef = NULL;
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
    char *path = data->state.up.path;
    *done = TRUE;
    if (conn->bits.user_passwd);
    if (strncasecompare(path, DICT_MATCH, sizeof(DICT_MATCH)-1) || strncasecompare(path, DICT_MATCH2, sizeof(DICT_MATCH2)-1) ||
        strncasecompare(path, DICT_MATCH3, sizeof(DICT_MATCH3)-1)) {
        word = strchr(path, ':');
        if (word) {
            word++;
            database = strchr(word, ':');
            if(database) {
                *database++ = (char)0;
                strategy = strchr(database, ':');
                if(strategy) {
                    *strategy++ = (char)0;
                    nthdef = strchr(strategy, ':');
                    if(nthdef) *nthdef = (char)0;
                }
            }
        }
        if ((word == NULL) || (*word == (char)0)) {
            infof(data, "lookup word is missing\n");
            word = (char *)"default";
        }
        if ((database == NULL) || (*database == (char)0)) database = (char *)"!";
        if ((strategy == NULL) || (*strategy == (char)0)) strategy = (char *)".";
        eword = unescape_word(data, word);
        if(!eword) return CURLE_OUT_OF_MEMORY;
        result = Curl_sendf(sockfd, conn, "CLIENT " LIBCURL_NAME " " LIBCURL_VERSION "\r\n" "MATCH " "%s " "%s " "%s\r\n" "QUIT\r\n",
                            database, strategy, eword);
        free(eword);
        if (result) {
            failf(data, "Failed sending DICT request");
            return result;
        }
        Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
    } else if(strncasecompare(path, DICT_DEFINE, sizeof(DICT_DEFINE)-1) || strncasecompare(path, DICT_DEFINE2, sizeof(DICT_DEFINE2)-1) ||
              strncasecompare(path, DICT_DEFINE3, sizeof(DICT_DEFINE3)-1)) {
        word = strchr(path, ':');
        if(word) {
            word++;
            database = strchr(word, ':');
            if(database) {
                *database++ = (char)0;
                nthdef = strchr(database, ':');
                if(nthdef) *nthdef = (char)0;
            }
        }
        if((word == NULL) || (*word == (char)0)) {
            infof(data, "lookup word is missing\n");
            word = (char *)"default";
        }
        if((database == NULL) || (*database == (char)0)) database = (char *)"!";
        eword = unescape_word(data, word);
        if(!eword) return CURLE_OUT_OF_MEMORY;
        result = Curl_sendf(sockfd, conn, "CLIENT " LIBCURL_NAME " " LIBCURL_VERSION "\r\n" "DEFINE " "%s " "%s\r\n" "QUIT\r\n",
                            database, eword);
        free(eword);
        if (result) {
          failf(data, "Failed sending DICT request");
          return result;
        }
        Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
    } else {
        ppath = strchr(path, '/');
        if (ppath) {
            int i;
            ppath++;
            for(i = 0; ppath[i]; i++) {
                if(ppath[i] == ':') ppath[i] = ' ';
            }
            result = Curl_sendf(sockfd, conn,"CLIENT " LIBCURL_NAME " " LIBCURL_VERSION "\r\n" "%s\r\n" "QUIT\r\n", ppath);
            if (result) {
                failf(data, "Failed sending DICT request");
                return result;
            }
            Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
        }
    }
    return CURLE_OK;
}
#endif