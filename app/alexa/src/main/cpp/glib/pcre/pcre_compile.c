#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define NLBLOCK cd
#define PSSTART start_pattern
#define PSEND   end_pattern

#include "pcre_internal.h"
#ifdef PCRE_DEBUG
#include "pcre_printint.src"
#endif

#define SETBIT(a,b) a[b/8] |= (1 << (b%8))
#define OFLOW_MAX (INT_MAX - 20)
#define COMPILE_WORK_SIZE (4096)
#define WORK_SIZE_CHECK (COMPILE_WORK_SIZE - 100)
#ifndef EBCDIC
static const short int escapes[] = {
     0,                       0,
     0,                       0,
     0,                       0,
     0,                       0,
     0,                       0,
     CHAR_COLON,              CHAR_SEMICOLON,
     CHAR_LESS_THAN_SIGN,     CHAR_EQUALS_SIGN,
     CHAR_GREATER_THAN_SIGN,  CHAR_QUESTION_MARK,
     CHAR_COMMERCIAL_AT,      -ESC_A,
     -ESC_B,                  -ESC_C,
     -ESC_D,                  -ESC_E,
     0,                       -ESC_G,
     -ESC_H,                  0,
     0,                       -ESC_K,
     0,                       0,
     -ESC_N,                  0,
     -ESC_P,                  -ESC_Q,
     -ESC_R,                  -ESC_S,
     0,                       0,
     -ESC_V,                  -ESC_W,
     -ESC_X,                  0,
     -ESC_Z,                  CHAR_LEFT_SQUARE_BRACKET,
     CHAR_BACKSLASH,          CHAR_RIGHT_SQUARE_BRACKET,
     CHAR_CIRCUMFLEX_ACCENT,  CHAR_UNDERSCORE,
     CHAR_GRAVE_ACCENT,       7,
     -ESC_b,                  0,
     -ESC_d,                  ESC_e,
     ESC_f,                   0,
     -ESC_h,                  0,
     0,                       -ESC_k,
     0,                       0,
     ESC_n,                   0,
     -ESC_p,                  0,
     ESC_r,                   -ESC_s,
     ESC_tee,                 0,
     -ESC_v,                  -ESC_w,
     0,                       0,
     -ESC_z
};
#else
static const short int escapes[] = {
/*  48 */     0,     0,      0,     '.',    '<',   '(',    '+',    '|',
/*  50 */   '&',     0,      0,       0,      0,     0,      0,      0,
/*  58 */     0,     0,    '!',     '$',    '*',   ')',    ';',    '~',
/*  60 */   '-',   '/',      0,       0,      0,     0,      0,      0,
/*  68 */     0,     0,    '|',     ',',    '%',   '_',    '>',    '?',
/*  70 */     0,     0,      0,       0,      0,     0,      0,      0,
/*  78 */     0,   '`',    ':',     '#',    '@',  '\'',    '=',    '"',
/*  80 */     0,     7, -ESC_b,       0, -ESC_d, ESC_e,  ESC_f,      0,
/*  88 */-ESC_h,     0,      0,     '{',      0,     0,      0,      0,
/*  90 */     0,     0, -ESC_k,     'l',      0, ESC_n,      0, -ESC_p,
/*  98 */     0, ESC_r,      0,     '}',      0,     0,      0,      0,
/*  A0 */     0,   '~', -ESC_s, ESC_tee,      0,-ESC_v, -ESC_w,      0,
/*  A8 */     0,-ESC_z,      0,       0,      0,   '[',      0,      0,
/*  B0 */     0,     0,      0,       0,      0,     0,      0,      0,
/*  B8 */     0,     0,      0,       0,      0,   ']',    '=',    '-',
/*  C0 */   '{',-ESC_A, -ESC_B,  -ESC_C, -ESC_D,-ESC_E,      0, -ESC_G,
/*  C8 */-ESC_H,     0,      0,       0,      0,     0,      0,      0,
/*  D0 */   '}',     0, -ESC_K,       0,      0,-ESC_N,      0, -ESC_P,
/*  D8 */-ESC_Q,-ESC_R,      0,       0,      0,     0,      0,      0,
/*  E0 */  '\\',     0, -ESC_S,       0,      0,-ESC_V, -ESC_W, -ESC_X,
/*  E8 */     0,-ESC_Z,      0,       0,      0,     0,      0,      0,
/*  F0 */     0,     0,      0,       0,      0,     0,      0,      0,
/*  F8 */     0,     0,      0,       0,      0,     0,      0,      0
};
#endif
typedef struct verbitem {
  int   len;
  int   op;
  int   op_arg;
} verbitem;
static const char verbnames[] =
  "\0"
  STRING_MARK0
  STRING_ACCEPT0
  STRING_COMMIT0
  STRING_F0
  STRING_FAIL0
  STRING_PRUNE0
  STRING_SKIP0
  STRING_THEN;
static const verbitem verbs[] = {
  { 0, -1,        OP_MARK },
  { 4, -1,        OP_MARK },
  { 6, OP_ACCEPT, -1 },
  { 6, OP_COMMIT, -1 },
  { 1, OP_FAIL,   -1 },
  { 4, OP_FAIL,   -1 },
  { 5, OP_PRUNE,  OP_PRUNE_ARG },
  { 4, OP_SKIP,   OP_SKIP_ARG  },
  { 4, OP_THEN,   OP_THEN_ARG  }
};
static const int verbcount = sizeof(verbs)/sizeof(verbitem);
static const char posix_names[] = STRING_alpha0 STRING_lower0 STRING_upper0 STRING_alnum0 STRING_ascii0 STRING_blank0 STRING_cntrl0 STRING_digit0
                                  STRING_graph0 STRING_print0 STRING_punct0 STRING_space0 STRING_word0  STRING_xdigit;
static const uschar posix_name_lengths[] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 6, 0 };
static const int posix_class_maps[] = {
  cbit_word,  cbit_digit, -2,             /* alpha */
  cbit_lower, -1,          0,             /* lower */
  cbit_upper, -1,          0,             /* upper */
  cbit_word,  -1,          2,             /* alnum - word without underscore */
  cbit_print, cbit_cntrl,  0,             /* ascii */
  cbit_space, -1,          1,             /* blank - a GNU extension */
  cbit_cntrl, -1,          0,             /* cntrl */
  cbit_digit, -1,          0,             /* digit */
  cbit_graph, -1,          0,             /* graph */
  cbit_print, -1,          0,             /* print */
  cbit_punct, -1,          0,             /* punct */
  cbit_space, -1,          0,             /* space */
  cbit_word,  -1,          0,             /* word - a Perl extension */
  cbit_xdigit,-1,          0              /* xdigit */
};
#ifdef SUPPORT_UCP
static const uschar *substitutes[] = {
  (uschar *)"\\P{Nd}",    /* \D */
  (uschar *)"\\p{Nd}",    /* \d */
  (uschar *)"\\P{Xsp}",   /* \S */       /* NOTE: Xsp is Perl space */
  (uschar *)"\\p{Xsp}",   /* \s */
  (uschar *)"\\P{Xwd}",   /* \W */
  (uschar *)"\\p{Xwd}"    /* \w */
};
static const uschar *posix_substitutes[] = {
  (uschar *)"\\p{L}",     /* alpha */
  (uschar *)"\\p{Ll}",    /* lower */
  (uschar *)"\\p{Lu}",    /* upper */
  (uschar *)"\\p{Xan}",   /* alnum */
  NULL,                   /* ascii */
  (uschar *)"\\h",        /* blank */
  NULL,                   /* cntrl */
  (uschar *)"\\p{Nd}",    /* digit */
  NULL,                   /* graph */
  NULL,                   /* print */
  NULL,                   /* punct */
  (uschar *)"\\p{Xps}",   /* space */    /* NOTE: Xps is POSIX space */
  (uschar *)"\\p{Xwd}",   /* word */
  NULL,                   /* xdigit */
  /* Negated cases */
  (uschar *)"\\P{L}",     /* ^alpha */
  (uschar *)"\\P{Ll}",    /* ^lower */
  (uschar *)"\\P{Lu}",    /* ^upper */
  (uschar *)"\\P{Xan}",   /* ^alnum */
  NULL,                   /* ^ascii */
  (uschar *)"\\H",        /* ^blank */
  NULL,                   /* ^cntrl */
  (uschar *)"\\P{Nd}",    /* ^digit */
  NULL,                   /* ^graph */
  NULL,                   /* ^print */
  NULL,                   /* ^punct */
  (uschar *)"\\P{Xps}",   /* ^space */   /* NOTE: Xps is POSIX space */
  (uschar *)"\\P{Xwd}",   /* ^word */
  NULL                    /* ^xdigit */
};
#define POSIX_SUBSIZE (sizeof(posix_substitutes)/sizeof(uschar *))
#endif
#define STRING(a)  # a
#define XSTRING(s) STRING(s)
static const char error_texts[] = "no error\0\\ at end of pattern\0\\c at end of pattern\0unrecognized character follows \\\0numbers out of order in {} quantifier\0"
                                  "number too big in {} quantifier\0missing terminating ] for character class\0invalid escape sequence in character class\0"
                                  "range out of order in character class\0nothing to repeat\0operand of unlimited repeat could match the empty string\0"
                                  "internal error: unexpected repeat\0unrecognized character after (? or (?-\0POSIX named classes are supported only within a "
                                  "class\0missing )\0reference to non-existent subpattern\0erroffset passed as NULL\0unknown option bit(s) set\0missing ) after "
                                  "comment\0parentheses nested too deeply\0regular expression is too large\0failed to get memory\0unmatched parentheses\0"
                                  "internal error: code overflow\0unrecognized character after (?<\0lookbehind assertion is not fixed length\0malformed number "
                                  "or name after (?(\0conditional group contains more than two branches\0assertion expected after (?(\0(?R or (?[+-]digits must "
                                  "be followed by )\0unknown POSIX class name\0POSIX collating elements are not supported\0this version of PCRE is not compiled "
                                  "with PCRE_UTF8 support\0spare error\0character value in \\x{...} sequence is too large\0invalid condition (?(0)\0\\C not "
                                  "allowed in lookbehind assertion\0PCRE does not support \\L, \\l, \\N{name}, \\U, or \\u\0number after (?C is > 255\0"
                                  "closing ) for (?C expected\0recursive call could loop indefinitely\0unrecognized character after (?P\0syntax error in "
                                  "subpattern name (missing terminator)\0two named subpatterns have the same name\0invalid UTF-8 string\0support for \\P, \\p, "
                                  "and \\X has not been compiled\0malformed \\P or \\p sequence\0unknown property name after \\P or \\p\0subpattern name is too "
                                  "long (maximum " XSTRING(MAX_NAME_SIZE) " characters)\0too many named subpatterns (maximum " XSTRING(MAX_NAME_COUNT) ")\0"
                                  "repeated subpattern is too long\0octal value is greater than \\377 (not in UTF-8 mode)\0internal error: overran compiling "
                                  "workspace\0internal error: previously-checked referenced subpattern not found\0DEFINE group contains more than one branch\0"
                                  "repeating a DEFINE group is not allowed\0inconsistent NEWLINE options\0\\g is not followed by a braced, angle-bracketed, or "
                                  "quoted name/number or by a plain number\0a numbered reference must not be zero\0an argument is not allowed for (*ACCEPT), "
                                  "(*FAIL), or (*COMMIT)\0(*VERB) not recognized\0number is too big\0subpattern name expected\0digit expected after (?+\0] is "
                                  "an invalid data character in JavaScript compatibility mode\0different names for subpatterns of the same number are not allowed\0"
                                  "(*MARK) must have an argument\0this version of PCRE is not compiled with PCRE_UCP support\0\\c must be followed by an ASCII "
                                  "character\0";
static BOOL compile_regex(int, int, uschar **, const uschar **, int *, BOOL, BOOL, int, int *, int *, branch_chain *, compile_data *, int *);
static const char *find_error_text(int n) {
    const char *s = error_texts;
    for (; n > 0; n--) {
        while (*s++ != 0) {};
        if (*s == 0) return "Error text not found (please report)";
    }
    return s;
}
static int check_escape(const uschar **ptrptr, int *errorcodeptr, int bracount, int options, BOOL isclass) {
    BOOL utf8 = (options & PCRE_UTF8) != 0;
    const uschar *ptr = *ptrptr + 1;
    int c, i;
    GETCHARINCTEST(c, ptr);
    ptr--;
    if (c == 0) *errorcodeptr = ERR1;
    #ifndef EBCDIC
    else if (c < CHAR_0 || c > CHAR_z) {}
    else if ((i = escapes[c - CHAR_0]) != 0) c = i;
    #else
    else if (c < 'a' || (ebcdic_chartab[c] & 0x0E) == 0) {}
    else if ((i = escapes[c - 0x48]) != 0)  c = i;
    #endif
    else {
        const uschar *oldptr;
        BOOL braced, negated;
        switch (c) {
            case CHAR_l: case CHAR_L: case CHAR_u: case CHAR_U: *errorcodeptr = ERR37; break;
            case CHAR_g:
                if (ptr[1] == CHAR_LESS_THAN_SIGN || ptr[1] == CHAR_APOSTROPHE) {
                    c = -ESC_g;
                    break;
                }
                if (ptr[1] == CHAR_LEFT_CURLY_BRACKET) {
                    const uschar *p;
                    for (p = ptr+2; *p != 0 && *p != CHAR_RIGHT_CURLY_BRACKET; p++)
                        if (*p != CHAR_MINUS && g_ascii_isdigit(*p) == 0) break;
                    if (*p != 0 && *p != CHAR_RIGHT_CURLY_BRACKET) {
                        c = -ESC_k;
                        break;
                    }
                    braced = TRUE;
                    ptr++;
                } else braced = FALSE;
                if (ptr[1] == CHAR_MINUS) {
                    negated = TRUE;
                    ptr++;
                } else negated = FALSE;
                c = 0;
                while(g_ascii_isdigit(ptr[1]) != 0) c = c * 10 + *(++ptr) - CHAR_0;
                if (c < 0) {
                    *errorcodeptr = ERR61;
                    break;
                }
                if (braced && *(++ptr) != CHAR_RIGHT_CURLY_BRACKET) {
                    *errorcodeptr = ERR57;
                    break;
                }
                if (c == 0) {
                    *errorcodeptr = ERR58;
                    break;
                }
                if (negated) {
                    if (c > bracount) {
                        *errorcodeptr = ERR15;
                        break;
                    }
                    c = bracount - (c - 1);
                }
                c = -(ESC_REF + c);
                break;
            case CHAR_1: case CHAR_2: case CHAR_3: case CHAR_4: case CHAR_5: case CHAR_6: case CHAR_7: case CHAR_8: case CHAR_9:
                if (!isclass) {
                    oldptr = ptr;
                    c -= CHAR_0;
                    while (g_ascii_isdigit(ptr[1]) != 0) c = c * 10 + *(++ptr) - CHAR_0;
                    if (c < 0) {
                        *errorcodeptr = ERR61;
                        break;
                    }
                    if (c < 10 || c <= bracount) {
                        c = -(ESC_REF + c);
                        break;
                    }
                    ptr = oldptr;
                }
                if ((c = *ptr) >= CHAR_8) {
                    ptr--;
                    c = 0;
                    break;
                }
            case CHAR_0:
                c -= CHAR_0;
                while(i++ < 2 && ptr[1] >= CHAR_0 && ptr[1] <= CHAR_7) c = c * 8 + *(++ptr) - CHAR_0;
                if (!utf8 && c > 255) *errorcodeptr = ERR51;
                break;
            case CHAR_x:
                if (ptr[1] == CHAR_LEFT_CURLY_BRACKET) {
                    const uschar *pt = ptr + 2;
                    int count = 0;
                    c = 0;
                    while (g_ascii_isxdigit(*pt) != 0) {
                        register int cc = *pt++;
                        if (c == 0 && cc == CHAR_0) continue;
                        count++;
                    #ifndef EBCDIC
                        if (cc >= CHAR_a) cc -= 32;
                        c = (c << 4) + cc - ((cc < CHAR_A)? CHAR_0 : (CHAR_A - 10));
                    #else
                        if (cc >= CHAR_a && cc <= CHAR_z) cc += 64;
                        c = (c << 4) + cc - ((cc >= CHAR_0) ? CHAR_0 : (CHAR_A - 10));
                    #endif
                    }
                    if (*pt == CHAR_RIGHT_CURLY_BRACKET) {
                    if (c < 0 || count > (utf8? 8 : 2)) *errorcodeptr = ERR34;
                        ptr = pt;
                        break;
                    }
                }
                c = 0;
                while (i++ < 2 && g_ascii_isxdigit(ptr[1]) != 0) {
                    int cc;
                    cc = *(++ptr);
                #ifndef EBCDIC
                    if (cc >= CHAR_a) cc -= 32;
                    c = c * 16 + cc - ((cc < CHAR_A)? CHAR_0 : (CHAR_A - 10));
                #else
                    if (cc <= CHAR_z) cc += 64;
                    c = c * 16 + cc - ((cc >= CHAR_0)? CHAR_0 : (CHAR_A - 10));
                #endif
                }
                break;
            case CHAR_c:
                c = *(++ptr);
                if (c == 0) {
                    *errorcodeptr = ERR2;
                    break;
                }
            #ifndef EBCDIC
                if (c > 127) {
                    *errorcodeptr = ERR68;
                    break;
                }
                if (c >= CHAR_a && c <= CHAR_z) c -= 32;
                c ^= 0x40;
            #else
                if (c >= CHAR_a && c <= CHAR_z) c += 64;
                c ^= 0xC0;
            #endif
                break;
            default:
            if ((options & PCRE_EXTRA) != 0)
                switch(c) {
                    default: *errorcodeptr = ERR3; break;
                }
            break;
        }
    }
    if (c == -ESC_N && ptr[1] == CHAR_LEFT_CURLY_BRACKET) *errorcodeptr = ERR37;
    if ((options & PCRE_UCP) != 0 && c <= -ESC_D && c >= -ESC_w) c -= (ESC_DU - ESC_D);
    *ptrptr = ptr;
    return c;
}
#ifdef SUPPORT_UCP
static int get_ucp(const uschar **ptrptr, BOOL *negptr, int *dptr, int *errorcodeptr) {
    int c, i, bot, top;
    const uschar *ptr = *ptrptr;
    char name[32];
    c = *(++ptr);
    if (c == 0) goto ERROR_RETURN;
    *negptr = FALSE;
    if (c == CHAR_LEFT_CURLY_BRACKET) {
        if (ptr[1] == CHAR_CIRCUMFLEX_ACCENT) {
            *negptr = TRUE;
            ptr++;
        }
        for (i = 0; i < (int)sizeof(name) - 1; i++) {
            c = *(++ptr);
            if (c == 0) goto ERROR_RETURN;
            if (c == CHAR_RIGHT_CURLY_BRACKET) break;
            name[i] = c;
        }
        if (c != CHAR_RIGHT_CURLY_BRACKET) goto ERROR_RETURN;
        name[i] = 0;
    } else {
        name[0] = c;
        name[1] = 0;
    }
    *ptrptr = ptr;
    bot = 0;
    top = _pcre_utt_size;
    while(bot < top) {
        i = (bot + top) >> 1;
        c = strcmp(name, _pcre_utt_names + _pcre_utt[i].name_offset);
        if (c == 0) {
          *dptr = _pcre_utt[i].value;
          return _pcre_utt[i].type;
        }
        if (c > 0) bot = i + 1; else top = i;
    }
    *errorcodeptr = ERR47;
    *ptrptr = ptr;
    return -1;
    ERROR_RETURN:
    *errorcodeptr = ERR46;
    *ptrptr = ptr;
    return -1;
}
#endif
static BOOL is_counted_repeat(const uschar *p) {
    if (g_ascii_isdigit(*p++) == 0) return FALSE;
    while(g_ascii_isdigit(*p) != 0) p++;
    if (*p == CHAR_RIGHT_CURLY_BRACKET) return TRUE;
    if (*p++ != CHAR_COMMA) return FALSE;
    if (*p == CHAR_RIGHT_CURLY_BRACKET) return TRUE;
    if (g_ascii_isdigit(*p++) == 0) return FALSE;
    while(g_ascii_isdigit(*p) != 0) p++;
    return (*p == CHAR_RIGHT_CURLY_BRACKET);
}
static const uschar *read_repeat_counts(const uschar *p, int *minp, int *maxp, int *errorcodeptr) {
    int min = 0;
    int max = -1;
    while (g_ascii_isdigit(*p) != 0) min = min * 10 + *p++ - CHAR_0;
    if (min < 0 || min > 65535) {
        *errorcodeptr = ERR5;
        return p;
    }
    if (*p == CHAR_RIGHT_CURLY_BRACKET) max = min;
    else {
        if (*(++p) != CHAR_RIGHT_CURLY_BRACKET) {
            max = 0;
            while(g_ascii_isdigit(*p) != 0) max = max * 10 + *p++ - CHAR_0;
            if (max < 0 || max > 65535) {
                *errorcodeptr = ERR5;
                return p;
            }
            if (max < min) {
                *errorcodeptr = ERR4;
                return p;
            }
        }
    }
    *minp = min;
    *maxp = max;
    return p;
}
static int find_parens_sub(uschar **ptrptr, compile_data *cd, const uschar *name, int lorn, BOOL xmode, BOOL utf8, int *count) {
    uschar *ptr = *ptrptr;
    int start_count = *count;
    int hwm_count = start_count;
    BOOL dup_parens = FALSE;
    if (ptr[0] == CHAR_LEFT_PARENTHESIS) {
        if (ptr[1] == CHAR_ASTERISK) ptr += 2;
        else if (ptr[1] != CHAR_QUESTION_MARK) {
            *count += 1;
            if (name == NULL && *count == lorn) return *count;
            ptr++;
        } else if (ptr[2] == CHAR_VERTICAL_LINE) {
            ptr += 3;
            dup_parens = TRUE;
        } else if (ptr[2] == CHAR_NUMBER_SIGN) {
            for (ptr += 3; *ptr != 0; ptr++) if (*ptr == CHAR_RIGHT_PARENTHESIS) break;
            goto FAIL_EXIT;
        } else if (ptr[2] == CHAR_LEFT_PARENTHESIS) {
            ptr += 2;
            if (ptr[1] != CHAR_QUESTION_MARK) {
                while (*ptr != 0 && *ptr != CHAR_RIGHT_PARENTHESIS) ptr++;
                if (*ptr != 0) ptr++;
            }
        } else {
            ptr += 2;
            if (*ptr == CHAR_P) ptr++;
            if ((*ptr == CHAR_LESS_THAN_SIGN && ptr[1] != CHAR_EXCLAMATION_MARK && ptr[1] != CHAR_EQUALS_SIGN) || *ptr == CHAR_APOSTROPHE) {
                int term;
                const uschar *thisname;
                *count += 1;
                if (name == NULL && *count == lorn) return *count;
                term = *ptr++;
                if (term == CHAR_LESS_THAN_SIGN) term = CHAR_GREATER_THAN_SIGN;
                thisname = ptr;
                while (*ptr != term) ptr++;
                if (name != NULL && lorn == ptr - thisname && strncmp((const char *)name, (const char *)thisname, lorn) == 0) return *count;
                term++;
            }
        }
    }
    for (; ptr < cd->end_pattern; ptr++) {
        if (*ptr == CHAR_BACKSLASH) {
            if (*(++ptr) == 0) goto FAIL_EXIT;
            if (*ptr == CHAR_Q)
                for (;;) {
                    while (*(++ptr) != 0 && *ptr != CHAR_BACKSLASH) {};
                    if (*ptr == 0) goto FAIL_EXIT;
                    if (*(++ptr) == CHAR_E) break;
                }
            continue;
        }
        if (*ptr == CHAR_LEFT_SQUARE_BRACKET) {
            BOOL negate_class = FALSE;
            for (;;) {
                if (ptr[1] == CHAR_BACKSLASH) {
                    if (ptr[2] == CHAR_E) ptr+= 2;
                    else if (strncmp((const char *)ptr+2, STR_Q STR_BACKSLASH STR_E, 3) == 0) ptr += 4;
                    else break;
                } else if (!negate_class && ptr[1] == CHAR_CIRCUMFLEX_ACCENT) {
                    negate_class = TRUE;
                    ptr++;
                } else break;
            }
            if (ptr[1] == CHAR_RIGHT_SQUARE_BRACKET && (cd->external_options & PCRE_JAVASCRIPT_COMPAT) == 0) ptr++;
            while (*(++ptr) != CHAR_RIGHT_SQUARE_BRACKET) {
                if (*ptr == 0) return -1;
                if (*ptr == CHAR_BACKSLASH) {
                    if (*(++ptr) == 0) goto FAIL_EXIT;
                    if (*ptr == CHAR_Q)
                      for (;;) {
                          while (*(++ptr) != 0 && *ptr != CHAR_BACKSLASH) {};
                          if (*ptr == 0) goto FAIL_EXIT;
                          if (*(++ptr) == CHAR_E) break;
                      }
                    continue;
                }
            }
            continue;
        }
        if (xmode && *ptr == CHAR_NUMBER_SIGN) {
            ptr++;
            while (*ptr != 0) {
                if (IS_NEWLINE(ptr)) { ptr += cd->nllen - 1; break; }
                ptr++;
            #ifdef SUPPORT_UTF8
                if (utf8) while ((*ptr & 0xc0) == 0x80) ptr++;
            #endif
            }
            if (*ptr == 0) goto FAIL_EXIT;
            continue;
        }
        if (*ptr == CHAR_LEFT_PARENTHESIS) {
            int rc = find_parens_sub(&ptr, cd, name, lorn, xmode, utf8, count);
            if (rc > 0) return rc;
            if (*ptr == 0) goto FAIL_EXIT;
        } else if (*ptr == CHAR_RIGHT_PARENTHESIS) {
            if (dup_parens && *count < hwm_count) *count = hwm_count;
            goto FAIL_EXIT;
        } else if (*ptr == CHAR_VERTICAL_LINE && dup_parens) {
            if (*count > hwm_count) hwm_count = *count;
            *count = start_count;
        }
    }
    FAIL_EXIT:
    *ptrptr = ptr;
    return -1;
}
static int find_parens(compile_data *cd, const uschar *name, int lorn, BOOL xmode, BOOL utf8) {
    uschar *ptr = (uschar *)cd->start_pattern;
    int count = 0;
    int rc;
    for (;;) {
        rc = find_parens_sub(&ptr, cd, name, lorn, xmode, utf8, &count);
        if (rc > 0 || *ptr++ == 0) break;
    }
    return rc;
}
static const uschar* first_significant_code(const uschar *code, int *options, int optbit, BOOL skipassert) {
    for (;;) {
        switch ((int)*code) {
            case OP_OPT:
                if (optbit > 0 && ((int)code[1] & optbit) != (*options & optbit)) *options = (int)code[1];
                code += 2;
                break;
            case OP_ASSERT_NOT: case OP_ASSERTBACK: case OP_ASSERTBACK_NOT:
                if (!skipassert) return code;
                /*do {
                    code += GET(code, 1);
                } while(*code == OP_ALT);*/
                code += _pcre_OP_lengths[*code];
                break;
            case OP_WORD_BOUNDARY: case OP_NOT_WORD_BOUNDARY: if (!skipassert) return code;
            case OP_CALLOUT: case OP_CREF: case OP_NCREF: case OP_RREF: case OP_NRREF: case OP_DEF: code += _pcre_OP_lengths[*code]; break;
            default: return code;
        }
    }
}
static int find_fixedlength(uschar *code, int options, BOOL atend, compile_data *cd) {
    int length = -1;
    register int branchlength = 0;
    register uschar *cc = code + 1 /*+ LINK_SIZE*/;
    for (;;) {
        int d;
        uschar *ce, *cs;
        register int op = *cc;
        switch (op) {
            case OP_CBRA: case OP_BRA: case OP_ONCE: case OP_COND:
                d = find_fixedlength(cc + ((op == OP_CBRA)? 2:0), options, atend, cd);
                if (d < 0) return d;
                branchlength += d;
                /*do {
                    cc += GET(cc, 1);
                } while (*cc == OP_ALT);*/
                cc += 1 /*+ LINK_SIZE*/;
                break;
            case OP_ALT: case OP_KET: case OP_KETRMAX: case OP_KETRMIN: case OP_END:
                if (length < 0) length = branchlength;
                else if (length != branchlength) return -1;
                if (*cc != OP_ALT) return length;
                cc += 1 /*+ LINK_SIZE*/;
                branchlength = 0;
                break;
            case OP_RECURSE:
                if (!atend) return -3;
                cs = ce = (uschar *)cd->start_code /*+ GET(cc, 1)*/;
                /*do {
                    ce += GET(ce, 1);
                } while (*ce == OP_ALT);*/
                if (cc > cs && cc < ce) return -1;
                d = find_fixedlength(cs + 2, options, atend, cd);
                if (d < 0) return d;
                branchlength += d;
                cc += 1 /*+ LINK_SIZE*/;
                break;
            case OP_ASSERT: case OP_ASSERT_NOT: case OP_ASSERTBACK: case OP_ASSERTBACK_NOT:
                /*do {
                    cc += GET(cc, 1);
                } while (*cc == OP_ALT);*/
            case OP_REVERSE: case OP_CREF: case OP_NCREF: case OP_RREF: case OP_NRREF: case OP_DEF: case OP_OPT: case OP_CALLOUT: case OP_SOD: case OP_SOM:
            case OP_SET_SOM: case OP_EOD: case OP_EODN: case OP_CIRC: case OP_DOLL: case OP_NOT_WORD_BOUNDARY: case OP_WORD_BOUNDARY:
                cc += _pcre_OP_lengths[*cc];
                break;
            case OP_CHAR: case OP_CHARNC: case OP_NOT:
                branchlength++;
                cc += 2;
            #ifdef SUPPORT_UTF8
                if ((options & PCRE_UTF8) != 0 && cc[-1] >= 0xc0) cc += _pcre_utf8_table4[cc[-1] & 0x3f];
            #endif
                break;
            case OP_EXACT:
                branchlength += GET2(cc,1);
                cc += 4;
            #ifdef SUPPORT_UTF8
                if ((options & PCRE_UTF8) != 0 && cc[-1] >= 0xc0) cc += _pcre_utf8_table4[cc[-1] & 0x3f];
            #endif
                break;
            case OP_TYPEEXACT:
                branchlength += GET2(cc,1);
                if (cc[3] == OP_PROP || cc[3] == OP_NOTPROP) cc += 2;
                cc += 4;
                break;
            case OP_PROP: case OP_NOTPROP: cc += 2;
            case OP_NOT_DIGIT: case OP_DIGIT: case OP_NOT_WHITESPACE: case OP_WHITESPACE: case OP_NOT_WORDCHAR: case OP_WORDCHAR: case OP_ANY: case OP_ALLANY:
                branchlength++;
                cc++;
                break;
            case OP_ANYBYTE: return -2;
        #ifdef SUPPORT_UTF8
            case OP_XCLASS: cc += GET(cc, 1) - 33;
        #endif
            case OP_CLASS:case OP_NCLASS:
                cc += 33;
                switch (*cc) {
                    case OP_CRSTAR: case OP_CRMINSTAR: case OP_CRQUERY: case OP_CRMINQUERY: return -1;
                    case OP_CRRANGE: case OP_CRMINRANGE:
                    if (GET2(cc,1) != GET2(cc,3)) return -1;
                    branchlength += GET2(cc,1);
                    cc += 5;
                    break;
                    default: branchlength++;
                }
                break;
            default: return -1;
        }
    }
}
const uschar * _pcre_find_bracket(const uschar *code, BOOL utf8, int number) {
    for (;;) {
        register int c = *code;
        if (c == OP_END) return NULL;
        //if (c == OP_XCLASS) code += GET(code, 1);
        else if (c == OP_REVERSE) {
            if (number < 0) return (uschar *)code;
            code += _pcre_OP_lengths[c];
        } else if (c == OP_CBRA) {
            //int n = GET2(code, 1+LINK_SIZE);
            //if (n == number) return (uschar *)code;
            code += _pcre_OP_lengths[c];
        } else {
            switch(c) {
                case OP_TYPESTAR: case OP_TYPEMINSTAR: case OP_TYPEPLUS: case OP_TYPEMINPLUS: case OP_TYPEQUERY: case OP_TYPEMINQUERY: case OP_TYPEPOSSTAR:
                case OP_TYPEPOSPLUS: case OP_TYPEPOSQUERY:
                    if (code[1] == OP_PROP || code[1] == OP_NOTPROP) code += 2;
                    break;
                case OP_TYPEUPTO: case OP_TYPEMINUPTO: case OP_TYPEEXACT: case OP_TYPEPOSUPTO:
                    if (code[3] == OP_PROP || code[3] == OP_NOTPROP) code += 2;
                    break;
                case OP_MARK: case OP_PRUNE_ARG: case OP_SKIP_ARG:
                    code += code[1];
                    break;
                case OP_THEN_ARG:
                    //code += code[1+LINK_SIZE];
                    break;
            }
            code += _pcre_OP_lengths[c];
        #ifdef SUPPORT_UTF8
            if (utf8)
                switch(c) {
                    case OP_CHAR: case OP_CHARNC: case OP_EXACT: case OP_UPTO: case OP_MINUPTO: case OP_POSUPTO: case OP_STAR: case OP_MINSTAR: case OP_POSSTAR:
                    case OP_PLUS: case OP_MINPLUS: case OP_POSPLUS: case OP_QUERY: case OP_MINQUERY: case OP_POSQUERY:
                        if (code[-1] >= 0xc0) code += _pcre_utf8_table4[code[-1] & 0x3f];
                        break;
                }
        #else
            (void)(utf8);
        #endif
        }
    }
}
static const uschar *find_recurse(const uschar *code, BOOL utf8) {
    for (;;) {
        register int c = *code;
        if (c == OP_END) return NULL;
        if (c == OP_RECURSE) return code;
        //if (c == OP_XCLASS) code += GET(code, 1);
        else {
            switch(c) {
                case OP_TYPESTAR: case OP_TYPEMINSTAR: case OP_TYPEPLUS: case OP_TYPEMINPLUS: case OP_TYPEQUERY: case OP_TYPEMINQUERY: case OP_TYPEPOSSTAR:
                case OP_TYPEPOSPLUS: case OP_TYPEPOSQUERY:
                    if (code[1] == OP_PROP || code[1] == OP_NOTPROP) code += 2;
                    break;
                case OP_TYPEPOSUPTO: case OP_TYPEUPTO: case OP_TYPEMINUPTO: case OP_TYPEEXACT:
                    if (code[3] == OP_PROP || code[3] == OP_NOTPROP) code += 2;
                    break;
                case OP_MARK: case OP_PRUNE_ARG: case OP_SKIP_ARG: code += code[1]; break;
                //case OP_THEN_ARG: code += code[1+LINK_SIZE]; break;
            }
            code += _pcre_OP_lengths[c];
        #ifdef SUPPORT_UTF8
            if (utf8)
                switch(c) {
                    case OP_CHAR: case OP_CHARNC: case OP_EXACT: case OP_UPTO: case OP_MINUPTO: case OP_POSUPTO: case OP_STAR: case OP_MINSTAR: case OP_POSSTAR:
                    case OP_PLUS: case OP_MINPLUS: case OP_POSPLUS: case OP_QUERY: case OP_MINQUERY: case OP_POSQUERY:
                        if (code[-1] >= 0xc0) code += _pcre_utf8_table4[code[-1] & 0x3f];
                        break;
                }
        #else
            (void)(utf8);  /* Keep compiler happy by referencing function argument */
    #endif
        }
    }
}
static BOOL could_be_empty_branch(const uschar *code, const uschar *endcode, BOOL utf8, compile_data *cd) {
    register int c;
    code = first_significant_code(code + _pcre_OP_lengths[*code], NULL, 0, TRUE);
    for ( ; code < endcode; code = first_significant_code(code + _pcre_OP_lengths[c], NULL, 0, TRUE)) {
        const uschar *ccode;
        c = *code;
        if (c == OP_ASSERT) {
            /*do {
                code += GET(code, 1);
            } while(*code == OP_ALT);*/
            c = *code;
            continue;
        }
        if (c == OP_BRAZERO || c == OP_BRAMINZERO || c == OP_SKIPZERO) {
            code += _pcre_OP_lengths[c];
            /*do {
                code += GET(code, 1);
            } while(*code == OP_ALT);*/
            c = *code;
            continue;
        }
        if (c == OP_RECURSE) {
            BOOL empty_branch = FALSE;
            const uschar *scode = cd->start_code /*+ GET(code, 1)*/;
            //if (GET(scode, 1) == 0) return TRUE;
            /*do {
                if (could_be_empty_branch(scode, endcode, utf8, cd)) {
                    empty_branch = TRUE;
                    break;
                }
                scode += GET(scode, 1);
            } while (*scode == OP_ALT);*/
            if (!empty_branch) return FALSE;
            continue;
        }
        if (c == OP_BRA || c == OP_CBRA || c == OP_ONCE || c == OP_COND) {
            //BOOL empty_branch;
            //if (GET(code, 1) == 0) return TRUE;
            /*if (c == OP_COND && code[GET(code, 1)] != OP_ALT) code += GET(code, 1);
            else {
                empty_branch = FALSE;
                do {
                    if (!empty_branch && could_be_empty_branch(code, endcode, utf8, cd)) empty_branch = TRUE;
                    code += GET(code, 1);
                } while (*code == OP_ALT);
                if (!empty_branch) return FALSE;
            }
            c = *code;*/
            continue;
        }
        switch(c) {
        #ifdef SUPPORT_UTF8
            case OP_XCLASS: ccode = code += GET(code, 1); goto CHECK_CLASS_REPEAT;
        #endif
            case OP_CLASS: case OP_NCLASS:
                ccode = code + 33;
        #ifdef SUPPORT_UTF8
            CHECK_CLASS_REPEAT:
        #endif
                switch (*ccode) {
                    case OP_CRSTAR: case OP_CRMINSTAR: case OP_CRQUERY: case OP_CRMINQUERY: break;
                    case OP_CRRANGE:
                    case OP_CRMINRANGE:
                        if (GET2(ccode, 1) > 0) return FALSE;
                        break;
                    default: return FALSE;
                }
                break;
            case OP_PROP: case OP_NOTPROP: case OP_EXTUNI: case OP_NOT_DIGIT: case OP_DIGIT: case OP_NOT_WHITESPACE: case OP_WHITESPACE: case OP_NOT_WORDCHAR:
            case OP_WORDCHAR: case OP_ANY: case OP_ALLANY: case OP_ANYBYTE: case OP_CHAR: case OP_CHARNC: case OP_NOT: case OP_PLUS: case OP_MINPLUS:
            case OP_POSPLUS: case OP_EXACT: case OP_NOTPLUS: case OP_NOTMINPLUS: case OP_NOTPOSPLUS: case OP_NOTEXACT: case OP_TYPEPLUS: case OP_TYPEMINPLUS:
            case OP_TYPEPOSPLUS: case OP_TYPEEXACT:
                return FALSE;
            case OP_TYPESTAR: case OP_TYPEMINSTAR: case OP_TYPEPOSSTAR: case OP_TYPEQUERY: case OP_TYPEMINQUERY: case OP_TYPEPOSQUERY:
                if (code[1] == OP_PROP || code[1] == OP_NOTPROP) code += 2;
                break;
            case OP_TYPEUPTO: case OP_TYPEMINUPTO: case OP_TYPEPOSUPTO:
                if (code[3] == OP_PROP || code[3] == OP_NOTPROP) code += 2;
                break;
            case OP_KET: case OP_KETRMAX: case OP_KETRMIN: case OP_ALT: return TRUE;
        #ifdef SUPPORT_UTF8
            case OP_STAR: case OP_MINSTAR: case OP_POSSTAR: case OP_QUERY: case OP_MINQUERY: case OP_POSQUERY:
                if (utf8 && code[1] >= 0xc0) code += _pcre_utf8_table4[code[1] & 0x3f];
                break;
            case OP_UPTO: case OP_MINUPTO: case OP_POSUPTO:
                if (utf8 && code[3] >= 0xc0) code += _pcre_utf8_table4[code[3] & 0x3f];
                break;
        #endif
            case OP_MARK: case OP_PRUNE_ARG: case OP_SKIP_ARG: code += code[1]; break;
            //case OP_THEN_ARG: code += code[1+LINK_SIZE]; break;
            default: break;
        }
    }
    return TRUE;
}
static BOOL could_be_empty(const uschar *code, const uschar *endcode, branch_chain *bcptr, BOOL utf8, compile_data *cd) {
    while (bcptr != NULL && bcptr->current_branch >= code) {
        if (!could_be_empty_branch(bcptr->current_branch, endcode, utf8, cd)) return FALSE;
        bcptr = bcptr->outer;
    }
    return TRUE;
}
static BOOL check_posix_syntax(const uschar *ptr, const uschar **endptr) {
    int terminator;
    terminator = *(++ptr);
    for (++ptr; *ptr != 0; ptr++) {
        if (*ptr == CHAR_BACKSLASH && ptr[1] == CHAR_RIGHT_SQUARE_BRACKET) ptr++;
        else {
            if (*ptr == CHAR_RIGHT_SQUARE_BRACKET) return FALSE;
            if (*ptr == terminator && ptr[1] == CHAR_RIGHT_SQUARE_BRACKET) {
                *endptr = ptr;
                return TRUE;
            }
        }
    }
    return FALSE;
}
static int check_posix_name(const uschar *ptr, int len) {
    const char *pn = posix_names;
    register int yield = 0;
    while (posix_name_lengths[yield] != 0) {
        if (len == posix_name_lengths[yield] && strncmp((const char *)ptr, pn, len) == 0) return yield;
        pn += posix_name_lengths[yield] + 1;
        yield++;
    }
    return -1;
}
static void adjust_recurse(uschar *group, int adjust, BOOL utf8, compile_data *cd, uschar *save_hwm) {
    /*uschar *ptr = group;
    while((ptr = (uschar *)find_recurse(ptr, utf8)) != NULL) {
        int offset;
        uschar *hc;
        for (hc = save_hwm; hc < cd->hwm; hc += LINK_SIZE) {
            offset = GET(hc, 0);
            if (cd->start_code + offset == ptr + 1) {
                PUT(hc, 0, offset + adjust);
                break;
            }
        }
        if (hc >= cd->hwm) {
            offset = GET(ptr, 1);
            if (cd->start_code + offset >= group) PUT(ptr, 1, offset + adjust);
        }
        ptr += 1 + LINK_SIZE;
    }*/
}
static uschar *auto_callout(uschar *code, const uschar *ptr, compile_data *cd) {
    *code++ = OP_CALLOUT;
    *code++ = 255;
    //PUT(code, 0, (int)(ptr - cd->start_pattern));
    //PUT(code, LINK_SIZE, 0);
    return code /*+ 2*LINK_SIZE*/;
}
static void complete_callout(uschar *previous_callout, const uschar *ptr, compile_data *cd) {
    //int length = (int)(ptr - cd->start_pattern - GET(previous_callout, 2));
    //PUT(previous_callout, 2 + LINK_SIZE, length);
}
#ifdef SUPPORT_UCP
static BOOL get_othercase_range(unsigned int *cptr, unsigned int d, unsigned int *ocptr, unsigned int *odptr) {
    unsigned int c, othercase, next;
    for (c = *cptr; c <= d; c++) { if ((othercase = UCD_OTHERCASE(c)) != c) break; }
    if (c > d) return FALSE;
    *ocptr = othercase;
    next = othercase + 1;
    for (++c; c <= d; c++) {
        if (UCD_OTHERCASE(c) != next) break;
        next++;
    }
    *odptr = next - 1;
    *cptr = c;
    return TRUE;
}
static BOOL check_char_prop(int c, int ptype, int pdata, BOOL negated) {
    int chartype = UCD_CHARTYPE(c);
    switch(ptype) {
        case PT_LAMP: return (chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt) == negated;
        case PT_GC: return (pdata == _pcre_ucp_gentype[chartype]) == negated;
        case PT_PC: return (pdata == chartype) == negated;
        case PT_SC: return (pdata == UCD_SCRIPT(c)) == negated;
        case PT_ALNUM: return (_pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N) == negated;
        case PT_SPACE: return (_pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR) == negated;
        case PT_PXSPACE: return (_pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR) == negated;
        case PT_WORD: return (_pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N || c == CHAR_UNDERSCORE) == negated;
    }
    return FALSE;
}
#endif
static BOOL check_auto_possessive(const uschar *previous, BOOL utf8, const uschar *ptr, int options, compile_data *cd) {
    int c, next;
    int op_code = *previous++;
    if ((options & PCRE_EXTENDED) != 0) {
        for (;;) {
            while ((cd->ctypes[*ptr] & ctype_space) != 0) ptr++;
            if (*ptr == CHAR_NUMBER_SIGN) {
                ptr++;
                while (*ptr != 0) {
                if (IS_NEWLINE(ptr)) { ptr += cd->nllen; break; }
                ptr++;
        #ifdef SUPPORT_UTF8
                if (utf8) while ((*ptr & 0xc0) == 0x80) ptr++;
        #endif
                }
            } else break;
        }
    }
    if (*ptr == CHAR_BACKSLASH) {
        int temperrorcode = 0;
        next = check_escape(&ptr, &temperrorcode, cd->bracount, options, FALSE);
        if (temperrorcode != 0) return FALSE;
        ptr++;
    } else if ((cd->ctypes[*ptr] & ctype_meta) == 0) {
    #ifdef SUPPORT_UTF8
        if (utf8) { GETCHARINC(next, ptr); } else
    #endif
        next = *ptr++;
    } else return FALSE;
    if ((options & PCRE_EXTENDED) != 0) {
        for (;;) {
            while ((cd->ctypes[*ptr] & ctype_space) != 0) ptr++;
            if (*ptr == CHAR_NUMBER_SIGN) {
                ptr++;
                while (*ptr != 0) {
                if (IS_NEWLINE(ptr)) { ptr += cd->nllen; break; }
                ptr++;
        #ifdef SUPPORT_UTF8
                if (utf8) while ((*ptr & 0xc0) == 0x80) ptr++;
        #endif
                }
            } else break;
        }
    }
    if (*ptr == CHAR_ASTERISK || *ptr == CHAR_QUESTION_MARK || strncmp((char *)ptr, STR_LEFT_CURLY_BRACKET STR_0 STR_COMMA, 3) == 0) return FALSE;
    if (next >= 0)
        switch(op_code) {
            case OP_CHAR:
            #ifdef SUPPORT_UTF8
                GETCHARTEST(c, previous);
            #else
                c = *previous;
            #endif
                return c != next;
            case OP_CHARNC:
            #ifdef SUPPORT_UTF8
                GETCHARTEST(c, previous);
            #else
                c = *previous;
            #endif
                if (c == next) return FALSE;
            #ifdef SUPPORT_UTF8
                if (utf8) {
                    unsigned int othercase;
                    if (next < 128) othercase = cd->fcc[next];
                    else
                #ifdef SUPPORT_UCP
                    othercase = UCD_OTHERCASE((unsigned int)next);
                #else
                    othercase = NOTACHAR;
                #endif
                    return (unsigned int)c != othercase;
                }
                else
            #endif
                return (c != cd->fcc[next]);
            case OP_NOT:
                if ((c = *previous) == next) return TRUE;
                if ((options & PCRE_CASELESS) == 0) return FALSE;
            #ifdef SUPPORT_UTF8
                if (utf8) {
                    unsigned int othercase;
                    if (next < 128) othercase = cd->fcc[next]; else
                #ifdef SUPPORT_UCP
                    othercase = UCD_OTHERCASE(next);
                #else
                    othercase = NOTACHAR;
                #endif
                    return (unsigned int)c == othercase;
                } else
            #endif
                return (c == cd->fcc[next]);
            case OP_DIGIT: return next > 127 || (cd->ctypes[next] & ctype_digit) == 0;
            case OP_NOT_DIGIT: return next <= 127 && (cd->ctypes[next] & ctype_digit) != 0;
            case OP_WHITESPACE: return next > 127 || (cd->ctypes[next] & ctype_space) == 0;
            case OP_NOT_WHITESPACE: return next <= 127 && (cd->ctypes[next] & ctype_space) != 0;
            case OP_WORDCHAR: return next > 127 || (cd->ctypes[next] & ctype_word) == 0;
            case OP_NOT_WORDCHAR: return next <= 127 && (cd->ctypes[next] & ctype_word) != 0;
            case OP_HSPACE: case OP_NOT_HSPACE:
                switch(next) {
                    case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                    case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                        return op_code == OP_NOT_HSPACE;
                    default: return op_code != OP_NOT_HSPACE;
                }
            case OP_ANYNL: case OP_VSPACE: case OP_NOT_VSPACE:
                switch(next) {
                    case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029: return op_code == OP_NOT_VSPACE;
                    default: return op_code != OP_NOT_VSPACE;
                }
        #ifdef SUPPORT_UCP
            case OP_PROP: return check_char_prop(next, previous[0], previous[1], FALSE);
            case OP_NOTPROP: return check_char_prop(next, previous[0], previous[1], TRUE);
        #endif
            default: return FALSE;
        }
    switch(op_code) {
        case OP_CHAR: case OP_CHARNC:
        #ifdef SUPPORT_UTF8
            GETCHARTEST(c, previous);
        #else
            c = *previous;
        #endif
            switch(-next) {
                case ESC_d: return c > 127 || (cd->ctypes[c] & ctype_digit) == 0;
                case ESC_D: return c <= 127 && (cd->ctypes[c] & ctype_digit) != 0;
                case ESC_s: return c > 127 || (cd->ctypes[c] & ctype_space) == 0;
                case ESC_S: return c <= 127 && (cd->ctypes[c] & ctype_space) != 0;
                case ESC_w: return c > 127 || (cd->ctypes[c] & ctype_word) == 0;
                case ESC_W: return c <= 127 && (cd->ctypes[c] & ctype_word) != 0;
                case ESC_h: case ESC_H:
                    switch(c) {
                        case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                        case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                            return -next != ESC_h;
                        default: return -next == ESC_h;
                    }
                case ESC_v: case ESC_V:
                    switch(c) {
                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029: return -next != ESC_v;
                        default: return -next == ESC_v;
                    }
            #ifdef SUPPORT_UCP
                case ESC_du: case ESC_DU: case ESC_wu: case ESC_WU: case ESC_su: case ESC_SU: {
                        int temperrorcode = 0;
                        ptr = substitutes[-next - ESC_DU];
                        next = check_escape(&ptr, &temperrorcode, 0, options, FALSE);
                        if (temperrorcode != 0) return FALSE;
                        ptr++;
                    }
                case ESC_p: case ESC_P: {
                        int ptype, pdata, errorcodeptr;
                        BOOL negated;
                        ptr--;
                        ptype = get_ucp(&ptr, &negated, &pdata, &errorcodeptr);
                        if (ptype < 0) return FALSE;
                        ptr++;
                        if (*ptr == CHAR_ASTERISK || *ptr == CHAR_QUESTION_MARK || strncmp((char *)ptr, STR_LEFT_CURLY_BRACKET STR_0 STR_COMMA, 3) == 0)
                            return FALSE;
                        return check_char_prop(c, ptype, pdata, (next == -ESC_P) != negated);
                    }
            #endif
                default: return FALSE;
            }
        case OP_DIGIT: return next == -ESC_D || next == -ESC_s || next == -ESC_W || next == -ESC_h || next == -ESC_v || next == -ESC_R;
        case OP_NOT_DIGIT: return next == -ESC_d;
        case OP_WHITESPACE: return next == -ESC_S || next == -ESC_d || next == -ESC_w || next == -ESC_R;
        case OP_NOT_WHITESPACE: return next == -ESC_s || next == -ESC_h || next == -ESC_v;
        case OP_HSPACE: return next == -ESC_S || next == -ESC_H || next == -ESC_d || next == -ESC_w || next == -ESC_v || next == -ESC_R;
        case OP_NOT_HSPACE: return next == -ESC_h;
        case OP_ANYNL: case OP_VSPACE: return next == -ESC_V || next == -ESC_d || next == -ESC_w;
        case OP_NOT_VSPACE: return next == -ESC_v || next == -ESC_R;
        case OP_WORDCHAR: return next == -ESC_W || next == -ESC_s || next == -ESC_h || next == -ESC_v || next == -ESC_R;
        case OP_NOT_WORDCHAR: return next == -ESC_w || next == -ESC_d;
        default: return FALSE;
    }
}
static BOOL compile_branch(int *optionsptr, uschar **codeptr, const uschar **ptrptr, int *errorcodeptr, int *firstbyteptr, int *reqbyteptr, branch_chain *bcptr,
                           compile_data *cd, int *lengthptr) {
    int repeat_type, op_type;
    int repeat_min = 0, repeat_max = 0;
    int bravalue = 0;
    int greedy_default, greedy_non_default;
    int firstbyte, reqbyte;
    int zeroreqbyte, zerofirstbyte;
    int req_caseopt, reqvary, tempreqvary;
    int options = *optionsptr;
    int after_manual_callout = 0;
    int length_prevgroup = 0;
    register int c;
    register uschar *code = *codeptr;
    uschar *last_code = code;
    uschar *orig_code = code;
    uschar *tempcode;
    BOOL inescq = FALSE;
    BOOL groupsetfirstbyte = FALSE;
    const uschar *ptr = *ptrptr;
    const uschar *tempptr;
    const uschar *nestptr = NULL;
    uschar *previous = NULL;
    uschar *previous_callout = NULL;
    uschar *save_hwm = NULL;
    uschar classbits[32];
    #ifdef SUPPORT_UTF8
    BOOL class_utf8;
    BOOL utf8 = (options & PCRE_UTF8) != 0;
    uschar *class_utf8data;
    uschar *class_utf8data_base;
    uschar utf8_char[6];
    #else
    BOOL utf8 = FALSE;
    uschar *utf8_char = NULL;
    #endif
    #ifdef PCRE_DEBUG
    if (lengthptr != NULL) DPRINTF((">> start branch\n"));
    #endif
    greedy_default = ((options & PCRE_UNGREEDY) != 0);
    greedy_non_default = greedy_default ^ 1;
    firstbyte = reqbyte = zerofirstbyte = zeroreqbyte = REQ_UNSET;
    req_caseopt = ((options & PCRE_CASELESS) != 0)? REQ_CASELESS : 0;
    for (;; ptr++) {
        BOOL negate_class;
        BOOL should_flip_negation;
        BOOL possessive_quantifier;
        BOOL is_quantifier;
        BOOL is_recurse;
        BOOL reset_bracount;
        int class_charcount;
        int class_lastchar;
        int newoptions;
        int recno;
        int refsign;
        int skipbytes;
        int subreqbyte;
        int subfirstbyte;
        int terminator;
        int mclength;
        uschar mcbuffer[8];
        c = *ptr;
        if (c == 0 && nestptr != NULL) {
            ptr = nestptr;
            nestptr = NULL;
            c = *ptr;
        }
        if (lengthptr != NULL) {
        #ifdef PCRE_DEBUG
            if (code > cd->hwm) cd->hwm = code;
        #endif
            if (code > cd->start_workspace + WORK_SIZE_CHECK) {
                *errorcodeptr = ERR52;
                goto FAILED;
            }
            if (code < last_code) code = last_code;
            if (OFLOW_MAX - *lengthptr < code - last_code) {
                *errorcodeptr = ERR20;
                goto FAILED;
            }
            *lengthptr += (int)(code - last_code);
            DPRINTF(("length=%d added %d c=%c\n", *lengthptr, code - last_code, c));
            if (previous != NULL) {
                if (previous > orig_code) {
                    memmove(orig_code, previous, code - previous);
                    code -= previous - orig_code;
                    previous = orig_code;
                }
            } else code = orig_code;
            last_code = code;
        } else if (cd->hwm > cd->start_workspace + WORK_SIZE_CHECK) {
            *errorcodeptr = ERR52;
            goto FAILED;
        }
        if (inescq && c != 0) {
            if (c == CHAR_BACKSLASH && ptr[1] == CHAR_E) {
                inescq = FALSE;
                ptr++;
                continue;
            } else {
                if (previous_callout != NULL) {
                    if (lengthptr == NULL) complete_callout(previous_callout, ptr, cd);
                    previous_callout = NULL;
                }
                if ((options & PCRE_AUTO_CALLOUT) != 0) {
                    previous_callout = code;
                    code = auto_callout(code, ptr, cd);
                }
                goto NORMAL_CHAR;
            }
        }
        is_quantifier = c == CHAR_ASTERISK || c == CHAR_PLUS || c == CHAR_QUESTION_MARK || (c == CHAR_LEFT_CURLY_BRACKET && is_counted_repeat(ptr+1));
        if (!is_quantifier && previous_callout != NULL && after_manual_callout-- <= 0) {
            if (lengthptr == NULL) complete_callout(previous_callout, ptr, cd);
            previous_callout = NULL;
        }
        if ((options & PCRE_EXTENDED) != 0) {
            if ((cd->ctypes[c] & ctype_space) != 0) continue;
            if (c == CHAR_NUMBER_SIGN) {
                ptr++;
                while(*ptr != 0) {
                    if (IS_NEWLINE(ptr)) { ptr += cd->nllen - 1; break; }
                    ptr++;
                #ifdef SUPPORT_UTF8
                    if (utf8) while ((*ptr & 0xc0) == 0x80) ptr++;
                #endif
                }
                if (*ptr != 0) continue;
                c = 0;
            }
        }
        if ((options & PCRE_AUTO_CALLOUT) != 0 && !is_quantifier) {
            previous_callout = code;
            code = auto_callout(code, ptr, cd);
        }
        switch(c) {
            case 0: case CHAR_VERTICAL_LINE: case CHAR_RIGHT_PARENTHESIS:
                *firstbyteptr = firstbyte;
                *reqbyteptr = reqbyte;
                *codeptr = code;
                *ptrptr = ptr;
                if (lengthptr != NULL) {
                    if (OFLOW_MAX - *lengthptr < code - last_code) {
                        *errorcodeptr = ERR20;
                        goto FAILED;
                    }
                    *lengthptr += (int)(code - last_code);
                    DPRINTF((">> end branch\n"));
                }
                return TRUE;
            case CHAR_CIRCUMFLEX_ACCENT:
                if ((options & PCRE_MULTILINE) != 0) {
                    if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
                }
                previous = NULL;
                *code++ = OP_CIRC;
                break;
            case CHAR_DOLLAR_SIGN:
                previous = NULL;
                *code++ = OP_DOLL;
                break;
            case CHAR_DOT:
                if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
                zerofirstbyte = firstbyte;
                zeroreqbyte = reqbyte;
                previous = code;
                *code++ = ((options & PCRE_DOTALL) != 0)? OP_ALLANY: OP_ANY;
                break;
            case CHAR_RIGHT_SQUARE_BRACKET:
                if ((cd->external_options & PCRE_JAVASCRIPT_COMPAT) != 0) {
                    *errorcodeptr = ERR64;
                    goto FAILED;
                }
                goto NORMAL_CHAR;
            case CHAR_LEFT_SQUARE_BRACKET:
                previous = code;
                if ((ptr[1] == CHAR_COLON || ptr[1] == CHAR_DOT || ptr[1] == CHAR_EQUALS_SIGN) && check_posix_syntax(ptr, &tempptr)) {
                    *errorcodeptr = (ptr[1] == CHAR_COLON)? ERR13 : ERR31;
                    goto FAILED;
                }
                negate_class = FALSE;
                for (;;) {
                    c = *(++ptr);
                    if (c == CHAR_BACKSLASH) {
                        if (ptr[1] == CHAR_E) ptr++;
                        else if (strncmp((const char *)ptr+1, STR_Q STR_BACKSLASH STR_E, 3) == 0) ptr += 3;
                        else break;
                    } else if (!negate_class && c == CHAR_CIRCUMFLEX_ACCENT) negate_class = TRUE;
                    else break;
                }
                if (c == CHAR_RIGHT_SQUARE_BRACKET && (cd->external_options & PCRE_JAVASCRIPT_COMPAT) != 0) {
                    *code++ = negate_class? OP_ALLANY : OP_FAIL;
                    if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
                    zerofirstbyte = firstbyte;
                    break;
                }
                should_flip_negation = FALSE;
                class_charcount = 0;
                class_lastchar = -1;
                memset(classbits, 0, 32 * sizeof(uschar));
            #ifdef SUPPORT_UTF8
                class_utf8 = FALSE;
                class_utf8data = code + LINK_SIZE + 2;
                class_utf8data_base = class_utf8data;
            #endif
                if (c != 0)
                    do {
                        const uschar *oldptr;
                    #ifdef SUPPORT_UTF8
                        if (utf8 && c > 127) { GETCHARLEN(c, ptr, ptr); }
                        if (lengthptr != NULL) {
                            *lengthptr += class_utf8data - class_utf8data_base;
                            class_utf8data = class_utf8data_base;
                        }
                    #endif
                        if (inescq) {
                            if (c == CHAR_BACKSLASH && ptr[1] == CHAR_E) {
                                inescq = FALSE;
                                ptr++;
                                continue;
                            }
                            goto CHECK_RANGE;
                        }
                        if (c == CHAR_LEFT_SQUARE_BRACKET && (ptr[1] == CHAR_COLON || ptr[1] == CHAR_DOT || ptr[1] == CHAR_EQUALS_SIGN) &&
                            check_posix_syntax(ptr, &tempptr)) {
                            BOOL local_negate = FALSE;
                            int posix_class, taboffset, tabopt;
                            register const uschar *cbits = cd->cbits;
                            uschar pbits[32];
                            if (ptr[1] != CHAR_COLON) {
                                *errorcodeptr = ERR31;
                                goto FAILED;
                            }
                            ptr += 2;
                            if (*ptr == CHAR_CIRCUMFLEX_ACCENT) {
                                local_negate = TRUE;
                                should_flip_negation = TRUE;
                                ptr++;
                            }
                            posix_class = check_posix_name(ptr, (int)(tempptr - ptr));
                            if (posix_class < 0) {
                                *errorcodeptr = ERR30;
                                goto FAILED;
                            }
                            if ((options & PCRE_CASELESS) != 0 && posix_class <= 2) posix_class = 0;
                        #ifdef SUPPORT_UCP
                            if ((options & PCRE_UCP) != 0) {
                                int pc = posix_class + ((local_negate)? POSIX_SUBSIZE/2 : 0);
                                if (posix_substitutes[pc] != NULL) {
                                    nestptr = tempptr + 1;
                                    ptr = posix_substitutes[pc] - 1;
                                    continue;
                                }
                            }
                        #endif
                            posix_class *= 3;
                            memcpy(pbits, cbits + posix_class_maps[posix_class],32 * sizeof(uschar));
                            taboffset = posix_class_maps[posix_class + 1];
                            tabopt = posix_class_maps[posix_class + 2];
                            if (taboffset >= 0) {
                                if (tabopt >= 0) for (c = 0; c < 32; c++) pbits[c] |= cbits[c + taboffset];
                                else for (c = 0; c < 32; c++) pbits[c] &= ~cbits[c + taboffset];
                            }
                            if (tabopt < 0) tabopt = -tabopt;
                            if (tabopt == 1) pbits[1] &= ~0x3c;
                            else if (tabopt == 2) pbits[11] &= 0x7f;
                            if (local_negate) for (c = 0; c < 32; c++) classbits[c] |= ~pbits[c];
                            else for (c = 0; c < 32; c++) classbits[c] |= pbits[c];
                            ptr = tempptr + 1;
                            class_charcount = 10;
                            continue;
                        }
                        if (c == CHAR_BACKSLASH) {
                            c = check_escape(&ptr, errorcodeptr, cd->bracount, options, TRUE);
                            if (*errorcodeptr != 0) goto FAILED;
                            if (-c == ESC_b) c = CHAR_BS;
                            else if (-c == ESC_Q) {
                                if (ptr[1] == CHAR_BACKSLASH && ptr[2] == CHAR_E) ptr += 2;
                                else inescq = TRUE;
                                continue;
                            } else if (-c == ESC_E) continue;
                            if (c < 0) {
                                register const uschar *cbits = cd->cbits;
                                class_charcount += 2;
                                switch(-c) {
                                #ifdef SUPPORT_UCP
                                    case ESC_du: case ESC_DU: case ESC_wu: case ESC_WU: case ESC_su: case ESC_SU:
                                        nestptr = ptr;
                                        ptr = substitutes[-c - ESC_DU] - 1;
                                        class_charcount -= 2;
                                        continue;
                                #endif
                                    case ESC_d:
                                        for (c = 0; c < 32; c++) classbits[c] |= cbits[c+cbit_digit];
                                        continue;
                                    case ESC_D:
                                        should_flip_negation = TRUE;
                                        for (c = 0; c < 32; c++) classbits[c] |= ~cbits[c+cbit_digit];
                                        continue;
                                    case ESC_w:
                                        for (c = 0; c < 32; c++) classbits[c] |= cbits[c+cbit_word];
                                        continue;
                                    case ESC_W:
                                        should_flip_negation = TRUE;
                                        for (c = 0; c < 32; c++) classbits[c] |= ~cbits[c+cbit_word];
                                        continue;
                                    case ESC_s:
                                        classbits[0] |= cbits[cbit_space];
                                        classbits[1] |= cbits[cbit_space+1] & ~0x08;
                                        for (c = 2; c < 32; c++) classbits[c] |= cbits[c+cbit_space];
                                        continue;
                                    case ESC_S:
                                        should_flip_negation = TRUE;
                                        for (c = 0; c < 32; c++) classbits[c] |= ~cbits[c+cbit_space];
                                        classbits[1] |= 0x08;    /* Perl 5.004 onwards omits VT from \s */
                                        continue;
                                    case ESC_h:
                                        SETBIT(classbits, 0x09);
                                        SETBIT(classbits, 0x20);
                                        SETBIT(classbits, 0xa0);
                                    #ifdef SUPPORT_UTF8
                                        if (utf8) {
                                            class_utf8 = TRUE;
                                            *class_utf8data++ = XCL_SINGLE;
                                            class_utf8data += _pcre_ord2utf8(0x1680, class_utf8data);
                                            *class_utf8data++ = XCL_SINGLE;
                                            class_utf8data += _pcre_ord2utf8(0x180e, class_utf8data);
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x2000, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x200A, class_utf8data);
                                            *class_utf8data++ = XCL_SINGLE;
                                            class_utf8data += _pcre_ord2utf8(0x202f, class_utf8data);
                                            *class_utf8data++ = XCL_SINGLE;
                                            class_utf8data += _pcre_ord2utf8(0x205f, class_utf8data);
                                            *class_utf8data++ = XCL_SINGLE;
                                            class_utf8data += _pcre_ord2utf8(0x3000, class_utf8data);
                                        }
                                    #endif
                                        continue;
                                    case ESC_H:
                                        for (c = 0; c < 32; c++) {
                                            int x = 0xff;
                                            switch (c) {
                                                case 0x09/8: x ^= 1 << (0x09%8); break;
                                                case 0x20/8: x ^= 1 << (0x20%8); break;
                                                case 0xa0/8: x ^= 1 << (0xa0%8); break;
                                                default: break;
                                            }
                                            classbits[c] |= x;
                                        }
                                    #ifdef SUPPORT_UTF8
                                        if (utf8) {
                                            class_utf8 = TRUE;
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x0100, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x167f, class_utf8data);
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x1681, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x180d, class_utf8data);
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x180f, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x1fff, class_utf8data);
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x200B, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x202e, class_utf8data);
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x2030, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x205e, class_utf8data);
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x2060, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x2fff, class_utf8data);
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x3001, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x7fffffff, class_utf8data);
                                        }
                                    #endif
                                        continue;
                                    case ESC_v:
                                        SETBIT(classbits, 0x0a);
                                        SETBIT(classbits, 0x0b);
                                        SETBIT(classbits, 0x0c);
                                        SETBIT(classbits, 0x0d);
                                        SETBIT(classbits, 0x85);
                                    #ifdef SUPPORT_UTF8
                                        if (utf8) {
                                            class_utf8 = TRUE;
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x2028, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x2029, class_utf8data);
                                        }
                                    #endif
                                        continue;
                                    case ESC_V:
                                        for (c = 0; c < 32; c++) {
                                            int x = 0xff;
                                            switch (c) {
                                                case 0x0a/8:
                                                    x ^= 1 << (0x0a%8);
                                                    x ^= 1 << (0x0b%8);
                                                    x ^= 1 << (0x0c%8);
                                                    x ^= 1 << (0x0d%8);
                                                    break;
                                                case 0x85/8: x ^= 1 << (0x85%8); break;
                                                default: break;
                                            }
                                          classbits[c] |= x;
                                        }
                                    #ifdef SUPPORT_UTF8
                                        if (utf8) {
                                            class_utf8 = TRUE;
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x0100, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x2027, class_utf8data);
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(0x2029, class_utf8data);
                                            class_utf8data += _pcre_ord2utf8(0x7fffffff, class_utf8data);
                                        }
                                    #endif
                                        continue;
                                #ifdef SUPPORT_UCP
                                    case ESC_p: case ESC_P: {
                                            BOOL negated;
                                            int pdata;
                                            int ptype = get_ucp(&ptr, &negated, &pdata, errorcodeptr);
                                            if (ptype < 0) goto FAILED;
                                            class_utf8 = TRUE;
                                            *class_utf8data++ = ((-c == ESC_p) != negated) ? XCL_PROP : XCL_NOTPROP;
                                            *class_utf8data++ = ptype;
                                            *class_utf8data++ = pdata;
                                            class_charcount -= 2;
                                            continue;
                                        }
                                #endif
                                    default:
                                        if ((options & PCRE_EXTRA) != 0) {
                                            *errorcodeptr = ERR7;
                                            goto FAILED;
                                        }
                                        class_charcount -= 2;
                                        c = *ptr;
                                        break;
                                }
                            }
                        }
                        CHECK_RANGE:
                        while(ptr[1] == CHAR_BACKSLASH && ptr[2] == CHAR_E) {
                            inescq = FALSE;
                            ptr += 2;
                        }
                        oldptr = ptr;
                        if (c == CHAR_CR || c == CHAR_NL) cd->external_flags |= PCRE_HASCRORLF;
                        if (!inescq && ptr[1] == CHAR_MINUS) {
                            int d;
                            ptr += 2;
                            while (*ptr == CHAR_BACKSLASH && ptr[1] == CHAR_E) ptr += 2;
                            inescq = FALSE;
                            ptr += 2;
                            while (*ptr == CHAR_BACKSLASH && ptr[1] == CHAR_Q) {
                                ptr += 2;
                                if (*ptr == CHAR_BACKSLASH && ptr[1] == CHAR_E) { ptr += 2; continue; }
                                inescq = TRUE;
                                break;
                            }
                            if (*ptr == 0 || (!inescq && *ptr == CHAR_RIGHT_SQUARE_BRACKET)) {
                                ptr = oldptr;
                                goto LONE_SINGLE_CHARACTER;
                            }
                        #ifdef SUPPORT_UTF8
                            if (utf8) { GETCHARLEN(d, ptr, ptr); }
                            else
                        #endif
                            d = *ptr;
                            if (!inescq && d == CHAR_BACKSLASH) {
                                d = check_escape(&ptr, errorcodeptr, cd->bracount, options, TRUE);
                                if (*errorcodeptr != 0) goto FAILED;
                                if (d < 0) {
                                    if (d == -ESC_b) d = CHAR_BS; else {
                                        ptr = oldptr;
                                        goto LONE_SINGLE_CHARACTER;
                                    }
                                }
                            }
                            if (d < c) {
                                *errorcodeptr = ERR8;
                                goto FAILED;
                            }
                            if (d == c) goto LONE_SINGLE_CHARACTER;
                            if (d == CHAR_CR || d == CHAR_NL) cd->external_flags |= PCRE_HASCRORLF;
                        #ifdef SUPPORT_UTF8
                            if (utf8 && (d > 255 || ((options & PCRE_CASELESS) != 0 && d > 127))) {
                                class_utf8 = TRUE;
                            #ifdef SUPPORT_UCP
                                if ((options & PCRE_CASELESS) != 0) {
                                    unsigned int occ, ocd;
                                    unsigned int cc = c;
                                    unsigned int origd = d;
                                    while (get_othercase_range(&cc, origd, &occ, &ocd)) {
                                        if (occ >= (unsigned int)c && ocd <= (unsigned int)d) continue;
                                        if (occ < (unsigned int)c  && ocd >= (unsigned int)c - 1) {
                                            c = occ;
                                            continue;
                                        }
                                        if (ocd > (unsigned int)d && occ <= (unsigned int)d + 1) {
                                            d = ocd;
                                            continue;
                                        }
                                        if (occ == ocd) *class_utf8data++ = XCL_SINGLE;
                                        else {
                                            *class_utf8data++ = XCL_RANGE;
                                            class_utf8data += _pcre_ord2utf8(occ, class_utf8data);
                                        }
                                        class_utf8data += _pcre_ord2utf8(ocd, class_utf8data);
                                    }
                                }
                            #endif
                                *class_utf8data++ = XCL_RANGE;
                                class_utf8data += _pcre_ord2utf8(c, class_utf8data);
                                class_utf8data += _pcre_ord2utf8(d, class_utf8data);
                            #ifdef SUPPORT_UCP
                                continue;
                            #else
                                if ((options & PCRE_CASELESS) == 0 || c > 127) continue;
                                d = 127;
                            #endif
                            }
                        #endif
                            class_charcount += d - c + 1;
                            class_lastchar = d;
                            if (lengthptr == NULL) for (; c <= d; c++) {
                                classbits[c/8] |= (1 << (c&7));
                                if ((options & PCRE_CASELESS) != 0) {
                                    int uc = cd->fcc[c];
                                    classbits[uc/8] |= (1 << (uc&7));
                                }
                            }
                            continue;
                        }
                        LONE_SINGLE_CHARACTER:
                    #ifdef SUPPORT_UTF8
                        if (utf8 && (c > 255 || ((options & PCRE_CASELESS) != 0 && c > 127))) {
                            class_utf8 = TRUE;
                            *class_utf8data++ = XCL_SINGLE;
                            class_utf8data += _pcre_ord2utf8(c, class_utf8data);
                        #ifdef SUPPORT_UCP
                            if ((options & PCRE_CASELESS) != 0) {
                                unsigned int othercase;
                                if ((othercase = UCD_OTHERCASE(c)) != c) {
                                    *class_utf8data++ = XCL_SINGLE;
                                    class_utf8data += _pcre_ord2utf8(othercase, class_utf8data);
                                }
                            }
                        #endif
                        } else
                    #endif
                        {
                            classbits[c/8] |= (1 << (c&7));
                            if ((options & PCRE_CASELESS) != 0) {
                                c = cd->fcc[c];
                                classbits[c/8] |= (1 << (c&7));
                            }
                            class_charcount++;
                            class_lastchar = c;
                        }
                    } while(((c = *(++ptr)) != 0 || (nestptr != NULL && (ptr = nestptr, nestptr = NULL, c = *(++ptr)) != 0)) &&
                            (c != CHAR_RIGHT_SQUARE_BRACKET || inescq));
                if (c == 0) {
                    *errorcodeptr = ERR6;
                    goto FAILED;
                }
            #ifdef SUPPORT_UTF8
                if (class_charcount == 1 && !class_utf8 && (!utf8 || !negate_class || class_lastchar < 128))
            #else
                if (class_charcount == 1)
            #endif
                {
                    zeroreqbyte = reqbyte;
                    if (negate_class) {
                        if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
                        zerofirstbyte = firstbyte;
                        *code++ = OP_NOT;
                        *code++ = class_lastchar;
                        break;
                    }
                #ifdef SUPPORT_UTF8
                    if (utf8 && class_lastchar > 127) mclength = _pcre_ord2utf8(class_lastchar, mcbuffer);
                    else
                #endif
                    {
                        mcbuffer[0] = class_lastchar;
                        mclength = 1;
                    }
                    goto ONE_CHAR;
                }
                if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
                zerofirstbyte = firstbyte;
                zeroreqbyte = reqbyte;
            #ifdef SUPPORT_UTF8
                if (class_utf8 && (!should_flip_negation || (options & PCRE_UCP) != 0)) {
                    *class_utf8data++ = XCL_END;    /* Marks the end of extra data */
                    *code++ = OP_XCLASS;
                    code += LINK_SIZE;
                    *code = negate_class? XCL_NOT : 0;
                    if (class_charcount > 0) {
                        *code++ |= XCL_MAP;
                        memmove(code + 32, code, class_utf8data - code);
                        memcpy(code, classbits, 32);
                        code = class_utf8data + 32;
                    } else code = class_utf8data;
                    PUT(previous, 1, code - previous);
                    break;
                }
            #endif
                *code++ = (negate_class == should_flip_negation) ? OP_CLASS : OP_NCLASS;
                if (negate_class) {
                    if (lengthptr == NULL) for (c = 0; c < 32; c++) code[c] = ~classbits[c];
                } else memcpy(code, classbits, 32);
                code += 32;
                break;
            case CHAR_LEFT_CURLY_BRACKET:
                if (!is_quantifier) goto NORMAL_CHAR;
                ptr = read_repeat_counts(ptr+1, &repeat_min, &repeat_max, errorcodeptr);
                if (*errorcodeptr != 0) goto FAILED;
                goto REPEAT;
            case CHAR_ASTERISK:
                repeat_min = 0;
                repeat_max = -1;
                goto REPEAT;
            case CHAR_PLUS:
                repeat_min = 1;
                repeat_max = -1;
                goto REPEAT;
            case CHAR_QUESTION_MARK:
                repeat_min = 0;
                repeat_max = 1;
                REPEAT:
                if (previous == NULL) {
                    *errorcodeptr = ERR9;
                    goto FAILED;
                }
                if (repeat_min == 0) {
                    firstbyte = zerofirstbyte;
                    reqbyte = zeroreqbyte;
                }
                reqvary = (repeat_min == repeat_max) ? 0 : REQ_VARY;
                op_type = 0;
                possessive_quantifier = FALSE;
                tempcode = previous;
                if (ptr[1] == CHAR_PLUS) {
                    repeat_type = 0;
                    possessive_quantifier = TRUE;
                    ptr++;
                } else if (ptr[1] == CHAR_QUESTION_MARK) {
                    repeat_type = greedy_non_default;
                    ptr++;
                } else repeat_type = greedy_default;
                if (*previous == OP_CHAR || *previous == OP_CHARNC) {
                #ifdef SUPPORT_UTF8
                    if (utf8 && (code[-1] & 0x80) != 0) {
                        uschar *lastchar = code - 1;
                        while((*lastchar & 0xc0) == 0x80) lastchar--;
                        c = code - lastchar;
                        memcpy(utf8_char, lastchar, c);
                        c |= 0x80;
                    } else
                #endif
                    {
                        c = code[-1];
                        if (repeat_min > 1) reqbyte = c | req_caseopt | cd->req_varyopt;
                    }
                    if (!possessive_quantifier && repeat_max < 0 && check_auto_possessive(previous, utf8, ptr + 1, options, cd)) {
                        repeat_type = 0;
                        possessive_quantifier = TRUE;
                    }
                    goto OUTPUT_SINGLE_REPEAT;
                } else if (*previous == OP_NOT) {
                    op_type = OP_NOTSTAR - OP_STAR;
                    c = previous[1];
                    if (!possessive_quantifier && repeat_max < 0 && check_auto_possessive(previous, utf8, ptr + 1, options, cd)) {
                        repeat_type = 0;
                        possessive_quantifier = TRUE;
                    }
                    goto OUTPUT_SINGLE_REPEAT;
                } else if (*previous < OP_EODN) {
                    uschar *oldcode;
                    int prop_type, prop_value;
                    op_type = OP_TYPESTAR - OP_STAR;
                    c = *previous;
                    if (!possessive_quantifier && repeat_max < 0 && check_auto_possessive(previous, utf8, ptr + 1, options, cd)) {
                        repeat_type = 0;
                        possessive_quantifier = TRUE;
                    }
                    OUTPUT_SINGLE_REPEAT:
                    if (*previous == OP_PROP || *previous == OP_NOTPROP) {
                        prop_type = previous[1];
                        prop_value = previous[2];
                    } else prop_type = prop_value = -1;
                    oldcode = code;
                    code = previous;
                    if (repeat_max == 0) goto END_REPEAT;
                    repeat_type += op_type;
                    if (repeat_min == 0) {
                        if (repeat_max == -1) *code++ = OP_STAR + repeat_type;
                        else if (repeat_max == 1) *code++ = OP_QUERY + repeat_type;
                        else {
                            *code++ = OP_UPTO + repeat_type;
                            PUT2INC(code, 0, repeat_max);
                        }
                    } else if (repeat_min == 1) {
                        if (repeat_max == -1) *code++ = OP_PLUS + repeat_type;
                        else {
                            code = oldcode;
                            if (repeat_max == 1) goto END_REPEAT;
                            *code++ = OP_UPTO + repeat_type;
                            PUT2INC(code, 0, repeat_max - 1);
                        }
                    } else {
                        *code++ = OP_EXACT + op_type;
                        PUT2INC(code, 0, repeat_min);
                        if (repeat_max < 0) {
                        #ifdef SUPPORT_UTF8
                            if (utf8 && c >= 128) {
                                memcpy(code, utf8_char, c & 7);
                                code += c & 7;
                            } else
                        #endif
                            {
                                *code++ = c;
                                if (prop_type >= 0) {
                                    *code++ = prop_type;
                                    *code++ = prop_value;
                                }
                            }
                            *code++ = OP_STAR + repeat_type;
                        } else if (repeat_max != repeat_min) {
                        #ifdef SUPPORT_UTF8
                            if (utf8 && c >= 128) {
                                memcpy(code, utf8_char, c & 7);
                                code += c & 7;
                            } else
                        #endif
                            *code++ = c;
                            if (prop_type >= 0) {
                                *code++ = prop_type;
                                *code++ = prop_value;
                            }
                            repeat_max -= repeat_min;
                            if (repeat_max == 1) *code++ = OP_QUERY + repeat_type;
                            else {
                                *code++ = OP_UPTO + repeat_type;
                                PUT2INC(code, 0, repeat_max);
                            }
                        }
                    }
                #ifdef SUPPORT_UTF8
                    if (utf8 && c >= 128) {
                        memcpy(code, utf8_char, c & 7);
                        code += c & 7;
                    } else
                #endif
                    *code++ = c;
                #ifdef SUPPORT_UCP
                    if (prop_type >= 0) {
                        *code++ = prop_type;
                        *code++ = prop_value;
                    }
                #endif
                } else if (*previous == OP_CLASS || *previous == OP_NCLASS ||
                        #ifdef SUPPORT_UTF8
                           *previous == OP_XCLASS ||
                        #endif
                           *previous == OP_REF) {
                    if (repeat_max == 0) {
                        code = previous;
                        goto END_REPEAT;
                    }
                    if (repeat_min == 0 && repeat_max == -1) *code++ = OP_CRSTAR + repeat_type;
                    else if (repeat_min == 1 && repeat_max == -1) *code++ = OP_CRPLUS + repeat_type;
                    else if (repeat_min == 0 && repeat_max == 1) *code++ = OP_CRQUERY + repeat_type;
                    else {
                    *code++ = OP_CRRANGE + repeat_type;
                    PUT2INC(code, 0, repeat_min);
                    if (repeat_max == -1) repeat_max = 0;
                    PUT2INC(code, 0, repeat_max);
                    }
                } else if (*previous == OP_BRA  || *previous == OP_CBRA || *previous == OP_ONCE || *previous == OP_COND) {
                    register int i;
                    int ketoffset = 0;
                    int len = (int)(code - previous);
                    uschar *bralink = NULL;
                    if (*previous == OP_COND /*&& previous[LINK_SIZE+1]*/ == OP_DEF) {
                        *errorcodeptr = ERR55;
                        goto FAILED;
                    }
                    if (repeat_max == -1) {
                        register uschar *ket = previous;
                        /*do {
                            ket += GET(ket, 1);
                        } while(*ket != OP_KET);*/
                        ketoffset = (int)(code - ket);
                    }
                    if (repeat_min == 0) {
                        if (repeat_max <= 1) {
                            *code = OP_END;
                            adjust_recurse(previous, 1, utf8, cd, save_hwm);
                            memmove(previous+1, previous, len);
                            code++;
                            if (repeat_max == 0) {
                                *previous++ = OP_SKIPZERO;
                                goto END_REPEAT;
                            }
                            *previous++ = OP_BRAZERO + repeat_type;
                        } else {
                            int offset;
                            *code = OP_END;
                            //adjust_recurse(previous, 2 + LINK_SIZE, utf8, cd, save_hwm);
                            //memmove(previous + 2 + LINK_SIZE, previous, len);
                            //code += 2 + LINK_SIZE;
                            *previous++ = OP_BRAZERO + repeat_type;
                            *previous++ = OP_BRA;
                            offset = (bralink == NULL) ? 0 : (int)(previous - bralink);
                            bralink = previous;
                            //PUTINC(previous, 0, offset);
                        }
                        repeat_max--;
                    } else {
                        if (repeat_min > 1) {
                            if (lengthptr != NULL) {
                                int delta = (repeat_min - 1)*length_prevgroup;
                                if ((INT64_OR_DOUBLE)(repeat_min - 1)* (INT64_OR_DOUBLE)length_prevgroup > (INT64_OR_DOUBLE)INT_MAX ||
                                    OFLOW_MAX - *lengthptr < delta) {
                                    *errorcodeptr = ERR20;
                                    goto FAILED;
                                }
                                *lengthptr += delta;
                            } else {
                                if (groupsetfirstbyte && reqbyte < 0) reqbyte = firstbyte;
                                for (i = 1; i < repeat_min; i++) {
                                    uschar *hc;
                                    uschar *this_hwm = cd->hwm;
                                    memcpy(code, previous, len);
                                    /*for (hc = save_hwm; hc < this_hwm; hc += LINK_SIZE) {
                                        PUT(cd->hwm, 0, GET(hc, 0) + len);
                                        cd->hwm += LINK_SIZE;
                                    }*/
                                    save_hwm = this_hwm;
                                    code += len;
                                }
                            }
                        }
                        if (repeat_max > 0) repeat_max -= repeat_min;
                    }
                    if (repeat_max >= 0) {
                        if (lengthptr != NULL && repeat_max > 0) {
                            int delta = repeat_max * (length_prevgroup + 1 + 2 /*+ 2*LINK_SIZE*/) - 2 /*- 2*LINK_SIZE*/;
                            if ((INT64_OR_DOUBLE)repeat_max * (INT64_OR_DOUBLE)(length_prevgroup + 1 + 2 /*+ 2*LINK_SIZE*/) > (INT64_OR_DOUBLE)INT_MAX ||
                                OFLOW_MAX - *lengthptr < delta) {
                                *errorcodeptr = ERR20;
                                goto FAILED;
                            }
                            *lengthptr += delta;
                        } else for (i = repeat_max - 1; i >= 0; i--) {
                            uschar *hc;
                            uschar *this_hwm = cd->hwm;
                            *code++ = OP_BRAZERO + repeat_type;
                            if (i != 0) {
                                int offset;
                                *code++ = OP_BRA;
                                offset = (bralink == NULL)? 0 : (int)(code - bralink);
                                bralink = code;
                                //PUTINC(code, 0, offset);
                            }
                            memcpy(code, previous, len);
                            /*for (hc = save_hwm; hc < this_hwm; hc += LINK_SIZE) {
                                PUT(cd->hwm, 0, GET(hc, 0) + len + ((i != 0)? 2+LINK_SIZE : 1));
                                cd->hwm += LINK_SIZE;
                            }*/
                            save_hwm = this_hwm;
                            code += len;
                        }
                        while (bralink != NULL) {
                            //int oldlinkoffset;
                            int offset = (int)(code - bralink + 1);
                            uschar *bra = code - offset;
                            //oldlinkoffset = GET(bra, 1);
                            //bralink = (oldlinkoffset == 0)? NULL : bralink - oldlinkoffset;
                            *code++ = OP_KET;
                            //PUTINC(code, 0, offset);
                            //PUT(bra, 1, offset);
                        }
                    } else {
                        uschar *ketcode = code - ketoffset;
                        uschar *bracode = ketcode /*- GET(ketcode, 1)*/;
                        *ketcode = OP_KETRMAX + repeat_type;
                        if (lengthptr == NULL && *bracode != OP_ONCE) {
                            uschar *scode = bracode;
                            /*do {
                                if (could_be_empty_branch(scode, ketcode, utf8, cd)) {
                                    *bracode += OP_SBRA - OP_BRA;
                                    break;
                                }
                                scode += GET(scode, 1);
                            } while (*scode == OP_ALT);*/
                        }
                    }
                } else if (*previous == OP_FAIL) goto END_REPEAT;
                else {
                    *errorcodeptr = ERR11;
                    goto FAILED;
                }
                if (possessive_quantifier) {
                    int len;
                    if (*tempcode == OP_TYPEEXACT) tempcode += _pcre_OP_lengths[*tempcode] + ((tempcode[3] == OP_PROP || tempcode[3] == OP_NOTPROP)? 2 : 0);
                    else if (*tempcode == OP_EXACT || *tempcode == OP_NOTEXACT) {
                        tempcode += _pcre_OP_lengths[*tempcode];
                    #ifdef SUPPORT_UTF8
                        if (utf8 && tempcode[-1] >= 0xc0) tempcode += _pcre_utf8_table4[tempcode[-1] & 0x3f];
                    #endif
                    }
                    len = (int)(code - tempcode);
                    if (len > 0)
                        switch (*tempcode) {
                            case OP_STAR:  *tempcode = OP_POSSTAR; break;
                            case OP_PLUS:  *tempcode = OP_POSPLUS; break;
                            case OP_QUERY: *tempcode = OP_POSQUERY; break;
                            case OP_UPTO:  *tempcode = OP_POSUPTO; break;
                            case OP_TYPESTAR:  *tempcode = OP_TYPEPOSSTAR; break;
                            case OP_TYPEPLUS:  *tempcode = OP_TYPEPOSPLUS; break;
                            case OP_TYPEQUERY: *tempcode = OP_TYPEPOSQUERY; break;
                            case OP_TYPEUPTO:  *tempcode = OP_TYPEPOSUPTO; break;
                            case OP_NOTSTAR:  *tempcode = OP_NOTPOSSTAR; break;
                            case OP_NOTPLUS:  *tempcode = OP_NOTPOSPLUS; break;
                            case OP_NOTQUERY: *tempcode = OP_NOTPOSQUERY; break;
                            case OP_NOTUPTO:  *tempcode = OP_NOTPOSUPTO; break;
                            default:
                                *code = OP_END;
                                adjust_recurse(tempcode, 1 /*+ LINK_SIZE*/, utf8, cd, save_hwm);
                                memmove(tempcode + 1/*+LINK_SIZE*/, tempcode, len);
                                code += 1 /*+ LINK_SIZE*/;
                                len += 1 /*+ LINK_SIZE*/;
                                tempcode[0] = OP_ONCE;
                                *code++ = OP_KET;
                                //PUTINC(code, 0, len);
                                //PUT(tempcode, 1, len);
                                break;
                        }
                }
                END_REPEAT:
                previous = NULL;
                cd->req_varyopt |= reqvary;
                break;
            case CHAR_LEFT_PARENTHESIS:
                newoptions = options;
                skipbytes = 0;
                bravalue = OP_CBRA;
                save_hwm = cd->hwm;
                reset_bracount = FALSE;
                if (*(++ptr) == CHAR_ASTERISK && ((cd->ctypes[ptr[1]] & ctype_letter) != 0 || ptr[1] == ':')) {
                    int i, namelen;
                    int arglen = 0;
                    const char *vn = verbnames;
                    const uschar *name = ptr + 1;
                    const uschar *arg = NULL;
                    previous = NULL;
                    while ((cd->ctypes[*++ptr] & ctype_letter) != 0) {};
                    namelen = (int)(ptr - name);
                    if (*ptr == CHAR_COLON) {
                        arg = ++ptr;
                        while ((cd->ctypes[*ptr] & (ctype_letter|ctype_digit)) != 0 || *ptr == '_') ptr++;
                        arglen = (int)(ptr - arg);
                    }
                    if (*ptr != CHAR_RIGHT_PARENTHESIS) {
                        *errorcodeptr = ERR60;
                        goto FAILED;
                    }
                    for (i = 0; i < verbcount; i++) {
                        if (namelen == verbs[i].len && strncmp((char *)name, vn, namelen) == 0) {
                            if (verbs[i].op == OP_ACCEPT) {
                                open_capitem *oc;
                                cd->had_accept = TRUE;
                                for (oc = cd->open_caps; oc != NULL; oc = oc->next) {
                                    *code++ = OP_CLOSE;
                                    PUT2INC(code, 0, oc->number);
                                }
                            }
                            if (arglen == 0) {
                                if (verbs[i].op < 0) {
                                    *errorcodeptr = ERR66;
                                    goto FAILED;
                                }
                                *code = verbs[i].op;
                                /*if (*code++ == OP_THEN) {
                                    PUT(code, 0, code - bcptr->current_branch - 1);
                                    code += LINK_SIZE;
                                }*/
                            } else {
                                if (verbs[i].op_arg < 0) {
                                    *errorcodeptr = ERR59;
                                    goto FAILED;
                                }
                                *code = verbs[i].op_arg;
                                /*if (*code++ == OP_THEN_ARG) {
                                    PUT(code, 0, code - bcptr->current_branch - 1);
                                    code += LINK_SIZE;
                                }*/
                                *code++ = arglen;
                                memcpy(code, arg, arglen);
                                code += arglen;
                                *code++ = 0;
                            }
                            break;
                        }
                        vn += verbs[i].len + 1;
                    }
                    if (i < verbcount) continue;
                    *errorcodeptr = ERR60;
                    goto FAILED;
                } else if (*ptr == CHAR_QUESTION_MARK) {
                    int i, set, unset, namelen;
                    int *optset;
                    const uschar *name;
                    uschar *slot;
                    switch(*(++ptr)) {
                        case CHAR_NUMBER_SIGN:
                            ptr++;
                            while (*ptr != 0 && *ptr != CHAR_RIGHT_PARENTHESIS) ptr++;
                            if (*ptr == 0) {
                                *errorcodeptr = ERR18;
                                goto FAILED;
                            }
                            continue;
                        case CHAR_VERTICAL_LINE: reset_bracount = TRUE;
                        case CHAR_COLON:
                            bravalue = OP_BRA;
                            ptr++;
                            break;
                        case CHAR_LEFT_PARENTHESIS:
                            bravalue = OP_COND;
                            if (ptr[1] == CHAR_QUESTION_MARK && (ptr[2] == CHAR_EQUALS_SIGN || ptr[2] == CHAR_EXCLAMATION_MARK || ptr[2] == CHAR_LESS_THAN_SIGN))
                                break;
                            //code[1+LINK_SIZE] = OP_CREF;
                            skipbytes = 3;
                            refsign = -1;
                            if (ptr[1] == CHAR_R && ptr[2] == CHAR_AMPERSAND) {
                                terminator = -1;
                                ptr += 2;
                                //code[1+LINK_SIZE] = OP_RREF;
                            } else if (ptr[1] == CHAR_LESS_THAN_SIGN) {
                                terminator = CHAR_GREATER_THAN_SIGN;
                                ptr++;
                            } else if (ptr[1] == CHAR_APOSTROPHE) {
                                terminator = CHAR_APOSTROPHE;
                                ptr++;
                            } else {
                                terminator = 0;
                                if (ptr[1] == CHAR_MINUS || ptr[1] == CHAR_PLUS) refsign = *(++ptr);
                            }
                            if ((cd->ctypes[ptr[1]] & ctype_word) == 0) {
                                ptr += 1;
                                *errorcodeptr = ERR28;
                                goto FAILED;
                            }
                            recno = 0;
                            name = ++ptr;
                            while ((cd->ctypes[*ptr] & ctype_word) != 0) {
                                if (recno >= 0) recno = (g_ascii_isdigit(*ptr) != 0) ? recno * 10 + *ptr - CHAR_0 : -1;
                                ptr++;
                            }
                            namelen = (int)(ptr - name);
                            if ((terminator > 0 && *ptr++ != terminator) || *ptr++ != CHAR_RIGHT_PARENTHESIS) {
                                ptr--;
                                *errorcodeptr = ERR26;
                                goto FAILED;
                            }
                            if (lengthptr != NULL) break;
                            if (refsign > 0) {
                                if (recno <= 0) {
                                    *errorcodeptr = ERR58;
                                    goto FAILED;
                                }
                                recno = (refsign == CHAR_MINUS) ? cd->bracount - recno + 1 : recno +cd->bracount;
                                if (recno <= 0 || recno > cd->final_bracount) {
                                    *errorcodeptr = ERR15;
                                    goto FAILED;
                                }
                                //PUT2(code, 2+LINK_SIZE, recno);
                                break;
                            }
                            slot = cd->name_table;
                            for (i = 0; i < cd->names_found; i++) {
                                if (strncmp((char *)name, (char *)slot+2, namelen) == 0) break;
                                slot += cd->name_entry_size;
                            }
                            if (i < cd->names_found) {
                                recno = GET2(slot, 0);
                                //PUT2(code, 2+LINK_SIZE, recno);
                                // code[1+LINK_SIZE]++;
                            } else if ((i = find_parens(cd, name, namelen,(options & PCRE_EXTENDED) != 0, utf8)) > 0) {
                                //PUT2(code, 2+LINK_SIZE, i);
                                //code[1+LINK_SIZE]++;
                            } else if (terminator != 0) {
                                *errorcodeptr = ERR15;
                                goto FAILED;
                            } else if (*name == CHAR_R) {
                                recno = 0;
                                for (i = 1; i < namelen; i++) {
                                    if (g_ascii_isdigit(name[i]) == 0) {
                                        *errorcodeptr = ERR15;
                                        goto FAILED;
                                    }
                                    recno = recno * 10 + name[i] - CHAR_0;
                                }
                                if (recno == 0) recno = RREF_ANY;
                                //code[1+LINK_SIZE] = OP_RREF;      /* Change test type */
                                //PUT2(code, 2+LINK_SIZE, recno);
                            } else if (namelen == 6 && strncmp((char *)name, STRING_DEFINE, 6) == 0) {
                                //code[1+LINK_SIZE] = OP_DEF;
                                skipbytes = 1;
                            } else if (recno > 0 && recno <= cd->final_bracount) {
                                //PUT2(code, 2+LINK_SIZE, recno);
                            } else {
                                *errorcodeptr = (recno == 0)? ERR35: ERR15;
                                goto FAILED;
                            }
                            break;
                        case CHAR_EQUALS_SIGN:
                            bravalue = OP_ASSERT;
                            ptr++;
                            break;
                        case CHAR_EXCLAMATION_MARK:
                            ptr++;
                            if (*ptr == CHAR_RIGHT_PARENTHESIS) {
                                *code++ = OP_FAIL;
                                previous = NULL;
                                continue;
                            }
                            bravalue = OP_ASSERT_NOT;
                            break;
                        case CHAR_LESS_THAN_SIGN:
                            switch(ptr[1]) {
                                case CHAR_EQUALS_SIGN:
                                    bravalue = OP_ASSERTBACK;
                                    ptr += 2;
                                    break;
                                case CHAR_EXCLAMATION_MARK:
                                    bravalue = OP_ASSERTBACK_NOT;
                                    ptr += 2;
                                    break;
                                default:
                                    if ((cd->ctypes[ptr[1]] & ctype_word) != 0) goto DEFINE_NAME;
                                    ptr++;
                                    *errorcodeptr = ERR24;
                                    goto FAILED;
                            }
                            break;
                        case CHAR_GREATER_THAN_SIGN:
                            bravalue = OP_ONCE;
                            ptr++;
                            break;
                        case CHAR_C:
                            previous_callout = code;
                            after_manual_callout = 1;
                            *code++ = OP_CALLOUT;
                            {
                                int n = 0;
                                while(g_ascii_isdigit(*(++ptr)) != 0) n = n * 10 + *ptr - CHAR_0;
                                if (*ptr != CHAR_RIGHT_PARENTHESIS) {
                                    *errorcodeptr = ERR39;
                                    goto FAILED;
                                }
                                if (n > 255) {
                                    *errorcodeptr = ERR38;
                                    goto FAILED;
                                }
                                *code++ = n;
                                //PUT(code, 0, (int)(ptr - cd->start_pattern + 1));
                                //PUT(code, LINK_SIZE, 0);
                                //code += 2 * LINK_SIZE;
                            }
                            previous = NULL;
                            continue;
                        case CHAR_P:
                            if (*(++ptr) == CHAR_EQUALS_SIGN || *ptr == CHAR_GREATER_THAN_SIGN) {
                                is_recurse = *ptr == CHAR_GREATER_THAN_SIGN;
                                terminator = CHAR_RIGHT_PARENTHESIS;
                                goto NAMED_REF_OR_RECURSE;
                            } else if (*ptr != CHAR_LESS_THAN_SIGN) {
                                *errorcodeptr = ERR41;
                                goto FAILED;
                            }
                        DEFINE_NAME:
                        case CHAR_APOSTROPHE: {
                                terminator = (*ptr == CHAR_LESS_THAN_SIGN) ? CHAR_GREATER_THAN_SIGN : CHAR_APOSTROPHE;
                                name = ++ptr;
                                while ((cd->ctypes[*ptr] & ctype_word) != 0) ptr++;
                                namelen = (int)(ptr - name);
                                if (lengthptr != NULL) {
                                    if (*ptr != terminator) {
                                        *errorcodeptr = ERR42;
                                        goto FAILED;
                                    }
                                    /*if (cd->names_found >= MAX_NAME_COUNT) {
                                        *errorcodeptr = ERR49;
                                        goto FAILED;
                                    }*/
                                    if (namelen + 3 > cd->name_entry_size) {
                                        cd->name_entry_size = namelen + 3;
                                        /*if (namelen > MAX_NAME_SIZE) {
                                            *errorcodeptr = ERR48;
                                            goto FAILED;
                                        }*/
                                    }
                                } else {
                                    BOOL dupname = FALSE;
                                    slot = cd->name_table;
                                    for (i = 0; i < cd->names_found; i++) {
                                        int crc = memcmp(name, slot+2, namelen);
                                        if (crc == 0) {
                                            if (slot[2+namelen] == 0) {
                                                if (GET2(slot, 0) != cd->bracount + 1 && (options & PCRE_DUPNAMES) == 0) {
                                                    *errorcodeptr = ERR43;
                                                    goto FAILED;
                                                } else dupname = TRUE;
                                            } else crc = -1;
                                        }
                                        if (crc < 0) {
                                            memmove(slot + cd->name_entry_size, slot,(cd->names_found - i) * cd->name_entry_size);
                                            break;
                                        }
                                        slot += cd->name_entry_size;
                                    }
                                    if (!dupname) {
                                        uschar *cslot = cd->name_table;
                                        for (i = 0; i < cd->names_found; i++) {
                                            if (cslot != slot) {
                                                if (GET2(cslot, 0) == cd->bracount + 1) {
                                                    *errorcodeptr = ERR65;
                                                    goto FAILED;
                                                }
                                            } else i--;
                                            cslot += cd->name_entry_size;
                                        }
                                    }
                                    PUT2(slot, 0, cd->bracount + 1);
                                    memcpy(slot + 2, name, namelen);
                                    slot[2+namelen] = 0;
                                }
                            }
                            cd->names_found++;
                            ptr++;
                            goto NUMBERED_GROUP;
                        case CHAR_AMPERSAND:
                            terminator = CHAR_RIGHT_PARENTHESIS;
                            is_recurse = TRUE;
                            NAMED_REF_OR_RECURSE:
                            name = ++ptr;
                            while((cd->ctypes[*ptr] & ctype_word) != 0) ptr++;
                            namelen = (int)(ptr - name);
                            if (lengthptr != NULL) {
                                const uschar *temp;
                                if (namelen == 0) {
                                    *errorcodeptr = ERR62;
                                    goto FAILED;
                                }
                                if (*ptr != terminator) {
                                    *errorcodeptr = ERR42;
                                    goto FAILED;
                                }
                                /*if (namelen > MAX_NAME_SIZE) {
                                    *errorcodeptr = ERR48;
                                    goto FAILED;
                                }*/
                                temp = cd->end_pattern;
                                cd->end_pattern = ptr;
                                recno = find_parens(cd, name, namelen,(options & PCRE_EXTENDED) != 0, utf8);
                                cd->end_pattern = temp;
                                if (recno < 0) recno = 0;
                            } else {
                                slot = cd->name_table;
                                for (i = 0; i < cd->names_found; i++) {
                                    if (strncmp((char*)name, (char*)slot+2, namelen) == 0 && slot[2+namelen] == 0) break;
                                    slot += cd->name_entry_size;
                                }
                                if (i < cd->names_found) recno = GET2(slot, 0);
                                else if ((recno = find_parens(cd, name, namelen,(options & PCRE_EXTENDED) != 0, utf8)) <= 0) {
                                    *errorcodeptr = ERR15;
                                    goto FAILED;
                                }
                            }
                            if (is_recurse) goto HANDLE_RECURSION;
                            else goto HANDLE_REFERENCE;
                        case CHAR_R: ptr++;
                        case CHAR_MINUS: case CHAR_PLUS: case CHAR_0: case CHAR_1: case CHAR_2: case CHAR_3: case CHAR_4: case CHAR_5: case CHAR_6:
                        case CHAR_7: case CHAR_8: case CHAR_9: {
                                const uschar *called;
                                terminator = CHAR_RIGHT_PARENTHESIS;
                                HANDLE_NUMERICAL_RECURSION:
                                if ((refsign = *ptr) == CHAR_PLUS) {
                                    ptr++;
                                    if (g_ascii_isdigit(*ptr) == 0) {
                                        *errorcodeptr = ERR63;
                                        goto FAILED;
                                    }
                                } else if (refsign == CHAR_MINUS) {
                                    if (g_ascii_isdigit(ptr[1]) == 0) goto OTHER_CHAR_AFTER_QUERY;
                                    ptr++;
                                }
                                recno = 0;
                                while(g_ascii_isdigit(*ptr) != 0) recno = recno * 10 + *ptr++ - CHAR_0;
                                if (*ptr != terminator) {
                                    *errorcodeptr = ERR29;
                                    goto FAILED;
                                }
                                if (refsign == CHAR_MINUS) {
                                    if (recno == 0) {
                                        *errorcodeptr = ERR58;
                                        goto FAILED;
                                    }
                                    recno = cd->bracount - recno + 1;
                                    if (recno <= 0) {
                                        *errorcodeptr = ERR15;
                                        goto FAILED;
                                    }
                                } else if (refsign == CHAR_PLUS) {
                                    if (recno == 0) {
                                        *errorcodeptr = ERR58;
                                        goto FAILED;
                                    }
                                    recno += cd->bracount;
                                }
                                HANDLE_RECURSION:
                                previous = code;
                                called = cd->start_code;
                                if (lengthptr == NULL) {
                                    *code = OP_END;
                                    if (recno != 0) called = _pcre_find_bracket(cd->start_code, utf8, recno);
                                    if (called == NULL) {
                                        if (find_parens(cd, NULL, recno,(options & PCRE_EXTENDED) != 0, utf8) < 0) {
                                            *errorcodeptr = ERR15;
                                            goto FAILED;
                                        }
                                        called = cd->start_code + recno;
                                        //PUTINC(cd->hwm, 0, (int)(code + 2 + LINK_SIZE - cd->start_code));
                                    }/* else if (GET(called, 1) == 0 && could_be_empty(called, code, bcptr, utf8, cd)) {
                                        *errorcodeptr = ERR40;
                                        goto FAILED;
                                    }*/
                                }
                                *code = OP_ONCE;
                                //PUT(code, 1, 2 + 2*LINK_SIZE);
                                code += 1 /*+ LINK_SIZE*/;
                                *code = OP_RECURSE;
                                //PUT(code, 1, (int)(called - cd->start_code));
                                code += 1 /*+ LINK_SIZE*/;
                                *code = OP_KET;
                                //PUT(code, 1, 2 + 2*LINK_SIZE);
                                code += 1 /*+ LINK_SIZE*/;
                                length_prevgroup = 3 /*+ 3*LINK_SIZE*/;
                            }
                            if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
                            continue;
                        default:
                            OTHER_CHAR_AFTER_QUERY:
                            set = unset = 0;
                            optset = &set;
                            while (*ptr != CHAR_RIGHT_PARENTHESIS && *ptr != CHAR_COLON) {
                                switch (*ptr++) {
                                    case CHAR_MINUS: optset = &unset; break;
                                    case CHAR_J:
                                        *optset |= PCRE_DUPNAMES;
                                        cd->external_flags |= PCRE_JCHANGED;
                                        break;
                                    case CHAR_i: *optset |= PCRE_CASELESS; break;
                                    case CHAR_m: *optset |= PCRE_MULTILINE; break;
                                    case CHAR_s: *optset |= PCRE_DOTALL; break;
                                    case CHAR_x: *optset |= PCRE_EXTENDED; break;
                                    case CHAR_U: *optset |= PCRE_UNGREEDY; break;
                                    case CHAR_X: *optset |= PCRE_EXTRA; break;
                                    default:
                                        *errorcodeptr = ERR12;
                                        ptr--;
                                        goto FAILED;
                                }
                            }
                            newoptions = (options | set) & (~unset);
                            if (*ptr == CHAR_RIGHT_PARENTHESIS) {
                                if (code == cd->start_code + 1 /*+ LINK_SIZE*/ && (lengthptr == NULL || *lengthptr == 2 /*+ 2*LINK_SIZE*/)) {
                                    cd->external_options = newoptions;
                                } else {
                                    if ((options & PCRE_IMS) != (newoptions & PCRE_IMS)) {
                                        *code++ = OP_OPT;
                                        *code++ = newoptions & PCRE_IMS;
                                    }
                                    greedy_default = ((newoptions & PCRE_UNGREEDY) != 0);
                                    greedy_non_default = greedy_default ^ 1;
                                    req_caseopt = ((newoptions & PCRE_CASELESS) != 0)? REQ_CASELESS : 0;
                                }
                                *optionsptr = options = newoptions;
                                previous = NULL;
                                continue;
                            }
                            bravalue = OP_BRA;
                            ptr++;
                    }
                } else if ((options & PCRE_NO_AUTO_CAPTURE) != 0) bravalue = OP_BRA;
                else {
                    NUMBERED_GROUP:
                    cd->bracount += 1;
                    //PUT2(code, 1+LINK_SIZE, cd->bracount);
                    skipbytes = 2;
                }
                previous = (bravalue >= OP_ONCE)? code : NULL;
                *code = bravalue;
                tempcode = code;
                tempreqvary = cd->req_varyopt;
                length_prevgroup = 0;
                if (!compile_regex(newoptions,options & PCRE_IMS, &tempcode, &ptr, errorcodeptr,(bravalue == OP_ASSERTBACK ||
                    bravalue == OP_ASSERTBACK_NOT), reset_bracount, skipbytes, &subfirstbyte, &subreqbyte, bcptr, cd,
                    (lengthptr == NULL) ? NULL : &length_prevgroup))
                    goto FAILED;
                if (bravalue == OP_COND && lengthptr == NULL) {
                    uschar *tc = code;
                    int condcount = 0;
                    /*do {
                        condcount++;
                        tc += GET(tc,1);
                    } while (*tc != OP_KET);
                    if (code[LINK_SIZE+1] == OP_DEF) {
                        if (condcount > 1) {
                            *errorcodeptr = ERR54;
                            goto FAILED;
                        }
                        bravalue = OP_DEF;
                    } else {
                    if (condcount > 2) {
                        *errorcodeptr = ERR27;
                        goto FAILED;
                    }*/
                    if (condcount == 1) subfirstbyte = subreqbyte = REQ_NONE;
                }
                if (*ptr != CHAR_RIGHT_PARENTHESIS) {
                    *errorcodeptr = ERR14;
                    goto FAILED;
                }
                if (lengthptr != NULL) {
                    if (OFLOW_MAX - *lengthptr < length_prevgroup - 2 /*- 2*LINK_SIZE*/) {
                        *errorcodeptr = ERR20;
                        goto FAILED;
                    }
                    *lengthptr += length_prevgroup - 2 /*- 2*LINK_SIZE*/;
                    *code++ = OP_BRA;
                    //PUTINC(code, 0, 1 + LINK_SIZE);
                    *code++ = OP_KET;
                    //PUTINC(code, 0, 1 + LINK_SIZE);
                    break;
                }
                code = tempcode;
                if (bravalue == OP_DEF) break;
                zeroreqbyte = reqbyte;
                zerofirstbyte = firstbyte;
                groupsetfirstbyte = FALSE;
                if (bravalue >= OP_ONCE) {
                    if (firstbyte == REQ_UNSET) {
                        if (subfirstbyte >= 0) {
                            firstbyte = subfirstbyte;
                            groupsetfirstbyte = TRUE;
                        } else firstbyte = REQ_NONE;
                        zerofirstbyte = REQ_NONE;
                    } else if (subfirstbyte >= 0 && subreqbyte < 0) subreqbyte = subfirstbyte | tempreqvary;
                    if (subreqbyte >= 0) reqbyte = subreqbyte;
                } else if (bravalue == OP_ASSERT && subreqbyte >= 0) reqbyte = subreqbyte;
                break;
            case CHAR_BACKSLASH:
                tempptr = ptr;
                c = check_escape(&ptr, errorcodeptr, cd->bracount, options, FALSE);
                if (*errorcodeptr != 0) goto FAILED;
                if (c < 0) {
                    if (-c == ESC_Q) {
                        if (ptr[1] == CHAR_BACKSLASH && ptr[2] == CHAR_E) ptr += 2;
                        else inescq = TRUE;
                        continue;
                    }
                    if (-c == ESC_E) continue;
                    if (firstbyte == REQ_UNSET && -c > ESC_b && -c < ESC_Z) firstbyte = REQ_NONE;
                    zerofirstbyte = firstbyte;
                    zeroreqbyte = reqbyte;
                    if (-c == ESC_g) {
                    const uschar *p;
                    save_hwm = cd->hwm;
                    terminator = (*(++ptr) == CHAR_LESS_THAN_SIGN) ? CHAR_GREATER_THAN_SIGN : CHAR_APOSTROPHE;
                    skipbytes = 0;
                    reset_bracount = FALSE;
                    if (ptr[1] != CHAR_PLUS && ptr[1] != CHAR_MINUS) {
                        BOOL isnumber = TRUE;
                        for (p = ptr + 1; *p != 0 && *p != terminator; p++) {
                            if ((cd->ctypes[*p] & ctype_digit) == 0) isnumber = FALSE;
                            if ((cd->ctypes[*p] & ctype_word) == 0) break;
                        }
                        if (*p != terminator) {
                            *errorcodeptr = ERR57;
                            break;
                        }
                        if (isnumber) {
                            ptr++;
                            goto HANDLE_NUMERICAL_RECURSION;
                        }
                        is_recurse = TRUE;
                        goto NAMED_REF_OR_RECURSE;
                    }
                    p = ptr + 2;
                    while (g_ascii_isdigit(*p) != 0) p++;
                    if (*p != terminator) {
                        *errorcodeptr = ERR57;
                        break;
                    }
                    ptr++;
                    goto HANDLE_NUMERICAL_RECURSION;
                    }
                    if (-c == ESC_k && (ptr[1] == CHAR_LESS_THAN_SIGN || ptr[1] == CHAR_APOSTROPHE || ptr[1] == CHAR_LEFT_CURLY_BRACKET)) {
                        is_recurse = FALSE;
                        terminator = (*(++ptr) == CHAR_LESS_THAN_SIGN) ? CHAR_GREATER_THAN_SIGN : (*ptr == CHAR_APOSTROPHE) ? CHAR_APOSTROPHE :
                                     CHAR_RIGHT_CURLY_BRACKET;
                        goto NAMED_REF_OR_RECURSE;
                    }
                    if (-c >= ESC_REF) {
                        open_capitem *oc;
                        recno = -c - ESC_REF;
                        HANDLE_REFERENCE:
                        if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
                        previous = code;
                        *code++ = OP_REF;
                        PUT2INC(code, 0, recno);
                        cd->backref_map |= (recno < 32)? (1 << recno) : 1;
                        if (recno > cd->top_backref) cd->top_backref = recno;
                        for (oc = cd->open_caps; oc != NULL; oc = oc->next) {
                            if (oc->number == recno) {
                                oc->flag = TRUE;
                                break;
                            }
                        }
                    }
                #ifdef SUPPORT_UCP
                    else if (-c == ESC_P || -c == ESC_p) {
                        BOOL negated;
                        int pdata;
                        int ptype = get_ucp(&ptr, &negated, &pdata, errorcodeptr);
                        if (ptype < 0) goto FAILED;
                        previous = code;
                        *code++ = ((-c == ESC_p) != negated)? OP_PROP : OP_NOTPROP;
                        *code++ = ptype;
                        *code++ = pdata;
                    }
                #else
                     else if (-c == ESC_X || -c == ESC_P || -c == ESC_p) {
                        *errorcodeptr = ERR45;
                        goto FAILED;
                     }
                #endif
                     else {
                     #ifdef SUPPORT_UCP
                         if (-c >= ESC_DU && -c <= ESC_wu) {
                             nestptr = ptr + 1;
                             ptr = substitutes[-c - ESC_DU] - 1;
                         } else
                     #endif
                         {
                             previous = (-c > ESC_b && -c < ESC_Z)? code : NULL;
                             *code++ = -c;
                         }
                     }
                    continue;
                }
            #ifdef SUPPORT_UTF8
                if (utf8 && c > 127) mclength = _pcre_ord2utf8(c, mcbuffer);
                else
            #endif
                {
                    mcbuffer[0] = c;
                    mclength = 1;
                }
                goto ONE_CHAR;
                default:
                NORMAL_CHAR:
                mclength = 1;
                mcbuffer[0] = c;
            #ifdef SUPPORT_UTF8
                if (utf8 && c >= 0xc0) {
                    while((ptr[1] & 0xc0) == 0x80) mcbuffer[mclength++] = *(++ptr);
                }
            #endif
                ONE_CHAR:
                previous = code;
                *code++ = ((options & PCRE_CASELESS) != 0)? OP_CHARNC : OP_CHAR;
                for (c = 0; c < mclength; c++) *code++ = mcbuffer[c];
                if (mcbuffer[0] == CHAR_CR || mcbuffer[0] == CHAR_NL) cd->external_flags |= PCRE_HASCRORLF;
                if (firstbyte == REQ_UNSET) {
                    zerofirstbyte = REQ_NONE;
                    zeroreqbyte = reqbyte;
                    if (mclength == 1 || req_caseopt == 0) {
                        firstbyte = mcbuffer[0] | req_caseopt;
                        if (mclength != 1) reqbyte = code[-1] | cd->req_varyopt;
                    } else firstbyte = reqbyte = REQ_NONE;
                } else {
                    zerofirstbyte = firstbyte;
                    zeroreqbyte = reqbyte;
                    if (mclength == 1 || req_caseopt == 0) reqbyte = code[-1] | req_caseopt | cd->req_varyopt;
                }
                break;
        }
    }
    FAILED:
    *ptrptr = ptr;
    return FALSE;
}
static BOOL compile_regex(int options, int oldims, uschar **codeptr, const uschar **ptrptr, int *errorcodeptr, BOOL lookbehind, BOOL reset_bracount,
                          int skipbytes, int *firstbyteptr, int *reqbyteptr, branch_chain *bcptr, compile_data *cd, int *lengthptr) {
    const uschar *ptr = *ptrptr;
    uschar *code = *codeptr;
    uschar *last_branch = code;
    uschar *start_bracket = code;
    uschar *reverse_count = NULL;
    open_capitem capitem;
    int capnumber = 0;
    int firstbyte, reqbyte;
    int branchfirstbyte, branchreqbyte;
    int length;
    int orig_bracount;
    int max_bracount;
    int old_external_options = cd->external_options;
    branch_chain bc;
    bc.outer = bcptr;
    bc.current_branch = code;
    firstbyte = reqbyte = REQ_UNSET;
    length = 2 + /*2*LINK_SIZE +*/ skipbytes;
    if (*code == OP_CBRA) {
        capnumber = GET2(code, 1 /*+ LINK_SIZE*/);
        capitem.number = capnumber;
        capitem.next = cd->open_caps;
        capitem.flag = FALSE;
        cd->open_caps = &capitem;
    }
    //PUT(code, 1, 0);
    code += 1 + /*LINK_SIZE +*/ skipbytes;
    orig_bracount = max_bracount = cd->bracount;
    for (;;) {
        if (reset_bracount) cd->bracount = orig_bracount;
        if ((options & PCRE_IMS) != oldims) {
            *code++ = OP_OPT;
            *code++ = options & PCRE_IMS;
            length += 2;
        }
        if (lookbehind) {
            *code++ = OP_REVERSE;
            reverse_count = code;
            //PUTINC(code, 0, 0);
            length += 1 /*+ LINK_SIZE*/;
        }
        if (!compile_branch(&options, &code, &ptr, errorcodeptr, &branchfirstbyte,&branchreqbyte, &bc, cd, (lengthptr == NULL)? NULL : &length)) {
            *ptrptr = ptr;
            return FALSE;
        }
        if (old_external_options != cd->external_options) oldims = cd->external_options & PCRE_IMS;
        if (cd->bracount > max_bracount) max_bracount = cd->bracount;
        if (lengthptr == NULL) {
            if (*last_branch != OP_ALT) {
                firstbyte = branchfirstbyte;
                reqbyte = branchreqbyte;
            } else {
                if (firstbyte >= 0 && firstbyte != branchfirstbyte) {
                    if (reqbyte < 0) reqbyte = firstbyte;
                    firstbyte = REQ_NONE;
                }
                if (firstbyte < 0 && branchfirstbyte >= 0 && branchreqbyte < 0) branchreqbyte = branchfirstbyte;
                if ((reqbyte & ~REQ_VARY) != (branchreqbyte & ~REQ_VARY)) reqbyte = REQ_NONE;
                else reqbyte |= branchreqbyte;
            }
            if (lookbehind) {
                int fixed_length;
                *code = OP_END;
                fixed_length = find_fixedlength(last_branch, options, FALSE, cd);
                DPRINTF(("fixed length = %d\n", fixed_length));
                if (fixed_length == -3) cd->check_lookbehind = TRUE;
                else if (fixed_length < 0) {
                    *errorcodeptr = (fixed_length == -2)? ERR36 : ERR25;
                    *ptrptr = ptr;
                    return FALSE;
                } //else { PUT(reverse_count, 0, fixed_length); }
            }
        }
        if (*ptr != CHAR_VERTICAL_LINE) {
            if (lengthptr == NULL) {
                int branch_length = (int)(code - last_branch);
                /*do {
                    int prev_length = GET(last_branch, 1);
                    PUT(last_branch, 1, branch_length);
                    branch_length = prev_length;
                    last_branch -= branch_length;
                } while (branch_length > 0);*/
            }
            *code = OP_KET;
            //PUT(code, 1, (int)(code - start_bracket));
            code += 1 /*+ LINK_SIZE*/;
            if (capnumber > 0) {
                if (cd->open_caps->flag) {
                    memmove(start_bracket + 1 /*+ LINK_SIZE*/, start_bracket,code - start_bracket);
                    *start_bracket = OP_ONCE;
                    code += 1 /*+ LINK_SIZE*/;
                    //PUT(start_bracket, 1, (int)(code - start_bracket));
                    *code = OP_KET;
                    //PUT(code, 1, (int)(code - start_bracket));
                    code += 1 /*+ LINK_SIZE*/;
                    length += 2 /*+ 2*LINK_SIZE*/;
                }
                cd->open_caps = cd->open_caps->next;
            }
            if ((options & PCRE_IMS) != oldims && *ptr == CHAR_RIGHT_PARENTHESIS) {
                *code++ = OP_OPT;
                *code++ = oldims;
                length += 2;
            }
            cd->bracount = max_bracount;
            *codeptr = code;
            *ptrptr = ptr;
            *firstbyteptr = firstbyte;
            *reqbyteptr = reqbyte;
            if (lengthptr != NULL) {
                if (OFLOW_MAX - *lengthptr < length) {
                    *errorcodeptr = ERR20;
                    return FALSE;
                }
                *lengthptr += length;
            }
            return TRUE;
        }
        if (lengthptr != NULL) {
        code = *codeptr + 1 /*+ LINK_SIZE*/ + skipbytes;
        length += 1 /*+ LINK_SIZE*/;
        } else {
        *code = OP_ALT;
        //PUT(code, 1, (int)(code - last_branch));
        bc.current_branch = last_branch = code;
        code += 1 /*+ LINK_SIZE*/;
        }
        ptr++;
    }
}
static BOOL is_anchored(register const uschar *code, int *options, unsigned int bracket_map, unsigned int backref_map) {
    /*do {
        const uschar *scode = first_significant_code(code + _pcre_OP_lengths[*code], options, PCRE_MULTILINE, FALSE);
        register int op = *scode;
        if (op == OP_BRA) {
            if (!is_anchored(scode, options, bracket_map, backref_map)) return FALSE;
        } else if (op == OP_CBRA) {
            int n = GET2(scode, 1+LINK_SIZE);
            int new_map = bracket_map | ((n < 32)? (1 << n) : 1);
            if (!is_anchored(scode, options, new_map, backref_map)) return FALSE;
        } else if (op == OP_ASSERT || op == OP_ONCE || op == OP_COND) {
            if (!is_anchored(scode, options, bracket_map, backref_map)) return FALSE;
        } else if ((op == OP_TYPESTAR || op == OP_TYPEMINSTAR || op == OP_TYPEPOSSTAR)) {
            if (scode[1] != OP_ALLANY || (bracket_map & backref_map) != 0) return FALSE;
        } else if (op != OP_SOD && op != OP_SOM && ((*options & PCRE_MULTILINE) != 0 || op != OP_CIRC)) return FALSE;
        code += GET(code, 1);
    } while(*code == OP_ALT);*/
    return TRUE;
}
static BOOL is_startline(const uschar *code, unsigned int bracket_map, unsigned int backref_map) {
    /*do {
    const uschar *scode = first_significant_code(code + _pcre_OP_lengths[*code],NULL, 0, FALSE);
    register int op = *scode;
    if (op == OP_COND) {
        scode += 1 + LINK_SIZE;
        if (*scode == OP_CALLOUT) scode += _pcre_OP_lengths[OP_CALLOUT];
        switch (*scode) {
            case OP_CREF: case OP_NCREF: case OP_RREF: case OP_NRREF: case OP_DEF: return FALSE;
            default:
                if (!is_startline(scode, bracket_map, backref_map)) return FALSE;
                do scode += GET(scode, 1); while (*scode == OP_ALT);
                scode += 1 + LINK_SIZE;
                break;
        }
        scode = first_significant_code(scode, NULL, 0, FALSE);
        op = *scode;
    }
    if (op == OP_BRA) {
        if (!is_startline(scode, bracket_map, backref_map)) return FALSE;
    } else if (op == OP_CBRA) {
        int n = GET2(scode, 1+LINK_SIZE);
        int new_map = bracket_map | ((n < 32)? (1 << n) : 1);
        if (!is_startline(scode, new_map, backref_map)) return FALSE;
        } else if (op == OP_ASSERT || op == OP_ONCE) {
            if (!is_startline(scode, bracket_map, backref_map)) return FALSE;
        } else if (op == OP_TYPESTAR || op == OP_TYPEMINSTAR || op == OP_TYPEPOSSTAR) {
            if (scode[1] != OP_ANY || (bracket_map & backref_map) != 0) return FALSE;
        } else if (op != OP_CIRC) return FALSE;
        code += GET(code, 1);
    } while (*code == OP_ALT);*/
    return TRUE;
}
static int find_firstassertedchar(const uschar *code, int *options, BOOL inassert) {
    register int c = -1;
    /*do {
        int d;
        const uschar *scode = first_significant_code(code + 1+LINK_SIZE, options, PCRE_CASELESS, TRUE);
        register int op = *scode;
        switch(op) {
            case OP_BRA: case OP_CBRA: case OP_ASSERT: case OP_ONCE: case OP_COND:
            if ((d = find_firstassertedchar(scode, options, op == OP_ASSERT)) < 0) return -1;
            if (c < 0) c = d; else if (c != d) return -1;
            break;
            case OP_EXACT:  scode += 2; case OP_CHAR: case OP_CHARNC: case OP_PLUS: case OP_MINPLUS: case OP_POSPLUS:
            if (!inassert) return -1;
            if (c < 0) {
            c = scode[1];
            if ((*options & PCRE_CASELESS) != 0) c |= REQ_CASELESS;
            } else if (c != scode[1]) return -1;
            break;
            default: return -1;
        }
       code += GET(code, 1);
    } while(*code == OP_ALT);*/
    return c;
}
PCRE_EXP_DEFN pcre *PCRE_CALL_CONVENTION pcre_compile(const char *pattern, int options, const char **errorptr, int *erroroffset, const unsigned char *tables) {
    return pcre_compile2(pattern, options, NULL, errorptr, erroroffset, tables);
}
PCRE_EXP_DEFN pcre *PCRE_CALL_CONVENTION pcre_compile2(const char *pattern, int options, int *errorcodeptr, const char **errorptr, int *erroroffset,
                                                       const unsigned char *tables) {
    real_pcre *re;
    int length = 1;
    int firstbyte, reqbyte, newline;
    int errorcode = 0;
    int skipatstart = 0;
    BOOL utf8;
    size_t size;
    uschar *code;
    const uschar *codestart;
    const uschar *ptr;
    compile_data compile_block;
    compile_data *cd = &compile_block;
    uschar cworkspace[COMPILE_WORK_SIZE];
    ptr = (const uschar*)pattern;
    if (errorptr == NULL) {
        if (errorcodeptr != NULL) *errorcodeptr = 99;
        return NULL;
    }
    *errorptr = NULL;
    if (errorcodeptr != NULL) *errorcodeptr = ERR0;
    if (erroroffset == NULL) {
        errorcode = ERR16;
        goto PCRE_EARLY_ERROR_RETURN2;
    }
    *erroroffset = 0;
    if (tables == NULL) tables = _pcre_default_tables;
    cd->lcc = tables + lcc_offset;
    cd->fcc = tables + fcc_offset;
    cd->cbits = tables + cbits_offset;
    cd->ctypes = tables + ctypes_offset;
    if ((options & ~PUBLIC_COMPILE_OPTIONS) != 0) {
        errorcode = ERR17;
        goto PCRE_EARLY_ERROR_RETURN;
    }
    while(ptr[skipatstart] == CHAR_LEFT_PARENTHESIS && ptr[skipatstart+1] == CHAR_ASTERISK) {
        int newnl = 0;
        int newbsr = 0;
        if (strncmp((char *)(ptr+skipatstart+2), STRING_UTF8_RIGHTPAR, 5) == 0) { skipatstart += 7; options |= PCRE_UTF8; continue; }
        else if (strncmp((char *)(ptr+skipatstart+2), STRING_UCP_RIGHTPAR, 4) == 0) { skipatstart += 6; options |= PCRE_UCP; continue; }
        else  if (strncmp((char *)(ptr+skipatstart+2), STRING_NO_START_OPT_RIGHTPAR, 13) == 0){ skipatstart += 15; options |= PCRE_NO_START_OPTIMIZE; continue; }
        if (strncmp((char *)(ptr+skipatstart+2), STRING_CR_RIGHTPAR, 3) == 0) { skipatstart += 5; newnl = PCRE_NEWLINE_CR; }
        else if (strncmp((char *)(ptr+skipatstart+2), STRING_LF_RIGHTPAR, 3)  == 0) { skipatstart += 5; newnl = PCRE_NEWLINE_LF; }
        else if (strncmp((char *)(ptr+skipatstart+2), STRING_CRLF_RIGHTPAR, 5)  == 0) { skipatstart += 7; newnl = PCRE_NEWLINE_CR + PCRE_NEWLINE_LF; }
        else if (strncmp((char *)(ptr+skipatstart+2), STRING_ANY_RIGHTPAR, 4) == 0) { skipatstart += 6; newnl = PCRE_NEWLINE_ANY; }
        else if (strncmp((char *)(ptr+skipatstart+2), STRING_ANYCRLF_RIGHTPAR, 8) == 0) { skipatstart += 10; newnl = PCRE_NEWLINE_ANYCRLF; }
        else if (strncmp((char *)(ptr+skipatstart+2), STRING_BSR_ANYCRLF_RIGHTPAR, 12) == 0) { skipatstart += 14; newbsr = PCRE_BSR_ANYCRLF; }
        else if (strncmp((char *)(ptr+skipatstart+2), STRING_BSR_UNICODE_RIGHTPAR, 12) == 0) { skipatstart += 14; newbsr = PCRE_BSR_UNICODE; }
        if (newnl != 0) options = (options & ~PCRE_NEWLINE_BITS) | newnl;
        else if (newbsr != 0) options = (options & ~(PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE)) | newbsr;
        else break;
    }
    utf8 = (options & PCRE_UTF8) != 0;
#ifdef SUPPORT_UTF8
    if (utf8 && (options & PCRE_NO_UTF8_CHECK) == 0 && (*erroroffset = _pcre_valid_utf8((USPTR)pattern, -1)) >= 0) {
        errorcode = ERR44;
        goto PCRE_EARLY_ERROR_RETURN2;
    }
#else
    if (utf8) {
        errorcode = ERR32;
        goto PCRE_EARLY_ERROR_RETURN;
    }
#endif
#ifndef SUPPORT_UCP
    if ((options & PCRE_UCP) != 0) {
        errorcode = ERR67;
        goto PCRE_EARLY_ERROR_RETURN;
    }
#endif
    switch (options & (PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE)) {
        case 0: case PCRE_BSR_ANYCRLF: case PCRE_BSR_UNICODE: break;
        default: errorcode = ERR56; goto PCRE_EARLY_ERROR_RETURN;
    }
    switch (options & PCRE_NEWLINE_BITS) {
        //case 0: newline = NEWLINE; break;
        case PCRE_NEWLINE_CR: newline = CHAR_CR; break;
        case PCRE_NEWLINE_LF: newline = CHAR_NL; break;
        case PCRE_NEWLINE_CR + PCRE_NEWLINE_LF: newline = (CHAR_CR << 8) | CHAR_NL; break;
        case PCRE_NEWLINE_ANY: newline = -1; break;
        case PCRE_NEWLINE_ANYCRLF: newline = -2; break;
        default: errorcode = ERR56; goto PCRE_EARLY_ERROR_RETURN;
    }
    if (newline == -2) cd->nltype = NLTYPE_ANYCRLF;
    else if (newline < 0) cd->nltype = NLTYPE_ANY;
    else {
        cd->nltype = NLTYPE_FIXED;
        if (newline > 255) {
            cd->nllen = 2;
            cd->nl[0] = (newline >> 8) & 255;
            cd->nl[1] = newline & 255;
        } else {
            cd->nllen = 1;
            cd->nl[0] = newline;
        }
    }
    cd->top_backref = 0;
    cd->backref_map = 0;
    DPRINTF(("------------------------------------------------------------------\n"));
    DPRINTF(("%s\n", pattern));
    cd->bracount = cd->final_bracount = 0;
    cd->names_found = 0;
    cd->name_entry_size = 0;
    cd->name_table = NULL;
    cd->start_workspace = cworkspace;
    cd->start_code = cworkspace;
    cd->hwm = cworkspace;
    cd->start_pattern = (const uschar *)pattern;
    cd->end_pattern = (const uschar *)(pattern + strlen(pattern));
    cd->req_varyopt = 0;
    cd->external_options = options;
    cd->external_flags = 0;
    cd->open_caps = NULL;
    ptr += skipatstart;
    code = cworkspace;
    *code = OP_BRA;
    (void)compile_regex(cd->external_options, cd->external_options & PCRE_IMS, &code, &ptr, &errorcode, FALSE, FALSE, 0, &firstbyte,
                        &reqbyte, NULL, cd, &length);
    if (errorcode != 0) goto PCRE_EARLY_ERROR_RETURN;
    DPRINTF(("end pre-compile: length=%d workspace=%d\n", length, cd->hwm - cworkspace));
    /*if (length > MAX_PATTERN_SIZE) {
        errorcode = ERR20;
        goto PCRE_EARLY_ERROR_RETURN;
    }*/
    size = length + sizeof(real_pcre) + cd->names_found * (cd->name_entry_size + 3);
    re = (real_pcre *)(pcre_malloc)(size);
    if (re == NULL) {
        errorcode = ERR21;
        goto PCRE_EARLY_ERROR_RETURN;
    }
    re->magic_number = MAGIC_NUMBER;
    re->size = (int)size;
    re->options = cd->external_options;
    re->flags = cd->external_flags;
    re->dummy1 = 0;
    re->first_byte = 0;
    re->req_byte = 0;
    re->name_table_offset = sizeof(real_pcre);
    re->name_entry_size = cd->name_entry_size;
    re->name_count = cd->names_found;
    re->ref_count = 0;
    re->tables = (tables == _pcre_default_tables)? NULL : tables;
    re->nullpad = NULL;
    cd->final_bracount = cd->bracount;
    cd->bracount = 0;
    cd->names_found = 0;
    cd->name_table = (uschar *)re + re->name_table_offset;
    codestart = cd->name_table + re->name_entry_size * re->name_count;
    cd->start_code = codestart;
    cd->hwm = cworkspace;
    cd->req_varyopt = 0;
    cd->had_accept = FALSE;
    cd->check_lookbehind = FALSE;
    cd->open_caps = NULL;
    ptr = (const uschar *)pattern + skipatstart;
    code = (uschar *)codestart;
    *code = OP_BRA;
    (void)compile_regex(re->options, re->options & PCRE_IMS, &code, &ptr, &errorcode, FALSE, FALSE, 0, &firstbyte, &reqbyte, NULL, cd,
               NULL);
    re->top_bracket = cd->bracount;
    re->top_backref = cd->top_backref;
    re->flags = cd->external_flags;
    if (cd->had_accept) reqbyte = -1;
    if (errorcode == 0 && *ptr != 0) errorcode = ERR22;
    *code++ = OP_END;
    #ifndef PCRE_DEBUG
    if (code - codestart > length) errorcode = ERR23;
    #endif
    /*while (errorcode == 0 && cd->hwm > cworkspace) {
        int offset, recno;
        const uschar *groupptr;
        cd->hwm -= LINK_SIZE;
        offset = GET(cd->hwm, 0);
        recno = GET(codestart, offset);
        groupptr = _pcre_find_bracket(codestart, utf8, recno);
        if (groupptr == NULL) errorcode = ERR53;
        else PUT(((uschar *)codestart), offset, (int)(groupptr - codestart));
    }*/
    if (errorcode == 0 && re->top_backref > re->top_bracket) errorcode = ERR15;
    /*if (cd->check_lookbehind) {
        uschar *cc = (uschar*)codestart;
        for (cc = (uschar *)_pcre_find_bracket(codestart, utf8, -1); cc != NULL; cc = (uschar *)_pcre_find_bracket(cc, utf8, -1)) {
            if (GET(cc, 1) == 0) {
                int fixed_length;
                uschar *be = cc - 1 - LINK_SIZE + GET(cc, -LINK_SIZE);
                int end_op = *be;
                *be = OP_END;
                fixed_length = find_fixedlength(cc, re->options, TRUE, cd);
                *be = end_op;
                DPRINTF(("fixed length = %d\n", fixed_length));
                if (fixed_length < 0) {
                    errorcode = (fixed_length == -2)? ERR36 : ERR25;
                    break;
                }
                PUT(cc, 1, fixed_length);
            }
            cc += 1 + LINK_SIZE;
        }
    }*/
    if (errorcode != 0) {
        (pcre_free)(re);
        PCRE_EARLY_ERROR_RETURN:
        *erroroffset = (int)(ptr - (const uschar *)pattern);
        PCRE_EARLY_ERROR_RETURN2:
        *errorptr = find_error_text(errorcode);
        if (errorcodeptr != NULL) *errorcodeptr = errorcode;
        return NULL;
    }
    if ((re->options & PCRE_ANCHORED) == 0) {
        int temp_options = re->options;
        if (is_anchored(codestart, &temp_options, 0, cd->backref_map)) re->options |= PCRE_ANCHORED;
        else {
            if (firstbyte < 0) firstbyte = find_firstassertedchar(codestart, &temp_options, FALSE);
            if (firstbyte >= 0) {
                int ch = firstbyte & 255;
                re->first_byte = ((firstbyte & REQ_CASELESS) != 0 && cd->fcc[ch] == ch)? ch : firstbyte;
                re->flags |= PCRE_FIRSTSET;
            } else if (is_startline(codestart, 0, cd->backref_map)) re->flags |= PCRE_STARTLINE;
        }
    }
    if (reqbyte >= 0 && ((re->options & PCRE_ANCHORED) == 0 || (reqbyte & REQ_VARY) != 0)) {
        int ch = reqbyte & 255;
        re->req_byte = ((reqbyte & REQ_CASELESS) != 0 && cd->fcc[ch] == ch)? (reqbyte & ~REQ_CASELESS) : reqbyte;
        re->flags |= PCRE_REQCHSET;
    }
    #ifdef PCRE_DEBUG
    printf("Length = %d top_bracket = %d top_backref = %d\n", length, re->top_bracket, re->top_backref);
    printf("Options=%08x\n", re->options);
    if ((re->flags & PCRE_FIRSTSET) != 0) {
        int ch = re->first_byte & 255;
        const char *caseless = ((re->first_byte & REQ_CASELESS) == 0) ? "" : " (caseless)";
        if (isprint(ch)) printf("First char = %c%s\n", ch, caseless);
        else printf("First char = \\x%02x%s\n", ch, caseless);
    }
    if ((re->flags & PCRE_REQCHSET) != 0) {
        int ch = re->req_byte & 255;
        const char *caseless = ((re->req_byte & REQ_CASELESS) == 0) ? "" : " (caseless)";
        if (isprint(ch)) printf("Req char = %c%s\n", ch, caseless);
        else printf("Req char = \\x%02x%s\n", ch, caseless);
    }
    pcre_printint(re, stdout, TRUE);
    if (code - codestart > length) {
        (pcre_free)(re);
        *errorptr = find_error_text(ERR23);
        *erroroffset = ptr - (uschar *)pattern;
        if (errorcodeptr != NULL) *errorcodeptr = ERR23;
        return NULL;
    }
    #endif
    return (pcre *)re;
}