#ifndef HEADER_CURL_GSSAPI_H
#define HEADER_CURL_GSSAPI_H

#include "curl_setup.h"
#include "urldata.h"

#ifdef HAVE_GSSAPI
extern gss_OID_desc Curl_spnego_mech_oid;
extern gss_OID_desc Curl_krb5_mech_oid;
OM_uint32 Curl_gss_init_sec_context(struct Curl_easy *data, OM_uint32 *minor_status, gss_ctx_id_t *context, gss_name_t target_name,
                                    gss_OID mech_type, gss_channel_bindings_t input_chan_bindings, gss_buffer_t input_token,
                                    gss_buffer_t output_token, const bool mutual_auth, OM_uint32 *ret_flags);
void Curl_gss_log_error(struct Curl_easy *data, const char *prefix,
                            OM_uint32 major, OM_uint32 minor);
#ifdef HAVE_OLD_GSSMIT
#define GSS_C_NT_HOSTBASED_SERVICE gss_nt_service_name
#define NCOMPAT 1
#endif
#define GSSAUTH_P_NONE      1
#define GSSAUTH_P_INTEGRITY 2
#define GSSAUTH_P_PRIVACY   4
#endif
#endif