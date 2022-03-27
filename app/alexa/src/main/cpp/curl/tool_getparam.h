#ifndef HEADER_CURL_TOOL_GETPARAM_H
#define HEADER_CURL_TOOL_GETPARAM_H

#include "tool_setup.h"
#include "curl_setup_once.h"

typedef enum {
    PARAM_OK = 0,
    PARAM_OPTION_AMBIGUOUS,
    PARAM_OPTION_UNKNOWN,
    PARAM_REQUIRES_PARAMETER,
    PARAM_BAD_USE,
    PARAM_HELP_REQUESTED,
    PARAM_MANUAL_REQUESTED,
    PARAM_VERSION_INFO_REQUESTED,
    PARAM_ENGINES_REQUESTED,
    PARAM_GOT_EXTRA_PARAMETER,
    PARAM_BAD_NUMERIC,
    PARAM_NEGATIVE_NUMERIC,
    PARAM_LIBCURL_DOESNT_SUPPORT,
    PARAM_LIBCURL_UNSUPPORTED_PROTOCOL,
    PARAM_NO_MEM,
    PARAM_NEXT_OPERATION,
    PARAM_NO_PREFIX,
    PARAM_NUMBER_TOO_LARGE,
    PARAM_NO_NOT_BOOLEAN,
    PARAM_LAST
} ParameterError;
struct GlobalConfig;
struct OperationConfig;
ParameterError getparameter(const char *flag, char *nextarg, bool *usedarg, struct GlobalConfig *global, struct OperationConfig *operation);
#ifdef UNITTESTS
void parse_cert_parameter(const char *cert_parameter, char **certname, char **passphrase);
#endif
ParameterError parse_args(struct GlobalConfig *config, int argc, argv_item_t argv[]);

#endif