#include "curl_setup.h"
#include "curl.h"
#include "curl_range.h"
#include "sendf.h"
#include "strtoofft.h"

#if !defined(CURL_DISABLE_FTP) || !defined(CURL_DISABLE_FILE)
CURLcode Curl_range(struct connectdata *conn) {
    curl_off_t from, to;
    char *ptr;
    char *ptr2;
    struct Curl_easy *data = conn->data;
    if(data->state.use_range && data->state.range) {
        CURLofft from_t;
        CURLofft to_t;
        from_t = curlx_strtoofft(data->state.range, &ptr, 0, &from);
        if(from_t == CURL_OFFT_FLOW) return CURLE_RANGE_ERROR;
        while(*ptr && (ISSPACE(*ptr) || (*ptr == '-'))) ptr++;
        to_t = curlx_strtoofft(ptr, &ptr2, 0, &to);
        if(to_t == CURL_OFFT_FLOW) return CURLE_RANGE_ERROR;
        if((to_t == CURL_OFFT_INVAL) && !from_t) {
            data->state.resume_from = from;
            DEBUGF(infof(data, "RANGE %" CURL_FORMAT_CURL_OFF_T " to end of file\n", from));
        } else if((from_t == CURL_OFFT_INVAL) && !to_t) {
            data->req.maxdownload = to;
            data->state.resume_from = -to;
            DEBUGF(infof(data, "RANGE the last %" CURL_FORMAT_CURL_OFF_T " bytes\n", to));
        } else {
            curl_off_t totalsize;
            if(from > to) return CURLE_RANGE_ERROR;
            totalsize = to - from;
            if(totalsize == CURL_OFF_T_MAX) return CURLE_RANGE_ERROR;
            data->req.maxdownload = totalsize + 1;
            data->state.resume_from = from;
            DEBUGF(infof(data, "RANGE from %" CURL_FORMAT_CURL_OFF_T " getting %" CURL_FORMAT_CURL_OFF_T " bytes\n", from, data->req.maxdownload));
        }
        DEBUGF(infof(data, "range-download from %" CURL_FORMAT_CURL_OFF_T " to %" CURL_FORMAT_CURL_OFF_T ", totally %" CURL_FORMAT_CURL_OFF_T " bytes\n",
               from, to, data->req.maxdownload));
    } else data->req.maxdownload = -1;
    return CURLE_OK;
}
#endif