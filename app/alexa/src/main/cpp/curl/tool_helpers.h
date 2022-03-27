#ifndef HEADER_CURL_TOOL_HELPERS_H
#define HEADER_CURL_TOOL_HELPERS_H

#include "tool_setup.h"

const char *param2text(int res);
int SetHTTPrequest(struct OperationConfig *config, HttpReq req, HttpReq *store);
void customrequest_helper(struct OperationConfig *config, HttpReq req, char *method);

#endif