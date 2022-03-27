#ifndef HEADER_CURL_SCHANNEL_H
#define HEADER_CURL_SCHANNEL_H

#include "../curl_setup.h"

#ifdef USE_SCHANNEL
#include <schnlsp.h>
#include <schannel.h>
#include "curl_sspi.h"
#include "urldata.h"

#if defined(HAVE_BORINGSSL) || defined(OPENSSL_IS_BORINGSSL)
# undef X509_NAME
# undef X509_CERT_PAIR
# undef X509_EXTENSIONS
#endif

extern const struct Curl_ssl Curl_ssl_schannel;
CURLcode Curl_verify_certificate(struct connectdata *conn, int sockindex);
#ifdef EXPOSE_SCHANNEL_INTERNAL_STRUCTS
#ifdef __MINGW32__
#include <_mingw.h>
#ifdef __MINGW64_VERSION_MAJOR
#define HAS_MANUAL_VERIFY_API
#endif
#else
#include <wincrypt.h>
#ifdef CERT_CHAIN_REVOCATION_CHECK_CHAIN
#define HAS_MANUAL_VERIFY_API
#endif
#endif
struct curl_schannel_cred {
    CredHandle cred_handle;
    TimeStamp time_stamp;
    int refcount;
};
struct curl_schannel_ctxt {
    CtxtHandle ctxt_handle;
    TimeStamp time_stamp;
};
struct ssl_backend_data {
    struct curl_schannel_cred *cred;
    struct curl_schannel_ctxt *ctxt;
    SecPkgContext_StreamSizes stream_sizes;
    size_t encdata_length, decdata_length;
    size_t encdata_offset, decdata_offset;
    unsigned char *encdata_buffer, *decdata_buffer;
    bool encdata_is_incomplete;
    unsigned long req_flags, ret_flags;
    CURLcode recv_unrecoverable_err;
    bool recv_sspi_close_notify;
    bool recv_connection_closed;
    bool use_alpn;
#ifdef HAS_MANUAL_VERIFY_API
    bool use_manual_cred_validation;
#endif
};
#endif
#endif
#endif