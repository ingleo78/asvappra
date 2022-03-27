#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

#define SET_BIT(c) start_bits[c/8] |= (1 << (c&7))
enum { SSB_FAIL, SSB_DONE, SSB_CONTINUE };
static int find_minlength(const uschar *code, const uschar *startcode, int options) {
    int length = -1;
    BOOL utf8 = (options & PCRE_UTF8) != 0;
    BOOL had_recurse = FALSE;
    register int branchlength = 0;
    register uschar *cc = (uschar *)code + 1;
    if (*code == OP_CBRA || *code == OP_SCBRA) cc += 2;
    for (;;) {
        int d, min;
        uschar *cs, *ce;
        register int op = *cc;
        switch (op) {
            case OP_COND: case OP_SCOND:
                cs = cc /*+ GET(cc, 1)*/;
                if (*cs != OP_ALT) {
                    cc = cs + 1 /*+ LINK_SIZE*/;
                    break;
                }
            case OP_CBRA: case OP_SCBRA: case OP_BRA: case OP_SBRA: case OP_ONCE:
                d = find_minlength(cc, startcode, options);
                if (d < 0) return d;
                branchlength += d;
                //do cc += GET(cc, 1); while (*cc == OP_ALT);
                cc += 1 /*+ LINK_SIZE*/;
                break;
            case OP_ALT: case OP_KET: case OP_KETRMAX: case OP_KETRMIN: case OP_END:
                if (length < 0 || (!had_recurse && branchlength < length)) length = branchlength;
                if (*cc != OP_ALT) return length;
                cc += 1 /*+ LINK_SIZE*/;
                branchlength = 0;
                had_recurse = FALSE;
                break;
            //case OP_ASSERT: case OP_ASSERT_NOT: case OP_ASSERTBACK: case OP_ASSERTBACK_NOT: do cc += GET(cc, 1); while (*cc == OP_ALT);
            case OP_REVERSE: case OP_CREF: case OP_NCREF: case OP_RREF: case OP_NRREF: case OP_DEF: case OP_OPT: case OP_CALLOUT: case OP_SOD: case OP_SOM:
            case OP_EOD: case OP_EODN: case OP_CIRC: case OP_DOLL: case OP_NOT_WORD_BOUNDARY: case OP_WORD_BOUNDARY:
                cc += _pcre_OP_lengths[*cc];
                break;
            case OP_BRAZERO: case OP_BRAMINZERO: case OP_SKIPZERO:
                cc += _pcre_OP_lengths[*cc];
                //do cc += GET(cc, 1); while (*cc == OP_ALT);
                cc += 1 /*+ LINK_SIZE*/;
                break;
            case OP_CHAR: case OP_CHARNC: case OP_NOT: case OP_PLUS: case OP_MINPLUS: case OP_POSPLUS: case OP_NOTPLUS: case OP_NOTMINPLUS: case OP_NOTPOSPLUS:
                branchlength++;
                cc += 2;
            #ifdef SUPPORT_UTF8
                if (utf8 && cc[-1] >= 0xc0) cc += _pcre_utf8_table4[cc[-1] & 0x3f];
            #endif
                break;
            case OP_TYPEPLUS: case OP_TYPEMINPLUS: case OP_TYPEPOSPLUS:
                branchlength++;
                cc += (cc[1] == OP_PROP || cc[1] == OP_NOTPROP)? 4 : 2;
                break;
            case OP_EXACT: case OP_NOTEXACT:
                branchlength += GET2(cc,1);
                cc += 4;
            #ifdef SUPPORT_UTF8
                if (utf8 && cc[-1] >= 0xc0) cc += _pcre_utf8_table4[cc[-1] & 0x3f];
            #endif
                break;
            case OP_TYPEEXACT:
                branchlength += GET2(cc,1);
                cc += (cc[3] == OP_PROP || cc[3] == OP_NOTPROP)? 6 : 4;
                break;
            case OP_PROP: case OP_NOTPROP: cc += 2;
            case OP_NOT_DIGIT: case OP_DIGIT: case OP_NOT_WHITESPACE: case OP_WHITESPACE: case OP_NOT_WORDCHAR: case OP_WORDCHAR: case OP_ANY: case OP_ALLANY:
            case OP_EXTUNI: case OP_HSPACE: case OP_NOT_HSPACE: case OP_VSPACE: case OP_NOT_VSPACE:
                branchlength++;
                cc++;
                break;
            case OP_ANYNL:
                branchlength += 2;
                cc++;
                break;
            case OP_ANYBYTE:
            #ifdef SUPPORT_UTF8
                if (utf8) return -1;
            #endif
                branchlength++;
                cc++;
                break;
            case OP_TYPESTAR: case OP_TYPEMINSTAR: case OP_TYPEQUERY: case OP_TYPEMINQUERY: case OP_TYPEPOSSTAR: case OP_TYPEPOSQUERY:
                if (cc[1] == OP_PROP || cc[1] == OP_NOTPROP) cc += 2;
                cc += _pcre_OP_lengths[op];
                break;
            case OP_TYPEUPTO: case OP_TYPEMINUPTO: case OP_TYPEPOSUPTO:
                if (cc[3] == OP_PROP || cc[3] == OP_NOTPROP) cc += 2;
                cc += _pcre_OP_lengths[op];
                break;
        #ifdef SUPPORT_UTF8
            case OP_XCLASS: cc += GET(cc, 1) - 33;
        #endif
            case OP_CLASS: case OP_NCLASS:
                cc += 33;
                switch (*cc) {
                    case OP_CRPLUS: case OP_CRMINPLUS: branchlength++;
                    case OP_CRSTAR: case OP_CRMINSTAR: case OP_CRQUERY: case OP_CRMINQUERY: cc++; break;
                    case OP_CRRANGE: case OP_CRMINRANGE:
                    branchlength += GET2(cc,1);
                    cc += 5;
                    break;
                    default: branchlength++; break;
                }
                break;
            case OP_REF:
                if ((options & PCRE_JAVASCRIPT_COMPAT) == 0) {
                    ce = cs = (uschar *)_pcre_find_bracket(startcode, utf8, GET2(cc, 1));
                    if (cs == NULL) return -2;
                    //do ce += GET(ce, 1); while (*ce == OP_ALT);
                    if (cc > cs && cc < ce) {
                    d = 0;
                    had_recurse = TRUE;
                    } else d = find_minlength(cs, startcode, options);
                } else d = 0;
                cc += 3;
                switch (*cc) {
                    case OP_CRSTAR: case OP_CRMINSTAR: case OP_CRQUERY: case OP_CRMINQUERY:
                    min = 0;
                    cc++;
                    break;
                    case OP_CRRANGE: case OP_CRMINRANGE:
                    min = GET2(cc, 1);
                    cc += 5;
                    break;
                    default: min = 1; break;
                }
                branchlength += min * d;
                break;
            case OP_RECURSE:
                cs = ce = (uschar*)startcode /*+ GET(cc, 1)*/;
                if (cs == NULL) return -2;
                //do ce += GET(ce, 1); while (*ce == OP_ALT);
                if (cc > cs && cc < ce) had_recurse = TRUE;
                else branchlength += find_minlength(cs, startcode, options);
                cc += 1 /*+ LINK_SIZE*/;
                break;
            case OP_UPTO: case OP_NOTUPTO: case OP_MINUPTO: case OP_NOTMINUPTO: case OP_POSUPTO: case OP_STAR: case OP_MINSTAR: case OP_NOTMINSTAR:
            case OP_POSSTAR: case OP_NOTPOSSTAR: case OP_QUERY: case OP_MINQUERY: case OP_NOTMINQUERY: case OP_POSQUERY: case OP_NOTPOSQUERY:
                cc += _pcre_OP_lengths[op];
            #ifdef SUPPORT_UTF8
                if (utf8 && cc[-1] >= 0xc0) cc += _pcre_utf8_table4[cc[-1] & 0x3f];
            #endif
                break;
            case OP_MARK: case OP_PRUNE_ARG: case OP_SKIP_ARG: cc += _pcre_OP_lengths[op] + cc[1]; break;
            case OP_THEN_ARG: cc += _pcre_OP_lengths[op] /*+ cc[1+LINK_SIZE]*/; break;
            default: cc += _pcre_OP_lengths[op]; break;
        }
    }
}
static const uschar *set_table_bit(uschar *start_bits, const uschar *p, BOOL caseless, compile_data *cd, BOOL utf8) {
    unsigned int c = *p;
    SET_BIT(c);
    #ifdef SUPPORT_UTF8
    if (utf8 && c > 127) {
        GETCHARINC(c, p);
    #ifdef SUPPORT_UCP
        if (caseless) {
            uschar buff[8];
            c = UCD_OTHERCASE(c);
            (void)_pcre_ord2utf8(c, buff);
            SET_BIT(buff[0]);
        }
    #endif
        return p;
    }
    #endif
    if (caseless && (cd->ctypes[c] & ctype_letter) != 0) SET_BIT(cd->fcc[c]);
    return p + 1;
}
static void set_type_bits(uschar *start_bits, int cbit_type, int table_limit, compile_data *cd) {
    register int c;
    for (c = 0; c < table_limit; c++) start_bits[c] |= cd->cbits[c+cbit_type];
    if (table_limit == 32) return;
    for (c = 128; c < 256; c++) {
        if ((cd->cbits[c/8] & (1 << (c&7))) != 0) {
            uschar buff[8];
            (void)_pcre_ord2utf8(c, buff);
            SET_BIT(buff[0]);
        }
    }
}
static void set_nottype_bits(uschar *start_bits, int cbit_type, int table_limit, compile_data *cd) {
    register int c;
    for (c = 0; c < table_limit; c++) start_bits[c] |= ~cd->cbits[c+cbit_type];
    if (table_limit != 32) for (c = 24; c < 32; c++) start_bits[c] = 0xff;
}
static int set_start_bits(const uschar *code, uschar *start_bits, BOOL caseless, BOOL utf8, compile_data *cd) {
    register int c;
    int yield = SSB_DONE;
    int table_limit = utf8? 16:32;
    do {
        const uschar *tcode = code + (((int)*code == OP_CBRA) ? 3 : 1) /*+ LINK_SIZE*/;
        BOOL try_next = TRUE;
        while (try_next) {
            int rc;
            switch(*tcode) {
                case OP_BRA: case OP_SBRA: case OP_CBRA: case OP_SCBRA: case OP_ONCE: case OP_ASSERT:
                    rc = set_start_bits(tcode, start_bits, caseless, utf8, cd);
                    if (rc == SSB_FAIL) return SSB_FAIL;
                    if (rc == SSB_DONE) try_next = FALSE;
                    else {
                        //do tcode += GET(tcode, 1); while (*tcode == OP_ALT);
                        tcode += 1 /*+ LINK_SIZE*/;
                    }
                    break;
                case OP_ALT:
                    yield = SSB_CONTINUE;
                    try_next = FALSE;
                    break;
                case OP_KET: case OP_KETRMAX: case OP_KETRMIN: return SSB_CONTINUE;
                case OP_CALLOUT: tcode += 2 /*+ 2*LINK_SIZE*/; break;
                case OP_ASSERT_NOT: case OP_ASSERTBACK: case OP_ASSERTBACK_NOT:
                    //do tcode += GET(tcode, 1); while (*tcode == OP_ALT);
                    tcode += 1 /*+ LINK_SIZE*/;
                    break;
                case OP_OPT:
                    caseless = (tcode[1] & PCRE_CASELESS) != 0;
                    tcode += 2;
                    break;
                case OP_BRAZERO: case OP_BRAMINZERO:
                    if (set_start_bits(++tcode, start_bits, caseless, utf8, cd) == SSB_FAIL) return SSB_FAIL;
                    //do tcode += GET(tcode,1); while (*tcode == OP_ALT);
                    tcode += 1 /*+ LINK_SIZE*/;
                    break;
                case OP_SKIPZERO:
                    tcode++;
                    //do tcode += GET(tcode,1); while (*tcode == OP_ALT);
                    tcode += 1 /*+ LINK_SIZE*/;
                    break;
                case OP_STAR: case OP_MINSTAR: case OP_POSSTAR: case OP_QUERY: case OP_MINQUERY: case OP_POSQUERY:
                    tcode = set_table_bit(start_bits, tcode + 1, caseless, cd, utf8);
                    break;
                case OP_UPTO: case OP_MINUPTO: case OP_POSUPTO:
                    tcode = set_table_bit(start_bits, tcode + 3, caseless, cd, utf8);
                    break;
                case OP_EXACT: tcode += 2;
                case OP_CHAR: case OP_CHARNC: case OP_PLUS: case OP_MINPLUS: case OP_POSPLUS:
                    (void)set_table_bit(start_bits, tcode + 1, caseless, cd, utf8);
                    try_next = FALSE;
                    break;
                case OP_HSPACE:
                    SET_BIT(0x09);
                    SET_BIT(0x20);
                    if (utf8) {
                        SET_BIT(0xC2);
                        SET_BIT(0xE1);
                        SET_BIT(0xE2);
                        SET_BIT(0xE3);
                    } else SET_BIT(0xA0);
                    try_next = FALSE;
                    break;
                case OP_ANYNL: case OP_VSPACE:
                    SET_BIT(0x0A);
                    SET_BIT(0x0B);
                    SET_BIT(0x0C);
                    SET_BIT(0x0D);
                    if (utf8) {
                        SET_BIT(0xC2);
                        SET_BIT(0xE2);
                    } else SET_BIT(0x85);
                    try_next = FALSE;
                    break;
                case OP_NOT_DIGIT:
                    set_nottype_bits(start_bits, cbit_digit, table_limit, cd);
                    try_next = FALSE;
                    break;
                case OP_DIGIT:
                    set_type_bits(start_bits, cbit_digit, table_limit, cd);
                    try_next = FALSE;
                    break;
                case OP_NOT_WHITESPACE:
                    set_nottype_bits(start_bits, cbit_space, table_limit, cd);
                    start_bits[1] |= 0x08;
                    try_next = FALSE;
                    break;
                case OP_WHITESPACE:
                    c = start_bits[1];
                    set_type_bits(start_bits, cbit_space, table_limit, cd);
                    start_bits[1] = (start_bits[1] & ~0x08) | c;
                    try_next = FALSE;
                    break;
                case OP_NOT_WORDCHAR:
                    set_nottype_bits(start_bits, cbit_word, table_limit, cd);
                    try_next = FALSE;
                    break;
                case OP_WORDCHAR:
                    set_type_bits(start_bits, cbit_word, table_limit, cd);
                    try_next = FALSE;
                    break;
                case OP_TYPEPLUS: case OP_TYPEMINPLUS: case OP_TYPEPOSPLUS: tcode++; break;
                case OP_TYPEEXACT: tcode += 3; break;
                case OP_TYPEUPTO: case OP_TYPEMINUPTO: case OP_TYPEPOSUPTO: tcode += 2;
                case OP_TYPESTAR: case OP_TYPEMINSTAR: case OP_TYPEPOSSTAR: case OP_TYPEQUERY: case OP_TYPEMINQUERY: case OP_TYPEPOSQUERY:
                    switch(tcode[1]) {
                        case OP_ANYNL: case OP_VSPACE:
                            SET_BIT(0x0A);
                            SET_BIT(0x0B);
                            SET_BIT(0x0C);
                            SET_BIT(0x0D);
                            if (utf8) {
                                SET_BIT(0xC2);
                                SET_BIT(0xE2);
                            } else SET_BIT(0x85);
                            break;
                        case OP_NOT_DIGIT: set_nottype_bits(start_bits, cbit_digit, table_limit, cd); break;
                        case OP_DIGIT: set_type_bits(start_bits, cbit_digit, table_limit, cd); break;
                        case OP_NOT_WHITESPACE:
                            set_nottype_bits(start_bits, cbit_space, table_limit, cd);
                            start_bits[1] |= 0x08;
                            break;
                        case OP_WHITESPACE:
                            c = start_bits[1];
                            set_type_bits(start_bits, cbit_space, table_limit, cd);
                            start_bits[1] = (start_bits[1] & ~0x08) | c;
                            break;
                        case OP_NOT_WORDCHAR: set_nottype_bits(start_bits, cbit_word, table_limit, cd); break;
                        case OP_WORDCHAR: set_type_bits(start_bits, cbit_word, table_limit, cd); break;
                        default:
                            SET_BIT(0x09);
                            SET_BIT(0x20);
                            if (utf8) {
                                SET_BIT(0xC2);
                                SET_BIT(0xE1);
                                SET_BIT(0xE2);
                                SET_BIT(0xE3);
                            } else SET_BIT(0xA0);
                            break;
                    }
                    tcode += 2;
                    break;
                case OP_NCLASS:
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        start_bits[24] |= 0xf0;
                        memset(start_bits+25, 0xff, 7);
                    }
                #endif
                case OP_CLASS: {
                        tcode++;
                    #ifdef SUPPORT_UTF8
                        if (utf8) {
                            for (c = 0; c < 16; c++) start_bits[c] |= tcode[c];
                            for (c = 128; c < 256; c++) {
                                if ((tcode[c/8] && (1 << (c&7))) != 0) {
                                    int d = (c >> 6) | 0xc0;
                                    start_bits[d/8] |= (1 << (d&7));
                                    c = (c & 0xc0) + 0x40 - 1;
                                }
                            }
                        } else
                    #endif
                        for (c = 0; c < 32; c++) start_bits[c] |= tcode[c];
                        tcode += 32;
                        switch (*tcode) {
                            case OP_CRSTAR: case OP_CRMINSTAR: case OP_CRQUERY: case OP_CRMINQUERY: tcode++; break;
                            case OP_CRRANGE: case OP_CRMINRANGE:
                                if (((tcode[1] << 8) + tcode[2]) == 0) tcode += 5;
                                else try_next = FALSE;
                                break;
                            default: try_next = FALSE; break;
                        }
                    }
                    break;
                default: return SSB_FAIL;
            }
        }
        //code += GET(code, 1);
    } while (*code == OP_ALT);
    return yield;
}
PCRE_EXP_DEFN pcre_extra * PCRE_CALL_CONVENTION pcre_study(const pcre *external_re, int options, const char **errorptr) {
    int min;
    BOOL bits_set = FALSE;
    uschar start_bits[32];
    pcre_extra *extra;
    pcre_study_data *study;
    const uschar *tables;
    uschar *code;
    compile_data compile_block;
    const real_pcre *re = (const real_pcre *)external_re;
    *errorptr = NULL;
    if (re == NULL || re->magic_number != MAGIC_NUMBER) {
        *errorptr = "argument is not a compiled regular expression";
        return NULL;
    }
    if ((options & ~PUBLIC_STUDY_OPTIONS) != 0) {
        *errorptr = "unknown or incorrect option bit(s) set";
        return NULL;
    }
    code = (uschar *)re + re->name_table_offset + (re->name_count * re->name_entry_size);
    if ((re->options & PCRE_ANCHORED) == 0 && (re->flags & (PCRE_FIRSTSET|PCRE_STARTLINE)) == 0) {
        tables = re->tables;
        if (tables == NULL) (void)pcre_fullinfo(external_re, NULL, PCRE_INFO_DEFAULT_TABLES, (void*)(&tables));
        compile_block.lcc = tables + lcc_offset;
        compile_block.fcc = tables + fcc_offset;
        compile_block.cbits = tables + cbits_offset;
        compile_block.ctypes = tables + ctypes_offset;
        memset(start_bits, 0, 32 * sizeof(uschar));
        bits_set = set_start_bits(code, start_bits,(re->options & PCRE_CASELESS) != 0, (re->options & PCRE_UTF8) != 0, &compile_block) == SSB_DONE;
    }
    min = find_minlength(code, code, re->options);
    if (!bits_set && min < 0) return NULL;
    extra = (pcre_extra *)(pcre_malloc)(sizeof(pcre_extra) + sizeof(pcre_study_data));
    if (extra == NULL) {
        *errorptr = "failed to get memory";
        return NULL;
    }
    study = (pcre_study_data *)((char *)extra + sizeof(pcre_extra));
    extra->flags = PCRE_EXTRA_STUDY_DATA;
    extra->study_data = study;
    study->size = sizeof(pcre_study_data);
    study->flags = 0;
    if (bits_set) {
        study->flags |= PCRE_STUDY_MAPPED;
        memcpy(study->start_bits, start_bits, sizeof(start_bits));
    }
    if (min >= 0) {
        study->flags |= PCRE_STUDY_MINLEN;
        study->minlength = min;
    }
    return extra;
}