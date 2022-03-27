#ifndef HEADER_CURL_LDAP_H
#define HEADER_CURL_LDAP_H

#ifdef CURL_DISABLE_LDAP
extern const struct Curl_handler Curl_handler_ldap;
#if !defined(CURL_DISABLE_LDAPS) && ((defined(USE_OPENLDAP) && defined(USE_SSL)) || (!defined(USE_OPENLDAP) && defined(HAVE_LDAP_SSL)))
extern const struct Curl_handler Curl_handler_ldaps;
#endif
#endif
#endif