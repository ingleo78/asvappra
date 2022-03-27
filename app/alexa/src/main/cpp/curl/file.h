#ifndef HEADER_CURL_FILE_H
#define HEADER_CURL_FILE_H

struct FILEPROTO {
    char *path;
    char *freepath;
    int fd;
};
#ifndef CURL_DISABLE_FILE
    extern const struct Curl_handler Curl_handler_file;
#endif

#endif