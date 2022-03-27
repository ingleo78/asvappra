#ifndef HEADER_CURL_TOOL_PARAMHLP_H
#define HEADER_CURL_TOOL_PARAMHLP_H

#include "tool_setup.h"

struct getout *new_getout(struct OperationConfig *config);
ParameterError file2string(char **bufp, FILE *file);
ParameterError file2memory(char **bufp, size_t *size, FILE *file);
void cleanarg(char *str);
ParameterError str2num(long *val, const char *str);
ParameterError str2unum(long *val, const char *str);
ParameterError str2unummax(long *val, const char *str, long max);
ParameterError str2udouble(double *val, const char *str, long max);
long proto2num(struct OperationConfig *config, long *val, const char *str);
int check_protocol(const char *str);
ParameterError str2offset(curl_off_t *val, const char *str);
CURLcode get_args(struct OperationConfig *config, const size_t i);
ParameterError add2list(struct curl_slist **list, const char *ptr);
int ftpfilemethod(struct OperationConfig *config, const char *str);
int ftpcccmethod(struct OperationConfig *config, const char *str);
long delegation(struct OperationConfig *config, const char *str);
ParameterError str2tls_max(long *val, const char *str);

#endif