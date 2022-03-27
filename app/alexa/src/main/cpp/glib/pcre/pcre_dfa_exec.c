#include "pcre_internal.h"

#define NLBLOCK md             /* Block containing newline information */
#define PSSTART start_subject  /* Field containing processed string start */
#define PSEND   end_subject    /* Field containing processed string end */
#define SP "                   "
#define OP_PROP_EXTRA       300
#define OP_EXTUNI_EXTRA     320
#define OP_ANYNL_EXTRA      340
#define OP_HSPACE_EXTRA     360
#define OP_VSPACE_EXTRA     380
static const uschar coptable[] = {
  0,                             /* End                                    */
  0, 0, 0, 0, 0,                 /* \A, \G, \K, \B, \b                     */
  0, 0, 0, 0, 0, 0,              /* \D, \d, \S, \s, \W, \w                 */
  0, 0, 0,                       /* Any, AllAny, Anybyte                   */
  0, 0,                          /* \P, \p                                 */
  0, 0, 0, 0, 0,                 /* \R, \H, \h, \V, \v                     */
  0,                             /* \X                                     */
  0, 0, 0, 0, 0,                 /* \Z, \z, Opt, ^, $                      */
  1,                             /* Char                                   */
  1,                             /* Charnc                                 */
  1,                             /* not                                    */
  /* Positive single-char repeats                                          */
  1, 1, 1, 1, 1, 1,              /* *, *?, +, +?, ?, ??                    */
  3, 3, 3,                       /* upto, minupto, exact                   */
  1, 1, 1, 3,                    /* *+, ++, ?+, upto+                      */
  /* Negative single-char repeats - only for chars < 256                   */
  1, 1, 1, 1, 1, 1,              /* NOT *, *?, +, +?, ?, ??                */
  3, 3, 3,                       /* NOT upto, minupto, exact               */
  1, 1, 1, 3,                    /* NOT *+, ++, ?+, updo+                  */
  /* Positive type repeats                                                 */
  1, 1, 1, 1, 1, 1,              /* Type *, *?, +, +?, ?, ??               */
  3, 3, 3,                       /* Type upto, minupto, exact              */
  1, 1, 1, 3,                    /* Type *+, ++, ?+, upto+                 */
  /* Character class & ref repeats                                         */
  0, 0, 0, 0, 0, 0,              /* *, *?, +, +?, ?, ??                    */
  0, 0,                          /* CRRANGE, CRMINRANGE                    */
  0,                             /* CLASS                                  */
  0,                             /* NCLASS                                 */
  0,                             /* XCLASS - variable length               */
  0,                             /* REF                                    */
  0,                             /* RECURSE                                */
  0,                             /* CALLOUT                                */
  0,                             /* Alt                                    */
  0,                             /* Ket                                    */
  0,                             /* KetRmax                                */
  0,                             /* KetRmin                                */
  0,                             /* Assert                                 */
  0,                             /* Assert not                             */
  0,                             /* Assert behind                          */
  0,                             /* Assert behind not                      */
  0,                             /* Reverse                                */
  0, 0, 0, 0,                    /* ONCE, BRA, CBRA, COND                  */
  0, 0, 0,                       /* SBRA, SCBRA, SCOND                     */
  0, 0,                          /* CREF, NCREF                            */
  0, 0,                          /* RREF, NRREF                            */
  0,                             /* DEF                                    */
  0, 0,                          /* BRAZERO, BRAMINZERO                    */
  0, 0, 0,                       /* MARK, PRUNE, PRUNE_ARG,                */
  0, 0, 0, 0,                    /* SKIP, SKIP_ARG, THEN, THEN_ARG,        */
  0, 0, 0, 0, 0                  /* COMMIT, FAIL, ACCEPT, CLOSE, SKIPZERO  */
};
static const uschar poptable[] = {
  0,                             /* End                                    */
  0, 0, 0, 1, 1,                 /* \A, \G, \K, \B, \b                     */
  1, 1, 1, 1, 1, 1,              /* \D, \d, \S, \s, \W, \w                 */
  1, 1, 1,                       /* Any, AllAny, Anybyte                   */
  1, 1,                          /* \P, \p                                 */
  1, 1, 1, 1, 1,                 /* \R, \H, \h, \V, \v                     */
  1,                             /* \X                                     */
  0, 0, 0, 0, 0,                 /* \Z, \z, Opt, ^, $                      */
  1,                             /* Char                                   */
  1,                             /* Charnc                                 */
  1,                             /* not                                    */
  /* Positive single-char repeats                                          */
  1, 1, 1, 1, 1, 1,              /* *, *?, +, +?, ?, ??                    */
  1, 1, 1,                       /* upto, minupto, exact                   */
  1, 1, 1, 1,                    /* *+, ++, ?+, upto+                      */
  /* Negative single-char repeats - only for chars < 256                   */
  1, 1, 1, 1, 1, 1,              /* NOT *, *?, +, +?, ?, ??                */
  1, 1, 1,                       /* NOT upto, minupto, exact               */
  1, 1, 1, 1,                    /* NOT *+, ++, ?+, upto+                  */
  /* Positive type repeats                                                 */
  1, 1, 1, 1, 1, 1,              /* Type *, *?, +, +?, ?, ??               */
  1, 1, 1,                       /* Type upto, minupto, exact              */
  1, 1, 1, 1,                    /* Type *+, ++, ?+, upto+                 */
  /* Character class & ref repeats                                         */
  1, 1, 1, 1, 1, 1,              /* *, *?, +, +?, ?, ??                    */
  1, 1,                          /* CRRANGE, CRMINRANGE                    */
  1,                             /* CLASS                                  */
  1,                             /* NCLASS                                 */
  1,                             /* XCLASS - variable length               */
  0,                             /* REF                                    */
  0,                             /* RECURSE                                */
  0,                             /* CALLOUT                                */
  0,                             /* Alt                                    */
  0,                             /* Ket                                    */
  0,                             /* KetRmax                                */
  0,                             /* KetRmin                                */
  0,                             /* Assert                                 */
  0,                             /* Assert not                             */
  0,                             /* Assert behind                          */
  0,                             /* Assert behind not                      */
  0,                             /* Reverse                                */
  0, 0, 0, 0,                    /* ONCE, BRA, CBRA, COND                  */
  0, 0, 0,                       /* SBRA, SCBRA, SCOND                     */
  0, 0,                          /* CREF, NCREF                            */
  0, 0,                          /* RREF, NRREF                            */
  0,                             /* DEF                                    */
  0, 0,                          /* BRAZERO, BRAMINZERO                    */
  0, 0, 0,                       /* MARK, PRUNE, PRUNE_ARG,                */
  0, 0, 0, 0,                    /* SKIP, SKIP_ARG, THEN, THEN_ARG,        */
  0, 0, 0, 0, 0                  /* COMMIT, FAIL, ACCEPT, CLOSE, SKIPZERO  */
};
static const uschar toptable1[] = {
  0, 0, 0, 0, 0, 0,
  ctype_digit, ctype_digit,
  ctype_space, ctype_space,
  ctype_word,  ctype_word,
  0, 0                            /* OP_ANY, OP_ALLANY */
};
static const uschar toptable2[] = {
  0, 0, 0, 0, 0, 0,
  ctype_digit, 0,
  ctype_space, 0,
  ctype_word,  0,
  1, 1                            /* OP_ANY, OP_ALLANY */
};
typedef struct stateblock {
  int offset;                     /* Offset to opcode */
  int count;                      /* Count for repeats */
  int ims;                        /* ims flag bits */
  int data;                       /* Some use extra data */
} stateblock;
#define INTS_PER_STATEBLOCK  (sizeof(stateblock)/sizeof(int))
#ifdef PCRE_DEBUG
static void pchars(unsigned char *p, int length, FILE *f) {
    int c;
    while (length-- > 0) {
        if (isprint(c = *(p++))) fprintf(f, "%c", c);
        else fprintf(f, "\\x%02x", c);
    }
}
#endif
#define ADD_ACTIVE(x,y)                                               \
  if (active_count++ < wscount) {                                     \
      next_active_state->offset = (x);                                \
      next_active_state->count  = (y);                                \
      next_active_state->ims    = ims;                                \
      next_active_state++;                                            \
      DPRINTF(("%.*sADD_ACTIVE(%d,%d)\n", rlevel*2-2, SP, (x), (y))); \
  } else return PCRE_ERROR_DFA_WSSIZE
#define ADD_ACTIVE_DATA(x,y,z)                                                     \
  if (active_count++ < wscount) {                                                  \
      next_active_state->offset = (x);                                             \
      next_active_state->count  = (y);                                             \
      next_active_state->ims    = ims;                                             \
      next_active_state->data   = (z);                                             \
      next_active_state++;                                                         \
      DPRINTF(("%.*sADD_ACTIVE_DATA(%d,%d,%d)\n", rlevel*2-2, SP, (x), (y), (z))); \
  } else return PCRE_ERROR_DFA_WSSIZE
#define ADD_NEW(x,y)                                               \
  if (new_count++ < wscount) {                                     \
      next_new_state->offset = (x);                                \
      next_new_state->count  = (y);                                \
      next_new_state->ims    = ims;                                \
      next_new_state++;                                            \
      DPRINTF(("%.*sADD_NEW(%d,%d)\n", rlevel*2-2, SP, (x), (y))); \
  } else return PCRE_ERROR_DFA_WSSIZE
#define ADD_NEW_DATA(x,y,z)                                                     \
  if (new_count++ < wscount) {                                                  \
      next_new_state->offset = (x);                                             \
      next_new_state->count  = (y);                                             \
      next_new_state->ims    = ims;                                             \
      next_new_state->data   = (z);                                             \
      next_new_state++;                                                         \
      DPRINTF(("%.*sADD_NEW_DATA(%d,%d,%d)\n", rlevel*2-2, SP, (x), (y), (z))); \
  } else return PCRE_ERROR_DFA_WSSIZE
static int internal_dfa_exec(dfa_match_data *md, const uschar *this_start_code, const uschar *current_subject, int start_offset, int *offsets, int offsetcount,
                             int *workspace, int wscount, int ims, int  rlevel, int  recursing) {
    stateblock *active_states, *new_states, *temp_states;
    stateblock *next_active_state, *next_new_state;
    const uschar *ctypes, *lcc, *fcc;
    const uschar *ptr;
    const uschar *end_code, *first_op;
    int active_count, new_count, match_count;
    const uschar *start_subject = md->start_subject;
    const uschar *end_subject = md->end_subject;
    const uschar *start_code = md->start_code;
    #ifdef SUPPORT_UTF8
    BOOL utf8 = (md->poptions & PCRE_UTF8) != 0;
    #else
    BOOL utf8 = FALSE;
    #endif
    rlevel++;
    offsetcount &= (-2);
    wscount -= 2;
    wscount = (wscount - (wscount % (INTS_PER_STATEBLOCK * 2))) / (2 * INTS_PER_STATEBLOCK);
    DPRINTF(("\n%.*s---------------------\n%.*sCall to internal_dfa_exec f=%d r=%d\n", rlevel*2-2, SP, rlevel*2-2, SP, rlevel, recursing));
    ctypes = md->tables + ctypes_offset;
    lcc = md->tables + lcc_offset;
    fcc = md->tables + fcc_offset;
    match_count = PCRE_ERROR_NOMATCH;
    active_states = (stateblock *)(workspace + 2);
    next_new_state = new_states = active_states + wscount;
    new_count = 0;
    first_op = this_start_code + 1 + /* LINK_SIZE +*/ ((*this_start_code == OP_CBRA || *this_start_code == OP_SCBRA)? 2:0);
    if (*first_op == OP_REVERSE) {
        int max_back = 0;
        int gone_back;
        end_code = this_start_code;
        /*do {
            int back = GET(end_code, 2+LINK_SIZE);
            if (back > max_back) max_back = back;
            end_code += GET(end_code, 1);
        } while (*end_code == OP_ALT);*/
    #ifdef SUPPORT_UTF8
        if (utf8) {
            for (gone_back = 0; gone_back < max_back; gone_back++) {
                if (current_subject <= start_subject) break;
                current_subject--;
                while (current_subject > start_subject && (*current_subject & 0xc0) == 0x80) current_subject--;
            }
        } else
    #endif
        {
            gone_back = (current_subject - max_back < start_subject) ? (int)(current_subject - start_subject) : max_back;
            current_subject -= gone_back;
        }
        if (current_subject < md->start_used_ptr) md->start_used_ptr = current_subject;
        end_code = this_start_code;
        /*do {
            int back = GET(end_code, 2+LINK_SIZE);
            if (back <= gone_back) {
                int bstate = (int)(end_code - start_code + 2 + 2*LINK_SIZE);
                ADD_NEW_DATA(-bstate, 0, gone_back - back);
            }
            end_code += GET(end_code, 1);
        } while (*end_code == OP_ALT);*/
    } else {
        end_code = this_start_code;
        if (rlevel == 1 && (md->moptions & PCRE_DFA_RESTART) != 0) {
            //do { end_code += GET(end_code, 1); } while (*end_code == OP_ALT);
            new_count = workspace[1];
            if (!workspace[0]) memcpy(new_states, active_states, new_count * sizeof(stateblock));
        } else {
            int length = 1 + /*LINK_SIZE +*/((*this_start_code == OP_CBRA || *this_start_code == OP_SCBRA)? 2:0);
            do {
                ADD_NEW((int)(end_code - start_code + length), 0);
            //    end_code += GET(end_code, 1);
            //    length = 1 + LINK_SIZE;
            } while (*end_code == OP_ALT);
        }
    }
    workspace[0] = 0;
    DPRINTF(("%.*sEnd state = %d\n", rlevel*2-2, SP, end_code - start_code));
    ptr = current_subject;
    for (;;) {
        int i, j;
        int clen, dlen;
        unsigned int c, d;
        int forced_fail = 0;
        BOOL could_continue = FALSE;
        temp_states = active_states;
        active_states = new_states;
        new_states = temp_states;
        active_count = new_count;
        new_count = 0;
        workspace[0] ^= 1;
        workspace[1] = active_count;
    #ifdef PCRE_DEBUG
        printf("%.*sNext character: rest of subject = \"", rlevel*2-2, SP);
        pchars((uschar *)ptr, strlen((char *)ptr), stdout);
        printf("\"\n");
        printf("%.*sActive states: ", rlevel*2-2, SP);
        for (i = 0; i < active_count; i++) printf("%d/%d ", active_states[i].offset, active_states[i].count);
        printf("\n");
    #endif
        next_active_state = active_states + active_count;
        next_new_state = new_states;
        if (ptr < end_subject) {
            clen = 1;
        #ifdef SUPPORT_UTF8
            if (utf8) { GETCHARLEN(c, ptr, clen); } else
        #endif
            c = *ptr;
        } else {
            clen = 0;
            c = NOTACHAR;
        }
        for (i = 0; i < active_count; i++) {
            stateblock *current_state = active_states + i;
            const uschar *code;
            int state_offset = current_state->offset;
            int count, codevalue, rrc;
        #ifdef PCRE_DEBUG
            printf ("%.*sProcessing state %d c=", rlevel*2-2, SP, state_offset);
            if (clen == 0) printf("EOL\n");
            else if (c > 32 && c < 127) printf("'%c'\n", c);
            else printf("0x%02x\n", c);
        #endif
            ims = current_state->ims;
            if (state_offset < 0) {
                if (current_state->data > 0) {
                    DPRINTF(("%.*sSkipping this character\n", rlevel*2-2, SP));
                    ADD_NEW_DATA(state_offset, current_state->count,current_state->data - 1);
                    continue;
                } else current_state->offset = state_offset = -state_offset;
            }
            for (j = 0; j < i; j++) {
                if (active_states[j].offset == state_offset && active_states[j].count == current_state->count) {
                    DPRINTF(("%.*sDuplicate state: skipped\n", rlevel*2-2, SP));
                    goto NEXT_ACTIVE_STATE;
                }
            }
            code = start_code + state_offset;
            codevalue = *code;
            if (clen == 0 && poptable[codevalue] != 0) could_continue = TRUE;
            if (coptable[codevalue] > 0) {
                dlen = 1;
            #ifdef SUPPORT_UTF8
                if (utf8) { GETCHARLEN(d, (code + coptable[codevalue]), dlen); } else
            #endif
                d = code[coptable[codevalue]];
                if (codevalue >= OP_TYPESTAR) {
                    switch(d) {
                        case OP_ANYBYTE: return PCRE_ERROR_DFA_UITEM;
                        case OP_NOTPROP:
                        case OP_PROP: codevalue += OP_PROP_EXTRA; break;
                        case OP_ANYNL: codevalue += OP_ANYNL_EXTRA; break;
                        case OP_EXTUNI: codevalue += OP_EXTUNI_EXTRA; break;
                        case OP_NOT_HSPACE:
                        case OP_HSPACE: codevalue += OP_HSPACE_EXTRA; break;
                        case OP_NOT_VSPACE:
                        case OP_VSPACE: codevalue += OP_VSPACE_EXTRA; break;
                        default: break;
                    }
                }
            } else {
              dlen = 0;
              d = NOTACHAR;
            }
            switch (codevalue) {
                case OP_TABLE_LENGTH: case OP_TABLE_LENGTH + ((sizeof(coptable) == OP_TABLE_LENGTH) && (sizeof(poptable) == OP_TABLE_LENGTH)): break;
                case OP_KET: case OP_KETRMIN: case OP_KETRMAX:
                    if (code != end_code) {
                        ADD_ACTIVE(state_offset + 1 /*+ LINK_SIZE*/, 0);
                        if (codevalue != OP_KET) {
                            ADD_ACTIVE(state_offset /*- GET(code, 1)*/, 0);
                        }
                    } else {
                        if (ptr > current_subject || ((md->moptions & PCRE_NOTEMPTY) == 0 && ((md->moptions & PCRE_NOTEMPTY_ATSTART) == 0 ||
                            current_subject > start_subject + md->start_offset))) {
                            if (match_count < 0) match_count = (offsetcount >= 2)? 1 : 0;
                            else if (match_count > 0 && ++match_count * 2 >= offsetcount) match_count = 0;
                            count = ((match_count == 0)? offsetcount : match_count * 2) - 2;
                            if (count > 0) memmove(offsets + 2, offsets, count * sizeof(int));
                            if (offsetcount >= 2) {
                                offsets[0] = (int)(current_subject - start_subject);
                                offsets[1] = (int)(ptr - start_subject);
                                DPRINTF(("%.*sSet matched string = \"%.*s\"\n", rlevel*2-2, SP, offsets[1] - offsets[0], current_subject));
                            }
                            if ((md->moptions & PCRE_DFA_SHORTEST) != 0) {
                                DPRINTF(("%.*sEnd of internal_dfa_exec %d: returning %d\n%.*s---------------------\n\n", rlevel*2-2, SP, rlevel,
                                        match_count, rlevel*2-2, SP));
                                return match_count;
                            }
                        }
                    }
                    break;
                case OP_ALT:
                    //do { code += GET(code, 1); } while (*code == OP_ALT);
                    ADD_ACTIVE((int)(code - start_code), 0);
                    break;
                case OP_BRA: case OP_SBRA:
                    do {
                        ADD_ACTIVE((int)(code - start_code + 1/* + LINK_SIZE*/), 0);
                        //code += GET(code, 1);
                    } while (*code == OP_ALT);
                    break;
                case OP_CBRA: case OP_SCBRA:
                    ADD_ACTIVE((int)(code - start_code + 3 /*+ LINK_SIZE*/),  0);
                    //code += GET(code, 1);
                    while (*code == OP_ALT) {
                        ADD_ACTIVE((int)(code - start_code + 1 /*+ LINK_SIZE*/),  0);
                        //code += GET(code, 1);
                    }
                    break;
                case OP_BRAZERO: case OP_BRAMINZERO:
                    ADD_ACTIVE(state_offset + 1, 0);
                    //code += 1 + GET(code, 2);
                    //while (*code == OP_ALT) code += GET(code, 1);
                    ADD_ACTIVE((int)(code - start_code + 1 /*+ LINK_SIZE*/), 0);
                    break;
                case OP_SKIPZERO:
                    //code += 1 + GET(code, 2);
                    //while (*code == OP_ALT) code += GET(code, 1);
                    ADD_ACTIVE((int)(code - start_code + 1 /*+ LINK_SIZE*/), 0);
                    break;
                case OP_CIRC:
                    if ((ptr == start_subject && (md->moptions & PCRE_NOTBOL) == 0) || ((ims & PCRE_MULTILINE) != 0 && ptr != end_subject && WAS_NEWLINE(ptr))) {
                        ADD_ACTIVE(state_offset + 1, 0);
                    }
                    break;
                case OP_EOD:
                    if (ptr >= end_subject) {
                        if ((md->moptions & PCRE_PARTIAL_HARD) != 0) could_continue = TRUE;
                        else { ADD_ACTIVE(state_offset + 1, 0); }
                    }
                    break;
                case OP_OPT:
                    ims = code[1];
                    ADD_ACTIVE(state_offset + 2, 0);
                    break;
                case OP_SOD:
                    if (ptr == start_subject) { ADD_ACTIVE(state_offset + 1, 0); }
                    break;
                case OP_SOM:
                    if (ptr == start_subject + start_offset) { ADD_ACTIVE(state_offset + 1, 0); }
                    break;
                case OP_ANY:
                    if (clen > 0 && !IS_NEWLINE(ptr)) { ADD_NEW(state_offset + 1, 0); }
                    break;
                case OP_ALLANY:
                    if (clen > 0) { ADD_NEW(state_offset + 1, 0); }
                    break;
                case OP_EODN:
                    if (clen == 0 && (md->moptions & PCRE_PARTIAL_HARD) != 0) could_continue = TRUE;
                    else if (clen == 0 || (IS_NEWLINE(ptr) && ptr == end_subject - md->nllen)) { ADD_ACTIVE(state_offset + 1, 0); }
                    break;
                case OP_DOLL:
                    if ((md->moptions & PCRE_NOTEOL) == 0) {
                        if (clen == 0 && (md->moptions & PCRE_PARTIAL_HARD) != 0) could_continue = TRUE;
                        else if (clen == 0 || ((md->poptions & PCRE_DOLLAR_ENDONLY) == 0 && IS_NEWLINE(ptr) && ((ims & PCRE_MULTILINE) != 0 ||
                                 ptr == end_subject - md->nllen))) {
                            ADD_ACTIVE(state_offset + 1, 0);
                        }
                    } else if ((ims & PCRE_MULTILINE) != 0 && IS_NEWLINE(ptr)) { ADD_ACTIVE(state_offset + 1, 0); }
                    break;
                case OP_DIGIT: case OP_WHITESPACE: case OP_WORDCHAR:
                    if (clen > 0 && c < 256 && ((ctypes[c] & toptable1[codevalue]) ^ toptable2[codevalue]) != 0) { ADD_NEW(state_offset + 1, 0); }
                    break;
                case OP_NOT_DIGIT: case OP_NOT_WHITESPACE: case OP_NOT_WORDCHAR:
                    if (clen > 0 && (c >= 256 || ((ctypes[c] & toptable1[codevalue]) ^ toptable2[codevalue]) != 0)) { ADD_NEW(state_offset + 1, 0); }
                    break;
                case OP_WORD_BOUNDARY: case OP_NOT_WORD_BOUNDARY: {
                        int left_word, right_word;
                        if (ptr > start_subject) {
                            const uschar *temp = ptr - 1;
                            if (temp < md->start_used_ptr) md->start_used_ptr = temp;
                        #ifdef SUPPORT_UTF8
                            if (utf8) BACKCHAR(temp);
                        #endif
                            GETCHARTEST(d, temp);
                        #ifdef SUPPORT_UCP
                            if ((md->poptions & PCRE_UCP) != 0) {
                                if (d == '_') left_word = TRUE;
                                else {
                                    int cat = UCD_CATEGORY(d);
                                    left_word = (cat == ucp_L || cat == ucp_N);
                                }
                            } else
                        #endif
                            left_word = d < 256 && (ctypes[d] & ctype_word) != 0;
                        } else left_word = FALSE;
                        if (clen > 0) {
                        #ifdef SUPPORT_UCP
                            if ((md->poptions & PCRE_UCP) != 0) {
                                if (c == '_') right_word = TRUE;
                                else {
                                    int cat = UCD_CATEGORY(c);
                                    right_word = (cat == ucp_L || cat == ucp_N);
                                }
                            } else
                        #endif
                            right_word = c < 256 && (ctypes[c] & ctype_word) != 0;
                        } else right_word = FALSE;
                        if ((left_word == right_word) == (codevalue == OP_NOT_WORD_BOUNDARY)) { ADD_ACTIVE(state_offset + 1, 0); }
                    }
                    break;
            #ifdef SUPPORT_UCP
                case OP_PROP: case OP_NOTPROP:
                    if (clen > 0) {
                        BOOL OK;
                        int chartype = UCD_CHARTYPE(c);
                        switch(code[1]) {
                            case PT_ANY: OK = TRUE; break;
                            case PT_LAMP: OK = chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt; break;
                            case PT_GC: OK = _pcre_ucp_gentype[chartype] == code[2]; break;
                            case PT_PC: OK = chartype == code[2]; break;
                            case PT_SC: OK = UCD_SCRIPT(c) == code[2]; break;
                            case PT_ALNUM: OK = _pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N; break;
                            case PT_SPACE: OK = _pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR; break;
                            case PT_PXSPACE:
                                OK = _pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR;
                                break;
                            case PT_WORD: OK = _pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N ||  c == CHAR_UNDERSCORE; break;
                            default: OK = codevalue != OP_PROP; break;
                        }
                        if (OK == (codevalue == OP_PROP)) { ADD_NEW(state_offset + 3, 0); }
                    }
                    break;
            #endif
                case OP_TYPEPLUS: case OP_TYPEMINPLUS: case OP_TYPEPOSPLUS:
                    count = current_state->count;  /* Already matched */
                    if (count > 0) { ADD_ACTIVE(state_offset + 2, 0); }
                    if (clen > 0) {
                        if ((c >= 256 && d != OP_DIGIT && d != OP_WHITESPACE && d != OP_WORDCHAR) || (c < 256 && (d != OP_ANY || !IS_NEWLINE(ptr)) &&
                            ((ctypes[c] & toptable1[d]) ^ toptable2[d]) != 0)) {
                            if (count > 0 && codevalue == OP_TYPEPOSPLUS) {
                                active_count--;
                                next_active_state--;
                            }
                            count++;
                            ADD_NEW(state_offset, count);
                        }
                    }
                    break;
                case OP_TYPEQUERY: case OP_TYPEMINQUERY: case OP_TYPEPOSQUERY:
                    ADD_ACTIVE(state_offset + 2, 0);
                    if (clen > 0) {
                        if ((c >= 256 && d != OP_DIGIT && d != OP_WHITESPACE && d != OP_WORDCHAR) || (c < 256 && (d != OP_ANY || !IS_NEWLINE(ptr)) &&
                            ((ctypes[c] & toptable1[d]) ^ toptable2[d]) != 0)) {
                            if (codevalue == OP_TYPEPOSQUERY) {
                                active_count--;
                                next_active_state--;
                            }
                            ADD_NEW(state_offset + 2, 0);
                        }
                    }
                    break;
                case OP_TYPESTAR: case OP_TYPEMINSTAR: case OP_TYPEPOSSTAR:
                    ADD_ACTIVE(state_offset + 2, 0);
                    if (clen > 0) {
                        if ((c >= 256 && d != OP_DIGIT && d != OP_WHITESPACE && d != OP_WORDCHAR) || (c < 256 && (d != OP_ANY || !IS_NEWLINE(ptr)) &&
                            ((ctypes[c] & toptable1[d]) ^ toptable2[d]) != 0)) {
                            if (codevalue == OP_TYPEPOSSTAR) {
                                active_count--;
                                next_active_state--;
                            }
                            ADD_NEW(state_offset, 0);
                        }
                    }
                    break;
                case OP_TYPEEXACT:
                    count = current_state->count;
                    if (clen > 0) {
                        if ((c >= 256 && d != OP_DIGIT && d != OP_WHITESPACE && d != OP_WORDCHAR) || (c < 256 && (d != OP_ANY || !IS_NEWLINE(ptr)) &&
                            ((ctypes[c] & toptable1[d]) ^ toptable2[d]) != 0)) {
                            if (++count >= GET2(code, 1)) { ADD_NEW(state_offset + 4, 0); }
                            else { ADD_NEW(state_offset, count); }
                        }
                    }
                    break;
                case OP_TYPEUPTO: case OP_TYPEMINUPTO: case OP_TYPEPOSUPTO:
                    ADD_ACTIVE(state_offset + 4, 0);
                    count = current_state->count;
                    if (clen > 0) {
                        if ((c >= 256 && d != OP_DIGIT && d != OP_WHITESPACE && d != OP_WORDCHAR) || (c < 256 && (d != OP_ANY || !IS_NEWLINE(ptr)) &&
                            ((ctypes[c] & toptable1[d]) ^ toptable2[d]) != 0)) {
                            if (codevalue == OP_TYPEPOSUPTO) {
                                active_count--;
                                next_active_state--;
                            }
                            if (++count >= GET2(code, 1)) { ADD_NEW(state_offset + 4, 0); }
                            else { ADD_NEW(state_offset, count); }
                        }
                    }
                    break;
            #ifdef SUPPORT_UCP
                case OP_PROP_EXTRA + OP_TYPEPLUS: case OP_PROP_EXTRA + OP_TYPEMINPLUS: case OP_PROP_EXTRA + OP_TYPEPOSPLUS:
                    count = current_state->count;
                    if (count > 0) { ADD_ACTIVE(state_offset + 4, 0); }
                    if (clen > 0) {
                        BOOL OK;
                        int chartype = UCD_CHARTYPE(c);
                        switch(code[2]) {
                            case PT_ANY: OK = TRUE; break;
                            case PT_LAMP: OK = chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt; break;
                            case PT_GC: OK = _pcre_ucp_gentype[chartype] == code[3]; break;
                            case PT_PC: OK = chartype == code[3]; break;
                            case PT_SC: OK = UCD_SCRIPT(c) == code[3]; break;
                            case PT_ALNUM: OK = _pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N; break;
                            case PT_SPACE: OK = _pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR; break;
                            case PT_PXSPACE:
                                OK = _pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR;
                                break;
                            case PT_WORD: OK = _pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N || c == CHAR_UNDERSCORE; break;
                            default: OK = codevalue != OP_PROP; break;
                        }
                        if (OK == (d == OP_PROP)) {
                            if (count > 0 && codevalue == OP_PROP_EXTRA + OP_TYPEPOSPLUS) {
                                active_count--;
                                next_active_state--;
                            }
                            count++;
                            ADD_NEW(state_offset, count);
                        }
                    }
                    break;
                case OP_EXTUNI_EXTRA + OP_TYPEPLUS: case OP_EXTUNI_EXTRA + OP_TYPEMINPLUS: case OP_EXTUNI_EXTRA + OP_TYPEPOSPLUS:
                    count = current_state->count;
                    if (count > 0) { ADD_ACTIVE(state_offset + 2, 0); }
                    if (clen > 0 && UCD_CATEGORY(c) != ucp_M) {
                        const uschar *nptr = ptr + clen;
                        int ncount = 0;
                        if (count > 0 && codevalue == OP_EXTUNI_EXTRA + OP_TYPEPOSPLUS) {
                            active_count--;
                            next_active_state--;
                        }
                        while (nptr < end_subject) {
                            int nd;
                            int ndlen = 1;
                            GETCHARLEN(nd, nptr, ndlen);
                            if (UCD_CATEGORY(nd) != ucp_M) break;
                            ncount++;
                            nptr += ndlen;
                        }
                        count++;
                        ADD_NEW_DATA(-state_offset, count, ncount);
                    }
                    break;
            #endif
                case OP_ANYNL_EXTRA + OP_TYPEPLUS: case OP_ANYNL_EXTRA + OP_TYPEMINPLUS: case OP_ANYNL_EXTRA + OP_TYPEPOSPLUS:
                    count = current_state->count;
                    if (count > 0) { ADD_ACTIVE(state_offset + 2, 0); }
                    if (clen > 0) {
                        int ncount = 0;
                        switch(c) {
                            case 0x000b: case 0x000c: case 0x0085: case 0x2028: case 0x2029:
                            if ((md->moptions & PCRE_BSR_ANYCRLF) != 0) break;
                            goto ANYNL01;
                            case 0x000d:
                            if (ptr + 1 < end_subject && ptr[1] == 0x0a) ncount = 1;
                            ANYNL01:
                            case 0x000a:
                                if (count > 0 && codevalue == OP_ANYNL_EXTRA + OP_TYPEPOSPLUS) {
                                    active_count--;
                                    next_active_state--;
                                }
                                count++;
                                ADD_NEW_DATA(-state_offset, count, ncount);
                                break;
                            default: break;
                        }
                    }
                    break;
                case OP_VSPACE_EXTRA + OP_TYPEPLUS: case OP_VSPACE_EXTRA + OP_TYPEMINPLUS: case OP_VSPACE_EXTRA + OP_TYPEPOSPLUS:
                    count = current_state->count;
                    if (count > 0) { ADD_ACTIVE(state_offset + 2, 0); }
                    if (clen > 0) {
                    BOOL OK;
                    switch(c) {
                        case 0x000a: case 0x000b: case 0x000c: case 0x000d: case 0x0085: case 0x2028: case 0x2029: OK = TRUE; break;
                        default: OK = FALSE; break;
                    }
                    if (OK == (d == OP_VSPACE)) {
                        if (count > 0 && codevalue == OP_VSPACE_EXTRA + OP_TYPEPOSPLUS) {
                            active_count--;
                            next_active_state--;
                        }
                        count++;
                        ADD_NEW_DATA(-state_offset, count, 0);
                        }
                    }
                    break;
                case OP_HSPACE_EXTRA + OP_TYPEPLUS: case OP_HSPACE_EXTRA + OP_TYPEMINPLUS: case OP_HSPACE_EXTRA + OP_TYPEPOSPLUS:
                    count = current_state->count;
                    if (count > 0) { ADD_ACTIVE(state_offset + 2, 0); }
                    if (clen > 0) {
                    BOOL OK;
                    switch(c) {
                        case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                        case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                            OK = TRUE;
                            break;
                        default: OK = FALSE; break;
                    }
                    if (OK == (d == OP_HSPACE)) {
                        if (count > 0 && codevalue == OP_HSPACE_EXTRA + OP_TYPEPOSPLUS) {
                            active_count--;
                            next_active_state--;
                        }
                        count++;
                        ADD_NEW_DATA(-state_offset, count, 0);
                        }
                    }
                    break;
            #ifdef SUPPORT_UCP
                case OP_PROP_EXTRA + OP_TYPEQUERY: case OP_PROP_EXTRA + OP_TYPEMINQUERY: case OP_PROP_EXTRA + OP_TYPEPOSQUERY: count = 4; goto QS1;
                case OP_PROP_EXTRA + OP_TYPESTAR: case OP_PROP_EXTRA + OP_TYPEMINSTAR: case OP_PROP_EXTRA + OP_TYPEPOSSTAR:
                    count = 0;
                    QS1:
                    ADD_ACTIVE(state_offset + 4, 0);
                    if (clen > 0) {
                        BOOL OK;
                        int chartype = UCD_CHARTYPE(c);
                        switch(code[2]) {
                            case PT_ANY: OK = TRUE; break;
                            case PT_LAMP: OK = chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt; break;
                            case PT_GC: OK = _pcre_ucp_gentype[chartype] == code[3]; break;
                            case PT_PC: OK = chartype == code[3]; break;
                            case PT_SC: OK = UCD_SCRIPT(c) == code[3]; break;
                            case PT_ALNUM: OK = _pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N; break;
                            case PT_SPACE: OK = _pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR; break;
                            case PT_PXSPACE:
                                OK = _pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR;
                                break;
                            case PT_WORD:
                                OK = _pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N || c == CHAR_UNDERSCORE;
                                break;
                            default: OK = codevalue != OP_PROP; break;
                        }
                        if (OK == (d == OP_PROP)) {
                            if (codevalue == OP_PROP_EXTRA + OP_TYPEPOSSTAR || codevalue == OP_PROP_EXTRA + OP_TYPEPOSQUERY) {
                                active_count--;
                                next_active_state--;
                            }
                            ADD_NEW(state_offset + count, 0);
                        }
                    }
                    break;
                case OP_EXTUNI_EXTRA + OP_TYPEQUERY: case OP_EXTUNI_EXTRA + OP_TYPEMINQUERY: case OP_EXTUNI_EXTRA + OP_TYPEPOSQUERY: count = 2; goto QS2;
                case OP_EXTUNI_EXTRA + OP_TYPESTAR: case OP_EXTUNI_EXTRA + OP_TYPEMINSTAR: case OP_EXTUNI_EXTRA + OP_TYPEPOSSTAR:
                    count = 0;
                    QS2:
                    ADD_ACTIVE(state_offset + 2, 0);
                    if (clen > 0 && UCD_CATEGORY(c) != ucp_M) {
                        const uschar *nptr = ptr + clen;
                        int ncount = 0;
                        if (codevalue == OP_EXTUNI_EXTRA + OP_TYPEPOSSTAR || codevalue == OP_EXTUNI_EXTRA + OP_TYPEPOSQUERY) {
                            active_count--;
                            next_active_state--;
                        }
                        while(nptr < end_subject) {
                            int nd;
                            int ndlen = 1;
                            GETCHARLEN(nd, nptr, ndlen);
                            if (UCD_CATEGORY(nd) != ucp_M) break;
                            ncount++;
                            nptr += ndlen;
                        }
                        ADD_NEW_DATA(-(state_offset + count), 0, ncount);
                    }
                    break;
            #endif
                case OP_ANYNL_EXTRA + OP_TYPEQUERY: case OP_ANYNL_EXTRA + OP_TYPEMINQUERY: case OP_ANYNL_EXTRA + OP_TYPEPOSQUERY: count = 2; goto QS3;
                case OP_ANYNL_EXTRA + OP_TYPESTAR: case OP_ANYNL_EXTRA + OP_TYPEMINSTAR: case OP_ANYNL_EXTRA + OP_TYPEPOSSTAR:
                    count = 0;
                    QS3:
                    ADD_ACTIVE(state_offset + 2, 0);
                    if (clen > 0) {
                        int ncount = 0;
                        switch(c) {
                            case 0x000b: case 0x000c: case 0x0085: case 0x2028: case 0x2029:
                            if ((md->moptions & PCRE_BSR_ANYCRLF) != 0) break;
                            goto ANYNL02;
                            case 0x000d: if (ptr + 1 < end_subject && ptr[1] == 0x0a) ncount = 1;
                            ANYNL02:
                            case 0x000a:
                            if (codevalue == OP_ANYNL_EXTRA + OP_TYPEPOSSTAR || codevalue == OP_ANYNL_EXTRA + OP_TYPEPOSQUERY) {
                            active_count--;
                            next_active_state--;
                            }
                            ADD_NEW_DATA(-(state_offset + count), 0, ncount);
                            break;
                            default: break;
                        }
                    }
                    break;
                case OP_VSPACE_EXTRA + OP_TYPEQUERY: case OP_VSPACE_EXTRA + OP_TYPEMINQUERY: case OP_VSPACE_EXTRA + OP_TYPEPOSQUERY: count = 2; goto QS4;
                case OP_VSPACE_EXTRA + OP_TYPESTAR: case OP_VSPACE_EXTRA + OP_TYPEMINSTAR: case OP_VSPACE_EXTRA + OP_TYPEPOSSTAR:
                    count = 0;
                    QS4:
                    ADD_ACTIVE(state_offset + 2, 0);
                    if (clen > 0) {
                        BOOL OK;
                        switch(c) {
                            case 0x000a: case 0x000b: case 0x000c: case 0x000d: case 0x0085: case 0x2028: case 0x2029: OK = TRUE; break;
                            default: OK = FALSE; break;
                        }
                        if (OK == (d == OP_VSPACE)) {
                            if (codevalue == OP_VSPACE_EXTRA + OP_TYPEPOSSTAR || codevalue == OP_VSPACE_EXTRA + OP_TYPEPOSQUERY) {
                                active_count--;
                                next_active_state--;
                            }
                            ADD_NEW_DATA(-(state_offset + count), 0, 0);
                        }
                    }
                    break;
                case OP_HSPACE_EXTRA + OP_TYPEQUERY: case OP_HSPACE_EXTRA + OP_TYPEMINQUERY: case OP_HSPACE_EXTRA + OP_TYPEPOSQUERY: count = 2; goto QS5;
                case OP_HSPACE_EXTRA + OP_TYPESTAR: case OP_HSPACE_EXTRA + OP_TYPEMINSTAR: case OP_HSPACE_EXTRA + OP_TYPEPOSSTAR:
                    count = 0;
                    QS5:
                    ADD_ACTIVE(state_offset + 2, 0);
                    if (clen > 0) {
                        BOOL OK;
                        switch(c) {
                            case 0x09:case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                            case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A:  case 0x202f: case 0x205f: case 0x3000:
                            OK = TRUE;
                            break;
                            default: OK = FALSE; break;
                        }
                        if (OK == (d == OP_HSPACE)) {
                            if (codevalue == OP_HSPACE_EXTRA + OP_TYPEPOSSTAR || codevalue == OP_HSPACE_EXTRA + OP_TYPEPOSQUERY) {
                                active_count--;
                                next_active_state--;
                            }
                            ADD_NEW_DATA(-(state_offset + count), 0, 0);
                        }
                    }
                    break;
            #ifdef SUPPORT_UCP
                case OP_PROP_EXTRA + OP_TYPEEXACT: case OP_PROP_EXTRA + OP_TYPEUPTO: case OP_PROP_EXTRA + OP_TYPEMINUPTO: case OP_PROP_EXTRA + OP_TYPEPOSUPTO:
                    if (codevalue != OP_PROP_EXTRA + OP_TYPEEXACT) { ADD_ACTIVE(state_offset + 6, 0); }
                    count = current_state->count;  /* Number already matched */
                    if (clen > 0) {
                        BOOL OK;
                        int chartype = UCD_CHARTYPE(c);
                        switch(code[4]) {
                            case PT_ANY: OK = TRUE; break;
                            case PT_LAMP: OK = chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt; break;
                            case PT_GC: OK = _pcre_ucp_gentype[chartype] == code[5]; break;
                            case PT_PC: OK = chartype == code[5]; break;
                            case PT_SC: OK = UCD_SCRIPT(c) == code[5]; break;
                            case PT_ALNUM: OK = _pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N; break;
                            case PT_SPACE: OK = _pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR; break;
                            case PT_PXSPACE:
                                OK = _pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR;
                                break;
                            case PT_WORD:
                                OK = _pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N || c == CHAR_UNDERSCORE;
                                break;
                            default: OK = codevalue != OP_PROP; break;
                        }
                        if (OK == (d == OP_PROP)) {
                            if (codevalue == OP_PROP_EXTRA + OP_TYPEPOSUPTO) {
                                active_count--;
                                next_active_state--;
                            }
                            if (++count >= GET2(code, 1)) { ADD_NEW(state_offset + 6, 0); }
                            else { ADD_NEW(state_offset, count); }
                        }
                    }
                    break;
                case OP_EXTUNI_EXTRA + OP_TYPEEXACT: case OP_EXTUNI_EXTRA + OP_TYPEUPTO: case OP_EXTUNI_EXTRA + OP_TYPEMINUPTO: case OP_EXTUNI_EXTRA + OP_TYPEPOSUPTO:
                    if (codevalue != OP_EXTUNI_EXTRA + OP_TYPEEXACT) { ADD_ACTIVE(state_offset + 4, 0); }
                    count = current_state->count;  /* Number already matched */
                    if (clen > 0 && UCD_CATEGORY(c) != ucp_M) {
                        const uschar *nptr = ptr + clen;
                        int ncount = 0;
                        if (codevalue == OP_EXTUNI_EXTRA + OP_TYPEPOSUPTO) {
                            active_count--;
                            next_active_state--;
                        }
                        while (nptr < end_subject) {
                            int nd;
                            int ndlen = 1;
                            GETCHARLEN(nd, nptr, ndlen);
                            if (UCD_CATEGORY(nd) != ucp_M) break;
                            ncount++;
                            nptr += ndlen;
                        }
                        if (++count >= GET2(code, 1)) { ADD_NEW_DATA(-(state_offset + 4), 0, ncount); }
                        else { ADD_NEW_DATA(-state_offset, count, ncount); }
                    }
                    break;
            #endif
                case OP_ANYNL_EXTRA + OP_TYPEEXACT: case OP_ANYNL_EXTRA + OP_TYPEUPTO: case OP_ANYNL_EXTRA + OP_TYPEMINUPTO: case OP_ANYNL_EXTRA + OP_TYPEPOSUPTO:
                    if (codevalue != OP_ANYNL_EXTRA + OP_TYPEEXACT) { ADD_ACTIVE(state_offset + 4, 0); }
                    count = current_state->count;  /* Number already matched */
                    if (clen > 0) {
                        int ncount = 0;
                        switch(c) {
                            case 0x000b: case 0x000c: case 0x0085: case 0x2028: case 0x2029:
                                if ((md->moptions & PCRE_BSR_ANYCRLF) != 0) break;
                                goto ANYNL03;
                            case 0x000d: if (ptr + 1 < end_subject && ptr[1] == 0x0a) ncount = 1;
                            ANYNL03:
                            case 0x000a:
                                if (codevalue == OP_ANYNL_EXTRA + OP_TYPEPOSUPTO) {
                                    active_count--;
                                    next_active_state--;
                                }
                                if (++count >= GET2(code, 1)) { ADD_NEW_DATA(-(state_offset + 4), 0, ncount); }
                                else { ADD_NEW_DATA(-state_offset, count, ncount); }
                                break;
                            default: break;
                        }
                    }
                    break;
                case OP_VSPACE_EXTRA + OP_TYPEEXACT: case OP_VSPACE_EXTRA + OP_TYPEUPTO: case OP_VSPACE_EXTRA + OP_TYPEMINUPTO:
                case OP_VSPACE_EXTRA + OP_TYPEPOSUPTO:
                    if (codevalue != OP_VSPACE_EXTRA + OP_TYPEEXACT) { ADD_ACTIVE(state_offset + 4, 0); }
                    count = current_state->count;
                    if (clen > 0) {
                        BOOL OK;
                        switch(c) {
                            case 0x000a: case 0x000b: case 0x000c: case 0x000d: case 0x0085: case 0x2028: case 0x2029: OK = TRUE; break;
                            default: OK = FALSE;
                        }
                        if (OK == (d == OP_VSPACE)) {
                            if (codevalue == OP_VSPACE_EXTRA + OP_TYPEPOSUPTO) {
                            active_count--;
                            next_active_state--;
                            }
                            if (++count >= GET2(code, 1)) { ADD_NEW_DATA(-(state_offset + 4), 0, 0); }
                            else { ADD_NEW_DATA(-state_offset, count, 0); }
                        }
                    }
                    break;
                case OP_HSPACE_EXTRA + OP_TYPEEXACT: case OP_HSPACE_EXTRA + OP_TYPEUPTO: case OP_HSPACE_EXTRA + OP_TYPEMINUPTO:
                case OP_HSPACE_EXTRA + OP_TYPEPOSUPTO:
                    if (codevalue != OP_HSPACE_EXTRA + OP_TYPEEXACT) { ADD_ACTIVE(state_offset + 4, 0); }
                    count = current_state->count;
                    if (clen > 0) {
                        BOOL OK;
                        switch(c) {
                            case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                            case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                                OK = TRUE;
                                break;

                            default: OK = FALSE; break;
                        }
                        if (OK == (d == OP_HSPACE)) {
                            if (codevalue == OP_HSPACE_EXTRA + OP_TYPEPOSUPTO) {
                                active_count--;
                                next_active_state--;
                            }
                            if (++count >= GET2(code, 1)) { ADD_NEW_DATA(-(state_offset + 4), 0, 0); }
                            else { ADD_NEW_DATA(-state_offset, count, 0); }
                        }
                    }
                    break;
                case OP_CHAR: if (clen > 0 && c == d) { ADD_NEW(state_offset + dlen + 1, 0); } break;
                case OP_CHARNC:
                    if (clen == 0) break;
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        if (c == d) { ADD_NEW(state_offset + dlen + 1, 0); } else {
                            unsigned int othercase;
                            if (c < 128) othercase = fcc[c];
                            else
                        #ifdef SUPPORT_UCP
                            othercase = UCD_OTHERCASE(c);
                        #else
                            othercase = NOTACHAR;
                        #endif
                            if (d == othercase) { ADD_NEW(state_offset + dlen + 1, 0); }
                        }
                    } else
                #endif
                    {
                        if (lcc[c] == lcc[d]) { ADD_NEW(state_offset + 2, 0); }
                    }
                    break;
            #ifdef SUPPORT_UCP
                case OP_EXTUNI:
                    if (clen > 0 && UCD_CATEGORY(c) != ucp_M) {
                        const uschar *nptr = ptr + clen;
                        int ncount = 0;
                        while (nptr < end_subject) {
                            int nclen = 1;
                            GETCHARLEN(c, nptr, nclen);
                            if (UCD_CATEGORY(c) != ucp_M) break;
                            ncount++;
                            nptr += nclen;
                        }
                        ADD_NEW_DATA(-(state_offset + 1), 0, ncount);
                    }
                    break;
            #endif
                case OP_ANYNL:
                    if (clen > 0)
                        switch(c) {
                            case 0x000b: case 0x000c: case 0x0085: case 0x2028: case 0x2029: if ((md->moptions & PCRE_BSR_ANYCRLF) != 0) break;
                            case 0x000a: ADD_NEW(state_offset + 1, 0); break;
                            case 0x000d:
                                if (ptr + 1 < end_subject && ptr[1] == 0x0a) { ADD_NEW_DATA(-(state_offset + 1), 0, 1); }
                                else { ADD_NEW(state_offset + 1, 0); }
                                break;
                        }
                        break;
                case OP_NOT_VSPACE:
                    if (clen > 0)
                        switch(c) {
                            case 0x000a: case 0x000b: case 0x000c: case 0x000d: case 0x0085: case 0x2028: case 0x2029: break;
                            default: ADD_NEW(state_offset + 1, 0); break;
                        }
                    break;
                case OP_VSPACE:
                    if (clen > 0)
                        switch(c) {
                            case 0x000a: case 0x000b: case 0x000c: case 0x000d: case 0x0085: case 0x2028: case 0x2029: ADD_NEW(state_offset + 1, 0); break;
                            default: break;
                        }
                    break;
                case OP_NOT_HSPACE:
                    if (clen > 0)
                        switch(c) {
                            case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                            case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                                break;
                            default: ADD_NEW(state_offset + 1, 0); break;
                        }
                    break;
                case OP_HSPACE:
                    if (clen > 0)
                        switch(c) {
                            case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                            case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                                ADD_NEW(state_offset + 1, 0);
                                break;
                        }
                    break;
                case OP_NOT:
                    if (clen > 0) {
                        unsigned int otherd = ((ims & PCRE_CASELESS) != 0)? fcc[d] : d;
                        if (c != d && c != otherd) { ADD_NEW(state_offset + dlen + 1, 0); }
                    }
                    break;
                case OP_PLUS: case OP_MINPLUS: case OP_POSPLUS: case OP_NOTPLUS: case OP_NOTMINPLUS: case OP_NOTPOSPLUS:
                    count = current_state->count;  /* Already matched */
                    if (count > 0) { ADD_ACTIVE(state_offset + dlen + 1, 0); }
                    if (clen > 0) {
                        unsigned int otherd = NOTACHAR;
                        if ((ims & PCRE_CASELESS) != 0) {
                        #ifdef SUPPORT_UTF8
                            if (utf8 && d >= 128) {
                            #ifdef SUPPORT_UCP
                                otherd = UCD_OTHERCASE(d);
                            #endif
                            } else
                        #endif
                            otherd = fcc[d];
                        }
                        if ((c == d || c == otherd) == (codevalue < OP_NOTSTAR)) {
                            if (count > 0 && (codevalue == OP_POSPLUS || codevalue == OP_NOTPOSPLUS)) {
                                active_count--;
                                next_active_state--;
                            }
                            count++;
                            ADD_NEW(state_offset, count);
                        }
                    }
                    break;
                case OP_QUERY: case OP_MINQUERY: case OP_POSQUERY: case OP_NOTQUERY: case OP_NOTMINQUERY: case OP_NOTPOSQUERY:
                    ADD_ACTIVE(state_offset + dlen + 1, 0);
                    if (clen > 0) {
                        unsigned int otherd = NOTACHAR;
                        if ((ims & PCRE_CASELESS) != 0) {
                        #ifdef SUPPORT_UTF8
                            if (utf8 && d >= 128) {
                            #ifdef SUPPORT_UCP
                                otherd = UCD_OTHERCASE(d);
                            #endif
                            } else
                        #endif
                            otherd = fcc[d];
                        }
                        if ((c == d || c == otherd) == (codevalue < OP_NOTSTAR)) {
                            if (codevalue == OP_POSQUERY || codevalue == OP_NOTPOSQUERY) {
                                active_count--;
                                next_active_state--;
                            }
                            ADD_NEW(state_offset + dlen + 1, 0);
                        }
                    }
                    break;
                case OP_STAR: case OP_MINSTAR: case OP_POSSTAR: case OP_NOTSTAR: case OP_NOTMINSTAR: case OP_NOTPOSSTAR:
                    ADD_ACTIVE(state_offset + dlen + 1, 0);
                    if (clen > 0) {
                        unsigned int otherd = NOTACHAR;
                        if ((ims & PCRE_CASELESS) != 0) {
                        #ifdef SUPPORT_UTF8
                            if (utf8 && d >= 128) {
                            #ifdef SUPPORT_UCP
                                otherd = UCD_OTHERCASE(d);
                            #endif
                            } else
                        #endif
                            otherd = fcc[d];
                        }
                        if ((c == d || c == otherd) == (codevalue < OP_NOTSTAR)) {
                            if (codevalue == OP_POSSTAR || codevalue == OP_NOTPOSSTAR) {
                                active_count--;
                                next_active_state--;
                            }
                            ADD_NEW(state_offset, 0);
                        }
                    }
                    break;
                case OP_EXACT: case OP_NOTEXACT:
                    count = current_state->count;
                    if (clen > 0) {
                        unsigned int otherd = NOTACHAR;
                        if ((ims & PCRE_CASELESS) != 0) {
                        #ifdef SUPPORT_UTF8
                            if (utf8 && d >= 128) {
                            #ifdef SUPPORT_UCP
                                otherd = UCD_OTHERCASE(d);
                            #endif
                            } else
                        #endif
                          otherd = fcc[d];
                        }
                        if ((c == d || c == otherd) == (codevalue < OP_NOTSTAR)) {
                            if (++count >= GET2(code, 1)) { ADD_NEW(state_offset + dlen + 3, 0); }
                            else { ADD_NEW(state_offset, count); }
                        }
                    }
                    break;
                case OP_UPTO: case OP_MINUPTO: case OP_POSUPTO: case OP_NOTUPTO: case OP_NOTMINUPTO: case OP_NOTPOSUPTO:
                    ADD_ACTIVE(state_offset + dlen + 3, 0);
                    count = current_state->count;
                    if (clen > 0) {
                        unsigned int otherd = NOTACHAR;
                        if ((ims & PCRE_CASELESS) != 0) {
                        #ifdef SUPPORT_UTF8
                            if (utf8 && d >= 128) {
                            #ifdef SUPPORT_UCP
                                otherd = UCD_OTHERCASE(d);
                            #endif
                            } else
                        #endif
                           otherd = fcc[d];
                        }
                        if ((c == d || c == otherd) == (codevalue < OP_NOTSTAR)) {
                            if (codevalue == OP_POSUPTO || codevalue == OP_NOTPOSUPTO) {
                            active_count--;
                            next_active_state--;
                            }
                            if (++count >= GET2(code, 1)) { ADD_NEW(state_offset + dlen + 3, 0); }
                            else { ADD_NEW(state_offset, count); }
                        }
                    }
                    break;
                case OP_CLASS: case OP_NCLASS: case OP_XCLASS: {
                        BOOL isinclass = FALSE;
                        int next_state_offset;
                        const uschar *ecode;
                        if (codevalue != OP_XCLASS) {
                            ecode = code + 33;
                            if (clen > 0) isinclass = (c > 255)? (codevalue == OP_NCLASS) : ((code[1 + c/8] & (1 << (c&7))) != 0);
                        } else {
                            //ecode = code + GET(code, 1);
                            if (clen > 0) isinclass = _pcre_xclass(c, code + 1 /*+ LINK_SIZE*/);
                        }
                        next_state_offset = (int)(ecode - start_code);
                        switch (*ecode) {
                            case OP_CRSTAR: case OP_CRMINSTAR:
                                ADD_ACTIVE(next_state_offset + 1, 0);
                                if (isinclass) { ADD_NEW(state_offset, 0); }
                                break;
                            case OP_CRPLUS: case OP_CRMINPLUS:
                                count = current_state->count;  /* Already matched */
                                if (count > 0) { ADD_ACTIVE(next_state_offset + 1, 0); }
                                if (isinclass) { count++; ADD_NEW(state_offset, count); }
                                break;
                            case OP_CRQUERY: case OP_CRMINQUERY:
                                ADD_ACTIVE(next_state_offset + 1, 0);
                                if (isinclass) { ADD_NEW(next_state_offset + 1, 0); }
                                break;
                            case OP_CRRANGE: case OP_CRMINRANGE:
                            count = current_state->count;
                            if (count >= GET2(ecode, 1)) { ADD_ACTIVE(next_state_offset + 5, 0); }
                            if (isinclass) {
                                int max = GET2(ecode, 3);
                                if (++count >= max && max != 0) { ADD_NEW(next_state_offset + 5, 0); }
                                else { ADD_NEW(state_offset, count); }
                            }
                            break;
                            default: if (isinclass) { ADD_NEW(next_state_offset, 0); } break;
                        }
                    }
                    break;
                case OP_FAIL: forced_fail++; break;
                case OP_ASSERT: case OP_ASSERT_NOT: case OP_ASSERTBACK: case OP_ASSERTBACK_NOT: {
                    int rc;
                    int local_offsets[2];
                    int local_workspace[1000];
                    const uschar *endasscode = code /*+ GET(code, 1)*/;
                    //while (*endasscode == OP_ALT) endasscode += GET(endasscode, 1);
                    rc = internal_dfa_exec(md, code, ptr,(int)(ptr - start_subject), local_offsets,sizeof(local_offsets)/sizeof(int),
                                           local_workspace,sizeof(local_workspace)/sizeof(int), ims, rlevel, recursing);
                    if (rc == PCRE_ERROR_DFA_UITEM) return rc;
                    if ((rc >= 0) == (codevalue == OP_ASSERT || codevalue == OP_ASSERTBACK))
                        { ADD_ACTIVE((int)(endasscode + /*LINK_SIZE +*/ 1 - start_code), 0); }
                    }
                    break;
                case OP_COND: case OP_SCOND: {/*
                        int local_offsets[1000];
                        int local_workspace[1000];
                        int codelink = GET(code, 1);
                        int condcode;
                        if (code[LINK_SIZE+1] == OP_CALLOUT) {
                            rrc = 0;
                            if (pcre_callout != NULL) {
                                pcre_callout_block cb;
                                cb.version          = 1;
                                cb.callout_number   = code[LINK_SIZE+2];
                                cb.offset_vector    = offsets;
                                cb.subject          = (PCRE_SPTR)start_subject;
                                cb.subject_length   = (int)(end_subject - start_subject);
                                cb.start_match      = (int)(current_subject - start_subject);
                                cb.current_position = (int)(ptr - start_subject);
                                cb.pattern_position = GET(code, LINK_SIZE + 3);
                                cb.next_item_length = GET(code, 3 + 2*LINK_SIZE);
                                cb.capture_top      = 1;
                                cb.capture_last     = -1;
                                cb.callout_data     = md->callout_data;
                                if ((rrc = (*pcre_callout)(&cb)) < 0) return rrc;
                            }
                            if (rrc > 0) break;
                            code += _pcre_OP_lengths[OP_CALLOUT];
                        }
                        condcode = code[LINK_SIZE+1];
                        if (condcode == OP_CREF || condcode == OP_NCREF) return PCRE_ERROR_DFA_UCOND;
                        if (condcode == OP_DEF) { ADD_ACTIVE(state_offset + codelink + LINK_SIZE + 1, 0); }
                        else if (condcode == OP_RREF || condcode == OP_NRREF) {
                            int value = GET2(code, LINK_SIZE+2);
                            if (value != RREF_ANY) return PCRE_ERROR_DFA_UCOND;
                            if (recursing > 0) { ADD_ACTIVE(state_offset + LINK_SIZE + 4, 0); }
                            else { ADD_ACTIVE(state_offset + codelink + LINK_SIZE + 1, 0); }
                        } else {
                            int rc;
                            const uschar *asscode = code + LINK_SIZE + 1;
                            const uschar *endasscode = asscode + GET(asscode, 1);
                            while (*endasscode == OP_ALT) endasscode += GET(endasscode, 1);
                            rc = internal_dfa_exec(md, asscode, ptr, (int)(ptr - start_subject), local_offsets, sizeof(local_offsets)/sizeof(int),
                                                   local_workspace,sizeof(local_workspace)/sizeof(int), ims, rlevel, recursing);
                            if (rc == PCRE_ERROR_DFA_UITEM) return rc;
                            if ((rc >= 0) == (condcode == OP_ASSERT || condcode == OP_ASSERTBACK)) {
                                ADD_ACTIVE((int)(endasscode + LINK_SIZE + 1 - start_code), 0);
                            } else { ADD_ACTIVE(state_offset + codelink + LINK_SIZE + 1, 0); }
                        }*/
                    }
                    break;
                case OP_RECURSE: {
                        int local_offsets[1000];
                        int local_workspace[1000];
                        int rc;
                        DPRINTF(("%.*sStarting regex recursion %d\n", rlevel*2-2, SP, recursing + 1));
                        rc = internal_dfa_exec(md,start_code /*+ GET(code, 1)*/, ptr,(int)(ptr - start_subject), local_offsets,
                                    sizeof(local_offsets)/sizeof(int),local_workspace,sizeof(local_workspace)/sizeof(int), ims, rlevel,
                                      recursing + 1);
                        DPRINTF(("%.*sReturn from regex recursion %d: rc=%d\n", rlevel*2-2, SP, recursing + 1, rc));
                        if (rc == 0) return PCRE_ERROR_DFA_RECURSE;
                        if (rc > 0) {
                            for (rc = rc*2 - 2; rc >= 0; rc -= 2) {
                                const uschar *p = start_subject + local_offsets[rc];
                                const uschar *pp = start_subject + local_offsets[rc+1];
                                int charcount = local_offsets[rc+1] - local_offsets[rc];
                                while (p < pp) if ((*p++ & 0xc0) == 0x80) charcount--;
                                if (charcount > 0) { ADD_NEW_DATA(-(state_offset + /*LINK_SIZE +*/ 1), 0, (charcount - 1)); }
                                else { ADD_ACTIVE(state_offset + /*LINK_SIZE +*/ 1, 0); }
                            }
                        } else if (rc != PCRE_ERROR_NOMATCH) return rc;
                    }
                    break;
                case OP_ONCE: {
                        int local_offsets[2];
                        int local_workspace[1000];
                        int rc = internal_dfa_exec(md, code, ptr,(int)(ptr - start_subject), local_offsets,sizeof(local_offsets)/sizeof(int),
                                                   local_workspace,sizeof(local_workspace)/sizeof(int), ims, rlevel, recursing);
                        if (rc >= 0) {
                            const uschar *end_subpattern = code;
                            int charcount = local_offsets[1] - local_offsets[0];
                            int next_state_offset, repeat_state_offset;
                            /*do {
                                end_subpattern += GET(end_subpattern, 1);
                            } while(*end_subpattern == OP_ALT);*/
                            next_state_offset = (int)(end_subpattern - start_code + /*LINK_SIZE +*/ 1);
                            repeat_state_offset = (*end_subpattern == OP_KETRMAX || *end_subpattern == OP_KETRMIN) ?
                                                  (int)(end_subpattern - start_code /*- GET(end_subpattern, 1)*/) : -1;
                            if (charcount == 0) { ADD_ACTIVE(next_state_offset, 0); }
                            else if (i + 1 >= active_count && new_count == 0) {
                                ptr += charcount;
                                clen = 0;
                                ADD_NEW(next_state_offset, 0);
                                if (repeat_state_offset >= 0) {
                                    next_active_state = active_states;
                                    active_count = 0;
                                    i = -1;
                                    ADD_ACTIVE(repeat_state_offset, 0);
                                }
                            } else {
                                const uschar *p = start_subject + local_offsets[0];
                                const uschar *pp = start_subject + local_offsets[1];
                                while (p < pp) if ((*p++ & 0xc0) == 0x80) charcount--;
                                ADD_NEW_DATA(-next_state_offset, 0, (charcount - 1));
                                if (repeat_state_offset >= 0) { ADD_NEW_DATA(-repeat_state_offset, 0, (charcount - 1)); }
                            }
                        } else if (rc != PCRE_ERROR_NOMATCH) return rc;
                    }
                    break;
                case OP_CALLOUT:
                    rrc = 0;
                    if (pcre_callout != NULL) {
                        pcre_callout_block cb;
                        cb.version          = 1;
                        cb.callout_number   = code[1];
                        cb.offset_vector    = offsets;
                        cb.subject          = (PCRE_SPTR)start_subject;
                        cb.subject_length   = (int)(end_subject - start_subject);
                        cb.start_match      = (int)(current_subject - start_subject);
                        cb.current_position = (int)(ptr - start_subject);
                        //cb.pattern_position = GET(code, 2);
                        //cb.next_item_length = GET(code, 2 + LINK_SIZE);
                        cb.capture_top      = 1;
                        cb.capture_last     = -1;
                        cb.callout_data     = md->callout_data;
                        if ((rrc = (*pcre_callout)(&cb)) < 0) return rrc;
                    }
                    if (rrc == 0) { ADD_ACTIVE(state_offset + _pcre_OP_lengths[OP_CALLOUT], 0); }
                    break;
                default: return PCRE_ERROR_DFA_UITEM;
            }
            NEXT_ACTIVE_STATE: continue;
        }
        if (new_count <= 0) {
            if (rlevel == 1 && could_continue && forced_fail != workspace[1] && ((md->moptions & PCRE_PARTIAL_HARD) != 0 ||
                ((md->moptions & PCRE_PARTIAL_SOFT) != 0 && match_count < 0)) && ptr >= end_subject && ptr > md->start_used_ptr) {
                if (offsetcount >= 2) {
                    offsets[0] = (int)(md->start_used_ptr - start_subject);
                    offsets[1] = (int)(end_subject - start_subject);
                }
                match_count = PCRE_ERROR_PARTIAL;
            }
            DPRINTF(("%.*sEnd of internal_dfa_exec %d: returning %d\n%.*s---------------------\n\n", rlevel*2-2, SP, rlevel, match_count, rlevel*2-2, SP));
            break;
        }
        ptr += clen;
    }
    return match_count;
}
PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_dfa_exec(const pcre *argument_re, const pcre_extra *extra_data, const char *subject, int length, int start_offset,
                                                     int options, int *offsets, int offsetcount, int *workspace, int wscount) {
    real_pcre *re = (real_pcre*)argument_re;
    dfa_match_data match_block;
    dfa_match_data *md = &match_block;
    BOOL utf8, anchored, startline, firstline;
    const uschar *current_subject, *end_subject, *lcc;
    pcre_study_data internal_study;
    const pcre_study_data *study = NULL;
    real_pcre internal_re;
    const uschar *req_byte_ptr;
    const uschar *start_bits = NULL;
    BOOL first_byte_caseless = FALSE;
    BOOL req_byte_caseless = FALSE;
    int first_byte = -1;
    int req_byte = -1;
    int req_byte2 = -1;
    int newline;
    if ((options & ~PUBLIC_DFA_EXEC_OPTIONS) != 0) return PCRE_ERROR_BADOPTION;
    if (re == NULL || subject == NULL || workspace == NULL || (offsets == NULL && offsetcount > 0)) return PCRE_ERROR_NULL;
    if (offsetcount < 0) return PCRE_ERROR_BADCOUNT;
    if (wscount < 20) return PCRE_ERROR_DFA_WSSIZE;
    if (start_offset < 0 || start_offset > length) return PCRE_ERROR_BADOFFSET;
    md->tables = re->tables;
    md->callout_data = NULL;
    if (extra_data != NULL) {
        unsigned int flags = extra_data->flags;
        if ((flags & PCRE_EXTRA_STUDY_DATA) != 0) study = (const pcre_study_data *)extra_data->study_data;
        if ((flags & PCRE_EXTRA_MATCH_LIMIT) != 0) return PCRE_ERROR_DFA_UMLIMIT;
        if ((flags & PCRE_EXTRA_MATCH_LIMIT_RECURSION) != 0) return PCRE_ERROR_DFA_UMLIMIT;
        if ((flags & PCRE_EXTRA_CALLOUT_DATA) != 0) md->callout_data = extra_data->callout_data;
        if ((flags & PCRE_EXTRA_TABLES) != 0) md->tables = extra_data->tables;
    }
    if (re->magic_number != MAGIC_NUMBER) {
        re = _pcre_try_flipped(re, &internal_re, study, &internal_study);
        if (re == NULL) return PCRE_ERROR_BADMAGIC;
        if (study != NULL) study = &internal_study;
    }
    current_subject = (const unsigned char*)subject + start_offset;
    end_subject = (const unsigned char*)subject + length;
    req_byte_ptr = current_subject - 1;
    #ifdef SUPPORT_UTF8
    utf8 = (re->options & PCRE_UTF8) != 0;
    #else
    utf8 = FALSE;
    #endif
    anchored = (options & (PCRE_ANCHORED|PCRE_DFA_RESTART)) != 0 || (re->options & PCRE_ANCHORED) != 0;
    md->start_code = (const uschar *)argument_re + re->name_table_offset + re->name_count * re->name_entry_size;
    md->start_subject = (const unsigned char *)subject;
    md->end_subject = end_subject;
    md->start_offset = start_offset;
    md->moptions = options;
    md->poptions = re->options;
    if ((md->moptions & (PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE)) == 0) {
        if ((re->options & (PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE)) != 0) md->moptions |= re->options & (PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE);
    #ifdef BSR_ANYCRLF
        else md->moptions |= PCRE_BSR_ANYCRLF;
    #endif
    }
    switch((((options & PCRE_NEWLINE_BITS) == 0)? re->options : (pcre_uint32)options) & PCRE_NEWLINE_BITS) {
        //case 0: newline = NEWLINE; break;
        case PCRE_NEWLINE_CR: newline = CHAR_CR; break;
        case PCRE_NEWLINE_LF: newline = CHAR_NL; break;
        case PCRE_NEWLINE_CR + PCRE_NEWLINE_LF: newline = (CHAR_CR << 8) | CHAR_NL; break;
        case PCRE_NEWLINE_ANY: newline = -1; break;
        case PCRE_NEWLINE_ANYCRLF: newline = -2; break;
        default: return PCRE_ERROR_BADNEWLINE;
    }
    if (newline == -2) md->nltype = NLTYPE_ANYCRLF;
    else if (newline < 0) md->nltype = NLTYPE_ANY;
    else {
        md->nltype = NLTYPE_FIXED;
        if (newline > 255) {
            md->nllen = 2;
            md->nl[0] = (newline >> 8) & 255;
            md->nl[1] = newline & 255;
        } else {
            md->nllen = 1;
            md->nl[0] = newline;
        }
    }
    #ifdef SUPPORT_UTF8
    if (utf8 && (options & PCRE_NO_UTF8_CHECK) == 0) {
        int tb;
        if ((tb = _pcre_valid_utf8((uschar *)subject, length)) >= 0)
            return (tb == length && (options & PCRE_PARTIAL_HARD) != 0)? PCRE_ERROR_SHORTUTF8 : PCRE_ERROR_BADUTF8;
        if (start_offset > 0 && start_offset < length) {
            tb = ((USPTR)subject)[start_offset] & 0xc0;
            if (tb == 0x80) return PCRE_ERROR_BADUTF8_OFFSET;
        }
    }
    #endif
    if (md->tables == NULL) md->tables = _pcre_default_tables;
    lcc = md->tables + lcc_offset;
    startline = (re->flags & PCRE_STARTLINE) != 0;
    firstline = (re->options & PCRE_FIRSTLINE) != 0;
    if (!anchored) {
        if ((re->flags & PCRE_FIRSTSET) != 0) {
            first_byte = re->first_byte & 255;
            if ((first_byte_caseless = ((re->first_byte & REQ_CASELESS) != 0)) == TRUE) first_byte = lcc[first_byte];
            else {
                if (!startline && study != NULL && (study->flags & PCRE_STUDY_MAPPED) != 0) start_bits = study->start_bits;
            }
        }
        if ((re->flags & PCRE_REQCHSET) != 0) {
            req_byte = re->req_byte & 255;
            req_byte_caseless = (re->req_byte & REQ_CASELESS) != 0;
            req_byte2 = (md->tables + fcc_offset)[req_byte];
        }
    }
    for (;;) {
        int rc;
        if ((options & PCRE_DFA_RESTART) == 0) {
            const uschar *save_end_subject = end_subject;
            if (firstline) {
                USPTR t = current_subject;
            #ifdef SUPPORT_UTF8
                if (utf8) {
                    while (t < md->end_subject && !IS_NEWLINE(t)) {
                        t++;
                        while (t < end_subject && (*t & 0xc0) == 0x80) t++;
                    }
                } else
            #endif
                while (t < md->end_subject && !IS_NEWLINE(t)) t++;
                end_subject = t;
            }
            if (((options | re->options) & PCRE_NO_START_OPTIMIZE) == 0) {
                if (first_byte >= 0) {
                    if (first_byte_caseless) {
                        while (current_subject < end_subject && lcc[*current_subject] != first_byte) current_subject++;
                    } else {
                        while(current_subject < end_subject && *current_subject != first_byte);
                    }
                    current_subject++;
                } else if (startline) {
                    if (current_subject > md->start_subject + start_offset) {
                    #ifdef SUPPORT_UTF8
                        if (utf8) {
                            while (current_subject < end_subject && !WAS_NEWLINE(current_subject)) {
                                current_subject++;
                                while(current_subject < end_subject && (*current_subject & 0xc0) == 0x80) current_subject++;
                            }
                        } else
                    #endif
                        while (current_subject < end_subject && !WAS_NEWLINE(current_subject)) current_subject++;
                        if (current_subject[-1] == CHAR_CR && (md->nltype == NLTYPE_ANY || md->nltype == NLTYPE_ANYCRLF) && current_subject < end_subject &&
                            *current_subject == CHAR_NL)
                            current_subject++;
                    }
                } else if (start_bits != NULL) {
                    while (current_subject < end_subject) {
                        register unsigned int c = *current_subject;
                        if ((start_bits[c/8] & (1 << (c&7))) == 0) {
                            current_subject++;
                        #ifdef SUPPORT_UTF8
                            if (utf8)
                                while(current_subject < end_subject && (*current_subject & 0xc0) == 0x80) current_subject++;
                        #endif
                        } else break;
                    }
                }
            }
            end_subject = save_end_subject;
            if ((options & PCRE_NO_START_OPTIMIZE) == 0 && (options & (PCRE_PARTIAL_HARD|PCRE_PARTIAL_SOFT)) == 0) {
                if (study != NULL && (study->flags & PCRE_STUDY_MINLEN) != 0 && (pcre_uint32)(end_subject - current_subject) < study->minlength)
                    return PCRE_ERROR_NOMATCH;
                if (req_byte >= 0 && end_subject - current_subject < REQ_BYTE_MAX) {
                    register const uschar *p = current_subject + ((first_byte >= 0) ? 1 : 0);
                    if (p > req_byte_ptr) {
                        if (req_byte_caseless) {
                            while (p < end_subject) {
                                register int pp = *p++;
                                if (pp == req_byte || pp == req_byte2) { p--; break; }
                            }
                        } else {
                            while (p < end_subject) {
                              if (*p++ == req_byte) { p--; break; }
                            }
                        }
                        if (p >= end_subject) break;
                        req_byte_ptr = p;
                    }
                }
            }
        }
        md->start_used_ptr = current_subject;
        rc = internal_dfa_exec(md, md->start_code, current_subject, start_offset, offsets, offsetcount, workspace, wscount,
                          re->options & (PCRE_CASELESS|PCRE_MULTILINE|PCRE_DOTALL),0,0);
        if (rc != PCRE_ERROR_NOMATCH || anchored) return rc;
        if (firstline && IS_NEWLINE(current_subject)) break;
        current_subject++;
        if (utf8) {
            while(current_subject < end_subject && (*current_subject & 0xc0) == 0x80) current_subject++;
        }
        if (current_subject > end_subject) break;
        if (current_subject[-1] == CHAR_CR && current_subject < end_subject && *current_subject == CHAR_NL && (re->flags & PCRE_HASCRORLF) == 0 &&
            (md->nltype == NLTYPE_ANY || md->nltype == NLTYPE_ANYCRLF || md->nllen == 2))
            current_subject++;
    }
    return PCRE_ERROR_NOMATCH;
}