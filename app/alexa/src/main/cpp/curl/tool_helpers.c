#include "tool_setup.h"
#include "strcase.h"
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "tool_cfgable.h"
#include "tool_msgs.h"
#include "tool_getparam.h"
#include "tool_helpers.h"
#include "memdebug.h"

const char *param2text(int res) {
    ParameterError error = (ParameterError)res;
    switch(error) {
        case PARAM_GOT_EXTRA_PARAMETER: return "had unsupported trailing garbage";
        case PARAM_OPTION_UNKNOWN: return "is unknown";
        case PARAM_OPTION_AMBIGUOUS: return "is ambiguous";
        case PARAM_REQUIRES_PARAMETER: return "requires parameter";
        case PARAM_BAD_USE: return "is badly used here";
        case PARAM_BAD_NUMERIC: return "expected a proper numerical parameter";
        case PARAM_NEGATIVE_NUMERIC: return "expected a positive numerical parameter";
        case PARAM_LIBCURL_DOESNT_SUPPORT: return "the installed libcurl version doesn't support this";
        case PARAM_LIBCURL_UNSUPPORTED_PROTOCOL: return "a specified protocol is unsupported by libcurl";
        case PARAM_NO_MEM: return "out of memory";
        case PARAM_NO_PREFIX: return "the given option can't be reversed with a --no- prefix";
        case PARAM_NUMBER_TOO_LARGE: return "too large number";
        case PARAM_NO_NOT_BOOLEAN: return "used '--no-' for option that isn't a boolean";
        default: return "unknown error";
    }
}
int SetHTTPrequest(struct OperationConfig *config, HttpReq req, HttpReq *store) {
    const char *reqname[]= {
        "",
        "GET (-G, --get)",
        "HEAD (-I, --head)",
        "multipart formpost (-F, --form)",
        "POST (-d, --data)"
    };
    if((*store == HTTPREQ_UNSPEC) || (*store == req)) {
        *store = req;
        return 0;
    }
    warnf(config->global, "You can only select one HTTP request method! You asked for both %s and %s.\n", reqname[req], reqname[*store]);
    return 1;
}
void customrequest_helper(struct OperationConfig *config, HttpReq req, char *method) {
    const char *dflt[]= {
        "GET",
        "GET",
        "HEAD",
        "POST",
        "POST"
    };
    if(!method);
    else if(curl_strequal(method, dflt[req])) notef(config->global, "Unnecessary use of -X or --request, %s is already inferred.\n", dflt[req]);
    else if(curl_strequal(method, "head")) {
        warnf(config->global,"Setting custom HTTP method to HEAD with -X/--request may not work "
              "the way you want. Consider using -I/--head instead.\n");
    }
}