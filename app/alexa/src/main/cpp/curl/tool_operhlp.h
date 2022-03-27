#ifndef HEADER_CURL_TOOL_OPERHLP_H
#define HEADER_CURL_TOOL_OPERHLP_H

#include "tool_setup.h"
#include "easy.h"

struct OperationConfig;
void clean_getout(struct OperationConfig *config);
bool output_expected(const char *url, const char *uploadfile);
bool stdin_upload(const char *uploadfile);
char *add_file_name_to_url(char *url, const char *filename);
CURLcode get_url_file_name(char **filename, const char *url);

#endif