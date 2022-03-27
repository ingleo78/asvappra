#include "curl_setup.h"


#ifdef CURL_DOES_CONVERSIONS
#if __ANDROID_API__ > 28
#include "curl.h"
#include "non-ascii.h"
#include "formdata.h"
#include "sendf.h"
#include "urldata.h"
#include "multiif.h"
#include "curl_memory.h"
#include "memdebug.h"
#include "config-tpf.h"

#ifdef HAVE_ICONV
#include <iconv.h>
#ifndef CURL_ICONV_CODESET_OF_NETWORK
#define CURL_ICONV_CODESET_OF_NETWORK "ISO8859-1"
#endif
#ifndef CURL_ICONV_CODESET_FOR_UTF8
#define CURL_ICONV_CODESET_FOR_UTF8   "UTF-8"
#endif
#define ICONV_ERROR  (size_t)-1
#endif

CURLcode Curl_convert_clone(struct Curl_easy *data, const char *indata, size_t insize, char **outbuf) {
    char *convbuf;
    CURLcode result;
    convbuf = malloc(insize);
    if(!convbuf) return CURLE_OUT_OF_MEMORY;
    memcpy(convbuf, indata, insize);
    result = Curl_convert_to_network(data, convbuf, insize);
    if(result) {
        free(convbuf);
        return result;
    }
    *outbuf = convbuf;
    return CURLE_OK;
}
CURLcode Curl_convert_to_network(struct Curl_easy *data, char *buffer, size_t length) {
    if(data && data->set.convtonetwork) {
        CURLcode result;
        Curl_set_in_callback(data, true);
        result = data->set.convtonetwork(buffer, length);
        Curl_set_in_callback(data, false);
        if(result) failf(data, "CURLOPT_CONV_TO_NETWORK_FUNCTION callback returned %d: %s", (int)result, curl_easy_strerror(result));
        return result;
    } else {
    #ifdef HAVE_ICONV
        iconv_t tmpcd = (iconv_t) -1;
        iconv_t *cd = &tmpcd;
        char *input_ptr, *output_ptr;
        size_t in_bytes, out_bytes, rc;
        if(data) cd = &data->outbound_cd;
        if(*cd == (iconv_t)-1) {
            *cd = iconv_open(CURL_ICONV_CODESET_OF_NETWORK, CURL_ICONV_CODESET_OF_HOST);
            if(*cd == (iconv_t)-1) {
                failf(data, "The iconv_open(\"%s\", \"%s\") call failed with errno %i: %s", CURL_ICONV_CODESET_OF_NETWORK, CURL_ICONV_CODESET_OF_HOST,
                      errno, strerror(errno));
                return CURLE_CONV_FAILED;
            }
        }
        input_ptr = output_ptr = buffer;
        in_bytes = out_bytes = length;
        rc = iconv(*cd, &input_ptr, &in_bytes, &output_ptr, &out_bytes);
        if(!data) iconv_close(tmpcd);
        if((rc == ICONV_ERROR) || (in_bytes != 0)) {
            failf(data, "The Curl_convert_to_network iconv call failed with errno %i: %s", errno, strerror(errno));
            return CURLE_CONV_FAILED;
        }
    #else
        failf(data, "CURLOPT_CONV_TO_NETWORK_FUNCTION callback required");
        return CURLE_CONV_REQD;
    #endif
    }
    return CURLE_OK;
}
CURLcode Curl_convert_from_network(struct Curl_easy *data, char *buffer, size_t length) {
    if(data && data->set.convfromnetwork) {
        CURLcode result;
        Curl_set_in_callback(data, true);
        result = data->set.convfromnetwork(buffer, length);
        Curl_set_in_callback(data, false);
        if(result) failf(data, "CURLOPT_CONV_FROM_NETWORK_FUNCTION callback returned %d: %s", (int)result, curl_easy_strerror(result));
        return result;
    } else {
    #ifdef HAVE_ICONV
        iconv_t tmpcd = (iconv_t) -1;
        iconv_t *cd = &tmpcd;
        char *input_ptr, *output_ptr;
        size_t in_bytes, out_bytes, rc;
        if(data) cd = &data->inbound_cd;
        if(*cd == (iconv_t)-1) {
            *cd = iconv_open(CURL_ICONV_CODESET_OF_HOST, CURL_ICONV_CODESET_OF_NETWORK);
            if(*cd == (iconv_t)-1) {
                failf(data, "The iconv_open(\"%s\", \"%s\") call failed with errno %i: %s", CURL_ICONV_CODESET_OF_HOST, CURL_ICONV_CODESET_OF_NETWORK,
                      errno, strerror(errno));
                return CURLE_CONV_FAILED;
            }
        }
        input_ptr = output_ptr = buffer;
        in_bytes = out_bytes = length;
        rc = iconv(*cd, &input_ptr, &in_bytes, &output_ptr, &out_bytes);
        if(!data) iconv_close(tmpcd);
        if((rc == ICONV_ERROR) || (in_bytes != 0)) {
            failf(data, "Curl_convert_from_network iconv call failed with errno %i: %s", errno, strerror(errno));
            return CURLE_CONV_FAILED;
        }
    #else
        failf(data, "CURLOPT_CONV_FROM_NETWORK_FUNCTION callback required");
        return CURLE_CONV_REQD;
    #endif
    }
    return CURLE_OK;
}
CURLcode Curl_convert_from_utf8(struct Curl_easy *data, char *buffer, size_t length) {
    if(data && data->set.convfromutf8) {
        CURLcode result;
        Curl_set_in_callback(data, true);
        result = data->set.convfromutf8(buffer, length);
        Curl_set_in_callback(data, false);
        if(result) failf(data, "CURLOPT_CONV_FROM_UTF8_FUNCTION callback returned %d: %s", (int)result, curl_easy_strerror(result));
        return result;
    } else {
    #ifdef HAVE_ICONV
        iconv_t tmpcd = (iconv_t) -1;
        iconv_t *cd = &tmpcd;
        char *input_ptr;
        char *output_ptr;
        size_t in_bytes, out_bytes, rc;
        if(data) cd = &data->utf8_cd;
        if(*cd == (iconv_t)-1) {
            *cd = iconv_open(CURL_ICONV_CODESET_OF_HOST, CURL_ICONV_CODESET_FOR_UTF8);
            if(*cd == (iconv_t)-1) {
                failf(data, "The iconv_open(\"%s\", \"%s\") call failed with errno %i: %s", CURL_ICONV_CODESET_OF_HOST, CURL_ICONV_CODESET_FOR_UTF8,
                      errno, strerror(errno));
                return CURLE_CONV_FAILED;
            }
        }
        input_ptr = output_ptr = buffer;
        in_bytes = out_bytes = length;
        rc = iconv(*cd, &input_ptr, &in_bytes, &output_ptr, &out_bytes);
        if(!data) iconv_close(tmpcd);
        if((rc == ICONV_ERROR) || (in_bytes != 0)) {
            failf(data, "The Curl_convert_from_utf8 iconv call failed with errno %i: %s", errno, strerror(errno));
            return CURLE_CONV_FAILED;
        }
        if(output_ptr < input_ptr) *output_ptr = 0x00;
    #else
        failf(data, "CURLOPT_CONV_FROM_UTF8_FUNCTION callback required");
        return CURLE_CONV_REQD;
    #endif
    }
    return CURLE_OK;
}
void Curl_convert_init(struct Curl_easy *data) {
#if defined(CURL_DOES_CONVERSIONS) && defined(HAVE_ICONV)
    data->outbound_cd = (iconv_t)-1;
    data->inbound_cd  = (iconv_t)-1;
    data->utf8_cd     = (iconv_t)-1;
#else
    (void)data;
#endif
}
void Curl_convert_setup(struct Curl_easy *data) {
    data->inbound_cd = iconv_open(CURL_ICONV_CODESET_OF_HOST, CURL_ICONV_CODESET_OF_NETWORK);
    data->outbound_cd = iconv_open(CURL_ICONV_CODESET_OF_NETWORK, CURL_ICONV_CODESET_OF_HOST);
    data->utf8_cd = iconv_open(CURL_ICONV_CODESET_OF_HOST, CURL_ICONV_CODESET_FOR_UTF8);
}
void Curl_convert_close(struct Curl_easy *data) {
#ifdef HAVE_ICONV
    if(data->inbound_cd != (iconv_t)-1) iconv_close(data->inbound_cd);
    if(data->outbound_cd != (iconv_t)-1) iconv_close(data->outbound_cd);
    if(data->utf8_cd != (iconv_t)-1) iconv_close(data->utf8_cd);
#else
    (void)data;
#endif
}
#endif
#endif