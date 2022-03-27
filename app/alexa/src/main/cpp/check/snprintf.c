#if HAVE_CONFIG_H
#include <config.h>
#endif
#if !TEST_SNPRINTF
#include <math.h>
#if defined(__NetBSD__) || \
    defined(__FreeBSD__) || \
    defined(__OpenBSD__) || \
    defined(__NeXT__) || \
    defined(__bsd__)
#define OS_BSD 1
#elif defined(sgi) || defined(__sgi)
#ifndef __c99
#define __c99
#endif
#define OS_IRIX 1
#define OS_SYSV 1
#elif defined(__svr4__)
#define OS_SYSV 1
#elif defined(__linux__)
#define OS_LINUX 1
#endif
#if HAVE_CONFIG_H
#ifdef HAVE_SNPRINTF
#undef HAVE_SNPRINTF
#endif
#ifdef HAVE_VSNPRINTF
#undef HAVE_VSNPRINTF
#endif
#ifdef snprintf
#undef snprintf
#endif
#ifdef vsnprintf
#undef vsnprintf
#endif
#else
#ifndef HAVE_STDARG_H
#define HAVE_STDARG_H 1
#endif
#ifndef HAVE_STDDEF_H
#define HAVE_STDDEF_H 1
#endif
#ifndef HAVE_STDINT_H
#define HAVE_STDINT_H 1
#endif
#ifndef HAVE_STDLIB_H
#define HAVE_STDLIB_H 1
#endif
#ifndef HAVE_INTTYPES_H
#define HAVE_INTTYPES_H 1
#endif
#ifndef HAVE_LOCALE_H
#define HAVE_LOCALE_H 1
#endif
#ifndef HAVE_LOCALECONV
#define HAVE_LOCALECONV 1
#endif
#ifndef HAVE_LCONV_DECIMAL_POINT
#define HAVE_LCONV_DECIMAL_POINT 1
#endif
#ifndef HAVE_LCONV_THOUSANDS_SEP
#define HAVE_LCONV_THOUSANDS_SEP 1
#endif
#ifndef HAVE_LONG_DOUBLE
#define HAVE_LONG_DOUBLE 1
#endif
#ifndef HAVE_LONG_LONG_INT
#define HAVE_LONG_LONG_INT 1
#endif
#ifndef HAVE_UNSIGNED_LONG_LONG_INT
#define HAVE_UNSIGNED_LONG_LONG_INT 1
#endif
#ifndef HAVE_INTMAX_T
#define HAVE_INTMAX_T 1
#endif
#ifndef HAVE_UINTMAX_T
#define HAVE_UINTMAX_T 1
#endif
#ifndef HAVE_UINTPTR_T
#define HAVE_UINTPTR_T 1
#endif
#ifndef HAVE_PTRDIFF_T
#define HAVE_PTRDIFF_T 1
#endif
#ifndef HAVE_VA_COPY
#define HAVE_VA_COPY 1
#endif
#ifndef HAVE___VA_COPY
#define HAVE___VA_COPY 1
#endif
#endif
#define snprintf rpl_snprintf
#define vsnprintf rpl_vsnprintf
#endif
#if !HAVE_SNPRINTF || !HAVE_VSNPRINTF
#include <stdio.h>
#include <string.h>
#ifdef VA_START
#undef VA_START
#endif
#ifdef VA_SHIFT
#undef VA_SHIFT
#endif
#if HAVE_STDARG_H
#include <stdarg.h>
#define VA_START(ap, last) va_start(ap, last)
#define VA_SHIFT(ap, value, type)
#else
#include <varargs.h>
#define VA_START(ap, last) va_start(ap)
#define VA_SHIFT(ap, value, type) value = va_arg(ap, type)
#endif
#if !HAVE_VSNPRINTF
#include <errno.h>
#include <limits.h>
#if !HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if HAVE_STDDEF_H
#include <stddef.h>
#endif
#if HAVE_STDINT_H
#include <stdint.h>
#endif
#ifndef ULONG_MAX
#ifdef UINT_MAX
#define ULONG_MAX UINT_MAX
#else
#define ULONG_MAX INT_MAX
#endif
#endif
#ifdef ULLONG
#undef ULLONG
#endif
#if HAVE_UNSIGNED_LONG_LONG_INT
#define ULLONG unsigned long long int
#ifndef ULLONG_MAX
#define ULLONG_MAX ULONG_MAX
#endif
#else
#define ULLONG unsigned long int
#ifdef ULLONG_MAX
#undef ULLONG_MAX
#endif	/* defined(ULLONG_MAX) */
#define ULLONG_MAX ULONG_MAX
#endif
#ifdef UINTMAX_T
#undef UINTMAX_T
#endif
#if defined(HAVE_UINTMAX_T) || defined(uintmax_t)
#define UINTMAX_T uintmax_t
#ifndef UINTMAX_MAX
#define UINTMAX_MAX ULLONG_MAX
#endif
#else
#define UINTMAX_T ULLONG
#ifdef UINTMAX_MAX
#undef UINTMAX_MAX
#endif	/* defined(UINTMAX_MAX) */
#define UINTMAX_MAX ULLONG_MAX
#endif
#ifndef LDOUBLE
#if HAVE_LONG_DOUBLE
#define LDOUBLE long double
#else
#define LDOUBLE double
#endif
#endif
#ifndef LLONG
#if HAVE_LONG_LONG_INT
#define LLONG long long int
#else
#define LLONG long int
#endif
#endif
#ifndef INTMAX_T
#if defined(HAVE_INTMAX_T) || defined(intmax_t)
#define INTMAX_T intmax_t
#else
#define INTMAX_T LLONG
#endif
#endif
#ifndef UINTPTR_T
#if defined(HAVE_UINTPTR_T) || defined(uintptr_t)
#define UINTPTR_T uintptr_t
#else
#define UINTPTR_T unsigned long int
#endif
#endif
#ifndef PTRDIFF_T
#if defined(HAVE_PTRDIFF_T) || defined(ptrdiff_t)
#define PTRDIFF_T ptrdiff_t
#else
#define PTRDIFF_T long int
#endif
#endif
#ifndef UPTRDIFF_T
#define UPTRDIFF_T PTRDIFF_T
#endif
#ifndef SSIZE_T
#define SSIZE_T size_t
#endif
#ifndef ERANGE
#define ERANGE E2BIG
#endif
#ifndef EOVERFLOW
#define EOVERFLOW ERANGE
#endif
#ifdef MAX_CONVERT_LENGTH
#undef MAX_CONVERT_LENGTH
#endif
#define MAX_CONVERT_LENGTH      43
#define PRINT_S_DEFAULT         0
#define PRINT_S_FLAGS           1
#define PRINT_S_WIDTH           2
#define PRINT_S_DOT             3
#define PRINT_S_PRECISION       4
#define PRINT_S_MOD             5
#define PRINT_S_CONV            6
#define PRINT_F_MINUS           (1 << 0)
#define PRINT_F_PLUS            (1 << 1)
#define PRINT_F_SPACE           (1 << 2)
#define PRINT_F_NUM             (1 << 3)
#define PRINT_F_ZERO            (1 << 4)
#define PRINT_F_QUOTE           (1 << 5)
#define PRINT_F_UP              (1 << 6)
#define PRINT_F_UNSIGNED        (1 << 7)
#define PRINT_F_TYPE_G          (1 << 8)
#define PRINT_F_TYPE_E          (1 << 9)
#define PRINT_C_CHAR            1
#define PRINT_C_SHORT           2
#define PRINT_C_LONG            3
#define PRINT_C_LDOUBLE         5
#define PRINT_C_SIZE            6
#define PRINT_C_PTRDIFF         7
#define PRINT_C_INTMAX          8
#ifndef MAX
#define MAX(x, y) ((x >= y) ? x : y)
#endif
#ifndef CHARTOINT
#define CHARTOINT(ch) (ch - '0')
#endif
#ifndef ISDIGIT
#define ISDIGIT(ch) ('0' <= (unsigned char)ch && (unsigned char)ch <= '9')
#endif
#ifndef ISNAN
#define ISNAN(x) (x != x)
#endif
#ifndef ISINF
#define ISINF(x) (x != 0.0 && x + x == x)
#endif
#ifdef OUTCHAR
#undef OUTCHAR
#endif
#define OUTCHAR(str, len, size, ch)                                          \
do {                                                                         \
	if (len + 1 < size)                                                  \
		str[len] = ch;                                               \
	(len)++;                                                             \
} while (/* CONSTCOND */ 0)

static void fmtstr(char *, size_t *, size_t, const char *, int, int, int);
static void fmtint(char *, size_t *, size_t, INTMAX_T, int, int, int, int);
static void fmtflt(char *, size_t *, size_t, LDOUBLE, int, int, int, int *);
static void printsep(char *, size_t *, size_t);
static int getnumsep(int);
static int getexponent(LDOUBLE);
static int convert(UINTMAX_T, char *, size_t, int, int);
static UINTMAX_T cast(LDOUBLE);
static UINTMAX_T myround(LDOUBLE);
static LDOUBLE mypow10(int);
#if HAVE_VSNPRINTF
extern int errno;
#endif
int rpl_vsnprintf(char *str, size_t size, const char *format, va_list args) {
	LDOUBLE fvalue;
	INTMAX_T value;
	unsigned char cvalue;
	const char *strvalue;
	INTMAX_T *intmaxptr;
	PTRDIFF_T *ptrdiffptr;
	SSIZE_T *sizeptr;
	long int *longptr;
	int *intptr;
	short int *shortptr;
	signed char *charptr;
	size_t len = 0;
	int overflow = 0;
	int base = 0;
	int cflags = 0;
	int flags = 0;
	int width = 0;
	int precision = -1;
	int state = PRINT_S_DEFAULT;
	char ch = *format++;
	if (str == NULL && size != 0) size = 0;
	while(ch != '\0')
		switch(state) {
			case PRINT_S_DEFAULT:
				if (ch == '%') state = PRINT_S_FLAGS;
				else OUTCHAR(str, len, size, ch);
				ch = *format++;
				break;
			case PRINT_S_FLAGS:
				switch(ch) {
					case '-':
						flags |= PRINT_F_MINUS;
						ch = *format++;
						break;
					case '+':
						flags |= PRINT_F_PLUS;
						ch = *format++;
						break;
					case ' ':
						flags |= PRINT_F_SPACE;
						ch = *format++;
						break;
					case '#':
						flags |= PRINT_F_NUM;
						ch = *format++;
						break;
					case '0':
						flags |= PRINT_F_ZERO;
						ch = *format++;
						break;
					case '\'':
						flags |= PRINT_F_QUOTE;
						ch = *format++;
						break;
					default: state = PRINT_S_WIDTH; break;
				}
				break;
			case PRINT_S_WIDTH:
				if (ISDIGIT(ch)) {
					ch = CHARTOINT(ch);
					if (width > (INT_MAX - ch) / 10) {
						overflow = 1;
						goto out;
					}
					width = 10 * width + ch;
					ch = *format++;
				} else if (ch == '*') {
					if ((width = va_arg(args, int)) < 0) {
						flags |= PRINT_F_MINUS;
						width = -width;
					}
					ch = *format++;
					state = PRINT_S_DOT;
				} else state = PRINT_S_DOT;
				break;
			case PRINT_S_DOT:
				if (ch == '.') {
					state = PRINT_S_PRECISION;
					ch = *format++;
				} else state = PRINT_S_MOD;
				break;
			case PRINT_S_PRECISION:
				if (precision == -1) precision = 0;
				if (ISDIGIT(ch)) {
					ch = CHARTOINT(ch);
					if (precision > (INT_MAX - ch) / 10) {
						overflow = 1;
						goto out;
					}
					precision = 10 * precision + ch;
					ch = *format++;
				} else if (ch == '*') {
					if ((precision = va_arg(args, int)) < 0) precision = -1;
					ch = *format++;
					state = PRINT_S_MOD;
				} else state = PRINT_S_MOD;
				break;
			case PRINT_S_MOD:
				switch(ch) {
					case 'h':
						ch = *format++;
						if (ch == 'h') {
							ch = *format++;
							cflags = PRINT_C_CHAR;
						} else cflags = PRINT_C_SHORT;
						break;
					case 'l':
						ch = *format++;
						cflags = PRINT_C_LONG;
						break;
					case 'L':
						cflags = PRINT_C_LDOUBLE;
						ch = *format++;
						break;
					case 'j':
						cflags = PRINT_C_INTMAX;
						ch = *format++;
						break;
					case 't':
						cflags = PRINT_C_PTRDIFF;
						ch = *format++;
						break;
					case 'z':
						cflags = PRINT_C_SIZE;
						ch = *format++;
						break;
					default: break;
				}
				state = PRINT_S_CONV;
				break;
			case PRINT_S_CONV:
				switch(ch) {
					case 'd': case 'i':
						switch(cflags) {
							case PRINT_C_CHAR: value = (signed char)va_arg(args, int); break;
							case PRINT_C_SHORT: value = (short int)va_arg(args, int); break;
							case PRINT_C_LONG: value = va_arg(args, long int); break;
							case PRINT_C_SIZE: value = va_arg(args, SSIZE_T); break;
							case PRINT_C_INTMAX: value = va_arg(args, INTMAX_T); break;
							case PRINT_C_PTRDIFF: value = va_arg(args, PTRDIFF_T); break;
							default: value = va_arg(args, int); break;
						}
						fmtint(str, &len, size, value, 10, width, precision, flags);
						break;
					case 'X': flags |= PRINT_F_UP;
					case 'x': base = 16;
					case 'o': if (base == 0) base = 8;
					case 'u':
						if (base == 0) base = 10;
						flags |= PRINT_F_UNSIGNED;
						switch(cflags) {
							case PRINT_C_CHAR: value = (unsigned char)va_arg(args, unsigned int); break;
							case PRINT_C_SHORT: value = (unsigned short int)va_arg(args, unsigned int); break;
							case PRINT_C_LONG: value = va_arg(args, unsigned long int); break;
							case PRINT_C_SIZE: value = va_arg(args, size_t); break;
							case PRINT_C_INTMAX: value = va_arg(args, UINTMAX_T); break;
							case PRINT_C_PTRDIFF: value = va_arg(args, UPTRDIFF_T); break;
							default: value = va_arg(args, unsigned int); break;
						}
						fmtint(str, &len, size, value, base, width, precision, flags);
						break;
					case 'A': case 'F': flags |= PRINT_F_UP;
					case 'a': case 'f':
						if (cflags == PRINT_C_LDOUBLE) fvalue = va_arg(args, LDOUBLE);
						else fvalue = va_arg(args, double);
						fmtflt(str, &len, size, fvalue, width, precision, flags, &overflow);
						if (overflow) goto out;
						break;
					case 'E': flags |= PRINT_F_UP;
					case 'e':
						flags |= PRINT_F_TYPE_E;
						if (cflags == PRINT_C_LDOUBLE) fvalue = va_arg(args, LDOUBLE);
						else fvalue = va_arg(args, double);
						fmtflt(str, &len, size, fvalue, width, precision, flags, &overflow);
						if (overflow) goto out;
						break;
					case 'G': flags |= PRINT_F_UP;
					case 'g':
						flags |= PRINT_F_TYPE_G;
						if (cflags == PRINT_C_LDOUBLE) fvalue = va_arg(args, LDOUBLE);
						else fvalue = va_arg(args, double);
						if (precision == 0) precision = 1;
						fmtflt(str, &len, size, fvalue, width, precision, flags, &overflow);
						if (overflow) goto out;
						break;
					case 'c':
						cvalue = va_arg(args, int);
						OUTCHAR(str, len, size, cvalue);
						break;
					case 's':
						strvalue = va_arg(args, char *);
						fmtstr(str, &len, size, strvalue, width, precision, flags);
						break;
					case 'p':
						if ((strvalue = (const char *)va_arg(args, void *)) == NULL) {
							fmtstr(str, &len, size, "(nil)", width, -1, flags);
						} else {
							flags |= PRINT_F_NUM;
							flags |= PRINT_F_UNSIGNED;
							fmtint(str, &len, size, (UINTPTR_T)strvalue, 16, width, precision, flags);
						}
						break;
					case 'n':
						switch(cflags) {
						case PRINT_C_CHAR:
							charptr = va_arg(args, signed char *);
							*charptr = len;
							break;
						case PRINT_C_SHORT:
							shortptr = va_arg(args, short int *);
							*shortptr = len;
							break;
						case PRINT_C_LONG:
							longptr = va_arg(args, long int *);
							*longptr = len;
							break;
						case PRINT_C_SIZE:
							sizeptr = va_arg(args, SSIZE_T *);
							*sizeptr = len;
							break;
						case PRINT_C_INTMAX:
							intmaxptr = va_arg(args, INTMAX_T *);
							*intmaxptr = len;
							break;
						case PRINT_C_PTRDIFF:
							ptrdiffptr = va_arg(args, PTRDIFF_T *);
							*ptrdiffptr = len;
							break;
						default:
							intptr = va_arg(args, int *);
							*intptr = len;
							break;
						}
						break;
					case '%': OUTCHAR(str, len, size, ch); break;
					default: break;
				}
				ch = *format++;
				state = PRINT_S_DEFAULT;
				base = cflags = flags = width = 0;
				precision = -1;
				break;
			default: break;
		}
out:
	if (len < size) str[len] = '\0';
	else if (size > 0) str[size - 1] = '\0';
	if (overflow || len >= INT_MAX) {
		errno = overflow ? EOVERFLOW : ERANGE;
		return -1;
	}
	return (int)len;
}
static void fmtstr(char *str, size_t *len, size_t size, const char *value, int width, int precision, int flags) {
	int padlen, strln;
	int noprecision = (precision == -1);
	if (value == NULL) value = "(null)";
	for (strln = 0; value[strln] != '\0' && (noprecision || strln < precision); strln++) continue;
	if ((padlen = width - strln) < 0) padlen = 0;
	if (flags & PRINT_F_MINUS) padlen = -padlen;
	while(padlen > 0) {
		OUTCHAR(str, *len, size, ' ');
		padlen--;
	}
	while(*value != '\0' && (noprecision || precision-- > 0)) {
		OUTCHAR(str, *len, size, *value);
		value++;
	}
	while(padlen < 0) {
		OUTCHAR(str, *len, size, ' ');
		padlen++;
	}
}
static void fmtint(char *str, size_t *len, size_t size, INTMAX_T value, int base, int width, int precision, int flags) {
	UINTMAX_T uvalue;
	char iconvert[MAX_CONVERT_LENGTH];
	char sign = 0;
	char hexprefix = 0;
	int spadlen = 0;
	int zpadlen = 0;
	int pos;
	int separators = (flags & PRINT_F_QUOTE);
	int noprecision = (precision == -1);
	if (flags & PRINT_F_UNSIGNED) uvalue = value;
	else {
		uvalue = (value >= 0) ? value : -value;
		if (value < 0) sign = '-';
		else if (flags & PRINT_F_PLUS) sign = '+';
		else if (flags & PRINT_F_SPACE) sign = ' ';
	}
	pos = convert(uvalue, iconvert, sizeof(iconvert), base,flags & PRINT_F_UP);
	if (flags & PRINT_F_NUM && uvalue != 0) {
		switch (base) {
			case 8:
				if (precision <= pos) precision = pos + 1;
				break;
			case 16:
				hexprefix = (flags & PRINT_F_UP) ? 'X' : 'x';
				break;
			default: break;
		}
	}
	if (separators)	separators = getnumsep(pos);
	zpadlen = precision - pos - separators;
	spadlen = width - separators - MAX(precision, pos) - ((sign != 0) ? 1 : 0) - ((hexprefix != 0) ? 2 : 0);
	if (zpadlen < 0) zpadlen = 0;
	if (spadlen < 0) spadlen = 0;
	if (flags & PRINT_F_MINUS) spadlen = -spadlen;
	else if (flags & PRINT_F_ZERO && noprecision) {
		zpadlen += spadlen;
		spadlen = 0;
	}
	while(spadlen > 0) {
		OUTCHAR(str, *len, size, ' ');
		spadlen--;
	}
	if (sign != 0) OUTCHAR(str, *len, size, sign);
	if (hexprefix != 0) {
		OUTCHAR(str, *len, size, '0');
		OUTCHAR(str, *len, size, hexprefix);
	}
	while(zpadlen > 0) {
		OUTCHAR(str, *len, size, '0');
		zpadlen--;
	}
	while(pos > 0) {
		pos--;
		OUTCHAR(str, *len, size, iconvert[pos]);
		if (separators > 0 && pos > 0 && pos % 3 == 0) printsep(str, len, size);
	}
	while(spadlen < 0) {
		OUTCHAR(str, *len, size, ' ');
		spadlen++;
	}
}
static void fmtflt(char *str, size_t *len, size_t size, LDOUBLE fvalue, int width, int precision, int flags, int *overflow) {
	LDOUBLE ufvalue;
	UINTMAX_T intpart;
	UINTMAX_T fracpart;
	UINTMAX_T mask;
	const char *infnan = NULL;
	char iconvert[MAX_CONVERT_LENGTH];
	char fconvert[MAX_CONVERT_LENGTH];
	char econvert[4];
	char esign = 0;
	char sign = 0;
	int leadfraczeros = 0;
	int exponent = 0;
	int emitpoint = 0;
	int omitzeros = 0;
	int omitcount = 0;
	int padlen = 0;
	int epos = 0;
	int fpos = 0;
	int ipos = 0;
	int separators = (flags & PRINT_F_QUOTE);
	int estyle = (flags & PRINT_F_TYPE_E);
#if HAVE_LOCALECONV && HAVE_LCONV_DECIMAL_POINT
	struct lconv *lc = localeconv();
#endif
	memset(iconvert, '\0', MAX_CONVERT_LENGTH);
	memset(fconvert, '\0', MAX_CONVERT_LENGTH);
	if (precision == -1) precision = 6;
	if (fvalue < 0.0) sign = '-';
	else if (flags & PRINT_F_PLUS) sign = '+';
	else if (flags & PRINT_F_SPACE) sign = ' ';
	if (ISNAN(fvalue)) infnan = (flags & PRINT_F_UP) ? "NAN" : "nan";
	else if (ISINF(fvalue)) infnan = (flags & PRINT_F_UP) ? "INF" : "inf";
	if (infnan != NULL) {
		if (sign != 0) iconvert[ipos++] = sign;
		while(*infnan != '\0') iconvert[ipos++] = *infnan++;
		fmtstr(str, len, size, iconvert, width, ipos, flags);
		return;
	}
	if (flags & PRINT_F_TYPE_E || flags & PRINT_F_TYPE_G) {
		if (flags & PRINT_F_TYPE_G) {
			precision--;
			if (!(flags & PRINT_F_NUM)) omitzeros = 1;
		}
		exponent = getexponent(fvalue);
		estyle = 1;
	}
again:
	switch(sizeof(UINTMAX_T)) {
		case 16:
			if (precision > 38) precision = 38;
			break;
		case 8:
			if (precision > 19) precision = 19;
			break;
		default:
			if (precision > 9) precision = 9;
			break;
	}
	ufvalue = (fvalue >= 0.0) ? fvalue : -fvalue;
	if (estyle)	ufvalue /= mypow10(exponent);
	if ((intpart = cast(ufvalue)) == UINTMAX_MAX) {
		*overflow = 1;
		return;
	}
	mask = mypow10(precision);
	if ((fracpart = myround(mask * (ufvalue - intpart))) >= mask) {
		intpart++;
		fracpart = 0;
		if (estyle && intpart == 10) {
			intpart = 1;
			exponent++;
		}
	}
	if (flags & PRINT_F_TYPE_G && estyle &&
	    precision + 1 > exponent && exponent >= -4) {
		precision -= exponent;
		estyle = 0;
		goto again;
	}
	if (estyle) {
		if (exponent < 0) {
			exponent = -exponent;
			esign = '-';
		} else esign = '+';
		epos = convert(exponent, econvert, 2, 10, 0);
		if (epos == 1) econvert[epos++] = '0';
		econvert[epos++] = esign;
		econvert[epos++] = (flags & PRINT_F_UP) ? 'E' : 'e';
	}
	ipos = convert(intpart, iconvert, sizeof(iconvert), 10, 0);
	if (fracpart != 0) fpos = convert(fracpart, fconvert, sizeof(fconvert), 10, 0);
	leadfraczeros = precision - fpos;
	if (omitzeros) {
		if (fpos > 0) while (omitcount < fpos && fconvert[omitcount] == '0') omitcount++;
		else {
			omitcount = precision;
			leadfraczeros = 0;
		}
		precision -= omitcount;
	}
	if (precision > 0 || flags & PRINT_F_NUM) emitpoint = 1;
	if (separators) separators = getnumsep(ipos);
	padlen = width - ipos - epos - precision - separators - (emitpoint ? 1 : 0) - ((sign != 0) ? 1 : 0);
	if (padlen < 0) padlen = 0;
	if (flags & PRINT_F_MINUS) padlen = -padlen;
	else if (flags & PRINT_F_ZERO && padlen > 0) {
		if (sign != 0) {
			OUTCHAR(str, *len, size, sign);
			sign = 0;
		}
		while(padlen > 0) {
			OUTCHAR(str, *len, size, '0');
			padlen--;
		}
	}
	while(padlen > 0) {
		OUTCHAR(str, *len, size, ' ');
		padlen--;
	}
	if (sign != 0) OUTCHAR(str, *len, size, sign);
	while (ipos > 0) {
		ipos--;
		OUTCHAR(str, *len, size, iconvert[ipos]);
		if (separators > 0 && ipos > 0 && ipos % 3 == 0) printsep(str, len, size);
	}
	if (emitpoint) {
	#if HAVE_LOCALECONV && HAVE_LCONV_DECIMAL_POINT
		if (lc->decimal_point != NULL && *lc->decimal_point != '\0') OUTCHAR(str, *len, size, *lc->decimal_point);
		else
	#endif
		OUTCHAR(str, *len, size, '.');
	}
	while(leadfraczeros > 0) {
		OUTCHAR(str, *len, size, '0');
		leadfraczeros--;
	}
	while(fpos > omitcount) {
		fpos--;
		OUTCHAR(str, *len, size, fconvert[fpos]);
	}
	while(epos > 0) {
		epos--;
		OUTCHAR(str, *len, size, econvert[epos]);
	}
	while(padlen < 0) {
		OUTCHAR(str, *len, size, ' ');
		padlen++;
	}
}
static void printsep(char *str, size_t *len, size_t size) {
#if HAVE_LOCALECONV && HAVE_LCONV_THOUSANDS_SEP
	struct lconv *lc = localeconv();
	int i;
	if (lc->thousands_sep != NULL) for (i = 0; lc->thousands_sep[i] != '\0'; i++) OUTCHAR(str, *len, size, lc->thousands_sep[i]);
	else
#endif
	OUTCHAR(str, *len, size, ',');
}
static int getnumsep(int digits) {
	int separators = (digits - ((digits % 3 == 0) ? 1 : 0)) / 3;
#if HAVE_LOCALECONV && HAVE_LCONV_THOUSANDS_SEP
	int strln;
	struct lconv *lc = localeconv();
	if (lc->thousands_sep != NULL) {
		for (strln = 0; lc->thousands_sep[strln] != '\0'; strln++) continue;
		separators *= strln;
	}
#endif
	return separators;
}
static int getexponent(LDOUBLE value) {
	LDOUBLE tmp = (value >= 0.0) ? value : -value;
	int exponent = 0;
	while(tmp < 1.0 && tmp > 0.0 && --exponent > -99) tmp *= 10;
	while(tmp >= 10.0 && ++exponent < 99) tmp /= 10;
	return exponent;
}
static int convert(UINTMAX_T value, char *buf, size_t size, int base, int caps) {
	const char *digits = caps ? "0123456789ABCDEF" : "0123456789abcdef";
	size_t pos = 0;
	do {
		buf[pos++] = digits[value % base];
		value /= base;
	} while (value != 0 && pos < size);
	return (int)pos;
}
static UINTMAX_T cast(LDOUBLE value) {
	UINTMAX_T result;
	if (value >= UINTMAX_MAX) return UINTMAX_MAX;
	result = value;
	return (result <= value) ? result : result - 1;
}
static UINTMAX_T myround(LDOUBLE value) {
	UINTMAX_T intpart = cast(value);
	return ((value -= intpart) < 0.5) ? intpart : intpart + 1;
}
static LDOUBLE mypow10(int exponent) {
	LDOUBLE result = 1;
	while(exponent > 0) {
		result *= 10;
		exponent--;
	}
	while(exponent < 0) {
		result /= 10;
		exponent++;
	}
	return result;
}
#endif
#if !HAVE_SNPRINTF
#if HAVE_STDARG_H
int rpl_snprintf(char *str, size_t size, const char *format, ...)
#else
int rpl_snprintf(va_alist) va_dcl
#endif
{
#if !HAVE_STDARG_H
	char *str;
	size_t size;
	char *format;
#endif
	va_list ap;
	int len;
	VA_START(ap, format);
	VA_SHIFT(ap, str, char *);
	VA_SHIFT(ap, size, size_t);
	VA_SHIFT(ap, format, const char *);
	len = vsnprintf(str, size, format, ap);
	va_end(ap);
	return len;
}
#endif
#else
int main(void);
#endif
#if !TEST_SNPRINTF
int main(void) {
	const char *float_fmt[] = {
	#if HAVE_LONG_LONG_INT && !OS_BSD && !OS_IRIX
		"%.16e",
		"%22.16e",
		"%022.16e",
		"%-22.16e",
		"%#+'022.16e",
	#endif
		"foo|%#+0123.9E|bar",
		"%-123.9e",
		"%123.9e",
		"%+23.9e",
		"%+05.8e",
		"%-05.8e",
		"%05.8e",
		"%+5.8e",
		"%-5.8e",
		"% 5.8e",
		"%5.8e",
		"%+4.9e",
	#if !OS_LINUX
		"%+#010.0e",
		"%#10.1e",
		"%10.5e",
		"% 10.5e",
		"%5.0e",
		"%5.e",
		"%#5.0e",
		"%#5.e",
		"%3.2e",
		"%3.1e",
		"%-1.5e",
		"%1.5e",
		"%01.3e",
		"%1.e",
		"%.1e",
		"%#.0e",
		"%+.0e",
		"% .0e",
		"%.0e",
		"%#.e",
		"%+.e",
		"% .e",
		"%.e",
		"%4e",
		"%e",
		"%E",
	#endif
	#if !OS_BSD && !OS_IRIX
		"% '022f",
		"%+'022f",
		"%-'22f",
		"%'22f",
	#if HAVE_LONG_LONG_INT
		"%.16f",
		"%22.16f",
		"%022.16f",
		"%-22.16f",
		"%#+'022.16f",
	#endif
	#endif
		"foo|%#+0123.9F|bar",
		"%-123.9f",
		"%123.9f",
		"%+23.9f",
		"%+#010.0f",
		"%#10.1f",
		"%10.5f",
		"% 10.5f",
		"%+05.8f",
		"%-05.8f",
		"%05.8f",
		"%+5.8f",
		"%-5.8f",
		"% 5.8f",
		"%5.8f",
		"%5.0f",
		"%5.f",
		"%#5.0f",
		"%#5.f",
		"%+4.9f",
		"%3.2f",
		"%3.1f",
		"%-1.5f",
		"%1.5f",
		"%01.3f",
		"%1.f",
		"%.1f",
		"%#.0f",
		"%+.0f",
		"% .0f",
		"%.0f",
		"%#.f",
		"%+.f",
		"% .f",
		"%.f",
		"%4f",
		"%f",
		"%F",
	#if !OS_BSD && !OS_IRIX && !OS_LINUX
		"% '022g",
		"%+'022g",
		"%-'22g",
		"%'22g",
	#if HAVE_LONG_LONG_INT
		"%.16g",
		"%22.16g",
		"%022.16g",
		"%-22.16g",
		"%#+'022.16g",
	#endif
	#endif
		"foo|%#+0123.9G|bar",
		"%-123.9g",
		"%123.9g",
		"%+23.9g",
		"%+05.8g",
		"%-05.8g",
		"%05.8g",
		"%+5.8g",
		"%-5.8g",
		"% 5.8g",
		"%5.8g",
		"%+4.9g",
	#if !OS_LINUX
		"%+#010.0g",
		"%#10.1g",
		"%10.5g",
		"% 10.5g",
		"%5.0g",
		"%5.g",
		"%#5.0g",
		"%#5.g",
		"%3.2g",
		"%3.1g",
		"%-1.5g",
		"%1.5g",
		"%01.3g",
		"%1.g",
		"%.1g",
		"%#.0g",
		"%+.0g",
		"% .0g",
		"%.0g",
		"%#.g",
		"%+.g",
		"% .g",
		"%.g",
		"%4g",
		"%g",
		"%G",
	#endif
		NULL
	};
	double float_val[] = {
		-4.136,
		-134.52,
		-5.04030201,
		-3410.01234,
		-999999.999999,
		-913450.29876,
		-913450.2,
		-91345.2,
		-9134.2,
		-913.2,
		-91.2,
		-9.2,
		-9.9,
		4.136,
		134.52,
		5.04030201,
		3410.01234,
		999999.999999,
		913450.29876,
		913450.2,
		91345.2,
		9134.2,
		913.2,
		91.2,
		9.2,
		9.9,
		9.96,
		9.996,
		9.9996,
		9.99996,
		9.999996,
		9.9999996,
		9.99999996,
		0.99999996,
		0.99999999,
		0.09999999,
		0.00999999,
		0.00099999,
		0.00009999,
		0.00000999,
		0.00000099,
		0.00000009,
		0.00000001,
		0.0000001,
		0.000001,
		0.00001,
		0.0001,
		0.001,
		0.01,
		0.1,
		1.0,
		1.5,
		-1.5,
		-1.0,
		-0.1,
	#if !OS_BSD
	#ifdef INFINITY
		INFINITY,
		-INFINITY,
	#endif
	#ifdef NAN
		NAN,
	#endif
	#endif
		0
	};
	const char *long_fmt[] = {
		"foo|%0123ld|bar",
	#if !OS_IRIX
		"% '0123ld",
		"%+'0123ld",
		"%-'123ld",
		"%'123ld",
	#endif
		"%123.9ld",
		"% 123.9ld",
		"%+123.9ld",
		"%-123.9ld",
		"%0123ld",
		"% 0123ld",
		"%+0123ld",
		"%-0123ld",
		"%10.5ld",
		"% 10.5ld",
		"%+10.5ld",
		"%-10.5ld",
		"%010ld",
		"% 010ld",
		"%+010ld",
		"%-010ld",
		"%4.2ld",
		"% 4.2ld",
		"%+4.2ld",
		"%-4.2ld",
		"%04ld",
		"% 04ld",
		"%+04ld",
		"%-04ld",
		"%5.5ld",
		"%+22.33ld",
		"%01.3ld",
		"%1.5ld",
		"%-1.5ld",
		"%44ld",
		"%4ld",
		"%4.0ld",
		"%4.ld",
		"%.44ld",
		"%.4ld",
		"%.0ld",
		"%.ld",
		"%ld",
		NULL
	};
	long int long_val[] = {
	#ifdef LONG_MAX
		LONG_MAX,
	#endif
	#ifdef LONG_MIN
		LONG_MIN,
	#endif
		-91340,
		91340,
		341,
		134,
		0203,
		-1,
		1,
		0
	};
	const char *ulong_fmt[] = {
		"foo|%0123lu|bar",
	#if !OS_IRIX
		"% '0123lu",
		"%+'0123lu",
		"%-'123lu",
		"%'123lu",
	#endif
		"%123.9lu",
		"% 123.9lu",
		"%+123.9lu",
		"%-123.9lu",
		"%0123lu",
		"% 0123lu",
		"%+0123lu",
		"%-0123lu",
		"%5.5lu",
		"%+22.33lu",
		"%01.3lu",
		"%1.5lu",
		"%-1.5lu",
		"%44lu",
		"%lu",
		"foo|%#0123lo|bar",
		"%#123.9lo",
		"%# 123.9lo",
		"%#+123.9lo",
		"%#-123.9lo",
		"%#0123lo",
		"%# 0123lo",
		"%#+0123lo",
		"%#-0123lo",
		"%#5.5lo",
		"%#+22.33lo",
		"%#01.3lo",
		"%#1.5lo",
		"%#-1.5lo",
		"%#44lo",
		"%#lo",
		"%123.9lo",
		"% 123.9lo",
		"%+123.9lo",
		"%-123.9lo",
		"%0123lo",
		"% 0123lo",
		"%+0123lo",
		"%-0123lo",
		"%5.5lo",
		"%+22.33lo",
		"%01.3lo",
		"%1.5lo",
		"%-1.5lo",
		"%44lo",
		"%lo",
		"foo|%#0123lX|bar",
		"%#123.9lx",
		"%# 123.9lx",
		"%#+123.9lx",
		"%#-123.9lx",
		"%#0123lx",
		"%# 0123lx",
		"%#+0123lx",
		"%#-0123lx",
		"%#5.5lx",
		"%#+22.33lx",
		"%#01.3lx",
		"%#1.5lx",
		"%#-1.5lx",
		"%#44lx",
		"%#lx",
		"%#lX",
		"%123.9lx",
		"% 123.9lx",
		"%+123.9lx",
		"%-123.9lx",
		"%0123lx",
		"% 0123lx",
		"%+0123lx",
		"%-0123lx",
		"%5.5lx",
		"%+22.33lx",
		"%01.3lx",
		"%1.5lx",
		"%-1.5lx",
		"%44lx",
		"%lx",
		"%lX",
		NULL
	};
	unsigned long int ulong_val[] = {
	#ifdef ULONG_MAX
		ULONG_MAX,
	#endif
		91340,
		341,
		134,
		0203,
		1,
		0
	};
	const char *llong_fmt[] = {
		"foo|%0123lld|bar",
		"%123.9lld",
		"% 123.9lld",
		"%+123.9lld",
		"%-123.9lld",
		"%0123lld",
		"% 0123lld",
		"%+0123lld",
		"%-0123lld",
		"%5.5lld",
		"%+22.33lld",
		"%01.3lld",
		"%1.5lld",
		"%-1.5lld",
		"%44lld",
		"%lld",
		NULL
	};
	LLONG llong_val[] = {
	#ifdef LLONG_MAX
		LLONG_MAX,
	#endif
	#ifdef LLONG_MIN
		LLONG_MIN,
	#endif
		-91340,
		91340,
		341,
		134,
		0203,
		-1,
		1,
		0
	};
	const char *string_fmt[] = {
		"foo|%10.10s|bar",
		"%-10.10s",
		"%10.10s",
		"%10.5s",
		"%5.10s",
		"%10.1s",
		"%1.10s",
		"%10.0s",
		"%0.10s",
		"%-42.5s",
		"%2.s",
		"%.10s",
		"%.1s",
		"%.0s",
		"%.s",
		"%4s",
		"%s",
		NULL
	};
	const char *string_val[] = {
		"Hello",
		"Hello, world!",
		"Sound check: One, two, three.",
		"This string is a little longer than the other strings.",
		"1",
		"",
		NULL
	};
#if !OS_SYSV
	const char *pointer_fmt[] = {
		"foo|%p|bar",
		"%42p",
		"%p",
		NULL
	};
	const char *pointer_val[] = {
		*pointer_fmt,
		*string_fmt,
		*string_val,
		NULL
	};
#endif
	char buf1[1024], buf2[1024];
	double value, digits = 9.123456789012345678901234567890123456789;
	int i, j, r1, r2, failed = 0, num = 0;
#ifndef TEST_NILS
#define TEST_NILS 0
#elif TEST_NILS
#undef TEST_NILS
#define TEST_NILS 1
#endif
#ifdef TEST
#undef TEST
#endif
#define TEST(fmt, val) \
	do { \
		for (i = 0; fmt[i] != NULL; i++) \
			for (j = 0; j == 0 || val[j - TEST_NILS] != 0; j++) { \
				r1 = sprintf(buf1, fmt[i], val[j]); \
				r2 = snprintf(buf2, sizeof(buf2), fmt[i], val[j]); \
				if (strcmp(buf1, buf2) != 0 || r1 != r2) { \
					(void)printf("Results don't match, format string: %s\n\t sprintf(3): [%s] (%d)\n\tsnprintf(3): [%s] (%d)\n", \
						fmt[i], buf1, r1, buf2, r2); \
					failed++; \
				} \
				num++; \
			} \
	} while(/* CONSTCOND */ 0);
#if HAVE_LOCALE_H
	(void)setlocale(LC_ALL, "");
#endif
	(void)puts("Testing our snprintf(3) against your system's sprintf(3).");
	TEST(float_fmt, float_val);
	TEST(long_fmt, long_val);
	TEST(ulong_fmt, ulong_val);
	TEST(llong_fmt, llong_val);
	TEST(string_fmt, string_val);
#if !OS_SYSV
	TEST(pointer_fmt, pointer_val);
#endif
	(void)printf("Result: %d out of %d tests failed.\n", failed, num);
	(void)fputs("Checking how many digits we support: ", stdout);
	for (i = 0; i < 100; i++) {
		value = pow(10, i) * digits;
		(void)sprintf(buf1, "%.1f", value);
		(void)snprintf(buf2, sizeof(buf2), "%.1f", value);
		if (strcmp(buf1, buf2) != 0) {
			(void)printf("apparently %d.\n", i);
			break;
		}
	}
	return (failed == 0) ? 0 : 1;
}
#endif