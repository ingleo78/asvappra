#ifndef _LBER_TYPES_H
#define _LBER_TYPES_H

#include "ldap_cdefs.h"

LDAP_BEGIN_DECL
#define LBER_INT_T int
#define LBER_TAG_T int
#define LBER_SOCKET_T int
#define LBER_LEN_T int
typedef LBER_INT_T ber_int_t;
typedef signed LBER_INT_T ber_sint_t;
typedef unsigned LBER_INT_T ber_uint_t;
typedef unsigned LBER_TAG_T ber_tag_t;
typedef LBER_SOCKET_T ber_socket_t;
typedef unsigned LBER_LEN_T ber_len_t;
typedef signed LBER_LEN_T ber_slen_t;
LDAP_END_DECL
#endif