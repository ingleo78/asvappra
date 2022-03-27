#ifndef _LDAP_CDEFS_H
#define _LDAP_CDEFS_H

#if defined(__cplusplus) || defined(c_plusplus)
#define LDAP_BEGIN_DECL	extern "C" {
#define LDAP_END_DECL	}
#else
#define LDAP_BEGIN_DECL
#define LDAP_END_DECL
#endif
#if !defined(LDAP_NO_PROTOTYPES) && (defined(LDAP_NEEDS_PROTOTYPES) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus))
#define LDAP_P(protos)	protos
#define LDAP_CONCAT1(x,y)	x ## y
#define LDAP_CONCAT(x,y)	LDAP_CONCAT1(x,y)
#define LDAP_STRING(x)	#x
#define LDAP_XSTRING(x)	LDAP_STRING(x)
#ifndef LDAP_CONST
#define LDAP_CONST	const
#endif
#else
#define LDAP_P(protos)	()
#define LDAP_CONCAT(x,y)	x/**/y
#define LDAP_STRING(x)	"x"
#ifndef LDAP_CONST
#define LDAP_CONST
#endif
#endif
#if (__GNUC__) * 1000 + (__GNUC_MINOR__) >= 2006
#define LDAP_GCCATTR(attrs)	__attribute__(attrs)
#else
#define LDAP_GCCATTR(attrs)
#endif
#if defined(_WIN32) && ((defined(LDAP_LIBS_DYNAMIC) && !defined(LBER_LIBRARY)) || (!defined(LDAP_LIBS_DYNAMIC) && defined(SLAPD_IMPORT)))
#define LBER_F(type)		extern __declspec(dllimport) type
#define LBER_V(type)		extern __declspec(dllimport) type
#else
#define LBER_F(type)		extern type
#define LBER_V(type)		extern type
#endif
#if defined(_WIN32) && ((defined(LDAP_LIBS_DYNAMIC) && !defined(LDAP_LIBRARY)) || (!defined(LDAP_LIBS_DYNAMIC) && defined(SLAPD_IMPORT)))
#define LDAP_F(type)		extern __declspec(dllimport) type
#define LDAP_V(type)		extern __declspec(dllimport) type
#else
#define LDAP_F(type)		extern type
#define LDAP_V(type)		extern type
#endif
#if defined(_WIN32) && defined(SLAPD_IMPORT)
#define LDAP_AVL_F(type)		extern __declspec(dllimport) type
#define LDAP_AVL_V(type)		extern __declspec(dllimport) type
#else
#define LDAP_AVL_F(type)		extern type
#define LDAP_AVL_V(type)		extern type
#endif
#if defined(_WIN32) && defined(SLAPD_IMPORT)
#define LDAP_LDIF_F(type)	extern __declspec(dllimport) type
#define LDAP_LDIF_V(type)	extern __declspec(dllimport) type
#else
#define LDAP_LDIF_F(type)	extern type
#define LDAP_LDIF_V(type)	extern type
#endif
#if defined(_WIN32) && defined(SLAPD_IMPORT)
#define LDAP_LUNICODE_F(type)	extern __declspec(dllimport) type
#define LDAP_LUNICODE_V(type)	extern __declspec(dllimport) type
#else
#define LDAP_LUNICODE_F(type)	extern type
#define LDAP_LUNICODE_V(type)	extern type
#endif
#if defined(_WIN32) && defined(SLAPD_IMPORT)
#define LDAP_LUTIL_F(type)	extern __declspec(dllimport) type
#define LDAP_LUTIL_V(type)	extern __declspec(dllimport) type
#else
#define LDAP_LUTIL_F(type)	extern type
#define LDAP_LUTIL_V(type)	extern type
#endif
#if defined(_WIN32) && defined(SLAPD_IMPORT)
#define LDAP_REWRITE_F(type)	extern __declspec(dllimport) type
#define LDAP_REWRITE_V(type)	extern __declspec(dllimport) type
#else
#define LDAP_REWRITE_F(type)	extern type
#define LDAP_REWRITE_V(type)	extern type
#endif
#if defined(_WIN32) && defined(SLAPD_IMPORT)
#define LDAP_SLAPD_F(type)	extern __declspec(dllimport) type
#define LDAP_SLAPD_V(type)	extern __declspec(dllimport) type
#else
#define LDAP_SLAPD_F(type)	extern type
#define LDAP_SLAPD_V(type)	extern type
#endif
#if defined(_WIN32) && defined(SLAPD_IMPORT)
#define LDAP_SLAPI_F(type)	extern __declspec(dllimport) type
#define LDAP_SLAPI_V(type)	extern __declspec(dllimport) type
#else
#define LDAP_SLAPI_F(type)	extern type
#define LDAP_SLAPI_V(type)	extern type
#endif
#if defined(_WIN32) && defined(SLAPD_IMPORT)
#define SLAPI_F(type)		extern __declspec(dllimport) type
#define SLAPI_V(type)		extern __declspec(dllimport) type
#else
#define SLAPI_F(type)		extern type
#define SLAPI_V(type)		extern type
#endif
#if (defined(__MINGW32__) && !defined(CSTATIC)) || (defined(_MSC_VER) && defined(_DLL))
#define LDAP_LIBC_F(type)	extern __declspec(dllimport) type
#define LDAP_LIBC_V(type)	extern __declspec(dllimport) type
#else
#define LDAP_LIBC_F(type)	extern type
#define LDAP_LIBC_V(type)	extern type
#endif
#endif