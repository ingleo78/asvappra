#ifndef PCRE_INTERNAL_H
#define PCRE_INTERNAL_H

#if 0
#define PCRE_DEBUG
#endif
#if defined EBCDIC && defined SUPPORT_UTF8
#error The use of both EBCDIC and SUPPORT_UTF8 is not supported.
#endif
#if defined SUPPORT_UCP && !defined SUPPORT_UTF8
#define SUPPORT_UTF8 1
#endif
#undef DPRINTF
#ifdef PCRE_DEBUG
#define DPRINTF(p) printf p
#else
#define DPRINTF(p) /* Nothing */
#endif
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../glib.h"

#ifndef PCRE_EXP_DECL
#  ifdef _WIN32
#    ifndef PCRE_STATIC
#      define PCRE_EXP_DECL       extern __declspec(dllexport)
#      define PCRE_EXP_DEFN       __declspec(dllexport)
#      define PCRE_EXP_DATA_DEFN  __declspec(dllexport)
#    else
#      define PCRE_EXP_DECL       extern
#      define PCRE_EXP_DEFN
#      define PCRE_EXP_DATA_DEFN
#    endif
#  else
#    ifdef __cplusplus
#      define PCRE_EXP_DECL       extern "C"
#    else
#      define PCRE_EXP_DECL       extern
#    endif
#    ifndef PCRE_EXP_DEFN
#      define PCRE_EXP_DEFN       PCRE_EXP_DECL
#    endif
#    ifndef PCRE_EXP_DATA_DEFN
#      define PCRE_EXP_DATA_DEFN
#    endif
#  endif
#endif
#ifndef PCRE_CALL_CONVENTION
#define PCRE_CALL_CONVENTION
#endif
#if USHRT_MAX == 65535
  typedef unsigned short pcre_uint16;
  typedef short pcre_int16;
#elif UINT_MAX == 65535
  typedef unsigned int pcre_uint16;
  typedef int pcre_int16;
#else
  #error Cannot determine a type for 16-bit unsigned integers
#endif
#if UINT_MAX == 4294967295
  typedef unsigned int pcre_uint32;
  typedef int pcre_int32;
#elif ULONG_MAX == 4294967295
  typedef unsigned long int pcre_uint32;
  typedef long int pcre_int32;
#else
  #error Cannot determine a type for 32-bit unsigned integers
#endif
#if HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#if defined INT64_MAX || defined int64_t
#define INT64_OR_DOUBLE int64_t
#else
#define INT64_OR_DOUBLE double
#endif
typedef unsigned char uschar;
#define NOTACHAR 0xffffffff
#define NLTYPE_FIXED    0
#define NLTYPE_ANY      1
#define NLTYPE_ANYCRLF  2
#define IS_NEWLINE(p)                                                                                                                               \
  ((NLBLOCK->nltype != NLTYPE_FIXED) ? ((p) < NLBLOCK->PSEND && _pcre_is_newline((p), NLBLOCK->nltype, NLBLOCK->PSEND, &(NLBLOCK->nllen),  utf8)) : \
   ((p) <= NLBLOCK->PSEND - NLBLOCK->nllen && (p)[0] == NLBLOCK->nl[0] && (NLBLOCK->nllen == 1 || (p)[1] == NLBLOCK->nl[1])))
#define WAS_NEWLINE(p)                                                                                                                                  \
  ((NLBLOCK->nltype != NLTYPE_FIXED) ? ((p) > NLBLOCK->PSSTART && _pcre_was_newline((p), NLBLOCK->nltype, NLBLOCK->PSSTART, &(NLBLOCK->nllen), utf8)) : \
   ((p) >= NLBLOCK->PSSTART + NLBLOCK->nllen && (p)[-NLBLOCK->nllen] == NLBLOCK->nl[0] && (NLBLOCK->nllen == 1 || (p)[-NLBLOCK->nllen+1] == NLBLOCK->nl[1])))
#ifdef CUSTOM_SUBJECT_PTR
#define PCRE_SPTR CUSTOM_SUBJECT_PTR
#define USPTR CUSTOM_SUBJECT_PTR
#else
#define PCRE_SPTR const char *
#define USPTR const unsigned char *
#endif
#include "pcre.h"
#include "ucp.h"
#ifdef VPCOMPAT
#define strlen(s)        _strlen(s)
#define strncmp(s1,s2,m) _strncmp(s1,s2,m)
#define memcmp(s,c,n)    _memcmp(s,c,n)
#define memcpy(d,s,n)    _memcpy(d,s,n)
#define memmove(d,s,n)   _memmove(d,s,n)
#define memset(s,c,n)    _memset(s,c,n)
#else
#ifndef HAVE_MEMMOVE
#undef  memmove
#ifdef HAVE_BCOPY
#define memmove(a, b, c) bcopy(b, a, c)
#else
static void *pcre_memmove(void *d, const void *s, size_t n) {
    size_t i;
    unsigned char *dest = (unsigned char*)d;
    const unsigned char *src = (const unsigned char*)s;
    if (dest > src) {
        dest += n;
        src += n;
        for (i = 0; i < n; ++i) *(--dest) = *(--src);
        return (void*)dest;
    } else {
        for (i = 0; i < n; ++i) *dest++ = *src++;
        return (void*)(dest - n);
    }
}
#define memmove(a, b, c) pcre_memmove(a, b, c)
#endif
#endif
#endif
#if LINK_SIZE != 2
#define PUT(a,n,d)  (a[n] = (d) >> 8), (a[(n)+1] = (d) & 255)
#define GET(a,n) (((a)[n] << 8) | (a)[(n)+1])
#define MAX_PATTERN_SIZE (1 << 16)
#elif LINK_SIZE == 3
#define PUT(a,n,d)  (a[n] = (d) >> 16), (a[(n)+1] = (d) >> 8), (a[(n)+2] = (d) & 255)
#define GET(a,n)  (((a)[n] << 16) | ((a)[(n)+1] << 8) | (a)[(n)+2])
#define MAX_PATTERN_SIZE (1 << 24)
#elif LINK_SIZE == 4
#define PUT(a,n,d)  (a[n] = (d) >> 24), (a[(n)+1] = (d) >> 16), (a[(n)+2] = (d) >> 8), (a[(n)+3] = (d) & 255)
#define GET(a,n)  (((a)[n] << 24) | ((a)[(n)+1] << 16) | ((a)[(n)+2] << 8) | (a)[(n)+3])
#define MAX_PATTERN_SIZE (1 << 30)
#else
#error LINK_SIZE must be either 2, 3, or 4
#endif
#define PUTINC(a,n,d)   PUT(a,n,d), a += LINK_SIZE
#define PUT2(a,n,d)   \
  a[n] = (d) >> 8;    \
  a[(n)+1] = (d) & 255
#define GET2(a,n)  (((a)[n] << 8) | (a)[(n)+1])
#define PUT2INC(a,n,d)  PUT2(a,n,d), a += 2
#ifndef SUPPORT_UTF8
#define GETCHAR(c, eptr) c = *eptr;
#define GETCHARTEST(c, eptr) c = *eptr;
#define GETCHARINC(c, eptr) c = *eptr++;
#define GETCHARINCTEST(c, eptr) c = *eptr++;
#define GETCHARLEN(c, eptr, len) c = *eptr;
#else
#define GETUTF8(c, eptr)                                                                                                                                           \
    {                                                                                                                                                              \
        if ((c & 0x20) == 0) c = ((c & 0x1f) << 6) | (eptr[1] & 0x3f);                                                                                             \
        else if ((c & 0x10) == 0) c = ((c & 0x0f) << 12) | ((eptr[1] & 0x3f) << 6) | (eptr[2] & 0x3f);                                                             \
        else if ((c & 0x08) == 0) c = ((c & 0x07) << 18) | ((eptr[1] & 0x3f) << 12) | ((eptr[2] & 0x3f) << 6) | (eptr[3] & 0x3f);                                  \
        else if ((c & 0x04) == 0) c = ((c & 0x03) << 24) | ((eptr[1] & 0x3f) << 18) | ((eptr[2] & 0x3f) << 12) | ((eptr[3] & 0x3f) << 6) | (eptr[4] & 0x3f);       \
        else c = ((c & 0x01) << 30) | ((eptr[1] & 0x3f) << 24) | ((eptr[2] & 0x3f) << 18) | ((eptr[3] & 0x3f) << 12) | ((eptr[4] & 0x3f) << 6) | (eptr[5] & 0x3f); \
    }
#define GETCHAR(c, eptr)           \
  c = *eptr;                       \
  if (c >= 0xc0) GETUTF8(c, eptr);
#define GETCHARTEST(c, eptr)               \
  c = *eptr;                               \
  if (utf8 && c >= 0xc0) GETUTF8(c, eptr);
#define GETUTF8INC(c, eptr)                                                                                                                                     \
    {                                                                                                                                                           \
        if ((c & 0x20) == 0) c = ((c & 0x1f) << 6) | (*eptr++ & 0x3f);                                                                                          \
        else if ((c & 0x10) == 0) {                                                                                                                             \
            c = ((c & 0x0f) << 12) | ((*eptr & 0x3f) << 6) | (eptr[1] & 0x3f);                                                                                  \
            eptr += 2;                                                                                                                                          \
        } else if ((c & 0x08) == 0) {                                                                                                                           \
            c = ((c & 0x07) << 18) | ((*eptr & 0x3f) << 12) | ((eptr[1] & 0x3f) << 6) | (eptr[2] & 0x3f);                                                       \
            eptr += 3;                                                                                                                                          \
        } else if ((c & 0x04) == 0) {                                                                                                                           \
            c = ((c & 0x03) << 24) | ((*eptr & 0x3f) << 18) | ((eptr[1] & 0x3f) << 12) | ((eptr[2] & 0x3f) << 6) | (eptr[3] & 0x3f);                            \
            eptr += 4;                                                                                                                                          \
        } else {                                                                                                                                                \
            c = ((c & 0x01) << 30) | ((*eptr & 0x3f) << 24) | ((eptr[1] & 0x3f) << 18) | ((eptr[2] & 0x3f) << 12) | ((eptr[3] & 0x3f) << 6) | (eptr[4] & 0x3f); \
            eptr += 5;                                                                                                                                          \
        }                                                                                                                                                       \
    }
#define GETCHARINC(c, eptr)           \
  c = *eptr++;                        \
  if (c >= 0xc0) GETUTF8INC(c, eptr);
#define GETCHARINCTEST(c, eptr)               \
  c = *eptr++;                                \
  if (utf8 && c >= 0xc0) GETUTF8INC(c, eptr);
#define GETUTF8LEN(c, eptr, len)                                                                                                                                  \
    {                                                                                                                                                             \
        if ((c & 0x20) == 0) {                                                                                                                                    \
            c = ((c & 0x1f) << 6) | (eptr[1] & 0x3f);                                                                                                             \
            len++;                                                                                                                                                \
        } else if ((c & 0x10)  == 0) {                                                                                                                            \
            c = ((c & 0x0f) << 12) | ((eptr[1] & 0x3f) << 6) | (eptr[2] & 0x3f);                                                                                  \
            len += 2;                                                                                                                                             \
        } else if ((c & 0x08)  == 0) {                                                                                                                            \
            c = ((c & 0x07) << 18) | ((eptr[1] & 0x3f) << 12) | ((eptr[2] & 0x3f) << 6) | (eptr[3] & 0x3f);                                                       \
            len += 3;                                                                                                                                             \
        } else if ((c & 0x04)  == 0) {                                                                                                                            \
            c = ((c & 0x03) << 24) | ((eptr[1] & 0x3f) << 18) | ((eptr[2] & 0x3f) << 12) | ((eptr[3] & 0x3f) << 6) | (eptr[4] & 0x3f);                            \
            len += 4;                                                                                                                                             \
        } else {                                                                                                                                                  \
            c = ((c & 0x01) << 30) | ((eptr[1] & 0x3f) << 24) | ((eptr[2] & 0x3f) << 18) | ((eptr[3] & 0x3f) << 12) | ((eptr[4] & 0x3f) << 6) | (eptr[5] & 0x3f); \
            len += 5;                                                                                                                                             \
        }                                                                                                                                                         \
    }
#define GETCHARLEN(c, eptr, len)          \
  c = *eptr;                              \
  if (c >= 0xc0) GETUTF8LEN(c, eptr, len);
#define GETCHARLENTEST(c, eptr, len)               \
  c = *eptr;                                       \
  if (utf8 && c >= 0xc0) GETUTF8LEN(c, eptr, len);
#define BACKCHAR(eptr) while((*eptr & 0xc0) == 0x80) eptr--
#endif
#ifndef offsetof
#define offsetof(p_type,field) ((size_t)&(((p_type *)0)->field))
#endif
#define PCRE_IMS (PCRE_CASELESS|PCRE_MULTILINE|PCRE_DOTALL)
#define PCRE_NOPARTIAL     0x0001
#define PCRE_FIRSTSET      0x0002
#define PCRE_REQCHSET      0x0004
#define PCRE_STARTLINE     0x0008
#define PCRE_JCHANGED      0x0010
#define PCRE_HASCRORLF     0x0020
#define PCRE_STUDY_MAPPED   0x01
#define PCRE_STUDY_MINLEN   0x02
#define PCRE_NEWLINE_BITS (PCRE_NEWLINE_CR|PCRE_NEWLINE_LF|PCRE_NEWLINE_ANY| PCRE_NEWLINE_ANYCRLF)
#define PUBLIC_COMPILE_OPTIONS  (PCRE_CASELESS|PCRE_EXTENDED|PCRE_ANCHORED|PCRE_MULTILINE| PCRE_DOTALL|PCRE_DOLLAR_ENDONLY|PCRE_EXTRA|PCRE_UNGREEDY|PCRE_UTF8| \
   PCRE_NO_AUTO_CAPTURE|PCRE_NO_UTF8_CHECK|PCRE_AUTO_CALLOUT|PCRE_FIRSTLINE| PCRE_DUPNAMES|PCRE_NEWLINE_BITS|PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE|                \
   PCRE_JAVASCRIPT_COMPAT|PCRE_UCP|PCRE_NO_START_OPTIMIZE)
#define PUBLIC_EXEC_OPTIONS  (PCRE_ANCHORED|PCRE_NOTBOL|PCRE_NOTEOL|PCRE_NOTEMPTY|PCRE_NOTEMPTY_ATSTART|PCRE_NO_UTF8_CHECK|PCRE_PARTIAL_HARD| \
   PCRE_PARTIAL_SOFT|PCRE_NEWLINE_BITS|PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE|PCRE_NO_START_OPTIMIZE)
#define PUBLIC_DFA_EXEC_OPTIONS  (PCRE_ANCHORED|PCRE_NOTBOL|PCRE_NOTEOL|PCRE_NOTEMPTY|PCRE_NOTEMPTY_ATSTART|PCRE_NO_UTF8_CHECK|PCRE_PARTIAL_HARD| \
   PCRE_PARTIAL_SOFT|PCRE_DFA_SHORTEST| PCRE_DFA_RESTART|PCRE_NEWLINE_BITS|PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE|PCRE_NO_START_OPTIMIZE)
#define PUBLIC_STUDY_OPTIONS 0
#define MAGIC_NUMBER  0x50435245UL
#define REQ_UNSET (-2)
#define REQ_NONE  (-1)
#define REQ_BYTE_MAX 1000
#define REQ_CASELESS 0x0100
#define REQ_VARY     0x0200
typedef gboolean BOOL;
#ifndef SUPPORT_UTF8
#define CHAR_HT '\t'
#define CHAR_VT '\v'
#define CHAR_FF '\f'
#define CHAR_CR '\r'
#define CHAR_NL '\n'
#define CHAR_BS '\b'
#define CHAR_BEL '\a'
#ifdef EBCDIC
#define CHAR_ESC '\047'
#define CHAR_DEL '\007'
#else
#define CHAR_ESC '\033'
#define CHAR_DEL '\177'
#endif
#define CHAR_SPACE ' '
#define CHAR_EXCLAMATION_MARK '!'
#define CHAR_QUOTATION_MARK '"'
#define CHAR_NUMBER_SIGN '#'
#define CHAR_DOLLAR_SIGN '$'
#define CHAR_PERCENT_SIGN '%'
#define CHAR_AMPERSAND '&'
#define CHAR_APOSTROPHE '\''
#define CHAR_LEFT_PARENTHESIS '('
#define CHAR_RIGHT_PARENTHESIS ')'
#define CHAR_ASTERISK '*'
#define CHAR_PLUS '+'
#define CHAR_COMMA ','
#define CHAR_MINUS '-'
#define CHAR_DOT '.'
#define CHAR_SLASH '/'
#define CHAR_0 '0'
#define CHAR_1 '1'
#define CHAR_2 '2'
#define CHAR_3 '3'
#define CHAR_4 '4'
#define CHAR_5 '5'
#define CHAR_6 '6'
#define CHAR_7 '7'
#define CHAR_8 '8'
#define CHAR_9 '9'
#define CHAR_COLON ':'
#define CHAR_SEMICOLON ';'
#define CHAR_LESS_THAN_SIGN '<'
#define CHAR_EQUALS_SIGN '='
#define CHAR_GREATER_THAN_SIGN '>'
#define CHAR_QUESTION_MARK '?'
#define CHAR_COMMERCIAL_AT '@'
#define CHAR_A 'A'
#define CHAR_B 'B'
#define CHAR_C 'C'
#define CHAR_D 'D'
#define CHAR_E 'E'
#define CHAR_F 'F'
#define CHAR_G 'G'
#define CHAR_H 'H'
#define CHAR_I 'I'
#define CHAR_J 'J'
#define CHAR_K 'K'
#define CHAR_L 'L'
#define CHAR_M 'M'
#define CHAR_N 'N'
#define CHAR_O 'O'
#define CHAR_P 'P'
#define CHAR_Q 'Q'
#define CHAR_R 'R'
#define CHAR_S 'S'
#define CHAR_T 'T'
#define CHAR_U 'U'
#define CHAR_V 'V'
#define CHAR_W 'W'
#define CHAR_X 'X'
#define CHAR_Y 'Y'
#define CHAR_Z 'Z'
#define CHAR_LEFT_SQUARE_BRACKET '['
#define CHAR_BACKSLASH '\\'
#define CHAR_RIGHT_SQUARE_BRACKET ']'
#define CHAR_CIRCUMFLEX_ACCENT '^'
#define CHAR_UNDERSCORE '_'
#define CHAR_GRAVE_ACCENT '`'
#define CHAR_a 'a'
#define CHAR_b 'b'
#define CHAR_c 'c'
#define CHAR_d 'd'
#define CHAR_e 'e'
#define CHAR_f 'f'
#define CHAR_g 'g'
#define CHAR_h 'h'
#define CHAR_i 'i'
#define CHAR_j 'j'
#define CHAR_k 'k'
#define CHAR_l 'l'
#define CHAR_m 'm'
#define CHAR_n 'n'
#define CHAR_o 'o'
#define CHAR_p 'p'
#define CHAR_q 'q'
#define CHAR_r 'r'
#define CHAR_s 's'
#define CHAR_t 't'
#define CHAR_u 'u'
#define CHAR_v 'v'
#define CHAR_w 'w'
#define CHAR_x 'x'
#define CHAR_y 'y'
#define CHAR_z 'z'
#define CHAR_LEFT_CURLY_BRACKET '{'
#define CHAR_VERTICAL_LINE '|'
#define CHAR_RIGHT_CURLY_BRACKET '}'
#define CHAR_TILDE '~'
#define STR_HT "\t"
#define STR_VT "\v"
#define STR_FF "\f"
#define STR_CR "\r"
#define STR_NL "\n"
#define STR_BS "\b"
#define STR_BEL "\a"
#ifdef EBCDIC
#define STR_ESC "\047"
#define STR_DEL "\007"
#else
#define STR_ESC "\033"
#define STR_DEL "\177"
#endif
#define STR_SPACE " "
#define STR_EXCLAMATION_MARK "!"
#define STR_QUOTATION_MARK "\""
#define STR_NUMBER_SIGN "#"
#define STR_DOLLAR_SIGN "$"
#define STR_PERCENT_SIGN "%"
#define STR_AMPERSAND "&"
#define STR_APOSTROPHE "'"
#define STR_LEFT_PARENTHESIS "("
#define STR_RIGHT_PARENTHESIS ")"
#define STR_ASTERISK "*"
#define STR_PLUS "+"
#define STR_COMMA ","
#define STR_MINUS "-"
#define STR_DOT "."
#define STR_SLASH "/"
#define STR_0 "0"
#define STR_1 "1"
#define STR_2 "2"
#define STR_3 "3"
#define STR_4 "4"
#define STR_5 "5"
#define STR_6 "6"
#define STR_7 "7"
#define STR_8 "8"
#define STR_9 "9"
#define STR_COLON ":"
#define STR_SEMICOLON ";"
#define STR_LESS_THAN_SIGN "<"
#define STR_EQUALS_SIGN "="
#define STR_GREATER_THAN_SIGN ">"
#define STR_QUESTION_MARK "?"
#define STR_COMMERCIAL_AT "@"
#define STR_A "A"
#define STR_B "B"
#define STR_C "C"
#define STR_D "D"
#define STR_E "E"
#define STR_F "F"
#define STR_G "G"
#define STR_H "H"
#define STR_I "I"
#define STR_J "J"
#define STR_K "K"
#define STR_L "L"
#define STR_M "M"
#define STR_N "N"
#define STR_O "O"
#define STR_P "P"
#define STR_Q "Q"
#define STR_R "R"
#define STR_S "S"
#define STR_T "T"
#define STR_U "U"
#define STR_V "V"
#define STR_W "W"
#define STR_X "X"
#define STR_Y "Y"
#define STR_Z "Z"
#define STR_LEFT_SQUARE_BRACKET  "["
#define STR_BACKSLASH "\\"
#define STR_RIGHT_SQUARE_BRACKET "]"
#define STR_CIRCUMFLEX_ACCENT "^"
#define STR_UNDERSCORE "_"
#define STR_GRAVE_ACCENT "`"
#define STR_a "a"
#define STR_b "b"
#define STR_c "c"
#define STR_d "d"
#define STR_e "e"
#define STR_f "f"
#define STR_g "g"
#define STR_h "h"
#define STR_i "i"
#define STR_j "j"
#define STR_k "k"
#define STR_l "l"
#define STR_m "m"
#define STR_n "n"
#define STR_o "o"
#define STR_p "p"
#define STR_q "q"
#define STR_r "r"
#define STR_s "s"
#define STR_t "t"
#define STR_u "u"
#define STR_v "v"
#define STR_w "w"
#define STR_x "x"
#define STR_y "y"
#define STR_z "z"
#define STR_LEFT_CURLY_BRACKET "{"
#define STR_VERTICAL_LINE "|"
#define STR_RIGHT_CURLY_BRACKET "}"
#define STR_TILDE "~"
#define STRING_ACCEPT0 "ACCEPT\0"
#define STRING_COMMIT0 "COMMIT\0"
#define STRING_F0 "F\0"
#define STRING_FAIL0 "FAIL\0"
#define STRING_MARK0 "MARK\0"
#define STRING_PRUNE0 "PRUNE\0"
#define STRING_SKIP0 "SKIP\0"
#define STRING_THEN "THEN"
#define STRING_alpha0 "alpha\0"
#define STRING_lower0 "lower\0"
#define STRING_upper0 "upper\0"
#define STRING_alnum0 "alnum\0"
#define STRING_ascii0 "ascii\0"
#define STRING_blank0 "blank\0"
#define STRING_cntrl0 "cntrl\0"
#define STRING_digit0 "digit\0"
#define STRING_graph0 "graph\0"
#define STRING_print0 "print\0"
#define STRING_punct0 "punct\0"
#define STRING_space0 "space\0"
#define STRING_word0 "word\0"
#define STRING_xdigit "xdigit"
#define STRING_DEFINE "DEFINE"
#define STRING_CR_RIGHTPAR "CR)"
#define STRING_LF_RIGHTPAR "LF)"
#define STRING_CRLF_RIGHTPAR "CRLF)"
#define STRING_ANY_RIGHTPAR "ANY)"
#define STRING_ANYCRLF_RIGHTPAR "ANYCRLF)"
#define STRING_BSR_ANYCRLF_RIGHTPAR "BSR_ANYCRLF)"
#define STRING_BSR_UNICODE_RIGHTPAR "BSR_UNICODE)"
#define STRING_UTF8_RIGHTPAR "UTF8)"
#define STRING_UCP_RIGHTPAR "UCP)"
#define STRING_NO_START_OPT_RIGHTPAR "NO_START_OPT)"
#else
#define CHAR_HT '\011'
#define CHAR_VT '\013'
#define CHAR_FF '\014'
#define CHAR_CR '\015'
#define CHAR_NL '\012'
#define CHAR_BS '\010'
#define CHAR_BEL '\007'
#define CHAR_ESC '\033'
#define CHAR_DEL '\177'
#define CHAR_SPACE '\040'
#define CHAR_EXCLAMATION_MARK '\041'
#define CHAR_QUOTATION_MARK '\042'
#define CHAR_NUMBER_SIGN '\043'
#define CHAR_DOLLAR_SIGN '\044'
#define CHAR_PERCENT_SIGN '\045'
#define CHAR_AMPERSAND '\046'
#define CHAR_APOSTROPHE '\047'
#define CHAR_LEFT_PARENTHESIS '\050'
#define CHAR_RIGHT_PARENTHESIS '\051'
#define CHAR_ASTERISK '\052'
#define CHAR_PLUS '\053'
#define CHAR_COMMA '\054'
#define CHAR_MINUS '\055'
#define CHAR_DOT '\056'
#define CHAR_SLASH '\057'
#define CHAR_0 '\060'
#define CHAR_1 '\061'
#define CHAR_2 '\062'
#define CHAR_3 '\063'
#define CHAR_4 '\064'
#define CHAR_5 '\065'
#define CHAR_6 '\066'
#define CHAR_7 '\067'
#define CHAR_8 '\070'
#define CHAR_9 '\071'
#define CHAR_COLON '\072'
#define CHAR_SEMICOLON '\073'
#define CHAR_LESS_THAN_SIGN '\074'
#define CHAR_EQUALS_SIGN '\075'
#define CHAR_GREATER_THAN_SIGN '\076'
#define CHAR_QUESTION_MARK '\077'
#define CHAR_COMMERCIAL_AT '\100'
#define CHAR_A '\101'
#define CHAR_B '\102'
#define CHAR_C '\103'
#define CHAR_D '\104'
#define CHAR_E '\105'
#define CHAR_F '\106'
#define CHAR_G '\107'
#define CHAR_H '\110'
#define CHAR_I '\111'
#define CHAR_J '\112'
#define CHAR_K '\113'
#define CHAR_L '\114'
#define CHAR_M '\115'
#define CHAR_N '\116'
#define CHAR_O '\117'
#define CHAR_P '\120'
#define CHAR_Q '\121'
#define CHAR_R '\122'
#define CHAR_S '\123'
#define CHAR_T '\124'
#define CHAR_U '\125'
#define CHAR_V '\126'
#define CHAR_W '\127'
#define CHAR_X '\130'
#define CHAR_Y '\131'
#define CHAR_Z '\132'
#define CHAR_LEFT_SQUARE_BRACKET '\133'
#define CHAR_BACKSLASH '\134'
#define CHAR_RIGHT_SQUARE_BRACKET '\135'
#define CHAR_CIRCUMFLEX_ACCENT '\136'
#define CHAR_UNDERSCORE '\137'
#define CHAR_GRAVE_ACCENT '\140'
#define CHAR_a '\141'
#define CHAR_b '\142'
#define CHAR_c '\143'
#define CHAR_d '\144'
#define CHAR_e '\145'
#define CHAR_f '\146'
#define CHAR_g '\147'
#define CHAR_h '\150'
#define CHAR_i '\151'
#define CHAR_j '\152'
#define CHAR_k '\153'
#define CHAR_l '\154'
#define CHAR_m '\155'
#define CHAR_n '\156'
#define CHAR_o '\157'
#define CHAR_p '\160'
#define CHAR_q '\161'
#define CHAR_r '\162'
#define CHAR_s '\163'
#define CHAR_t '\164'
#define CHAR_u '\165'
#define CHAR_v '\166'
#define CHAR_w '\167'
#define CHAR_x '\170'
#define CHAR_y '\171'
#define CHAR_z '\172'
#define CHAR_LEFT_CURLY_BRACKET '\173'
#define CHAR_VERTICAL_LINE '\174'
#define CHAR_RIGHT_CURLY_BRACKET '\175'
#define CHAR_TILDE '\176'
#define STR_HT "\011"
#define STR_VT "\013"
#define STR_FF "\014"
#define STR_CR "\015"
#define STR_NL "\012"
#define STR_BS "\010"
#define STR_BEL "\007"
#define STR_ESC "\033"
#define STR_DEL "\177"
#define STR_SPACE "\040"
#define STR_EXCLAMATION_MARK "\041"
#define STR_QUOTATION_MARK "\042"
#define STR_NUMBER_SIGN "\043"
#define STR_DOLLAR_SIGN "\044"
#define STR_PERCENT_SIGN "\045"
#define STR_AMPERSAND "\046"
#define STR_APOSTROPHE "\047"
#define STR_LEFT_PARENTHESIS "\050"
#define STR_RIGHT_PARENTHESIS "\051"
#define STR_ASTERISK "\052"
#define STR_PLUS "\053"
#define STR_COMMA "\054"
#define STR_MINUS "\055"
#define STR_DOT "\056"
#define STR_SLASH "\057"
#define STR_0 "\060"
#define STR_1 "\061"
#define STR_2 "\062"
#define STR_3 "\063"
#define STR_4 "\064"
#define STR_5 "\065"
#define STR_6 "\066"
#define STR_7  "\067"
#define STR_8 "\070"
#define STR_9 "\071"
#define STR_COLON "\072"
#define STR_SEMICOLON "\073"
#define STR_LESS_THAN_SIGN "\074"
#define STR_EQUALS_SIGN "\075"
#define STR_GREATER_THAN_SIGN "\076"
#define STR_QUESTION_MARK "\077"
#define STR_COMMERCIAL_AT "\100"
#define STR_A "\101"
#define STR_B "\102"
#define STR_C "\103"
#define STR_D "\104"
#define STR_E "\105"
#define STR_F "\106"
#define STR_G "\107"
#define STR_H "\110"
#define STR_I "\111"
#define STR_J "\112"
#define STR_K "\113"
#define STR_L "\114"
#define STR_M "\115"
#define STR_N "\116"
#define STR_O "\117"
#define STR_P "\120"
#define STR_Q "\121"
#define STR_R "\122"
#define STR_S "\123"
#define STR_T "\124"
#define STR_U "\125"
#define STR_V "\126"
#define STR_W "\127"
#define STR_X "\130"
#define STR_Y "\131"
#define STR_Z "\132"
#define STR_LEFT_SQUARE_BRACKET "\133"
#define STR_BACKSLASH "\134"
#define STR_RIGHT_SQUARE_BRACKET "\135"
#define STR_CIRCUMFLEX_ACCENT "\136"
#define STR_UNDERSCORE "\137"
#define STR_GRAVE_ACCENT "\140"
#define STR_a "\141"
#define STR_b "\142"
#define STR_c "\143"
#define STR_d "\144"
#define STR_e "\145"
#define STR_f "\146"
#define STR_g "\147"
#define STR_h "\150"
#define STR_i "\151"
#define STR_j "\152"
#define STR_k "\153"
#define STR_l "\154"
#define STR_m "\155"
#define STR_n "\156"
#define STR_o "\157"
#define STR_p "\160"
#define STR_q "\161"
#define STR_r "\162"
#define STR_s "\163"
#define STR_t "\164"
#define STR_u "\165"
#define STR_v "\166"
#define STR_w "\167"
#define STR_x "\170"
#define STR_y "\171"
#define STR_z "\172"
#define STR_LEFT_CURLY_BRACKET "\173"
#define STR_VERTICAL_LINE "\174"
#define STR_RIGHT_CURLY_BRACKET "\175"
#define STR_TILDE "\176"
#define STRING_ACCEPT0 STR_A STR_C STR_C STR_E STR_P STR_T "\0"
#define STRING_COMMIT0 STR_C STR_O STR_M STR_M STR_I STR_T "\0"
#define STRING_F0 STR_F "\0"
#define STRING_FAIL0 STR_F STR_A STR_I STR_L "\0"
#define STRING_MARK0 STR_M STR_A STR_R STR_K "\0"
#define STRING_PRUNE0 STR_P STR_R STR_U STR_N STR_E "\0"
#define STRING_SKIP0 STR_S STR_K STR_I STR_P "\0"
#define STRING_THEN STR_T STR_H STR_E STR_N
#define STRING_alpha0 STR_a STR_l STR_p STR_h STR_a "\0"
#define STRING_lower0 STR_l STR_o STR_w STR_e STR_r "\0"
#define STRING_upper0 STR_u STR_p STR_p STR_e STR_r "\0"
#define STRING_alnum0 STR_a STR_l STR_n STR_u STR_m "\0"
#define STRING_ascii0 STR_a STR_s STR_c STR_i STR_i "\0"
#define STRING_blank0 STR_b STR_l STR_a STR_n STR_k "\0"
#define STRING_cntrl0 STR_c STR_n STR_t STR_r STR_l "\0"
#define STRING_digit0 STR_d STR_i STR_g STR_i STR_t "\0"
#define STRING_graph0 STR_g STR_r STR_a STR_p STR_h "\0"
#define STRING_print0 STR_p STR_r STR_i STR_n STR_t "\0"
#define STRING_punct0 STR_p STR_u STR_n STR_c STR_t "\0"
#define STRING_space0 STR_s STR_p STR_a STR_c STR_e "\0"
#define STRING_word0 STR_w STR_o STR_r STR_d       "\0"
#define STRING_xdigit STR_x STR_d STR_i STR_g STR_i STR_t
#define STRING_DEFINE STR_D STR_E STR_F STR_I STR_N STR_E
#define STRING_CR_RIGHTPAR STR_C STR_R STR_RIGHT_PARENTHESIS
#define STRING_LF_RIGHTPAR STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_CRLF_RIGHTPAR STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_ANY_RIGHTPAR STR_A STR_N STR_Y STR_RIGHT_PARENTHESIS
#define STRING_ANYCRLF_RIGHTPAR STR_A STR_N STR_Y STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_BSR_ANYCRLF_RIGHTPAR STR_B STR_S STR_R STR_UNDERSCORE STR_A STR_N STR_Y STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_BSR_UNICODE_RIGHTPAR STR_B STR_S STR_R STR_UNDERSCORE STR_U STR_N STR_I STR_C STR_O STR_D STR_E STR_RIGHT_PARENTHESIS
#define STRING_UTF8_RIGHTPAR STR_U STR_T STR_F STR_8 STR_RIGHT_PARENTHESIS
#define STRING_UCP_RIGHTPAR STR_U STR_C STR_P STR_RIGHT_PARENTHESIS
#define STRING_NO_START_OPT_RIGHTPAR STR_N STR_O STR_UNDERSCORE STR_S STR_T STR_A STR_R STR_T STR_UNDERSCORE STR_O STR_P STR_T STR_RIGHT_PARENTHESIS
#endif
#ifndef ESC_e
#define ESC_e CHAR_ESC
#endif
#ifndef ESC_f
#define ESC_f CHAR_FF
#endif
#ifndef ESC_n
#define ESC_n CHAR_NL
#endif
#ifndef ESC_r
#define ESC_r CHAR_CR
#endif
#ifndef ESC_tee
#define ESC_tee CHAR_HT
#endif
#define PT_ANY        0    /* Any property - matches all chars */
#define PT_LAMP       1    /* L& - the union of Lu, Ll, Lt */
#define PT_GC         2    /* Specified general characteristic (e.g. L) */
#define PT_PC         3    /* Specified particular characteristic (e.g. Lu) */
#define PT_SC         4    /* Script (e.g. Han) */
#define PT_ALNUM      5    /* Alphanumeric - the union of L and N */
#define PT_SPACE      6    /* Perl space - Z plus 9,10,12,13 */
#define PT_PXSPACE    7    /* POSIX space - Z plus 9,10,11,12,13 */
#define PT_WORD       8    /* Word - L plus N plus underscore */
#define XCL_NOT    0x01    /* Flag: this is a negative class */
#define XCL_MAP    0x02    /* Flag: a 32-byte map is present */
#define XCL_END       0    /* Marks end of individual items */
#define XCL_SINGLE    1    /* Single item (one multibyte char) follows */
#define XCL_RANGE     2    /* A range (two multibyte chars) follows */
#define XCL_PROP      3    /* Unicode property (2-byte property code follows) */
#define XCL_NOTPROP   4    /* Unicode inverted property (ditto) */
enum { ESC_A = 1, ESC_G, ESC_K, ESC_B, ESC_b, ESC_D, ESC_d, ESC_S, ESC_s,
       ESC_W, ESC_w, ESC_N, ESC_dum, ESC_C, ESC_P, ESC_p, ESC_R, ESC_H,
       ESC_h, ESC_V, ESC_v, ESC_X, ESC_Z, ESC_z,
       ESC_E, ESC_Q, ESC_g, ESC_k,
       ESC_DU, ESC_du, ESC_SU, ESC_su, ESC_WU, ESC_wu,
       ESC_REF };
enum {
  OP_END,            /* 0 End of pattern */
  OP_SOD,            /* 1 Start of data: \A */
  OP_SOM,            /* 2 Start of match (subject + offset): \G */
  OP_SET_SOM,        /* 3 Set start of match (\K) */
  OP_NOT_WORD_BOUNDARY,  /*  4 \B */
  OP_WORD_BOUNDARY,      /*  5 \b */
  OP_NOT_DIGIT,          /*  6 \D */
  OP_DIGIT,              /*  7 \d */
  OP_NOT_WHITESPACE,     /*  8 \S */
  OP_WHITESPACE,         /*  9 \s */
  OP_NOT_WORDCHAR,       /* 10 \W */
  OP_WORDCHAR,           /* 11 \w */
  OP_ANY,            /* 12 Match any character except newline */
  OP_ALLANY,         /* 13 Match any character */
  OP_ANYBYTE,        /* 14 Match any byte (\C); different to OP_ANY for UTF-8 */
  OP_NOTPROP,        /* 15 \P (not Unicode property) */
  OP_PROP,           /* 16 \p (Unicode property) */
  OP_ANYNL,          /* 17 \R (any newline sequence) */
  OP_NOT_HSPACE,     /* 18 \H (not horizontal whitespace) */
  OP_HSPACE,         /* 19 \h (horizontal whitespace) */
  OP_NOT_VSPACE,     /* 20 \V (not vertical whitespace) */
  OP_VSPACE,         /* 21 \v (vertical whitespace) */
  OP_EXTUNI,         /* 22 \X (extended Unicode sequence */
  OP_EODN,           /* 23 End of data or \n at end of data: \Z. */
  OP_EOD,            /* 24 End of data: \z */
  OP_OPT,            /* 25 Set runtime options */
  OP_CIRC,           /* 26 Start of line - varies with multiline switch */
  OP_DOLL,           /* 27 End of line - varies with multiline switch */
  OP_CHAR,           /* 28 Match one character, casefully */
  OP_CHARNC,         /* 29 Match one character, caselessly */
  OP_NOT,            /* 30 Match one character, not the following one */
  OP_STAR,           /* 31 The maximizing and minimizing versions of */
  OP_MINSTAR,        /* 32 these six opcodes must come in pairs, with */
  OP_PLUS,           /* 33 the minimizing one second. */
  OP_MINPLUS,        /* 34 This first set applies to single characters.*/
  OP_QUERY,          /* 35 */
  OP_MINQUERY,       /* 36 */
  OP_UPTO,           /* 37 From 0 to n matches */
  OP_MINUPTO,        /* 38 */
  OP_EXACT,          /* 39 Exactly n matches */
  OP_POSSTAR,        /* 40 Possessified star */
  OP_POSPLUS,        /* 41 Possessified plus */
  OP_POSQUERY,       /* 42 Posesssified query */
  OP_POSUPTO,        /* 43 Possessified upto */
  OP_NOTSTAR,        /* 44 The maximizing and minimizing versions of */
  OP_NOTMINSTAR,     /* 45 these six opcodes must come in pairs, with */
  OP_NOTPLUS,        /* 46 the minimizing one second. They must be in */
  OP_NOTMINPLUS,     /* 47 exactly the same order as those above. */
  OP_NOTQUERY,       /* 48 This set applies to "not" single characters. */
  OP_NOTMINQUERY,    /* 49 */
  OP_NOTUPTO,        /* 50 From 0 to n matches */
  OP_NOTMINUPTO,     /* 51 */
  OP_NOTEXACT,       /* 52 Exactly n matches */
  OP_NOTPOSSTAR,     /* 53 Possessified versions */
  OP_NOTPOSPLUS,     /* 54 */
  OP_NOTPOSQUERY,    /* 55 */
  OP_NOTPOSUPTO,     /* 56 */
  OP_TYPESTAR,       /* 57 The maximizing and minimizing versions of */
  OP_TYPEMINSTAR,    /* 58 these six opcodes must come in pairs, with */
  OP_TYPEPLUS,       /* 59 the minimizing one second. These codes must */
  OP_TYPEMINPLUS,    /* 60 be in exactly the same order as those above. */
  OP_TYPEQUERY,      /* 61 This set applies to character types such as \d */
  OP_TYPEMINQUERY,   /* 62 */
  OP_TYPEUPTO,       /* 63 From 0 to n matches */
  OP_TYPEMINUPTO,    /* 64 */
  OP_TYPEEXACT,      /* 65 Exactly n matches */
  OP_TYPEPOSSTAR,    /* 66 Possessified versions */
  OP_TYPEPOSPLUS,    /* 67 */
  OP_TYPEPOSQUERY,   /* 68 */
  OP_TYPEPOSUPTO,    /* 69 */
  OP_CRSTAR,         /* 70 The maximizing and minimizing versions of */
  OP_CRMINSTAR,      /* 71 all these opcodes must come in pairs, with */
  OP_CRPLUS,         /* 72 the minimizing one second. These codes must */
  OP_CRMINPLUS,      /* 73 be in exactly the same order as those above. */
  OP_CRQUERY,        /* 74 These are for character classes and back refs */
  OP_CRMINQUERY,     /* 75 */
  OP_CRRANGE,        /* 76 These are different to the three sets above. */
  OP_CRMINRANGE,     /* 77 */
  OP_CLASS,          /* 78 Match a character class, chars < 256 only */
  OP_NCLASS,
  OP_XCLASS,         /* 80 Extended class for handling UTF-8 chars within the
                           class. This does both positive and negative. */
  OP_REF,            /* 81 Match a back reference */
  OP_RECURSE,        /* 82 Match a numbered subpattern (possibly recursive) */
  OP_CALLOUT,        /* 83 Call out to external function if provided */
  OP_ALT,            /* 84 Start of alternation */
  OP_KET,            /* 85 End of group that doesn't have an unbounded repeat */
  OP_KETRMAX,        /* 86 These two must remain together and in this */
  OP_KETRMIN,        /* 87 order. They are for groups the repeat for ever. */
  OP_ASSERT,         /* 88 Positive lookahead */
  OP_ASSERT_NOT,     /* 89 Negative lookahead */
  OP_ASSERTBACK,     /* 90 Positive lookbehind */
  OP_ASSERTBACK_NOT, /* 91 Negative lookbehind */
  OP_REVERSE,        /* 92 Move pointer back - used in lookbehind assertions */
  OP_ONCE,           /* 93 Atomic group */
  OP_BRA,            /* 94 Start of non-capturing bracket */
  OP_CBRA,           /* 95 Start of capturing bracket */
  OP_COND,           /* 96 Conditional group */
  OP_SBRA,           /* 97 Start of non-capturing bracket, check empty  */
  OP_SCBRA,          /* 98 Start of capturing bracket, check empty */
  OP_SCOND,          /* 99 Conditional group, check empty */
  OP_CREF,           /* 100 Used to hold a capture number as condition */
  OP_NCREF,          /* 101 Same, but generaged by a name reference*/
  OP_RREF,           /* 102 Used to hold a recursion number as condition */
  OP_NRREF,          /* 103 Same, but generaged by a name reference*/
  OP_DEF,            /* 104 The DEFINE condition */
  OP_BRAZERO,        /* 105 These two must remain together and in this */
  OP_BRAMINZERO,     /* 106 order. */
  OP_MARK,           /* 107 always has an argument */
  OP_PRUNE,          /* 108 */
  OP_PRUNE_ARG,      /* 109 same, but with argument */
  OP_SKIP,           /* 110 */
  OP_SKIP_ARG,       /* 111 same, but with argument */
  OP_THEN,           /* 112 */
  OP_THEN_ARG,       /* 113 same, but with argument */
  OP_COMMIT,         /* 114 */
  OP_FAIL,           /* 115 */
  OP_ACCEPT,         /* 116 */
  OP_CLOSE,          /* 117 Used before OP_ACCEPT to close open captures */
  OP_SKIPZERO,       /* 118 */
  OP_TABLE_LENGTH
};
#define OP_NAME_LIST \
  "End", "\\A", "\\G", "\\K", "\\B", "\\b", "\\D", "\\d",         \
  "\\S", "\\s", "\\W", "\\w", "Any", "AllAny", "Anybyte",         \
  "notprop", "prop", "\\R", "\\H", "\\h", "\\V", "\\v",           \
  "extuni",  "\\Z", "\\z",                                        \
  "Opt", "^", "$", "char", "charnc", "not",                       \
  "*", "*?", "+", "+?", "?", "??", "{", "{", "{",                 \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??", "{", "{", "{",                 \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??", "{", "{", "{",                 \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??", "{", "{",                      \
  "class", "nclass", "xclass", "Ref", "Recurse", "Callout",       \
  "Alt", "Ket", "KetRmax", "KetRmin", "Assert", "Assert not",     \
  "AssertB", "AssertB not", "Reverse",                            \
  "Once", "Bra", "CBra", "Cond", "SBra", "SCBra", "SCond",        \
  "Cond ref", "Cond nref", "Cond rec", "Cond nrec", "Cond def",   \
  "Brazero", "Braminzero",                                        \
  "*MARK", "*PRUNE", "*PRUNE", "*SKIP", "*SKIP",                  \
  "*THEN", "*THEN", "*COMMIT", "*FAIL", "*ACCEPT",                \
  "Close", "Skip zero"
#define OP_LENGTHS \
  1,                             /* End                                    */ \
  1, 1, 1, 1, 1,                 /* \A, \G, \K, \B, \b                     */ \
  1, 1, 1, 1, 1, 1,              /* \D, \d, \S, \s, \W, \w                 */ \
  1, 1, 1,                       /* Any, AllAny, Anybyte                   */ \
  3, 3,                          /* \P, \p                                 */ \
  1, 1, 1, 1, 1,                 /* \R, \H, \h, \V, \v                     */ \
  1,                             /* \X                                     */ \
  1, 1, 2, 1, 1,                 /* \Z, \z, Opt, ^, $                      */ \
  2,                             /* Char  - the minimum length             */ \
  2,                             /* Charnc  - the minimum length           */ \
  2,                             /* not                                    */ \
  /* Positive single-char repeats                            ** These are  */ \
  2, 2, 2, 2, 2, 2,              /* *, *?, +, +?, ?, ??      ** minima in  */ \
  4, 4, 4,                       /* upto, minupto, exact     ** UTF-8 mode */ \
  2, 2, 2, 4,                    /* *+, ++, ?+, upto+                      */ \
  /* Negative single-char repeats - only for chars < 256                   */ \
  2, 2, 2, 2, 2, 2,              /* NOT *, *?, +, +?, ?, ??                */ \
  4, 4, 4,                       /* NOT upto, minupto, exact               */ \
  2, 2, 2, 4,                    /* Possessive *, +, ?, upto               */ \
  /* Positive type repeats                                                 */ \
  2, 2, 2, 2, 2, 2,              /* Type *, *?, +, +?, ?, ??               */ \
  4, 4, 4,                       /* Type upto, minupto, exact              */ \
  2, 2, 2, 4,                    /* Possessive *+, ++, ?+, upto+           */ \
  /* Character class & ref repeats                                         */ \
  1, 1, 1, 1, 1, 1,              /* *, *?, +, +?, ?, ??                    */ \
  5, 5,                          /* CRRANGE, CRMINRANGE                    */ \
 33,                             /* CLASS                                  */ \
 33,                             /* NCLASS                                 */ \
  0,                             /* XCLASS - variable length               */ \
  3,                             /* REF                                    */ \
  1+LINK_SIZE,                   /* RECURSE                                */ \
  2+2*LINK_SIZE,                 /* CALLOUT                                */ \
  1+LINK_SIZE,                   /* Alt                                    */ \
  1+LINK_SIZE,                   /* Ket                                    */ \
  1+LINK_SIZE,                   /* KetRmax                                */ \
  1+LINK_SIZE,                   /* KetRmin                                */ \
  1+LINK_SIZE,                   /* Assert                                 */ \
  1+LINK_SIZE,                   /* Assert not                             */ \
  1+LINK_SIZE,                   /* Assert behind                          */ \
  1+LINK_SIZE,                   /* Assert behind not                      */ \
  1+LINK_SIZE,                   /* Reverse                                */ \
  1+LINK_SIZE,                   /* ONCE                                   */ \
  1+LINK_SIZE,                   /* BRA                                    */ \
  3+LINK_SIZE,                   /* CBRA                                   */ \
  1+LINK_SIZE,                   /* COND                                   */ \
  1+LINK_SIZE,                   /* SBRA                                   */ \
  3+LINK_SIZE,                   /* SCBRA                                  */ \
  1+LINK_SIZE,                   /* SCOND                                  */ \
  3, 3,                          /* CREF, NCREF                            */ \
  3, 3,                          /* RREF, NRREF                            */ \
  1,                             /* DEF                                    */ \
  1, 1,                          /* BRAZERO, BRAMINZERO                    */ \
  3, 1, 3,                       /* MARK, PRUNE, PRUNE_ARG                 */ \
  1, 3,                          /* SKIP, SKIP_ARG                         */ \
  1+LINK_SIZE, 3+LINK_SIZE,      /* THEN, THEN_ARG                         */ \
  1, 1, 1, 3, 1                  /* COMMIT, FAIL, ACCEPT, CLOSE, SKIPZERO  */
#define RREF_ANY  0xffff
enum { ERR0,  ERR1,  ERR2,  ERR3,  ERR4,  ERR5,  ERR6,  ERR7,  ERR8,  ERR9,
       ERR10, ERR11, ERR12, ERR13, ERR14, ERR15, ERR16, ERR17, ERR18, ERR19,
       ERR20, ERR21, ERR22, ERR23, ERR24, ERR25, ERR26, ERR27, ERR28, ERR29,
       ERR30, ERR31, ERR32, ERR33, ERR34, ERR35, ERR36, ERR37, ERR38, ERR39,
       ERR40, ERR41, ERR42, ERR43, ERR44, ERR45, ERR46, ERR47, ERR48, ERR49,
       ERR50, ERR51, ERR52, ERR53, ERR54, ERR55, ERR56, ERR57, ERR58, ERR59,
       ERR60, ERR61, ERR62, ERR63, ERR64, ERR65, ERR66, ERR67, ERR68,
       ERRCOUNT };
typedef struct real_pcre {
  pcre_uint32 magic_number;
  pcre_uint32 size;               /* Total that was malloced */
  pcre_uint32 options;            /* Public options */
  pcre_uint16 flags;              /* Private flags */
  pcre_uint16 dummy1;             /* For future use */
  pcre_uint16 top_bracket;
  pcre_uint16 top_backref;
  pcre_uint16 first_byte;
  pcre_uint16 req_byte;
  pcre_uint16 name_table_offset;  /* Offset to name table that follows */
  pcre_uint16 name_entry_size;    /* Size of any name items */
  pcre_uint16 name_count;         /* Number of name items */
  pcre_uint16 ref_count;          /* Reference count */
  const unsigned char *tables;    /* Pointer to tables or NULL for std */
  const unsigned char *nullpad;   /* NULL padding */
} real_pcre;
typedef struct pcre_study_data {
  pcre_uint32 size;               /* Total that was malloced */
  pcre_uint32 flags;              /* Private flags */
  uschar start_bits[32];          /* Starting char bits */
  pcre_uint32 minlength;          /* Minimum subject length */
} pcre_study_data;
typedef struct open_capitem {
  struct open_capitem *next;    /* Chain link */
  pcre_uint16 number;           /* Capture number */
  pcre_uint16 flag;             /* Set TRUE if recursive back ref */
} open_capitem;
typedef struct compile_data {
  const uschar *lcc;            /* Points to lower casing table */
  const uschar *fcc;            /* Points to case-flipping table */
  const uschar *cbits;          /* Points to character type table */
  const uschar *ctypes;         /* Points to table of type maps */
  const uschar *start_workspace;/* The start of working space */
  const uschar *start_code;     /* The start of the compiled code */
  const uschar *start_pattern;  /* The start of the pattern */
  const uschar *end_pattern;    /* The end of the pattern */
  open_capitem *open_caps;      /* Chain of open capture items */
  uschar *hwm;                  /* High watermark of workspace */
  uschar *name_table;           /* The name/number table */
  int  names_found;             /* Number of entries so far */
  int  name_entry_size;         /* Size of each entry */
  int  bracount;                /* Count of capturing parens as we compile */
  int  final_bracount;          /* Saved value after first pass */
  int  top_backref;             /* Maximum back reference */
  unsigned int backref_map;     /* Bitmap of low back refs */
  int  external_options;        /* External (initial) options */
  int  external_flags;          /* External flag bits to be set */
  int  req_varyopt;             /* "After variable item" flag for reqbyte */
  BOOL had_accept;              /* (*ACCEPT) encountered */
  BOOL check_lookbehind;        /* Lookbehinds need later checking */
  int  nltype;                  /* Newline type */
  int  nllen;                   /* Newline string length */
  uschar nl[4];                 /* Newline string when fixed length */
} compile_data;
typedef struct branch_chain {
  struct branch_chain *outer;
  uschar *current_branch;
} branch_chain;
typedef struct recursion_info {
  struct recursion_info *prevrec; /* Previous recursion record (or NULL) */
  int group_num;                /* Number of group that was called */
  const uschar *after_call;     /* "Return value": points after the call in the expr */
  int *offset_save;             /* Pointer to start of saved offsets */
  int saved_max;                /* Number of saved offsets */
  int save_offset_top;          /* Current value of offset_top */
} recursion_info;
typedef struct eptrblock {
  struct eptrblock *epb_prev;
  USPTR epb_saved_eptr;
} eptrblock;
typedef struct match_data {
  unsigned long int match_call_count;      /* As it says */
  unsigned long int match_limit;           /* As it says */
  unsigned long int match_limit_recursion; /* As it says */
  int   *offset_vector;         /* Offset vector */
  int    offset_end;            /* One past the end */
  int    offset_max;            /* The maximum usable for return data */
  int    nltype;                /* Newline type */
  int    nllen;                 /* Newline string length */
  int    name_count;            /* Number of names in name table */
  int    name_entry_size;       /* Size of entry in names table */
  uschar *name_table;           /* Table of names */
  uschar nl[4];                 /* Newline string when fixed */
  const uschar *lcc;            /* Points to lower casing table */
  const uschar *ctypes;         /* Points to table of type maps */
  BOOL   offset_overflow;       /* Set if too many extractions */
  BOOL   notbol;                /* NOTBOL flag */
  BOOL   noteol;                /* NOTEOL flag */
  BOOL   utf8;                  /* UTF8 flag */
  BOOL   jscript_compat;        /* JAVASCRIPT_COMPAT flag */
  BOOL   use_ucp;               /* PCRE_UCP flag */
  BOOL   endonly;               /* Dollar not before final \n */
  BOOL   notempty;              /* Empty string match not wanted */
  BOOL   notempty_atstart;      /* Empty string match at start not wanted */
  BOOL   hitend;                /* Hit the end of the subject at some point */
  BOOL   bsr_anycrlf;           /* \R is just any CRLF, not full Unicode */
  const uschar *start_code;     /* For use when recursing */
  USPTR  start_subject;         /* Start of the subject string */
  USPTR  end_subject;           /* End of the subject string */
  USPTR  start_match_ptr;       /* Start of matched string */
  USPTR  end_match_ptr;         /* Subject position at end match */
  USPTR  start_used_ptr;        /* Earliest consulted character */
  int    partial;               /* PARTIAL options */
  int    end_offset_top;        /* Highwater mark at end of match */
  int    capture_last;          /* Most recent capture number */
  int    start_offset;          /* The start offset value */
  eptrblock *eptrchain;         /* Chain of eptrblocks for tail recursions */
  int    eptrn;                 /* Next free eptrblock */
  recursion_info *recursive;    /* Linked list of recursion data */
  void  *callout_data;          /* To pass back to callouts */
  const uschar *mark;           /* Mark pointer to pass back */
} match_data;
typedef struct dfa_match_data {
  const uschar *start_code;     /* Start of the compiled pattern */
  const uschar *start_subject;  /* Start of the subject string */
  const uschar *end_subject;    /* End of subject string */
  const uschar *start_used_ptr; /* Earliest consulted character */
  const uschar *tables;         /* Character tables */
  int   start_offset;           /* The start offset value */
  int   moptions;               /* Match options */
  int   poptions;               /* Pattern options */
  int    nltype;                /* Newline type */
  int    nllen;                 /* Newline string length */
  uschar nl[4];                 /* Newline string when fixed */
  void  *callout_data;          /* To pass back to callouts */
} dfa_match_data;
#define ctype_space   0x01
#define ctype_letter  0x02
#define ctype_digit   0x04
#define ctype_xdigit  0x08
#define ctype_word    0x10   /* alphanumeric or '_' */
#define ctype_meta    0x80   /* regexp meta char or zero (end pattern) */
#define cbit_space     0      /* [:space:] or \s */
#define cbit_xdigit   32      /* [:xdigit:] */
#define cbit_digit    64      /* [:digit:] or \d */
#define cbit_upper    96      /* [:upper:] */
#define cbit_lower   128      /* [:lower:] */
#define cbit_word    160      /* [:word:] or \w */
#define cbit_graph   192      /* [:graph:] */
#define cbit_print   224      /* [:print:] */
#define cbit_punct   256      /* [:punct:] */
#define cbit_cntrl   288      /* [:cntrl:] */
#define cbit_length  320      /* Length of the cbits table */
#define lcc_offset      0
#define fcc_offset    256
#define cbits_offset  512
#define ctypes_offset (cbits_offset + cbit_length)
#define tables_length (ctypes_offset + 256)
typedef struct {
  pcre_uint16 name_offset;
  pcre_uint16 type;
  pcre_uint16 value;
} ucp_type_table;
extern const int _pcre_utf8_table1[];
extern const int _pcre_utf8_table2[];
extern const int _pcre_utf8_table3[];
extern const uschar _pcre_utf8_table4[];
extern const int _pcre_utf8_table1_size;
extern const char _pcre_utt_names[];
extern const ucp_type_table _pcre_utt[];
extern const int _pcre_utt_size;
extern const uschar _pcre_default_tables[];
extern const uschar _pcre_OP_lengths[];
extern const uschar *_pcre_find_bracket(const uschar *, BOOL, int);
extern BOOL _pcre_is_newline(USPTR, int, USPTR, int *, BOOL);
extern int _pcre_ord2utf8(int, uschar *);
extern real_pcre *_pcre_try_flipped(const real_pcre *, real_pcre *, const pcre_study_data *, pcre_study_data *);
#define  _pcre_valid_utf8(USPTR, int) TRUE
extern BOOL _pcre_was_newline(USPTR, int, USPTR, int *, BOOL);
extern BOOL _pcre_xclass(int, const uschar *);
typedef struct {
  uschar script;
  uschar chartype;
  pcre_int32 other_case;
} ucd_record;
extern const ucd_record _pcre_ucd_records[];
extern const uschar _pcre_ucd_stage1[];
extern const pcre_uint16 _pcre_ucd_stage2[];
extern const int _pcre_ucp_gentype[];
extern unsigned int _pcre_ucp_othercase (const unsigned int);
#define UCD_CHARTYPE(ch)  g_unichar_type(ch)
#define UCD_SCRIPT(ch)  g_unichar_get_script(ch)
#define UCD_CATEGORY(ch)  _pcre_ucp_gentype[UCD_CHARTYPE(ch)]
#define UCD_OTHERCASE(ch) _pcre_ucp_othercase(ch)
#endif