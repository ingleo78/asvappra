#ifndef CHECK_H
#define CHECK_H

#include <stddef.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include "check_stdint.h"
#include "config.h"

typedef unsigned int size_t;
#ifdef __cplusplus
#define CK_CPPSTART extern "C" {
#define CK_CPPEND }
CK_CPPSTART
#endif
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#define GCC_VERSION_AT_LEAST(major, minor, patch) \
((__GNUC__ > (major)) || \
 (__GNUC__ == (major) && __GNUC_MINOR__ > (minor)) || \
 (__GNUC__ == (major) && __GNUC_MINOR__ == (minor) && __GNUC_PATCHLEVEL__ >= (patch)) )
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#define GCC_VERSION_AT_LEAST(major, minor, patch) \
((__GNUC__ > (major)) || \
 (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
#else
#define GCC_VERSION_AT_LEAST(major, minor, patch) 0
#endif
#if GCC_VERSION_AT_LEAST(2,95,3)
#define CK_ATTRIBUTE_UNUSED __attribute__ ((unused))
#define CK_ATTRIBUTE_FORMAT(a, b, c) __attribute__ ((format (a, b, c)))
#else
#define CK_ATTRIBUTE_UNUSED
#define CK_ATTRIBUTE_FORMAT(a, b, c)
#endif
#if GCC_VERSION_AT_LEAST(2,5,0)
#define CK_ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
#define CK_ATTRIBUTE_NORETURN
#endif
#if GCC_VERSION_AT_LEAST(4,7,4) && (__STDC_VERSION__ >= 199901L)
/* Operator _Pragma introduced in C99 */
#define CK_DIAGNOSTIC_STRINGIFY(x) #x
#define CK_DIAGNOSTIC_HELPER1(y) CK_DIAGNOSTIC_STRINGIFY(GCC diagnostic ignored y)
#define CK_DIAGNOSTIC_HELPER2(z) CK_DIAGNOSTIC_HELPER1(#z)
#define CK_DIAGNOSTIC_PUSH_IGNORE(w) \
    _Pragma("GCC diagnostic push") \
    _Pragma(CK_DIAGNOSTIC_HELPER2(w))
#define CK_DIAGNOSTIC_POP(w) _Pragma ("GCC diagnostic pop")
#else
#define CK_DIAGNOSTIC_PUSH_IGNORE(w)
#define CK_DIAGNOSTIC_POP(w)
#endif
#undef GCC_VERSION_AT_LEAST
#if defined(_MSC_VER)
#define pid_t int
#endif
#define CK_EXPORT
#ifndef CK_DLL_EXP
#define CK_DLL_EXP
#endif
#define CHECK_MAJOR_VERSION (0)
#define CHECK_MINOR_VERSION (15)
#define CHECK_MICRO_VERSION (2)
CK_DLL_EXP extern int CK_EXPORT check_major_version;
CK_DLL_EXP extern int CK_EXPORT check_minor_version;
CK_DLL_EXP extern int CK_EXPORT check_micro_version;
#ifndef NULL
#define NULL ((void*)0)
#endif
typedef struct TCase TCase;
typedef void (*TFun) (int);
typedef void (*SFun) (void);
typedef struct Suite Suite;
typedef struct TTest {
    const char *name;
    TFun fn;
    const char *file;
    int line;
} TTest;
CK_DLL_EXP Suite *CK_EXPORT suite_create(const char *name);
CK_DLL_EXP int CK_EXPORT suite_tcase(Suite * s, const char *tcname);
CK_DLL_EXP void CK_EXPORT suite_add_tcase(Suite * s, TCase * tc);
CK_DLL_EXP TCase *CK_EXPORT tcase_create(const char *name);
CK_DLL_EXP void CK_EXPORT tcase_set_tags(TCase * tc, const char *tags);
#define tcase_add_test(tc,tf) tcase_add_test_raise_signal(tc,tf,0)
#define tcase_add_test_raise_signal(tc,ttest,signal)  _tcase_add_test((tc),(ttest),(signal), 0, 0, 1)
#define tcase_add_exit_test(tc, ttest, expected_exit_value)  _tcase_add_test((tc),(ttest),0,(expected_exit_value),0,1)
#define tcase_add_loop_test(tc,ttest,s,e)  _tcase_add_test((tc),(ttest),0,0,(s),(e))
#define tcase_add_loop_test_raise_signal(tc,ttest,signal,s,e)  _tcase_add_test((tc),(ttest),(signal),0,(s),(e))
#define tcase_add_loop_exit_test(tc,ttest,expected_exit_value,s,e)  _tcase_add_test((tc),(ttest),0,(expected_exit_value),(s),(e))
CK_DLL_EXP void CK_EXPORT _tcase_add_test(TCase * tc, const TTest * ttest, int _signal, int allowed_exit_value, int start, int end);
CK_DLL_EXP void CK_EXPORT tcase_add_unchecked_fixture(TCase * tc, SFun setup, SFun teardown);
CK_DLL_EXP void CK_EXPORT tcase_add_checked_fixture(TCase * tc, SFun setup, SFun teardown);
CK_DLL_EXP void CK_EXPORT tcase_set_timeout(TCase * tc, double timeout);
CK_DLL_EXP void CK_EXPORT tcase_fn_start(const char *fname, const char *file, int line);
CK_DLL_EXP const char* CK_EXPORT tcase_name(void);
#define START_TEST(__testname)\
  static void __testname ## _fn (int _i CK_ATTRIBUTE_UNUSED); \
  static const TTest __testname ## _ttest = {""# __testname, __testname ## _fn, __FILE__, __LINE__}; \
  static const TTest * __testname = & __testname ## _ttest; \
  static void __testname ## _fn (int _i CK_ATTRIBUTE_UNUSED)
#define END_TEST
#define fail_unless(expr, ...) \
  (expr) ? _mark_point(__FILE__, __LINE__) : _ck_assert_failed(__FILE__, __LINE__, "Assertion '"#expr"' failed" , ## __VA_ARGS__, NULL)
#define fail_if(expr, ...)\
  (expr) ? _ck_assert_failed(__FILE__, __LINE__, "Failure '"#expr"' occurred" , ## __VA_ARGS__, NULL) : _mark_point(__FILE__, __LINE__)
#define fail(...) _ck_assert_failed(__FILE__, __LINE__, "Failed" , ## __VA_ARGS__, NULL)
#if HAVE_FORK
CK_DLL_EXP void CK_EXPORT _ck_assert_failed(const char *file, int line, const char *expr, const char *msg, ...) CK_ATTRIBUTE_NORETURN CK_ATTRIBUTE_FORMAT(printf, 4, 5);
#else
CK_DLL_EXP void CK_EXPORT _ck_assert_failed(const char *file, int line, const char *expr, const char *msg, ...) CK_ATTRIBUTE_FORMAT(printf, 4, 5);
#endif
#define ck_assert(expr) ck_assert_msg(expr, NULL)
#define ck_assert_msg(expr, ...) \
  (expr) ? _mark_point(__FILE__, __LINE__) : _ck_assert_failed(__FILE__, __LINE__, "Assertion '"#expr"' failed" , ## __VA_ARGS__)
#define ck_abort() ck_abort_msg(NULL)
#define ck_abort_msg(...) _ck_assert_failed(__FILE__, __LINE__, "Failed" , ## __VA_ARGS__)
#define _ck_assert_int(X, OP, Y) do { \
  intmax_t _ck_x = (X); \
  intmax_t _ck_y = (Y); \
  ck_assert_msg(_ck_x OP _ck_y, "Assertion '%s' failed: %s == %jd, %s == %jd", #X" "#OP" "#Y, #X, _ck_x, #Y, _ck_y); \
} while(0);
#define ck_assert_int_eq(X, Y) _ck_assert_int(X, ==, Y)
#define ck_assert_int_ne(X, Y) _ck_assert_int(X, !=, Y)
#define ck_assert_int_lt(X, Y) _ck_assert_int(X, <, Y)
#define ck_assert_int_le(X, Y) _ck_assert_int(X, <=, Y)
#define ck_assert_int_gt(X, Y) _ck_assert_int(X, >, Y)
#define ck_assert_int_ge(X, Y) _ck_assert_int(X, >=, Y)
#define _ck_assert_uint(X, OP, Y) do { \
  uintmax_t _ck_x = (X); \
  uintmax_t _ck_y = (Y); \
  ck_assert_msg(_ck_x OP _ck_y, "Assertion '%s' failed: %s == %ju, %s == %ju", #X" "#OP" "#Y, #X, _ck_x, #Y, _ck_y); \
} while(0);
#define ck_assert_uint_eq(X, Y) _ck_assert_uint(X, ==, Y)
#define ck_assert_uint_ne(X, Y) _ck_assert_uint(X, !=, Y)
#define ck_assert_uint_lt(X, Y) _ck_assert_uint(X, <, Y)
#define ck_assert_uint_le(X, Y) _ck_assert_uint(X, <=, Y)
#define ck_assert_uint_gt(X, Y) _ck_assert_uint(X, >, Y)
#define ck_assert_uint_ge(X, Y) _ck_assert_uint(X, >=, Y)
#ifndef CK_FLOATING_DIG
#define CK_FLOATING_DIG 6
#endif
#define _ck_assert_floating(X, OP, Y, TP, TM) do { \
  TP _ck_x = (X); \
  TP _ck_y = (Y); \
  ck_assert_msg(_ck_x OP _ck_y, "Assertion '%s' failed: %s == %.*" TM "g, %s == %.*" TM "g", #X" "#OP" "#Y, #X, (int)CK_FLOATING_DIG, \
                _ck_x, #Y, (int)CK_FLOATING_DIG, _ck_y); \
} while(0);
#define _ck_assert_floating_finite(X, TP, TM) \
do { \
  TP _ck_x = (X); \
  ck_assert_msg(isfinite(_ck_x), "Assertion '%s' failed: %s == %.*" TM "g", #X" is finite", #X, (int)CK_FLOATING_DIG, _ck_x); \
} while(0);
#define _ck_assert_floating_infinite(X, TP, TM) \
do { \
  TP _ck_x = (X); \
  ck_assert_msg(isinf(_ck_x), "Assertion '%s' failed: %s == %.*" TM "g", #X" is infinite", #X, (int)CK_FLOATING_DIG, _ck_x); \
} while(0);
#define _ck_assert_floating_nan(X, TP, TM) \
do { \
  TP _ck_x = (X); \
  ck_assert_msg(isnan(_ck_x), "Assertion '%s' failed: %s == %.*" TM "g", #X" is NaN", #X, (int)CK_FLOATING_DIG, _ck_x); \
} while(0);
#define _ck_assert_floating_nonnan(X, TP, TM) \
do { \
  TP _ck_x = (X); \
  ck_assert_msg(!isnan(_ck_x), "Assertion '%s' failed: %s == %.*" TM "g", #X" is not NaN", #X, (int)CK_FLOATING_DIG, _ck_x); \
} while(0);
#define _ck_assert_floating_op_tol(X, OP, Y, T, D, TP, TM) do { \
  TP _ck_x = (X); \
  TP _ck_y = (Y); \
  TP _ck_t = (T); \
  ck_assert_msg((_ck_x - _ck_y) OP _ck_t * (D), "Assertion '%s' failed: %s == %.*" TM "g, %s == %.*" TM "g, %s == %.*" TM "g", #X" "\
                #OP"= "#Y", error < "#T, #X, (int)CK_FLOATING_DIG, _ck_x, #Y, (int)CK_FLOATING_DIG, _ck_y, #T,\
                (int)CK_FLOATING_DIG, _ck_t); \
} while(0);
#define _ck_assert_floating_absdiff_op_tol(X, Y, OP, T, TP, TM) \
do { \
  TP _ck_x = (X); \
  TP _ck_y = (Y); \
  TP _ck_t = (T); \
  ck_assert_msg(fabsl(_ck_y - _ck_x) OP _ck_t, "Assertion '%s' failed: %s == %.*" TM "g, %s == %.*" TM "g, %s == %.*" TM "g", \
                "fabsl("#Y" - "#X") "#OP" "#T, #X, (int)CK_FLOATING_DIG, _ck_x, #Y, (int)CK_FLOATING_DIG, _ck_y, #T, \
                (int)CK_FLOATING_DIG, _ck_t); \
} while(0);
#define ck_assert_float_eq(X, Y) _ck_assert_floating(X, ==, Y, float, "")
#define ck_assert_float_ne(X, Y) _ck_assert_floating(X, !=, Y, float, "")
#define ck_assert_float_lt(X, Y) _ck_assert_floating(X, <, Y, float, "")
#define ck_assert_float_le(X, Y) _ck_assert_floating(X, <=, Y, float, "")
#define ck_assert_float_gt(X, Y) _ck_assert_floating(X, >, Y, float, "")
#define ck_assert_float_ge(X, Y) _ck_assert_floating(X, >=, Y, float, "")
#define ck_assert_float_eq_tol(X, Y, T)  _ck_assert_floating_absdiff_op_tol(X, Y, <, T, float, "")
#define ck_assert_float_ne_tol(X, Y, T) _ck_assert_floating_absdiff_op_tol(X, Y, >=, T, float, "")
#define ck_assert_float_ge_tol(X, Y, T) _ck_assert_floating_op_tol(X, >, Y, T, -1, float, "")
#define ck_assert_float_le_tol(X, Y, T) _ck_assert_floating_op_tol(X, <, Y, T, 1, float, "")
#define ck_assert_float_finite(X) _ck_assert_floating_finite(X, float, "")
#define ck_assert_float_infinite(X) _ck_assert_floating_infinite(X, float, "")
#define ck_assert_float_nan(X) _ck_assert_floating_nan(X, float, "")
#define ck_assert_float_nonnan(X) _ck_assert_floating_nonnan(X, float, "")
#define ck_assert_double_eq(X, Y) _ck_assert_floating(X, ==, Y, double, "")
#define ck_assert_double_ne(X, Y) _ck_assert_floating(X, !=, Y, double, "")
#define ck_assert_double_lt(X, Y) _ck_assert_floating(X, <, Y, double, "")
#define ck_assert_double_le(X, Y) _ck_assert_floating(X, <=, Y, double, "")
#define ck_assert_double_gt(X, Y) _ck_assert_floating(X, >, Y, double, "")
#define ck_assert_double_ge(X, Y) _ck_assert_floating(X, >=, Y, double, "")
#define ck_assert_double_eq_tol(X, Y, T)  _ck_assert_floating_absdiff_op_tol(X, Y, <, T, double, "")
#define ck_assert_double_ne_tol(X, Y, T) _ck_assert_floating_absdiff_op_tol(X, Y, >=, T, double, "")
#define ck_assert_double_ge_tol(X, Y, T) _ck_assert_floating_op_tol(X, >, Y, T, -1, double, "")
#define ck_assert_double_le_tol(X, Y, T) _ck_assert_floating_op_tol(X, <, Y, T, 1, double, "")
#define ck_assert_double_finite(X) _ck_assert_floating_finite(X, double, "")
#define ck_assert_double_infinite(X) _ck_assert_floating_infinite(X, double, "")
#define ck_assert_double_nan(X) _ck_assert_floating_nan(X, double, "")
#define ck_assert_double_nonnan(X) _ck_assert_floating_nonnan(X, double, "")
#define ck_assert_ldouble_eq(X, Y) _ck_assert_floating(X, ==, Y, long double, "L")
#define ck_assert_ldouble_ne(X, Y) _ck_assert_floating(X, !=, Y, long double, "L")
#define ck_assert_ldouble_lt(X, Y) _ck_assert_floating(X, <, Y, long double, "L")
#define ck_assert_ldouble_le(X, Y) _ck_assert_floating(X, <=, Y, long double, "L")
#define ck_assert_ldouble_gt(X, Y) _ck_assert_floating(X, >, Y, long double, "L")
#define ck_assert_ldouble_ge(X, Y) _ck_assert_floating(X, >=, Y, long double, "L")
#define ck_assert_ldouble_eq_tol(X, Y, T)  _ck_assert_floating_absdiff_op_tol(X, Y, <, T, long double, "L")
#define ck_assert_ldouble_ne_tol(X, Y, T) _ck_assert_floating_absdiff_op_tol(X, Y, >=, T, long double, "L")
#define ck_assert_ldouble_ge_tol(X, Y, T) _ck_assert_floating_op_tol(X, >, Y, T, -1, long double, "L")
#define ck_assert_ldouble_le_tol(X, Y, T) _ck_assert_floating_op_tol(X, <, Y, T, 1, long double, "L")
#define ck_assert_ldouble_finite(X) _ck_assert_floating_finite(X, long double, "L")
#define ck_assert_ldouble_infinite(X) _ck_assert_floating_infinite(X, long double, "L")
#define ck_assert_ldouble_nan(X) _ck_assert_floating_nan(X, long double, "L")
#define ck_assert_ldouble_nonnan(X) _ck_assert_floating_nonnan(X, long double, "L")
#define _ck_assert_str(X, OP, Y, NULLEQ, NULLNE) do { \
  const char* _ck_x = (X); \
  const char* _ck_y = (Y); \
  const char* _ck_x_s; \
  const char* _ck_y_s; \
  const char* _ck_x_q; \
  const char* _ck_y_q; \
  if (_ck_x != NULL) { \
    _ck_x_q = "\""; \
    _ck_x_s = _ck_x; \
  } else { \
    _ck_x_q = ""; \
    _ck_x_s = "(null)"; \
  } \
  if (_ck_y != NULL) { \
    _ck_y_q = "\""; \
    _ck_y_s = _ck_y; \
  } else { \
    _ck_y_q = ""; \
    _ck_y_s = "(null)"; \
  } \
  ck_assert_msg((NULLEQ && (_ck_x == NULL) && (_ck_y == NULL)) || (NULLNE && ((_ck_x == NULL) || (_ck_y == NULL)) && (_ck_x != _ck_y)) || \
                ((_ck_x != NULL) && (_ck_y != NULL) && (0 OP strcmp(_ck_y, _ck_x))), "Assertion '%s' failed: %s == %s%s%s, %s == %s%s%s", \
                #X" "#OP" "#Y, #X, _ck_x_q, _ck_x_s, _ck_x_q, #Y, _ck_y_q, _ck_y_s, _ck_y_q); \
} while(0);
#define ck_assert_str_eq(X, Y) _ck_assert_str(X, ==, Y, 0, 0)
#define ck_assert_str_ne(X, Y) _ck_assert_str(X, !=, Y, 0, 0)
#define ck_assert_str_lt(X, Y) _ck_assert_str(X, <, Y, 0, 0)
#define ck_assert_str_le(X, Y) _ck_assert_str(X, <=, Y, 0, 0)
#define ck_assert_str_gt(X, Y) _ck_assert_str(X, >, Y, 0, 0)
#define ck_assert_str_ge(X, Y) _ck_assert_str(X, >=, Y, 0, 0)
#define ck_assert_pstr_eq(X, Y) _ck_assert_str(X, ==, Y, 1, 0)
#define ck_assert_pstr_ne(X, Y) _ck_assert_str(X, !=, Y, 0, 1)
#ifndef CK_MAX_ASSERT_MEM_PRINT_SIZE
#define CK_MAX_ASSERT_MEM_PRINT_SIZE 64
#endif
#ifndef CK_MAX_ASSERT_MEM_PRINT_SIZE
#define CK_MAX_ASSERT_MEM_PRINT_SIZE 64
#endif
#define _ck_assert_mem(X, OP, Y, L) do { \
  const uint8_t* _ck_x = (const uint8_t*)(X); \
  const uint8_t* _ck_y = (const uint8_t*)(Y); \
  size_t _ck_l = (L); \
  char _ck_x_str[CK_MAX_ASSERT_MEM_PRINT_SIZE * 2 + 1]; \
  char _ck_y_str[CK_MAX_ASSERT_MEM_PRINT_SIZE * 2 + 1]; \
  static const char _ck_hexdigits[] = "0123456789abcdef"; \
  size_t _ck_i; \
  size_t _ck_maxl = (_ck_l > CK_MAX_ASSERT_MEM_PRINT_SIZE) ? CK_MAX_ASSERT_MEM_PRINT_SIZE : _ck_l; \
  for (_ck_i = 0; _ck_i < _ck_maxl; _ck_i++) { \
    _ck_x_str[_ck_i * 2  ]   = _ck_hexdigits[(_ck_x[_ck_i] >> 4) & 0xF]; \
    _ck_y_str[_ck_i * 2  ]   = _ck_hexdigits[(_ck_y[_ck_i] >> 4) & 0xF]; \
    _ck_x_str[_ck_i * 2 + 1] = _ck_hexdigits[_ck_x[_ck_i] & 0xF]; \
    _ck_y_str[_ck_i * 2 + 1] = _ck_hexdigits[_ck_y[_ck_i] & 0xF]; \
  } \
  _ck_x_str[_ck_i * 2] = 0; \
  _ck_y_str[_ck_i * 2] = 0; \
  if (_ck_maxl != _ck_l) { \
    _ck_x_str[_ck_i * 2 - 2] = '.'; \
    _ck_y_str[_ck_i * 2 - 2] = '.'; \
    _ck_x_str[_ck_i * 2 - 1] = '.'; \
    _ck_y_str[_ck_i * 2 - 1] = '.'; \
  } \
  ck_assert_msg(0 OP memcmp(_ck_y, _ck_x, _ck_l), "Assertion '%s' failed: %s == \"%s\", %s == \"%s\"", #X" "#OP" "#Y, #X, _ck_x_str, \
                #Y, _ck_y_str); \
} while(0);
#define ck_assert_mem_eq(X, Y, L) _ck_assert_mem(X, ==, Y, L)
#define ck_assert_mem_ne(X, Y, L) _ck_assert_mem(X, !=, Y, L)
#define ck_assert_mem_lt(X, Y, L) _ck_assert_mem(X, <, Y, L)
#define ck_assert_mem_le(X, Y, L) _ck_assert_mem(X, <=, Y, L)
#define ck_assert_mem_gt(X, Y, L) _ck_assert_mem(X, >, Y, L)
#define ck_assert_mem_ge(X, Y, L) _ck_assert_mem(X, >=, Y, L)
#define _ck_assert_ptr(X, OP, Y) do { \
  const void* _ck_x = (X); \
  const void* _ck_y = (Y); \
  ck_assert_msg(_ck_x OP _ck_y, "Assertion '%s' failed: %s == %#lx, %s == %#lx", #X" "#OP" "#Y, #X, (unsigned long)(uintptr_t)_ck_x, #Y, \
                (unsigned long)(uintptr_t)_ck_y); \
} while(0);
#define _ck_assert_ptr_null(X, OP) do { \
  const void* _ck_x = (X); \
  ck_assert_msg(_ck_x OP NULL, "Assertion '%s' failed: %s == %#lx", #X" "#OP" NULL", #X, (unsigned long)(uintptr_t)_ck_x); \
} while(0);
#define ck_assert_ptr_eq(X, Y) _ck_assert_ptr(X, ==, Y)
#define ck_assert_ptr_ne(X, Y) _ck_assert_ptr(X, !=, Y)
#define ck_assert_ptr_null(X) _ck_assert_ptr_null(X, ==)
#define ck_assert_ptr_nonnull(X) _ck_assert_ptr_null(X, !=)
#define mark_point() _mark_point(__FILE__,__LINE__)
CK_DLL_EXP void CK_EXPORT _mark_point(const char *file, int line);
enum test_result {
    CK_TEST_RESULT_INVALID,
    CK_PASS,
    CK_FAILURE,
    CK_ERROR
};
enum print_output {
    CK_SILENT,
    CK_MINIMAL,
    CK_NORMAL,
    CK_VERBOSE,
    CK_ENV,
    CK_SUBUNIT,
    CK_LAST
};
typedef struct SRunner SRunner;
typedef struct TestResult TestResult;
enum ck_result_ctx {
    CK_CTX_INVALID,
    CK_CTX_SETUP,
    CK_CTX_TEST,
    CK_CTX_TEARDOWN
};
CK_DLL_EXP int CK_EXPORT tr_rtype(TestResult *tr);
CK_DLL_EXP enum ck_result_ctx CK_EXPORT tr_ctx(TestResult *tr);
CK_DLL_EXP const char *CK_EXPORT tr_msg(TestResult *tr);
CK_DLL_EXP int CK_EXPORT tr_lno(TestResult *tr);
CK_DLL_EXP const char *CK_EXPORT tr_lfile(TestResult *tr);
CK_DLL_EXP const char *CK_EXPORT tr_tcname(TestResult *tr);
CK_DLL_EXP SRunner *CK_EXPORT srunner_create(Suite *s);
CK_DLL_EXP void CK_EXPORT srunner_add_suite(SRunner *sr, Suite *s);
CK_DLL_EXP void CK_EXPORT srunner_free(SRunner *sr);
CK_DLL_EXP void CK_EXPORT srunner_run_all(SRunner *sr, enum print_output print_mode);
CK_DLL_EXP void CK_EXPORT srunner_run(SRunner *sr, const char *sname, const char *tcname, enum print_output print_mode);
CK_DLL_EXP void CK_EXPORT srunner_run_tagged(SRunner *sr, const char *sname, const char *tcname, const char *include_tags,
					                         const char *exclude_tags, enum print_output print_mode);
CK_DLL_EXP int CK_EXPORT srunner_ntests_failed(SRunner *sr);
CK_DLL_EXP int CK_EXPORT srunner_ntests_run(SRunner *sr);
CK_DLL_EXP TestResult **CK_EXPORT srunner_failures(SRunner *sr);
CK_DLL_EXP TestResult **CK_EXPORT srunner_results(SRunner *sr);
CK_DLL_EXP void CK_EXPORT srunner_print(SRunner *sr, enum print_output print_mode);
CK_DLL_EXP void CK_EXPORT srunner_set_log(SRunner *sr, const char *fname);
CK_DLL_EXP int CK_EXPORT srunner_has_log(SRunner *sr);
CK_DLL_EXP const char *CK_EXPORT srunner_log_fname(SRunner *sr);
CK_DLL_EXP void CK_EXPORT srunner_set_xml(SRunner *sr, const char *fname);
CK_DLL_EXP int CK_EXPORT srunner_has_xml(SRunner *sr);
CK_DLL_EXP const char *CK_EXPORT srunner_xml_fname(SRunner *sr);
CK_DLL_EXP void CK_EXPORT srunner_set_tap(SRunner *sr, const char *fname);
CK_DLL_EXP int CK_EXPORT srunner_has_tap(SRunner *sr);
CK_DLL_EXP const char *CK_EXPORT srunner_tap_fname(SRunner *sr);
enum fork_status {
    CK_FORK_GETENV,
    CK_FORK,
    CK_NOFORK
};
CK_DLL_EXP enum fork_status CK_EXPORT srunner_fork_status(SRunner *sr);
CK_DLL_EXP void CK_EXPORT srunner_set_fork_status(SRunner *sr, enum fork_status fstat);
CK_DLL_EXP pid_t CK_EXPORT check_fork(void);
CK_DLL_EXP void CK_EXPORT check_waitpid_and_exit(pid_t pid) CK_ATTRIBUTE_NORETURN;
CK_DLL_EXP void CK_EXPORT check_set_max_msg_size(size_t max_msg_size);
#ifdef __cplusplus
CK_CPPEND
#endif
#endif