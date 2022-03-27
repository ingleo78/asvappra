#ifndef HEADER_CURL_FTPLISTPARSER_H
#define HEADER_CURL_FTPLISTPARSER_H

#include "curl_setup.h"

#ifndef CURL_DISABLE_FTP
size_t Curl_ftp_parselist(char *buffer, size_t size, size_t nmemb, void *connptr);
struct ftp_parselist_data;
CURLcode Curl_ftp_parselist_geterror(struct ftp_parselist_data *pl_data);
struct ftp_parselist_data *Curl_ftp_parselist_data_alloc(void);
void Curl_ftp_parselist_data_free(struct ftp_parselist_data **pl_data);
#endif
#endif