#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

#define NLBLOCK md
#define PSSTART start_subject
#define PSEND   end_subject
#undef min
#undef max
#define match_condassert     0x01
#define match_cbegroup       0x02
#define MATCH_MATCH        1
#define MATCH_NOMATCH      0
#define MATCH_ACCEPT       (-999)
#define MATCH_COMMIT       (-998)
#define MATCH_PRUNE        (-997)
#define MATCH_SKIP         (-996)
#define MATCH_SKIP_ARG     (-995)
#define MATCH_THEN         (-994)
#define MRRETURN(ra)        \
    {                       \
        md->mark = markptr; \
        RRETURN(ra);        \
    }
#define REC_STACK_SAVE_MAX 30
static const char rep_min[] = { 0, 0, 1, 1, 0, 0 };
static const char rep_max[] = { 0, 0, 0, 0, 1, 1 };
#ifdef PCRE_DEBUGstatic
void pchars(const uschar *p, int length, BOOL is_subject, match_data *md) {
    unsigned int c;
    if (is_subject && length > md->end_subject - p) length = md->end_subject - p;
    while(length-- > 0) if (isprint(c = *(p++))) printf("%c", c); else printf("\\x%02x", c);
}
#endif
static BOOL match_ref(int offset, register USPTR eptr, int length, match_data *md, unsigned long int ims) {
    USPTR p = md->start_subject + md->offset_vector[offset];
    #ifdef PCRE_DEBUG
    if (eptr >= md->end_subject) printf("matching subject <null>");
    else {
        printf("matching subject ");
        pchars(eptr, length, TRUE, md);
    }
    printf(" against backref ");
    pchars(p, length, FALSE, md);
    printf("\n");
    #endif
    if (length > md->end_subject - eptr) return FALSE;
    if ((ims & PCRE_CASELESS) != 0) {
    #ifdef SUPPORT_UTF8
    #ifdef SUPPORT_UCP
        if (md->utf8) {
            USPTR endptr = eptr + length;
            while(eptr < endptr) {
                int c, d;
                GETCHARINC(c, eptr);
                GETCHARINC(d, p);
                if (c != d && c != UCD_OTHERCASE(d)) return FALSE;
            }
        } else
    #endif
    #endif
        while (length-- > 0) { if (md->lcc[*p++] != md->lcc[*eptr++]) return FALSE; }
    } else { while (length-- > 0) if (*p++ != *eptr++) return FALSE; }
    return TRUE;
}
enum { RM1=1, RM2,  RM3,  RM4,  RM5,  RM6,  RM7,  RM8,  RM9,  RM10,
       RM11,  RM12, RM13, RM14, RM15, RM16, RM17, RM18, RM19, RM20,
       RM21,  RM22, RM23, RM24, RM25, RM26, RM27, RM28, RM29, RM30,
       RM31,  RM32, RM33, RM34, RM35, RM36, RM37, RM38, RM39, RM40,
       RM41,  RM42, RM43, RM44, RM45, RM46, RM47, RM48, RM49, RM50,
       RM51,  RM52, RM53, RM54, RM55, RM56, RM57, RM58, RM59, RM60,
       RM61,  RM62 };
#ifndef NO_RECURSE
#define REGISTER register
#ifdef PCRE_DEBUG
#define RMATCH(ra,rb,rc,rd,re,rf,rg,rw)                            \
    {                                                              \
        printf("match() called in line %d\n", __LINE__);           \
        rrc = match(ra,rb,mstart,markptr,rc,rd,re,rf,rg,rdepth+1); \
        printf("to line %d\n", __LINE__);                          \
    }
#define RRETURN(ra)                                                \
    {                                                              \
        printf("match() returned %d from line %d ", ra, __LINE__); \
        return ra;                                                 \
    }
#else
#define RMATCH(ra,rb,rc,rd,re,rf,rg,rw)  rrc = match(ra,rb,mstart,markptr,rc,rd,re,rf,rg,rdepth+1)
#define RRETURN(ra) return ra
#endif
#else
#define REGISTER
#define RMATCH(ra,rb,rc,rd,re,rf,rg,rw)                                           \
    {                                                                             \
        heapframe *newframe = (heapframe *)(pcre_stack_malloc)(sizeof(heapframe));\
        if (newframe == NULL) RRETURN(PCRE_ERROR_NOMEMORY);                       \
        frame->Xwhere = rw;                                                       \
        newframe->Xeptr = ra;                                                     \
        newframe->Xecode = rb;                                                    \
        newframe->Xmstart = mstart;                                               \
        newframe->Xmarkptr = markptr;                                             \
        newframe->Xoffset_top = rc;                                               \
        newframe->Xims = re;                                                      \
        newframe->Xeptrb = rf;                                                    \
        newframe->Xflags = rg;                                                    \
        newframe->Xrdepth = frame->Xrdepth + 1;                                   \
        newframe->Xprevframe = frame;                                             \
        frame = newframe;                                                         \
        DPRINTF(("restarting from line %d\n", __LINE__));                         \
        goto HEAP_RECURSE;                                                        \
        L_##rw:                                                                   \
        DPRINTF(("jumped back to line %d\n", __LINE__));                          \
    }
#define RRETURN(ra)                   \
    {                                 \
        heapframe *oldframe = frame;  \
        frame = oldframe->Xprevframe; \
        (pcre_stack_free)(oldframe);  \
        if (frame != NULL) {          \
            rrc = ra;                 \
            goto HEAP_RETURN;         \
        }                             \
        return ra;                    \
    }
typedef struct heapframe {
  struct heapframe *Xprevframe;
  USPTR Xeptr;
  const uschar *Xecode;
  USPTR Xmstart;
  USPTR Xmarkptr;
  int Xoffset_top;
  long int Xims;
  eptrblock *Xeptrb;
  int Xflags;
  unsigned int Xrdepth;
  USPTR Xcallpat;
#ifdef SUPPORT_UTF8
  USPTR Xcharptr;
#endif
  USPTR Xdata;
  USPTR Xnext;
  USPTR Xpp;
  USPTR Xprev;
  USPTR Xsaved_eptr;
  recursion_info Xnew_recursive;
  BOOL Xcur_is_word;
  BOOL Xcondition;
  BOOL Xprev_is_word;
  unsigned long int Xoriginal_ims;
#ifdef SUPPORT_UCP
  int Xprop_type;
  int Xprop_value;
  int Xprop_fail_result;
  int Xprop_category;
  int Xprop_chartype;
  int Xprop_script;
  int Xoclength;
  uschar Xocchars[8];
#endif
  int Xcodelink;
  int Xctype;
  unsigned int Xfc;
  int Xfi;
  int Xlength;
  int Xmax;
  int Xmin;
  int Xnumber;
  int Xoffset;
  int Xop;
  int Xsave_capture_last;
  int Xsave_offset1, Xsave_offset2, Xsave_offset3;
  int Xstacksave[REC_STACK_SAVE_MAX];
  eptrblock Xnewptrb;
  int Xwhere;
} heapframe;
#endif
#define CHECK_PARTIAL()                                                             \
    if (md->partial != 0 && eptr >= md->end_subject && eptr > md->start_used_ptr) { \
        md->hitend = TRUE;                                                          \
        if (md->partial > 1) MRRETURN(PCRE_ERROR_PARTIAL);                          \
    }
#define SCHECK_PARTIAL()                                   \
    if (md->partial != 0 && eptr > md->start_used_ptr) {   \
        md->hitend = TRUE;                                 \
        if (md->partial > 1) MRRETURN(PCRE_ERROR_PARTIAL); \
    }
static int match(REGISTER USPTR eptr, REGISTER const uschar *ecode, USPTR mstart, const uschar *markptr, int offset_top, match_data *md, unsigned long int ims,
                 eptrblock *eptrb, int flags, unsigned int rdepth) {
    register int  rrc;
    register int  i;
    register unsigned int c;
    register BOOL utf8;
    BOOL minimize, possessive;
    int condcode;
    #ifdef NO_RECURSE
    heapframe *frame = (heapframe*)(pcre_stack_malloc)(sizeof(heapframe));
    if (frame == NULL) RRETURN(PCRE_ERROR_NOMEMORY);
    frame->Xprevframe = NULL;
    frame->Xeptr = eptr;
    frame->Xecode = ecode;
    frame->Xmstart = mstart;
    frame->Xmarkptr = markptr;
    frame->Xoffset_top = offset_top;
    frame->Xims = ims;
    frame->Xeptrb = eptrb;
    frame->Xflags = flags;
    frame->Xrdepth = rdepth;
    HEAP_RECURSE:
    #define eptr  frame->Xeptr
    #define ecode  frame->Xecode
    #define mstart  frame->Xmstart
    #define markptr  frame->Xmarkptr
    #define offset_top  frame->Xoffset_top
    #define ims  frame->Xims
    #define eptrb  frame->Xeptrb
    #define flags  frame->Xflags
    #define rdepth  frame->Xrdepth
    #ifdef SUPPORT_UTF8
    #define charptr  frame->Xcharptr
    #endif
    #define callpat  frame->Xcallpat
    #define codelink  frame->Xcodelink
    #define data  frame->Xdata
    #define next  frame->Xnext
    #define pp  frame->Xpp
    #define prev  frame->Xprev
    #define saved_eptr  frame->Xsaved_eptr
    #define new_recursive  frame->Xnew_recursive
    #define cur_is_word  frame->Xcur_is_word
    #define condition  frame->Xcondition
    #define prev_is_word  frame->Xprev_is_word
    #define original_ims  frame->Xoriginal_ims
    #ifdef SUPPORT_UCP
    #define prop_type  frame->Xprop_type
    #define prop_value  frame->Xprop_value
    #define prop_fail_result  frame->Xprop_fail_result
    #define prop_category  frame->Xprop_category
    #define prop_chartype  frame->Xprop_chartype
    #define prop_script  frame->Xprop_script
    #define oclength  frame->Xoclength
    #define occhars  frame->Xocchars
    #endif
    #define ctype  frame->Xctype
    #define fc  frame->Xfc
    #define fi  frame->Xfi
    #define length  frame->Xlength
    #define max frame->Xmax
    #define min  frame->Xmin
    #define number frame->Xnumber
    #define offset  frame->Xoffset
    #define op frame->Xop
    #define save_capture_last frame->Xsave_capture_last
    #define save_offset1 frame->Xsave_offset1
    #define save_offset2  frame->Xsave_offset2
    #define save_offset3 frame->Xsave_offset3
    #define stacksave frame->Xstacksave
    #define newptrb frame->Xnewptrb
    #else
    #define fi i
    #define fc c
    #ifdef SUPPORT_UTF8
    const uschar *charptr;
    #endif
    const uschar *callpat;
    const uschar *data;
    const uschar *next;
    USPTR pp;
    const uschar *prev;
    USPTR saved_eptr;
    recursion_info new_recursive;
    BOOL cur_is_word;
    BOOL condition;
    BOOL prev_is_word;
    unsigned long int original_ims;
    #ifdef SUPPORT_UCP
    int prop_type;
    int prop_value;
    int prop_fail_result;
    int prop_category;
    int prop_chartype;
    int prop_script;
    int oclength;
    uschar occhars[8];
    #endif
    int codelink;
    int ctype;
    int length;
    int max;
    int min;
    int number;
    int offset;
    int op;
    int save_capture_last;
    int save_offset1, save_offset2, save_offset3;
    int stacksave[REC_STACK_SAVE_MAX];
    eptrblock newptrb;
    #endif
    #ifdef SUPPORT_UCP
    prop_value = 0;
    prop_fail_result = 0;
    #endif
    TAIL_RECURSE:
    #ifdef SUPPORT_UTF8
    utf8 = md->utf8;
    #else
    utf8 = FALSE;
    #endif
    if (md->match_call_count++ >= md->match_limit) RRETURN(PCRE_ERROR_MATCHLIMIT);
    if (rdepth >= md->match_limit_recursion) RRETURN(PCRE_ERROR_RECURSIONLIMIT);
    original_ims = ims;
    if ((flags & match_cbegroup) != 0) {
        newptrb.epb_saved_eptr = eptr;
        newptrb.epb_prev = eptrb;
        eptrb = &newptrb;
    }
    for (;;) {
        minimize = possessive = FALSE;
        op = *ecode;
        switch(op) {
            case OP_MARK:
                markptr = ecode + 2;
                RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode] + ecode[1], offset_top, md, ims, eptrb, flags, RM55);
                if (rrc == MATCH_SKIP_ARG && strcmp((char *)markptr, (char *)(md->start_match_ptr)) == 0) {
                    md->start_match_ptr = eptr;
                    RRETURN(MATCH_SKIP);
                }
                if (md->mark == NULL) md->mark = markptr;
                RRETURN(rrc);
            case OP_FAIL: MRRETURN(MATCH_NOMATCH);
            case OP_COMMIT:
                RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode], offset_top, md, ims, eptrb, flags, RM52);
                if (rrc != MATCH_NOMATCH && rrc != MATCH_PRUNE && rrc != MATCH_SKIP && rrc != MATCH_SKIP_ARG && rrc != MATCH_THEN) RRETURN(rrc);
                MRRETURN(MATCH_COMMIT);
            case OP_PRUNE:
                RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode], offset_top, md, ims, eptrb, flags, RM51);
                if (rrc != MATCH_NOMATCH && rrc != MATCH_THEN) RRETURN(rrc);
                MRRETURN(MATCH_PRUNE);
            case OP_PRUNE_ARG:
                RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode] + ecode[1], offset_top, md, ims, eptrb, flags, RM56);
                if (rrc != MATCH_NOMATCH && rrc != MATCH_THEN) RRETURN(rrc);
                md->mark = ecode + 2;
                RRETURN(MATCH_PRUNE);
            case OP_SKIP:
                RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode], offset_top, md, ims, eptrb, flags, RM53);
                if (rrc != MATCH_NOMATCH && rrc != MATCH_PRUNE && rrc != MATCH_THEN) RRETURN(rrc);
                md->start_match_ptr = eptr;
                MRRETURN(MATCH_SKIP);
            case OP_SKIP_ARG:
                RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode] + ecode[1], offset_top, md, ims, eptrb, flags, RM57);
                if (rrc != MATCH_NOMATCH && rrc != MATCH_PRUNE && rrc != MATCH_THEN) RRETURN(rrc);
                md->start_match_ptr = ecode + 2;
                RRETURN(MATCH_SKIP_ARG);
            case OP_THEN:
                RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode], offset_top, md, ims, eptrb, flags, RM54);
                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                md->start_match_ptr = ecode/* - GET(ecode, 1)*/;
                MRRETURN(MATCH_THEN);
            case OP_THEN_ARG:
                RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode] + ecode[1/*+LINK_SIZE*/], offset_top, md, ims, eptrb, flags, RM58);
                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                md->start_match_ptr = ecode /*- GET(ecode, 1)*/;
                md->mark = ecode + /*LINK_SIZE +*/ 2;
                RRETURN(MATCH_THEN);
            case OP_CBRA: case OP_SCBRA:
                number = GET2(ecode, 1/* + LINK_SIZE*/);
                offset = number << 1;
            #ifdef PCRE_DEBUG
                printf("start bracket %d\n", number);
                printf("subject=");
                pchars(eptr, 16, TRUE, md);
                printf("\n");
            #endif
                if (offset < md->offset_max) {
                    save_offset1 = md->offset_vector[offset];
                    save_offset2 = md->offset_vector[offset+1];
                    save_offset3 = md->offset_vector[md->offset_end - number];
                    save_capture_last = md->capture_last;
                    DPRINTF(("saving %d %d %d\n", save_offset1, save_offset2, save_offset3));
                    md->offset_vector[md->offset_end - number] = (int)(eptr - md->start_subject);
                    flags = (op == OP_SCBRA)? match_cbegroup : 0;
                    /*do {
                        RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode], offset_top, md, ims, eptrb, flags, RM1);
                        if (rrc != MATCH_NOMATCH && (rrc != MATCH_THEN || md->start_match_ptr != ecode)) RRETURN(rrc);
                        md->capture_last = save_capture_last;
                        //ecode += GET(ecode, 1);
                    } while (*ecode == OP_ALT);*/
                    DPRINTF(("bracket %d failed\n", number));
                    md->offset_vector[offset] = save_offset1;
                    md->offset_vector[offset+1] = save_offset2;
                    md->offset_vector[md->offset_end - number] = save_offset3;
                    if (rrc != MATCH_THEN) md->mark = markptr;
                    RRETURN(MATCH_NOMATCH);
                }
            case OP_BRA: case OP_SBRA:
                DPRINTF(("start non-capturing bracket\n"));
                flags = (op >= OP_SBRA)? match_cbegroup : 0;
                /*for (;;) {
                    if (ecode[GET(ecode, 1)] != OP_ALT) {
                        if (flags == 0) {
                            ecode += _pcre_OP_lengths[*ecode];
                            DPRINTF(("bracket 0 tail recursion\n"));
                            goto TAIL_RECURSE;
                        }
                        RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode], offset_top, md, ims, eptrb, flags, RM48);
                        if (rrc == MATCH_NOMATCH) md->mark = markptr;
                        RRETURN(rrc);
                    }
                    RMATCH(eptr, ecode + _pcre_OP_lengths[*ecode], offset_top, md, ims, eptrb, flags, RM2);
                    if (rrc != MATCH_NOMATCH && (rrc != MATCH_THEN || md->start_match_ptr != ecode)) RRETURN(rrc);
                    ecode += GET(ecode, 1);
                }*/
            case OP_COND: case OP_SCOND:/*
                codelink= GET(ecode, 1);
                if (ecode[LINK_SIZE+1] == OP_CALLOUT) {
                    if (pcre_callout != NULL) {
                        pcre_callout_block cb;
                        cb.version          = 1;
                        cb.callout_number   = ecode[LINK_SIZE+2];
                        cb.offset_vector    = md->offset_vector;
                        cb.subject          = (PCRE_SPTR)md->start_subject;
                        cb.subject_length   = (int)(md->end_subject - md->start_subject);
                        cb.start_match      = (int)(mstart - md->start_subject);
                        cb.current_position = (int)(eptr - md->start_subject);
                        cb.pattern_position = GET(ecode, LINK_SIZE + 3);
                        cb.next_item_length = GET(ecode, 3 + 2*LINK_SIZE);
                        cb.capture_top      = offset_top/2;
                        cb.capture_last     = md->capture_last;
                        cb.callout_data     = md->callout_data;
                        if ((rrc = (*pcre_callout)(&cb)) > 0) MRRETURN(MATCH_NOMATCH);
                        if (rrc < 0) RRETURN(rrc);
                    }
                    ecode += _pcre_OP_lengths[OP_CALLOUT];
                }
                condcode = ecode[LINK_SIZE+1];
                if (condcode == OP_RREF || condcode == OP_NRREF) {
                    if (md->recursive == NULL) {
                        condition = FALSE;
                        ecode += GET(ecode, 1);
                    } else {
                        int recno = GET2(ecode, LINK_SIZE + 2);
                        condition =  (recno == RREF_ANY || recno == md->recursive->group_num);
                        if (!condition && condcode == OP_NRREF && recno != RREF_ANY) {
                            uschar *slotA = md->name_table;
                            for (i = 0; i < md->name_count; i++) {
                                if (GET2(slotA, 0) == recno) break;
                                slotA += md->name_entry_size;
                            }
                            if (i < md->name_count) {
                                uschar *slotB = slotA;
                                while (slotB > md->name_table) {
                                    slotB -= md->name_entry_size;
                                    if (strcmp((char *)slotA + 2, (char *)slotB + 2) == 0) {
                                        condition = GET2(slotB, 0) == md->recursive->group_num;
                                        if (condition) break;
                                    } else break;
                                }
                                if (!condition) {
                                    slotB = slotA;
                                    for (i++; i < md->name_count; i++) {
                                        slotB += md->name_entry_size;
                                        if (strcmp((char *)slotA + 2, (char *)slotB + 2) == 0) {
                                            condition = GET2(slotB, 0) == md->recursive->group_num;
                                            if (condition) break;
                                        } else break;
                                    }
                                }
                            }
                        }
                        ecode += condition ? 3 : GET(ecode, 1);
                    }
                } else if (condcode == OP_CREF || condcode == OP_NCREF) {
                    offset = GET2(ecode, LINK_SIZE+2) << 1;
                    condition = offset < offset_top && md->offset_vector[offset] >= 0;
                    if (!condition && condcode == OP_NCREF) {
                        int refno = offset >> 1;
                        uschar *slotA = md->name_table;
                        for (i = 0; i < md->name_count; i++) {
                            if (GET2(slotA, 0) == refno) break;
                            slotA += md->name_entry_size;
                        }
                        if (i < md->name_count) {
                            uschar *slotB = slotA;
                            while (slotB > md->name_table) {
                                slotB -= md->name_entry_size;
                                if (strcmp((char *)slotA + 2, (char *)slotB + 2) == 0) {
                                    offset = GET2(slotB, 0) << 1;
                                    condition = offset < offset_top && md->offset_vector[offset] >= 0;
                                    if (condition) break;
                                } else break;
                            }
                            if (!condition) {
                                slotB = slotA;
                                for (i++; i < md->name_count; i++) {
                                    slotB += md->name_entry_size;
                                    if (strcmp((char *)slotA + 2, (char *)slotB + 2) == 0) {
                                        offset = GET2(slotB, 0) << 1;
                                        condition = offset < offset_top && md->offset_vector[offset] >= 0;
                                        if (condition) break;
                                    } else break;
                                }
                            }
                        }
                    }
                    ecode += condition? 3 : GET(ecode, 1);
                } else if (condcode == OP_DEF) {
                    condition = FALSE;
                    ecode += GET(ecode, 1);
                } else {
                    RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, NULL, match_condassert, RM3);
                    if (rrc == MATCH_MATCH) {
                        condition = TRUE;
                        ecode += 1 + LINK_SIZE + GET(ecode, LINK_SIZE + 2);
                        while (*ecode == OP_ALT) ecode += GET(ecode, 1);
                    } else if (rrc != MATCH_NOMATCH && (rrc != MATCH_THEN || md->start_match_ptr != ecode)) { RRETURN(rrc); }
                    else {
                        condition = FALSE;
                        ecode += codelink;
                    }
                }
                if (condition || *ecode == OP_ALT) {
                    ecode += 1 + LINK_SIZE;
                    if (op == OP_SCOND) {
                        RMATCH(eptr, ecode, offset_top, md, ims, eptrb, match_cbegroup, RM49);
                        RRETURN(rrc);
                    } else {
                        flags = 0;
                        goto TAIL_RECURSE;
                    }
                } else ecode += 1 + LINK_SIZE;*/
                break;
            case OP_CLOSE:
                number = GET2(ecode, 1);
                offset = number << 1;
            #ifdef PCRE_DEBUG
                printf("end bracket %d at *ACCEPT", number);
                printf("\n");
            #endif
                md->capture_last = number;
                if (offset >= md->offset_max) md->offset_overflow = TRUE;
                else {
                    md->offset_vector[offset] = md->offset_vector[md->offset_end - number];
                    md->offset_vector[offset+1] = (int)(eptr - md->start_subject);
                    if (offset_top <= offset) offset_top = offset + 2;
                }
                ecode += 3;
                break;
                case OP_ACCEPT: case OP_END:
                if (md->recursive != NULL && md->recursive->group_num == 0) {
                    recursion_info *rec = md->recursive;
                    DPRINTF(("End of pattern in a (?0) recursion\n"));
                    md->recursive = rec->prevrec;
                    memmove(md->offset_vector, rec->offset_save,rec->saved_max * sizeof(int));
                    offset_top = rec->save_offset_top;
                    ims = original_ims;
                    ecode = rec->after_call;
                    break;
                }
                if (eptr == mstart && (md->notempty || (md->notempty_atstart && mstart == md->start_subject + md->start_offset))) MRRETURN(MATCH_NOMATCH);
                md->end_match_ptr = eptr;
                md->end_offset_top = offset_top;
                md->start_match_ptr = mstart;
                rrc = (op == OP_END)? MATCH_MATCH : MATCH_ACCEPT;
                MRRETURN(rrc);
            case OP_OPT:
                ims = ecode[1];
                ecode += 2;
                DPRINTF(("ims set to %02lx\n", ims));
                break;
            case OP_ASSERT: case OP_ASSERTBACK:
                /*do {
                    RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, NULL, 0, RM4);
                    if (rrc == MATCH_MATCH || rrc == MATCH_ACCEPT) {
                        mstart = md->start_match_ptr;
                        break;
                    }
                    if (rrc != MATCH_NOMATCH && (rrc != MATCH_THEN || md->start_match_ptr != ecode)) RRETURN(rrc);
                    ecode += GET(ecode, 1);
                } while(*ecode == OP_ALT);
                if (*ecode == OP_KET) MRRETURN(MATCH_NOMATCH);
                if ((flags & match_condassert) != 0) RRETURN(MATCH_MATCH);
                do {
                    ecode += GET(ecode,1);
                } while (*ecode == OP_ALT);
                ecode += 1 + LINK_SIZE;
                offset_top = md->end_offset_top;*/
                continue;
            case OP_ASSERT_NOT: case OP_ASSERTBACK_NOT:
                /*do {
                    RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, NULL, 0, RM5);
                    if (rrc == MATCH_MATCH || rrc == MATCH_ACCEPT) MRRETURN(MATCH_NOMATCH);
                    if (rrc == MATCH_SKIP || rrc == MATCH_PRUNE || rrc == MATCH_COMMIT) {
                        do {
                            ecode += GET(ecode,1);
                        } while(*ecode == OP_ALT);
                        break;
                    }
                    if (rrc != MATCH_NOMATCH && (rrc != MATCH_THEN || md->start_match_ptr != ecode)) RRETURN(rrc);
                    ecode += GET(ecode,1);
                } while(*ecode == OP_ALT);
                if ((flags & match_condassert) != 0) RRETURN(MATCH_MATCH);
                ecode += 1 + LINK_SIZE;*/
                continue;
            case OP_REVERSE:
            #ifdef SUPPORT_UTF8
                if (utf8) {
                    i = GET(ecode, 1);
                    while (i-- > 0) {
                        eptr--;
                        if (eptr < md->start_subject) MRRETURN(MATCH_NOMATCH);
                        BACKCHAR(eptr);
                    }
                } else
            #endif
                /*{
                    eptr -= GET(ecode, 1);
                    if (eptr < md->start_subject) MRRETURN(MATCH_NOMATCH);
                }
                if (eptr < md->start_used_ptr) md->start_used_ptr = eptr;
                ecode += 1 + LINK_SIZE;*/
                break;
            case OP_CALLOUT:
                if (pcre_callout != NULL) {
                    pcre_callout_block cb;
                    cb.version          = 1;
                    cb.callout_number   = ecode[1];
                    cb.offset_vector    = md->offset_vector;
                    cb.subject          = (PCRE_SPTR)md->start_subject;
                    cb.subject_length   = (int)(md->end_subject - md->start_subject);
                    cb.start_match      = (int)(mstart - md->start_subject);
                    cb.current_position = (int)(eptr - md->start_subject);
                    //cb.pattern_position = GET(ecode, 2);
                    //cb.next_item_length = GET(ecode, 2 + LINK_SIZE);
                    cb.capture_top      = offset_top/2;
                    cb.capture_last     = md->capture_last;
                    cb.callout_data     = md->callout_data;
                    if ((rrc = (*pcre_callout)(&cb)) > 0) MRRETURN(MATCH_NOMATCH);
                    if (rrc < 0) RRETURN(rrc);
                }
                //ecode += 2 + 2*LINK_SIZE;
                break;
            case OP_RECURSE: {
                    callpat = md->start_code /*+ GET(ecode, 1)*/;
                    new_recursive.group_num = (callpat == md->start_code)? 0 : GET2(callpat, 1 /*+ LINK_SIZE*/);
                    new_recursive.prevrec = md->recursive;
                    md->recursive = &new_recursive;
                    ecode += 1 /*+ LINK_SIZE*/;
                    new_recursive.after_call = ecode;
                    new_recursive.saved_max = md->offset_end;
                    if (new_recursive.saved_max <= REC_STACK_SAVE_MAX) new_recursive.offset_save = stacksave;
                    else {
                    new_recursive.offset_save = (int*)(pcre_malloc)(new_recursive.saved_max * sizeof(int));
                    if (new_recursive.offset_save == NULL) RRETURN(PCRE_ERROR_NOMEMORY);
                    }
                    memcpy(new_recursive.offset_save, md->offset_vector,new_recursive.saved_max * sizeof(int));
                    new_recursive.save_offset_top = offset_top;
                    DPRINTF(("Recursing into group %d\n", new_recursive.group_num));
                    flags = (*callpat >= OP_SBRA)? match_cbegroup : 0;
                    do {
                        RMATCH(eptr, callpat + _pcre_OP_lengths[*callpat], offset_top, md, ims, eptrb, flags, RM6);
                        if (rrc == MATCH_MATCH || rrc == MATCH_ACCEPT) {
                            DPRINTF(("Recursion matched\n"));
                            md->recursive = new_recursive.prevrec;
                            if (new_recursive.offset_save != stacksave) (pcre_free)(new_recursive.offset_save);
                            MRRETURN(MATCH_MATCH);
                        } else if (rrc != MATCH_NOMATCH && (rrc != MATCH_THEN || md->start_match_ptr != ecode)) {
                            DPRINTF(("Recursion gave error %d\n", rrc));
                            if (new_recursive.offset_save != stacksave) (pcre_free)(new_recursive.offset_save);
                            RRETURN(rrc);
                        }
                        md->recursive = &new_recursive;
                        memcpy(md->offset_vector, new_recursive.offset_save,new_recursive.saved_max * sizeof(int));
                        //callpat += GET(callpat, 1);
                    } while (*callpat == OP_ALT);
                    DPRINTF(("Recursion didn't match\n"));
                    md->recursive = new_recursive.prevrec;
                    if (new_recursive.offset_save != stacksave) (pcre_free)(new_recursive.offset_save);
                    MRRETURN(MATCH_NOMATCH);
                }
            case OP_ONCE:
                prev = ecode;
                saved_eptr = eptr;
                /*do {
                    RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, eptrb, 0, RM7);
                    if (rrc == MATCH_MATCH) {
                        mstart = md->start_match_ptr;
                        break;
                    }
                    if (rrc != MATCH_NOMATCH && (rrc != MATCH_THEN || md->start_match_ptr != ecode)) RRETURN(rrc);
                    ecode += GET(ecode,1);
                } while (*ecode == OP_ALT);
                if (*ecode != OP_ONCE && *ecode != OP_ALT) RRETURN(MATCH_NOMATCH);
                do {
                    ecode += GET(ecode, 1);
                } while(*ecode == OP_ALT);*/
                offset_top = md->end_offset_top;
                eptr = md->end_match_ptr;
                if (*ecode == OP_KET || eptr == saved_eptr) {
                    //ecode += 1+LINK_SIZE;
                    break;
                }
                /*if (ecode[1+LINK_SIZE] == OP_OPT) {
                    ims = (ims & ~PCRE_IMS) | ecode[4];
                    DPRINTF(("ims set to %02lx at group repeat\n", ims));
                }*/
                if (*ecode == OP_KETRMIN) {
                    //RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, eptrb, 0, RM8);
                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                    ecode = prev;
                    flags = 0;
                    goto TAIL_RECURSE;
                } else  {
                    RMATCH(eptr, prev, offset_top, md, ims, eptrb, match_cbegroup, RM9);
                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                    //ecode += 1 + LINK_SIZE;
                    flags = 0;
                    goto TAIL_RECURSE;
                }
            case OP_ALT:
                /*do {
                    ecode += GET(ecode,1);
                } while (*ecode == OP_ALT);*/
                break;
            case OP_BRAZERO: {
                    next = ecode+1;
                    RMATCH(eptr, next, offset_top, md, ims, eptrb, 0, RM10);
                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                    /*do {
                        next += GET(next,1);
                    } while (*next == OP_ALT);*/
                    ecode = next + 1 /*+ LINK_SIZE*/;
                }
                break;
            case OP_BRAMINZERO: {
                    next = ecode+1;
                    /*do {
                        next += GET(next, 1);
                    } while (*next == OP_ALT);*/
                    RMATCH(eptr, next + 1 /*+ LINK_SIZE*/, offset_top, md, ims, eptrb, 0, RM11);
                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                    ecode++;
                }
                break;
            case OP_SKIPZERO: {
                    next = ecode+1;
                    /*do {
                        next += GET(next,1);
                    } while (*next == OP_ALT);*/
                    ecode = next + 1 /*+ LINK_SIZE*/;
                }
                break;
            case OP_KET: case OP_KETRMIN: case OP_KETRMAX:
                prev = ecode /*- GET(ecode, 1)*/;
                if (*prev >= OP_SBRA) {
                    saved_eptr = eptrb->epb_saved_eptr;
                    eptrb = eptrb->epb_prev;
                } else saved_eptr = NULL;
                if (*prev == OP_ASSERT || *prev == OP_ASSERT_NOT || *prev == OP_ASSERTBACK || *prev == OP_ASSERTBACK_NOT || *prev == OP_ONCE) {
                    md->end_match_ptr = eptr;
                    md->end_offset_top = offset_top;
                    md->start_match_ptr = mstart;
                    MRRETURN(MATCH_MATCH);
                }
                if (*prev == OP_CBRA || *prev == OP_SCBRA) {
                    number = GET2(prev, 1 /*+ LINK_SIZE*/);
                    offset = number << 1;
                #ifdef PCRE_DEBUG
                    printf("end bracket %d", number);
                    printf("\n");
                #endif
                    md->capture_last = number;
                    if (offset >= md->offset_max) md->offset_overflow = TRUE;
                    else {
                        md->offset_vector[offset] = md->offset_vector[md->offset_end - number];
                        md->offset_vector[offset+1] = (int)(eptr - md->start_subject);
                        if (offset_top <= offset) offset_top = offset + 2;
                    }
                    if (md->recursive != NULL && md->recursive->group_num == number) {
                        recursion_info *rec = md->recursive;
                        DPRINTF(("Recursion (%d) succeeded - continuing\n", number));
                        md->recursive = rec->prevrec;
                        memcpy(md->offset_vector, rec->offset_save,rec->saved_max * sizeof(int));
                        offset_top = rec->save_offset_top;
                        ecode = rec->after_call;
                        ims = original_ims;
                        break;
                    }
                }
                ims = original_ims;
                if (*ecode == OP_KET || eptr == saved_eptr) {
                    ecode += 1 /*+ LINK_SIZE*/;
                    break;
                }
                flags = (*prev >= OP_SBRA) ? match_cbegroup : 0;
                if (*ecode == OP_KETRMIN) {
                    RMATCH(eptr, ecode + 1 /*+ LINK_SIZE*/, offset_top, md, ims, eptrb, 0, RM12);
                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                    if (flags != 0) {
                        RMATCH(eptr, prev, offset_top, md, ims, eptrb, flags, RM50);
                        RRETURN(rrc);
                    }
                    ecode = prev;
                    goto TAIL_RECURSE;
                } else {
                    RMATCH(eptr, prev, offset_top, md, ims, eptrb, flags, RM13);
                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                    ecode += 1 /*+ LINK_SIZE*/;
                    flags = 0;
                    goto TAIL_RECURSE;
                }
            case OP_CIRC:
                if (md->notbol && eptr == md->start_subject) MRRETURN(MATCH_NOMATCH);
                if ((ims & PCRE_MULTILINE) != 0) {
                    if (eptr != md->start_subject && (eptr == md->end_subject || !WAS_NEWLINE(eptr))) MRRETURN(MATCH_NOMATCH);
                    ecode++;
                    break;
                }
            case OP_SOD:
                if (eptr != md->start_subject) MRRETURN(MATCH_NOMATCH);
                ecode++;
                break;
            case OP_SOM:
                if (eptr != md->start_subject + md->start_offset) MRRETURN(MATCH_NOMATCH);
                ecode++;
                break;
            case OP_SET_SOM:
                mstart = eptr;
                ecode++;
                break;
            case OP_DOLL:
                if ((ims & PCRE_MULTILINE) != 0) {
                    if (eptr < md->end_subject) {
                        if (!IS_NEWLINE(eptr)) MRRETURN(MATCH_NOMATCH);
                    } else {
                        if (md->noteol) MRRETURN(MATCH_NOMATCH);
                        SCHECK_PARTIAL();
                    }
                    ecode++;
                    break;
                } else {
                    if (md->noteol) MRRETURN(MATCH_NOMATCH);
                    if (!md->endonly) goto ASSERT_NL_OR_EOS;
                }
            case OP_EOD:
                if (eptr < md->end_subject) MRRETURN(MATCH_NOMATCH);
                SCHECK_PARTIAL();
                ecode++;
                break;
            case OP_EODN:
                ASSERT_NL_OR_EOS:
                if (eptr < md->end_subject && (!IS_NEWLINE(eptr) || eptr != md->end_subject - md->nllen)) MRRETURN(MATCH_NOMATCH);
                SCHECK_PARTIAL();
                ecode++;
                break;
            case OP_NOT_WORD_BOUNDARY: case OP_WORD_BOUNDARY: {
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        if (eptr == md->start_subject) prev_is_word = FALSE;
                        else {
                            USPTR lastptr = eptr - 1;
                            while((*lastptr & 0xc0) == 0x80) lastptr--;
                            if (lastptr < md->start_used_ptr) md->start_used_ptr = lastptr;
                            GETCHAR(c, lastptr);
                        #ifdef SUPPORT_UCP
                            if (md->use_ucp) {
                                if (c == '_') prev_is_word = TRUE;
                                else {
                                    int cat = UCD_CATEGORY(c);
                                    prev_is_word = (cat == ucp_L || cat == ucp_N);
                                }
                            } else
                        #endif
                            prev_is_word = c < 256 && (md->ctypes[c] & ctype_word) != 0;
                        }
                        if (eptr >= md->end_subject) {
                            SCHECK_PARTIAL();
                            cur_is_word = FALSE;
                        } else {
                            GETCHAR(c, eptr);
                        #ifdef SUPPORT_UCP
                            if (md->use_ucp) {
                                if (c == '_') cur_is_word = TRUE;
                                else {
                                    int cat = UCD_CATEGORY(c);
                                    cur_is_word = (cat == ucp_L || cat == ucp_N);
                                }
                            } else
                        #endif
                            cur_is_word = c < 256 && (md->ctypes[c] & ctype_word) != 0;
                        }
                    } else
                    #endif
                    {
                        if (eptr == md->start_subject) prev_is_word = FALSE; else {
                            if (eptr <= md->start_used_ptr) md->start_used_ptr = eptr - 1;
                    #ifdef SUPPORT_UCP
                        if (md->use_ucp) {
                            c = eptr[-1];
                            if (c == '_') prev_is_word = TRUE;
                            else {
                                int cat = UCD_CATEGORY(c);
                                prev_is_word = (cat == ucp_L || cat == ucp_N);
                            }
                        } else
                    #endif
                            prev_is_word = ((md->ctypes[eptr[-1]] & ctype_word) != 0);
                        }
                        if (eptr >= md->end_subject) {
                            SCHECK_PARTIAL();
                            cur_is_word = FALSE;
                        } else
                    #ifdef SUPPORT_UCP
                        if (md->use_ucp) {
                            c = *eptr;
                            if (c == '_') cur_is_word = TRUE;
                            else {
                                int cat = UCD_CATEGORY(c);
                                cur_is_word = (cat == ucp_L || cat == ucp_N);
                            }
                        } else
                    #endif
                        cur_is_word = ((md->ctypes[*eptr] & ctype_word) != 0);
                    }
                    if ((*ecode++ == OP_WORD_BOUNDARY) ? cur_is_word == prev_is_word : cur_is_word != prev_is_word) MRRETURN(MATCH_NOMATCH);
                }
                break;
            case OP_ANY: if (IS_NEWLINE(eptr)) MRRETURN(MATCH_NOMATCH);
            case OP_ALLANY:
                if (eptr++ >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                if (utf8) while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
                ecode++;
                break;
            case OP_ANYBYTE:
                if (eptr++ >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                ecode++;
                break;
            case OP_NOT_DIGIT:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                if (
                #ifdef SUPPORT_UTF8
                   c < 256 &&
                #endif
                   (md->ctypes[c] & ctype_digit) != 0) MRRETURN(MATCH_NOMATCH);
                ecode++;
                break;
            case OP_DIGIT:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                if (
                #ifdef SUPPORT_UTF8
                   c >= 256 ||
                #endif
                   (md->ctypes[c] & ctype_digit) == 0) MRRETURN(MATCH_NOMATCH);
                ecode++;
                break;
            case OP_NOT_WHITESPACE:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                if (
                #ifdef SUPPORT_UTF8
                   c < 256 &&
                #endif
                   (md->ctypes[c] & ctype_space) != 0) MRRETURN(MATCH_NOMATCH);
                ecode++;
                break;
            case OP_WHITESPACE:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                if (
                #ifdef SUPPORT_UTF8
                   c >= 256 ||
                #endif
                   (md->ctypes[c] & ctype_space) == 0) MRRETURN(MATCH_NOMATCH);
                ecode++;
                break;
            case OP_NOT_WORDCHAR:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                if (
                #ifdef SUPPORT_UTF8
                   c < 256 &&
                #endif
                   (md->ctypes[c] & ctype_word) != 0) MRRETURN(MATCH_NOMATCH);
                ecode++;
                break;
            case OP_WORDCHAR:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                if (
                #ifdef SUPPORT_UTF8
                   c >= 256 ||
                #endif
                   (md->ctypes[c] & ctype_word) == 0) MRRETURN(MATCH_NOMATCH);
                ecode++;
                break;
            case OP_ANYNL:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                switch(c) {
                    case 0x000d: if (eptr < md->end_subject && *eptr == 0x0a) eptr++; break;
                    case 0x000a: break;
                    case 0x000b: case 0x000c: case 0x0085: case 0x2028: case 0x2029: if (md->bsr_anycrlf) MRRETURN(MATCH_NOMATCH); break;
                    default: MRRETURN(MATCH_NOMATCH);
                }
                ecode++;
                break;
            case OP_NOT_HSPACE:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                switch(c) {
                    case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                    case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                        MRRETURN(MATCH_NOMATCH);
                    default: break;
                }
                ecode++;
                break;
            case OP_HSPACE:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                switch(c) {
                    case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
                    case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                        break;
                    default: MRRETURN(MATCH_NOMATCH);
                }
                ecode++;
                break;
            case OP_NOT_VSPACE:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                switch(c) {
                    case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029: MRRETURN(MATCH_NOMATCH);
                    default: break;
                }
                ecode++;
                break;
            case OP_VSPACE:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                switch(c) {
                    case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029: break;
                    default: MRRETURN(MATCH_NOMATCH);
                }
                ecode++;
                break;
        #ifdef SUPPORT_UCP
            case OP_PROP: case OP_NOTPROP:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                {
                    int chartype = UCD_CHARTYPE(c);
                    switch(ecode[1]) {
                        case PT_ANY: if (op == OP_NOTPROP) MRRETURN(MATCH_NOMATCH); break;
                        case PT_LAMP:
                            if ((chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt) == (op == OP_NOTPROP)) MRRETURN(MATCH_NOMATCH);
                            break;
                        case PT_GC: if ((ecode[2] != _pcre_ucp_gentype[chartype]) == (op == OP_PROP)) MRRETURN(MATCH_NOMATCH); break;
                        case PT_PC: if ((ecode[2] != chartype) == (op == OP_PROP)) MRRETURN(MATCH_NOMATCH); break;
                        case PT_SC: if ((ecode[2] != UCD_SCRIPT(c)) == (op == OP_PROP)) MRRETURN(MATCH_NOMATCH); break;
                        case PT_ALNUM:
                            if ((_pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N) == (op == OP_NOTPROP)) MRRETURN(MATCH_NOMATCH);
                            break;
                        case PT_SPACE:
                            if ((_pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR) == (op == OP_NOTPROP))
                                MRRETURN(MATCH_NOMATCH);
                            break;
                        case PT_PXSPACE:
                            if ((_pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF ||
                                c == CHAR_CR) == (op == OP_NOTPROP))
                                MRRETURN(MATCH_NOMATCH);
                            break;
                        case PT_WORD:
                            if ((_pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N || c == CHAR_UNDERSCORE) == (op == OP_NOTPROP))
                                MRRETURN(MATCH_NOMATCH);
                            break;
                        default: RRETURN(PCRE_ERROR_INTERNAL);
                    }
                    ecode += 3;
                }
                break;
            case OP_EXTUNI:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                GETCHARINCTEST(c, eptr);
                {
                    int category = UCD_CATEGORY(c);
                    if (category == ucp_M) MRRETURN(MATCH_NOMATCH);
                    while (eptr < md->end_subject) {
                        int len = 1;
                        if (!utf8) c = *eptr; else {
                          GETCHARLEN(c, eptr, len);
                        }
                        category = UCD_CATEGORY(c);
                        if (category != ucp_M) break;
                        eptr += len;
                    }
                }
                ecode++;
                break;
        #endif
            case OP_REF: {
                    offset = GET2(ecode, 1) << 1;
                    ecode += 3;
                    if (offset >= offset_top || md->offset_vector[offset] < 0) length = (md->jscript_compat)? 0 : (int)(md->end_subject - eptr + 1);
                    else length = md->offset_vector[offset+1] - md->offset_vector[offset];
                    switch(*ecode) {
                        case OP_CRSTAR: case OP_CRMINSTAR: case OP_CRPLUS: case OP_CRMINPLUS: case OP_CRQUERY: case OP_CRMINQUERY:
                            c = *ecode++ - OP_CRSTAR;
                            minimize = (c & 1) != 0;
                            min = rep_min[c];
                            max = rep_max[c];
                            if (max == 0) max = INT_MAX;
                            break;
                        case OP_CRRANGE: case OP_CRMINRANGE:
                            minimize = (*ecode == OP_CRMINRANGE);
                            min = GET2(ecode, 1);
                            max = GET2(ecode, 3);
                            if (max == 0) max = INT_MAX;
                            ecode += 5;
                            break;
                        default:
                            if (!match_ref(offset, eptr, length, md, ims)) {
                                CHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            eptr += length;
                            continue;
                    }
                    if (length == 0) continue;
                    for (i = 1; i <= min; i++) {
                        if (!match_ref(offset, eptr, length, md, ims)) {
                            CHECK_PARTIAL();
                            MRRETURN(MATCH_NOMATCH);
                        }
                        eptr += length;
                    }
                    if (min == max) continue;
                    if (minimize) {
                        for (fi = min;; fi++) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM14);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (fi >= max) MRRETURN(MATCH_NOMATCH);
                            if (!match_ref(offset, eptr, length, md, ims)) {
                                CHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            eptr += length;
                        }
                    } else {
                        pp = eptr;
                        for (i = min; i < max; i++) {
                            if (!match_ref(offset, eptr, length, md, ims)) {
                                CHECK_PARTIAL();
                                break;
                            }
                            eptr += length;
                        }
                        while (eptr >= pp) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM15);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            eptr -= length;
                        }
                        MRRETURN(MATCH_NOMATCH);
                    }
                }
            case OP_NCLASS: case OP_CLASS: {
                    data = ecode + 1;
                    ecode += 33;
                    switch (*ecode) {
                        case OP_CRSTAR: case OP_CRMINSTAR: case OP_CRPLUS: case OP_CRMINPLUS: case OP_CRQUERY: case OP_CRMINQUERY:
                            c = *ecode++ - OP_CRSTAR;
                            minimize = (c & 1) != 0;
                            min = rep_min[c];
                            max = rep_max[c];
                            if (max == 0) max = INT_MAX;
                            break;
                        case OP_CRRANGE: case OP_CRMINRANGE:
                            minimize = (*ecode == OP_CRMINRANGE);
                            min = GET2(ecode, 1);
                            max = GET2(ecode, 3);
                            if (max == 0) max = INT_MAX;
                            ecode += 5;
                            break;
                        default: min = max = 1; break;
                    }
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        for (i = 1; i <= min; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            GETCHARINC(c, eptr);
                            if (c > 255) {
                                if (op == OP_CLASS) MRRETURN(MATCH_NOMATCH);
                            } else {
                                if ((data[c/8] & (1 << (c&7))) == 0) MRRETURN(MATCH_NOMATCH);
                            }
                        }
                    } else
                #endif
                    {
                        for (i = 1; i <= min; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            c = *eptr++;
                            if ((data[c/8] & (1 << (c&7))) == 0) MRRETURN(MATCH_NOMATCH);
                        }
                    }
                    if (min == max) continue;
                    if (minimize) {
                    #ifdef SUPPORT_UTF8
                        if (utf8) {
                            for (fi = min;; fi++) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM16);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                GETCHARINC(c, eptr);
                                if (c > 255) {
                                  if (op == OP_CLASS) MRRETURN(MATCH_NOMATCH);
                                } else {
                                  if ((data[c/8] & (1 << (c&7))) == 0) MRRETURN(MATCH_NOMATCH);
                                }
                            }
                        } else
                    #endif
                        {
                            for (fi = min;; fi++) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM17);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                c = *eptr++;
                                if ((data[c/8] & (1 << (c&7))) == 0) MRRETURN(MATCH_NOMATCH);
                            }
                        }
                    } else {
                        pp = eptr;
                    #ifdef SUPPORT_UTF8
                        if (utf8) {
                            for (i = min; i < max; i++) {
                                int len = 1;
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    break;
                                }
                                GETCHARLEN(c, eptr, len);
                                if (c > 255) {
                                    if (op == OP_CLASS) break;
                                } else {
                                    if ((data[c/8] & (1 << (c&7))) == 0) break;
                                }
                                eptr += len;
                            }
                            for (;;) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM18);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (eptr-- == pp) break;
                                BACKCHAR(eptr);
                            }
                        } else
                    #endif
                        {
                            for (i = min; i < max; i++) {
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    break;
                                }
                                c = *eptr;
                                if ((data[c/8] & (1 << (c&7))) == 0) break;
                                eptr++;
                            }
                            while (eptr >= pp) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM19);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                eptr--;
                            }
                        }
                        MRRETURN(MATCH_NOMATCH);
                    }
                }
        #ifdef SUPPORT_UTF8
            case OP_XCLASS: {
                    data = ecode + 1 + LINK_SIZE;
                    ecode += GET(ecode, 1);
                    switch (*ecode) {
                        case OP_CRSTAR: case OP_CRMINSTAR: case OP_CRPLUS: case OP_CRMINPLUS: case OP_CRQUERY: case OP_CRMINQUERY:
                            c = *ecode++ - OP_CRSTAR;
                            minimize = (c & 1) != 0;
                            min = rep_min[c];
                            max = rep_max[c];
                            if (max == 0) max = INT_MAX;
                            break;
                        case OP_CRRANGE: case OP_CRMINRANGE:
                            minimize = (*ecode == OP_CRMINRANGE);
                            min = GET2(ecode, 1);
                            max = GET2(ecode, 3);
                            if (max == 0) max = INT_MAX;
                            ecode += 5;
                            break;
                        default: min = max = 1; break;
                    }
                    for (i = 1; i <= min; i++) {
                        if (eptr >= md->end_subject) {
                            SCHECK_PARTIAL();
                            MRRETURN(MATCH_NOMATCH);
                        }
                        GETCHARINCTEST(c, eptr);
                        if (!_pcre_xclass(c, data)) MRRETURN(MATCH_NOMATCH);
                    }
                    if (min == max) continue;
                    if (minimize) {
                        for (fi = min;; fi++) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM20);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (fi >= max) MRRETURN(MATCH_NOMATCH);
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            GETCHARINCTEST(c, eptr);
                            if (!_pcre_xclass(c, data)) MRRETURN(MATCH_NOMATCH);
                        }
                    } else {
                        pp = eptr;
                        for (i = min; i < max; i++) {
                            int len = 1;
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                break;
                            }
                            GETCHARLENTEST(c, eptr, len);
                            if (!_pcre_xclass(c, data)) break;
                            eptr += len;
                        }
                        for(;;) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM21);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (eptr-- == pp) break;
                            if (utf8) BACKCHAR(eptr);
                        }
                        MRRETURN(MATCH_NOMATCH);
                    }
                }
        #endif
            case OP_CHAR:
            #ifdef SUPPORT_UTF8
                if (utf8) {
                    length = 1;
                    ecode++;
                    GETCHARLEN(fc, ecode, length);
                    if (length > md->end_subject - eptr) {
                        CHECK_PARTIAL();
                        MRRETURN(MATCH_NOMATCH);
                    }
                    while(length-- > 0) if (*ecode++ != *eptr++) MRRETURN(MATCH_NOMATCH);
                } else
            #endif
                {
                    if (md->end_subject - eptr < 1) {
                        SCHECK_PARTIAL();
                        MRRETURN(MATCH_NOMATCH);
                    }
                    if (ecode[1] != *eptr++) MRRETURN(MATCH_NOMATCH);
                    ecode += 2;
                }
                break;
            case OP_CHARNC:
            #ifdef SUPPORT_UTF8
                if (utf8) {
                    length = 1;
                    ecode++;
                    GETCHARLEN(fc, ecode, length);
                    if (length > md->end_subject - eptr) {
                        CHECK_PARTIAL();
                        MRRETURN(MATCH_NOMATCH);
                    }
                    if (fc < 128) {
                        if (md->lcc[*ecode++] != md->lcc[*eptr++]) MRRETURN(MATCH_NOMATCH);
                    } else {
                        unsigned int dc;
                        GETCHARINC(dc, eptr);
                        ecode += length;
                        if (fc != dc) {
                        #ifdef SUPPORT_UCP
                            if (dc != UCD_OTHERCASE(fc))
                        #endif
                            MRRETURN(MATCH_NOMATCH);
                        }
                    }
                } else
            #endif
                {
                    if (md->end_subject - eptr < 1) {
                        SCHECK_PARTIAL();
                        MRRETURN(MATCH_NOMATCH);
                    }
                    if (md->lcc[ecode[1]] != md->lcc[*eptr++]) MRRETURN(MATCH_NOMATCH);
                    ecode += 2;
                }
                break;
            case OP_EXACT:
                min = max = GET2(ecode, 1);
                ecode += 3;
                goto REPEATCHAR;
            case OP_POSUPTO: possessive = TRUE;
            case OP_UPTO: case OP_MINUPTO:
                min = 0;
                max = GET2(ecode, 1);
                minimize = *ecode == OP_MINUPTO;
                ecode += 3;
                goto REPEATCHAR;
            case OP_POSSTAR:
                possessive = TRUE;
                min = 0;
                max = INT_MAX;
                ecode++;
                goto REPEATCHAR;
            case OP_POSPLUS:
                possessive = TRUE;
                min = 1;
                max = INT_MAX;
                ecode++;
                goto REPEATCHAR;
            case OP_POSQUERY:
                possessive = TRUE;
                min = 0;
                max = 1;
                ecode++;
                goto REPEATCHAR;
            case OP_STAR: case OP_MINSTAR: case OP_PLUS: case OP_MINPLUS: case OP_QUERY: case OP_MINQUERY:
                c = *ecode++ - OP_STAR;
                minimize = (c & 1) != 0;
                min = rep_min[c];
                max = rep_max[c];
                if (max == 0) max = INT_MAX;
                REPEATCHAR:
            #ifdef SUPPORT_UTF8
                if (utf8) {
                    length = 1;
                    charptr = ecode;
                    GETCHARLEN(fc, ecode, length);
                    ecode += length;
                    if (length > 1) {
                    #ifdef SUPPORT_UCP
                        unsigned int othercase;
                        if ((ims & PCRE_CASELESS) != 0 && (othercase = UCD_OTHERCASE(fc)) != fc) oclength = _pcre_ord2utf8(othercase, occhars);
                        else oclength = 0;
                    #endif
                        for (i = 1; i <= min; i++) {
                            if (eptr <= md->end_subject - length && memcmp(eptr, charptr, length) == 0) eptr += length;
                        #ifdef SUPPORT_UCP
                            else if (oclength > 0 && eptr <= md->end_subject - oclength && memcmp(eptr, occhars, oclength) == 0) eptr += oclength;
                        #endif
                            else {
                                CHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                        }
                        if (min == max) continue;
                        if (minimize) {
                            for (fi = min;; fi++) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM22);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                if (eptr <= md->end_subject - length && memcmp(eptr, charptr, length) == 0) eptr += length;
                            #ifdef SUPPORT_UCP
                                else if (oclength > 0 && eptr <= md->end_subject - oclength && memcmp(eptr, occhars, oclength) == 0) eptr += oclength;
                            #endif
                                else {
                                    CHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                            }
                        } else {
                            pp = eptr;
                            for (i = min; i < max; i++) {
                                if (eptr <= md->end_subject - length && memcmp(eptr, charptr, length) == 0) eptr += length;
                            #ifdef SUPPORT_UCP
                                else if (oclength > 0 && eptr <= md->end_subject - oclength && memcmp(eptr, occhars, oclength) == 0) eptr += oclength;
                            #endif
                                else {
                                    CHECK_PARTIAL();
                                    break;
                                }
                            }
                            if (possessive) continue;
                            for(;;) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM23);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (eptr == pp) { MRRETURN(MATCH_NOMATCH); }
                            #ifdef SUPPORT_UCP
                                eptr--;
                                BACKCHAR(eptr);
                            #else
                                eptr -= length;
                            #endif
                            }
                        }
                    }
                } else
            #endif
                fc = *ecode++;
                DPRINTF(("matching %c{%d,%d} against subject %.*s\n", fc, min, max, max, eptr));
                if ((ims & PCRE_CASELESS) != 0) {
                    fc = md->lcc[fc];
                    for (i = 1; i <= min; i++) {
                        if (eptr >= md->end_subject) {
                            SCHECK_PARTIAL();
                            MRRETURN(MATCH_NOMATCH);
                        }
                        if (fc != md->lcc[*eptr++]) MRRETURN(MATCH_NOMATCH);
                    }
                    if (min == max) continue;
                    if (minimize) {
                        for (fi = min;; fi++) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM24);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (fi >= max) MRRETURN(MATCH_NOMATCH);
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            if (fc != md->lcc[*eptr++]) MRRETURN(MATCH_NOMATCH);
                        }
                    } else {
                        pp = eptr;
                        for (i = min; i < max; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                break;
                            }
                            if (fc != md->lcc[*eptr]) break;
                            eptr++;
                        }
                        if (possessive) continue;
                        while (eptr >= pp) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM25);
                            eptr--;
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                        }
                        MRRETURN(MATCH_NOMATCH);
                    }
                } else  {
                    for (i = 1; i <= min; i++) {
                        if (eptr >= md->end_subject) {
                            SCHECK_PARTIAL();
                            MRRETURN(MATCH_NOMATCH);
                        }
                        if (fc != *eptr++) MRRETURN(MATCH_NOMATCH);
                    }
                    if (min == max) continue;
                    if (minimize) {
                        for (fi = min;; fi++) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM26);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (fi >= max) MRRETURN(MATCH_NOMATCH);
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            if (fc != *eptr++) MRRETURN(MATCH_NOMATCH);
                        }
                    } else {
                        pp = eptr;
                        for (i = min; i < max; i++) {
                          if (eptr >= md->end_subject) {
                              SCHECK_PARTIAL();
                              break;
                          }
                          if (fc != *eptr) break;
                          eptr++;
                        }
                        if (possessive) continue;
                        while (eptr >= pp) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM27);
                            eptr--;
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                        }
                        MRRETURN(MATCH_NOMATCH);
                    }
                }
            case OP_NOT:
                if (eptr >= md->end_subject) {
                    SCHECK_PARTIAL();
                    MRRETURN(MATCH_NOMATCH);
                }
                ecode++;
                GETCHARINCTEST(c, eptr);
                if ((ims & PCRE_CASELESS) != 0) {
                #ifdef SUPPORT_UTF8
                    if (c < 256)
                #endif
                    c = md->lcc[c];
                    if (md->lcc[*ecode++] == c) MRRETURN(MATCH_NOMATCH);
                } else {
                    if (*ecode++ == c) MRRETURN(MATCH_NOMATCH);
                }
                break;
            case OP_NOTEXACT:
                min = max = GET2(ecode, 1);
                ecode += 3;
                goto REPEATNOTCHAR;
            case OP_NOTUPTO: case OP_NOTMINUPTO:
                min = 0;
                max = GET2(ecode, 1);
                minimize = *ecode == OP_NOTMINUPTO;
                ecode += 3;
                goto REPEATNOTCHAR;
            case OP_NOTPOSSTAR:
                possessive = TRUE;
                min = 0;
                max = INT_MAX;
                ecode++;
                goto REPEATNOTCHAR;
            case OP_NOTPOSPLUS:
                possessive = TRUE;
                min = 1;
                max = INT_MAX;
                ecode++;
                goto REPEATNOTCHAR;
            case OP_NOTPOSQUERY:
                possessive = TRUE;
                min = 0;
                max = 1;
                ecode++;
                goto REPEATNOTCHAR;
            case OP_NOTPOSUPTO:
                possessive = TRUE;
                min = 0;
                max = GET2(ecode, 1);
                ecode += 3;
                goto REPEATNOTCHAR;
            case OP_NOTSTAR: case OP_NOTMINSTAR: case OP_NOTPLUS: case OP_NOTMINPLUS: case OP_NOTQUERY: case OP_NOTMINQUERY:
                c = *ecode++ - OP_NOTSTAR;
                minimize = (c & 1) != 0;
                min = rep_min[c];
                max = rep_max[c];
                if (max == 0) max = INT_MAX;
                REPEATNOTCHAR:
                fc = *ecode++;
                DPRINTF(("negative matching %c{%d,%d} against subject %.*s\n", fc, min, max, max, eptr));
                if ((ims & PCRE_CASELESS) != 0) {
                    fc = md->lcc[fc];
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        register unsigned int d;
                        for (i = 1; i <= min; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            GETCHARINC(d, eptr);
                            if (d < 256) d = md->lcc[d];
                            if (fc == d) MRRETURN(MATCH_NOMATCH);
                        }
                    } else
                #endif
                    {
                        for (i = 1; i <= min; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            if (fc == md->lcc[*eptr++]) MRRETURN(MATCH_NOMATCH);
                        }
                    }
                    if (min == max) continue;
                    if (minimize) {
                    #ifdef SUPPORT_UTF8
                        if (utf8) {
                            register unsigned int d;
                            for (fi = min;; fi++) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM28);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                GETCHARINC(d, eptr);
                                if (d < 256) d = md->lcc[d];
                                if (fc == d) MRRETURN(MATCH_NOMATCH);
                            }
                        } else
                    #endif
                        {
                            for (fi = min;; fi++) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM29);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                if (fc == md->lcc[*eptr++]) MRRETURN(MATCH_NOMATCH);
                            }
                        }
                    } else {
                        pp = eptr;
                    #ifdef SUPPORT_UTF8
                        if (utf8) {
                            register unsigned int d;
                            for (i = min; i < max; i++) {
                                int len = 1;
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    break;
                                }
                                GETCHARLEN(d, eptr, len);
                                if (d < 256) d = md->lcc[d];
                                if (fc == d) break;
                                eptr += len;
                            }
                            if (possessive) continue;
                            for(;;) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM30);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (eptr-- == pp) break;
                                BACKCHAR(eptr);
                            }
                        } else
                    #endif
                        {
                            for (i = min; i < max; i++) {
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    break;
                                }
                                if (fc == md->lcc[*eptr]) break;
                                eptr++;
                            }
                            if (possessive) continue;
                            while (eptr >= pp) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM31);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                eptr--;
                            }
                        }
                        MRRETURN(MATCH_NOMATCH);
                    }
                } else {
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        register unsigned int d;
                        for (i = 1; i <= min; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            GETCHARINC(d, eptr);
                            if (fc == d) MRRETURN(MATCH_NOMATCH);
                        }
                    } else
                #endif
                    {
                        for (i = 1; i <= min; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            if (fc == *eptr++) MRRETURN(MATCH_NOMATCH);
                        }
                    }
                    if (min == max) continue;
                    if (minimize) {
                    #ifdef SUPPORT_UTF8
                        if (utf8) {
                            register unsigned int d;
                            for (fi = min;; fi++) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM32);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                GETCHARINC(d, eptr);
                                if (fc == d) MRRETURN(MATCH_NOMATCH);
                            }
                        } else
                    #endif
                        {
                            for (fi = min;; fi++) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM33);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (fi >= max) MRRETURN(MATCH_NOMATCH);
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            if (fc == *eptr++) MRRETURN(MATCH_NOMATCH);
                            }
                        }
                    } else {
                        pp = eptr;
                    #ifdef SUPPORT_UTF8
                        if (utf8) {
                            register unsigned int d;
                            for (i = min; i < max; i++) {
                                int len = 1;
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    break;
                                }
                                GETCHARLEN(d, eptr, len);
                                if (fc == d) break;
                                eptr += len;
                            }
                            if (possessive) continue;
                            for(;;) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM34);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                if (eptr-- == pp) break;
                                BACKCHAR(eptr);
                            }
                        } else
                    #endif
                        {
                            for (i = min; i < max; i++) {
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    break;
                                }
                                if (fc == *eptr) break;
                                eptr++;
                            }
                            if (possessive) continue;
                            while (eptr >= pp) {
                                RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM35);
                                if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                eptr--;
                            }
                        }
                        MRRETURN(MATCH_NOMATCH);
                    }
                }
            case OP_TYPEEXACT:
                min = max = GET2(ecode, 1);
                minimize = TRUE;
                ecode += 3;
                goto REPEATTYPE;
            case OP_TYPEUPTO: case OP_TYPEMINUPTO:
                min = 0;
                max = GET2(ecode, 1);
                minimize = *ecode == OP_TYPEMINUPTO;
                ecode += 3;
                goto REPEATTYPE;
            case OP_TYPEPOSSTAR:
                possessive = TRUE;
                min = 0;
                max = INT_MAX;
                ecode++;
                goto REPEATTYPE;
            case OP_TYPEPOSPLUS:
                possessive = TRUE;
                min = 1;
                max = INT_MAX;
                ecode++;
                goto REPEATTYPE;
            case OP_TYPEPOSQUERY:
                possessive = TRUE;
                min = 0;
                max = 1;
                ecode++;
                goto REPEATTYPE;
            case OP_TYPEPOSUPTO:
                possessive = TRUE;
                min = 0;
                max = GET2(ecode, 1);
                ecode += 3;
                goto REPEATTYPE;
            case OP_TYPESTAR: case OP_TYPEMINSTAR: case OP_TYPEPLUS: case OP_TYPEMINPLUS: case OP_TYPEQUERY: case OP_TYPEMINQUERY:
                c = *ecode++ - OP_TYPESTAR;
                minimize = (c & 1) != 0;
                min = rep_min[c];
                max = rep_max[c];
                if (max == 0) max = INT_MAX;
                REPEATTYPE:
                ctype = *ecode++;
            #ifdef SUPPORT_UCP
                if (ctype == OP_PROP || ctype == OP_NOTPROP) {
                    prop_fail_result = ctype == OP_NOTPROP;
                    prop_type = *ecode++;
                    prop_value = *ecode++;
                } else prop_type = -1;
            #endif
                if (min > 0) {
                #ifdef SUPPORT_UCP
                    if (prop_type >= 0) {
                        switch(prop_type) {
                            case PT_ANY:
                                if (prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                }
                                break;
                            case PT_LAMP:
                                for (i = 1; i <= min; i++) {
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                GETCHARINCTEST(c, eptr);
                                prop_chartype = UCD_CHARTYPE(c);
                                if ((prop_chartype == ucp_Lu || prop_chartype == ucp_Ll || prop_chartype == ucp_Lt) == prop_fail_result)
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case PT_GC:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == prop_value) == prop_fail_result)
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case PT_PC:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_chartype = UCD_CHARTYPE(c);
                                    if ((prop_chartype == prop_value) == prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case PT_SC:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_script = UCD_SCRIPT(c);
                                    if ((prop_script == prop_value) == prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case PT_ALNUM:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_L || prop_category == ucp_N) == prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case PT_SPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR) == prop_fail_result)
                                        MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case PT_PXSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR) == prop_fail_result)
                                        MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case PT_WORD:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_L || prop_category == ucp_N || c == CHAR_UNDERSCORE) == prop_fail_result)
                                        MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            default: RRETURN(PCRE_ERROR_INTERNAL);
                        }
                    } else if (ctype == OP_EXTUNI) {
                        for (i = 1; i <= min; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            GETCHARINCTEST(c, eptr);
                            prop_category = UCD_CATEGORY(c);
                            if (prop_category == ucp_M) MRRETURN(MATCH_NOMATCH);
                            while (eptr < md->end_subject) {
                                int len = 1;
                                if (!utf8) c = *eptr;
                                else { GETCHARLEN(c, eptr, len); }
                                prop_category = UCD_CATEGORY(c);
                                if (prop_category != ucp_M) break;
                                eptr += len;
                            }
                        }
                    } else
                #endif
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        switch(ctype) {
                            case OP_ANY:
                            for (i = 1; i <= min; i++) {
                                if (eptr >= md->end_subject) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                if (IS_NEWLINE(eptr)) MRRETURN(MATCH_NOMATCH);
                                eptr++;
                                while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
                            }
                            break;
                            case OP_ALLANY:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    eptr++;
                                    while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
                                }
                                break;
                            case OP_ANYBYTE:
                                if (eptr > md->end_subject - min) MRRETURN(MATCH_NOMATCH);
                                eptr += min;
                                break;
                            case OP_ANYNL:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINC(c, eptr);
                                    switch(c) {
                                    default: MRRETURN(MATCH_NOMATCH);
                                    case 0x000d:
                                    if (eptr < md->end_subject && *eptr == 0x0a) eptr++;
                                    break;
                                    case 0x000a: break;
                                    case 0x000b: case 0x000c: case 0x0085: case 0x2028: case 0x2029:
                                    if (md->bsr_anycrlf) MRRETURN(MATCH_NOMATCH);
                                    break;
                                    }
                                }
                                break;
                            case OP_NOT_HSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINC(c, eptr);
                                    switch(c) {
                                        case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004:
                                        case 0x2005: case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                                        MRRETURN(MATCH_NOMATCH);
                                        default: break;
                                    }
                                }
                                break;
                            case OP_HSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINC(c, eptr);
                                    switch(c) {
                                        case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004:
                                        case 0x2005: case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                                        break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                }
                                break;
                            case OP_NOT_VSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINC(c, eptr);
                                    switch(c) {
                                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029: MRRETURN(MATCH_NOMATCH);
                                        default: break;
                                    }
                                }
                                break;
                            case OP_VSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINC(c, eptr);
                                    switch(c) {
                                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029: break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                }
                                break;
                            case OP_NOT_DIGIT:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINC(c, eptr);
                                    if (c < 128 && (md->ctypes[c] & ctype_digit) != 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case OP_DIGIT:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if (*eptr >= 128 || (md->ctypes[*eptr++] & ctype_digit) == 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case OP_NOT_WHITESPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if (*eptr < 128 && (md->ctypes[*eptr] & ctype_space) != 0) MRRETURN(MATCH_NOMATCH);
                                    while(++eptr < md->end_subject && (*eptr & 0xc0) == 0x80);
                                }
                                break;
                            case OP_WHITESPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if (*eptr >= 128 || (md->ctypes[*eptr++] & ctype_space) == 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case OP_NOT_WORDCHAR:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if (*eptr < 128 && (md->ctypes[*eptr] & ctype_word) != 0) MRRETURN(MATCH_NOMATCH);
                                    while(++eptr < md->end_subject && (*eptr & 0xc0) == 0x80);
                                }
                                break;
                            case OP_WORDCHAR:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if (*eptr >= 128 || (md->ctypes[*eptr++] & ctype_word) == 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            default: RRETURN(PCRE_ERROR_INTERNAL);
                        }
                    } else
                #endif
                    {
                        switch(ctype){
                            case OP_ANY:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if (IS_NEWLINE(eptr)) MRRETURN(MATCH_NOMATCH);
                                    eptr++;
                                }
                                break;
                            case OP_ALLANY:
                                if (eptr > md->end_subject - min) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                eptr += min;
                                break;
                            case OP_ANYBYTE:
                                if (eptr > md->end_subject - min) {
                                    SCHECK_PARTIAL();
                                    MRRETURN(MATCH_NOMATCH);
                                }
                                eptr += min;
                                break;
                            case OP_ANYNL:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    switch(*eptr++) {
                                        case 0x000d:
                                            if (eptr < md->end_subject && *eptr == 0x0a) eptr++;
                                            break;
                                        case 0x000a: break;
                                        case 0x000b: case 0x000c: case 0x0085:
                                            if (md->bsr_anycrlf) MRRETURN(MATCH_NOMATCH);
                                            break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                }
                                break;
                            case OP_NOT_HSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    switch(*eptr++) {
                                        case 0x09: case 0x20: case 0xa0: MRRETURN(MATCH_NOMATCH);
                                        default: break;
                                    }
                                }
                                break;
                            case OP_HSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    switch(*eptr++) {
                                        case 0x09: case 0x20: case 0xa0: break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                }
                                break;
                            case OP_NOT_VSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    switch(*eptr++) {
                                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: MRRETURN(MATCH_NOMATCH);
                                        default: break;
                                    }
                                }
                                break;
                            case OP_VSPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    switch(*eptr++) {
                                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                }
                                break;
                            case OP_NOT_DIGIT:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if ((md->ctypes[*eptr++] & ctype_digit) != 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case OP_DIGIT:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if ((md->ctypes[*eptr++] & ctype_digit) == 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case OP_NOT_WHITESPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if ((md->ctypes[*eptr++] & ctype_space) != 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case OP_WHITESPACE:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if ((md->ctypes[*eptr++] & ctype_space) == 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case OP_NOT_WORDCHAR:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if ((md->ctypes[*eptr++] & ctype_word) != 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            case OP_WORDCHAR:
                                for (i = 1; i <= min; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    if ((md->ctypes[*eptr++] & ctype_word) == 0) MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                            default: RRETURN(PCRE_ERROR_INTERNAL);
                        }
                    }
                }
                if (min == max) continue;
                if (minimize) {
                #ifdef SUPPORT_UCP
                    if (prop_type >= 0) {
                        switch(prop_type) {
                            case PT_ANY:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM36);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    if (prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                            case PT_LAMP:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM37);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_chartype = UCD_CHARTYPE(c);
                                    if ((prop_chartype == ucp_Lu || prop_chartype == ucp_Ll || prop_chartype == ucp_Lt) == prop_fail_result)
                                        MRRETURN(MATCH_NOMATCH);
                                }
                            case PT_GC:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM38);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == prop_value) == prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                            case PT_PC:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM39);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_chartype = UCD_CHARTYPE(c);
                                    if ((prop_chartype == prop_value) == prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                            case PT_SC:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM40);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_script = UCD_SCRIPT(c);
                                    if ((prop_script == prop_value) == prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                            case PT_ALNUM:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM59);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_L || prop_category == ucp_N) == prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                            case PT_SPACE:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM60);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR) == prop_fail_result)
                                        MRRETURN(MATCH_NOMATCH);
                                }
                            case PT_PXSPACE:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM61);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR) == prop_fail_result)
                                        MRRETURN(MATCH_NOMATCH);
                                }
                            case PT_WORD:
                                for (fi = min;; fi++) {
                                    RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM62);
                                    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                                    if (fi >= max) MRRETURN(MATCH_NOMATCH);
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        MRRETURN(MATCH_NOMATCH);
                                    }
                                    GETCHARINCTEST(c, eptr);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_L || prop_category == ucp_N || c == CHAR_UNDERSCORE) == prop_fail_result) MRRETURN(MATCH_NOMATCH);
                                }
                            default: RRETURN(PCRE_ERROR_INTERNAL);
                        }
                    } else if (ctype == OP_EXTUNI) {
                        for (fi = min;; fi++) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM41);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (fi >= max) MRRETURN(MATCH_NOMATCH);
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            GETCHARINCTEST(c, eptr);
                            prop_category = UCD_CATEGORY(c);
                            if (prop_category == ucp_M) MRRETURN(MATCH_NOMATCH);
                            while (eptr < md->end_subject) {
                                int len = 1;
                                if (!utf8) c = *eptr;
                                else { GETCHARLEN(c, eptr, len); }
                                prop_category = UCD_CATEGORY(c);
                                if (prop_category != ucp_M) break;
                                eptr += len;
                            }
                        }
                    } else
                #endif
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        for (fi = min;; fi++) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM42);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (fi >= max) MRRETURN(MATCH_NOMATCH);
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            if (ctype == OP_ANY && IS_NEWLINE(eptr)) MRRETURN(MATCH_NOMATCH);
                            GETCHARINC(c, eptr);
                            switch(ctype) {
                                case OP_ANY: case OP_ALLANY: case OP_ANYBYTE: break;
                                case OP_ANYNL:
                                    switch(c) {
                                        case 0x000d:
                                        if (eptr < md->end_subject && *eptr == 0x0a) eptr++;
                                        break;
                                        case 0x000a: break;
                                        case 0x000b: case 0x000c: case 0x0085: case 0x2028: case 0x2029:
                                        if (md->bsr_anycrlf) MRRETURN(MATCH_NOMATCH);
                                        break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                    break;
                                case OP_NOT_HSPACE:
                                    switch(c) {
                                        case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004:
                                        case 0x2005: case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                                            MRRETURN(MATCH_NOMATCH);
                                        default: break;
                                    }
                                    break;
                                case OP_HSPACE:
                                    switch(c) {
                                        case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e: case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004:
                                        case 0x2005: case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                                            break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                    break;
                                case OP_NOT_VSPACE:
                                    switch(c) {
                                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029: MRRETURN(MATCH_NOMATCH);
                                        default: break;
                                    }
                                    break;
                                case OP_VSPACE:
                                    switch(c) {
                                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029: break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                    break;
                                case OP_NOT_DIGIT:
                                    if (c < 256 && (md->ctypes[c] & ctype_digit) != 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_DIGIT:
                                    if (c >= 256 || (md->ctypes[c] & ctype_digit) == 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_NOT_WHITESPACE:
                                    if (c < 256 && (md->ctypes[c] & ctype_space) != 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_WHITESPACE:
                                    if  (c >= 256 || (md->ctypes[c] & ctype_space) == 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_NOT_WORDCHAR:
                                    if (c < 256 && (md->ctypes[c] & ctype_word) != 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_WORDCHAR:
                                    if (c >= 256 || (md->ctypes[c] & ctype_word) == 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                default: RRETURN(PCRE_ERROR_INTERNAL);
                            }
                        }
                    } else
                #endif
                    {
                        for (fi = min;; fi++) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM43);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (fi >= max) MRRETURN(MATCH_NOMATCH);
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                MRRETURN(MATCH_NOMATCH);
                            }
                            if (ctype == OP_ANY && IS_NEWLINE(eptr)) MRRETURN(MATCH_NOMATCH);
                            c = *eptr++;
                            switch(ctype) {
                                case OP_ANY: case OP_ALLANY: case OP_ANYBYTE: break;
                                case OP_ANYNL:
                                switch(c) {
                                    case 0x000d:
                                        if (eptr < md->end_subject && *eptr == 0x0a) eptr++;
                                        break;
                                    case 0x000a: break;
                                    case 0x000b: case 0x000c: case 0x0085:
                                        if (md->bsr_anycrlf) MRRETURN(MATCH_NOMATCH);
                                        break;
                                    default: MRRETURN(MATCH_NOMATCH);
                                }
                                break;
                                case OP_NOT_HSPACE:
                                    switch(c) {
                                        case 0x09: case 0x20: case 0xa0: MRRETURN(MATCH_NOMATCH);
                                        default: break;
                                    }
                                    break;
                                case OP_HSPACE:
                                    switch(c) {
                                        case 0x09: case 0x20: case 0xa0: break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                    break;
                                case OP_NOT_VSPACE:
                                    switch(c) {
                                        case 0x0a: case 0x0b: case 0x0c:  case 0x0d: case 0x85: MRRETURN(MATCH_NOMATCH);
                                        default: break;
                                    }
                                    break;
                                case OP_VSPACE:
                                    switch(c) {
                                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: break;
                                        default: MRRETURN(MATCH_NOMATCH);
                                    }
                                    break;
                                case OP_NOT_DIGIT:
                                    if ((md->ctypes[c] & ctype_digit) != 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_DIGIT:
                                    if ((md->ctypes[c] & ctype_digit) == 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_NOT_WHITESPACE:
                                    if ((md->ctypes[c] & ctype_space) != 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_WHITESPACE:
                                    if  ((md->ctypes[c] & ctype_space) == 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_NOT_WORDCHAR:
                                    if ((md->ctypes[c] & ctype_word) != 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                case OP_WORDCHAR:
                                    if ((md->ctypes[c] & ctype_word) == 0) MRRETURN(MATCH_NOMATCH);
                                    break;
                                default: RRETURN(PCRE_ERROR_INTERNAL);
                            }
                        }
                    }
                } else {
                    pp = eptr;
                #ifdef SUPPORT_UCP
                    if (prop_type >= 0) {
                        switch(prop_type) {
                            case PT_ANY:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARENTEST(c, eptr, len);
                                    if (prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            case PT_LAMP:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLENTEST(c, eptr, len);
                                    prop_chartype = UCD_CHARTYPE(c);
                                    if ((prop_chartype == ucp_Lu || prop_chartype == ucp_Ll || prop_chartype == ucp_Lt) == prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            case PT_GC:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLENTEST(c, eptr, len);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == prop_value) == prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            case PT_PC:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLENTEST(c, eptr, len);
                                    prop_chartype = UCD_CHARTYPE(c);
                                    if ((prop_chartype == prop_value) == prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            case PT_SC:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLENTEST(c, eptr, len);
                                    prop_script = UCD_SCRIPT(c);
                                    if ((prop_script == prop_value) == prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            case PT_ALNUM:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLENTEST(c, eptr, len);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_L || prop_category == ucp_N) == prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            case PT_SPACE:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLENTEST(c, eptr, len);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR) == prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            case PT_PXSPACE:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLENTEST(c, eptr, len);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR) == prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            case PT_WORD:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLENTEST(c, eptr, len);
                                    prop_category = UCD_CATEGORY(c);
                                    if ((prop_category == ucp_L || prop_category == ucp_N || c == CHAR_UNDERSCORE) == prop_fail_result) break;
                                    eptr+= len;
                                }
                                break;
                            default: RRETURN(PCRE_ERROR_INTERNAL);
                        }
                        if (possessive) continue;
                        for(;;) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM44);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (eptr-- == pp) break;
                            if (utf8) BACKCHAR(eptr);
                        }
                    } else if (ctype == OP_EXTUNI) {
                        for (i = min; i < max; i++) {
                            if (eptr >= md->end_subject) {
                                SCHECK_PARTIAL();
                                break;
                            }
                            GETCHARINCTEST(c, eptr);
                            prop_category = UCD_CATEGORY(c);
                            if (prop_category == ucp_M) break;
                            while (eptr < md->end_subject) {
                                int len = 1;
                                if (!utf8) c = *eptr; else {
                                  GETCHARLEN(c, eptr, len);
                                }
                                prop_category = UCD_CATEGORY(c);
                                if (prop_category != ucp_M) break;
                                eptr += len;
                            }
                        }
                        if (possessive) continue;
                        for(;;) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM45);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (eptr-- == pp) break;
                            for (;;) {
                                int len = 1;
                                if (!utf8) c = *eptr;
                                else {
                                    BACKCHAR(eptr);
                                    GETCHARLEN(c, eptr, len);
                                }
                                prop_category = UCD_CATEGORY(c);
                                if (prop_category != ucp_M) break;
                                eptr--;
                            }
                        }
                    } else
                #endif
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        switch(ctype) {
                            case OP_ANY:
                                if (max < INT_MAX) {
                                    for (i = min; i < max; i++) {
                                        if (eptr >= md->end_subject) {
                                            SCHECK_PARTIAL();
                                            break;
                                        }
                                        if (IS_NEWLINE(eptr)) break;
                                        eptr++;
                                        while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
                                    }
                                } else {
                                    for (i = min; i < max; i++) {
                                        if (eptr >= md->end_subject) {
                                            SCHECK_PARTIAL();
                                            break;
                                        }
                                        if (IS_NEWLINE(eptr)) break;
                                        eptr++;
                                        while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
                                    }
                                }
                                break;
                            case OP_ALLANY:
                                if (max < INT_MAX) {
                                    for (i = min; i < max; i++) {
                                        if (eptr >= md->end_subject) {
                                            SCHECK_PARTIAL();
                                            break;
                                        }
                                        eptr++;
                                        while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
                                    }
                                } else eptr = md->end_subject;
                                break;
                            case OP_ANYBYTE:
                                c = max - min;
                                if (c > (unsigned int)(md->end_subject - eptr)) {
                                    eptr = md->end_subject;
                                    SCHECK_PARTIAL();
                                } else eptr += c;
                                break;
                            case OP_ANYNL:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    if (c == 0x000d) {
                                        if (++eptr >= md->end_subject) break;
                                        if (*eptr == 0x000a) eptr++;
                                    } else {
                                        if (c != 0x000a && (md->bsr_anycrlf || (c != 0x000b && c != 0x000c && c != 0x0085 && c != 0x2028 && c != 0x2029))) break;
                                        eptr += len;
                                    }
                                }
                                break;
                            case OP_NOT_HSPACE: case OP_HSPACE:
                                for (i = min; i < max; i++) {
                                    BOOL gotspace;
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    switch(c) {
                                        case 0x09: case 0x20: case 0xa0: case 0x1680: case 0x180e:  case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004:
                                        case 0x2005: case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A: case 0x202f: case 0x205f: case 0x3000:
                                            gotspace = TRUE;
                                            break;
                                        default: gotspace = FALSE; break;
                                    }
                                    if (gotspace == (ctype == OP_NOT_HSPACE)) break;
                                    eptr += len;
                                }
                                break;
                            case OP_NOT_VSPACE: case OP_VSPACE:
                                for (i = min; i < max; i++) {
                                    BOOL gotspace;
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    switch(c) {
                                        case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x85: case 0x2028: case 0x2029:
                                            gotspace = TRUE;
                                            break;
                                        default: gotspace = FALSE; break;
                                    }
                                    if (gotspace == (ctype == OP_NOT_VSPACE)) break;
                                    eptr += len;
                                }
                                break;
                            case OP_NOT_DIGIT:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    if (c < 256 && (md->ctypes[c] & ctype_digit) != 0) break;
                                    eptr+= len;
                                }
                                break;
                            case OP_DIGIT:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    if (c >= 256 ||(md->ctypes[c] & ctype_digit) == 0) break;
                                    eptr+= len;
                                }
                                break;
                            case OP_NOT_WHITESPACE:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    if (c < 256 && (md->ctypes[c] & ctype_space) != 0) break;
                                    eptr+= len;
                                }
                                break;
                            case OP_WHITESPACE:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    if (c >= 256 ||(md->ctypes[c] & ctype_space) == 0) break;
                                    eptr+= len;
                                }
                                break;
                            case OP_NOT_WORDCHAR:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    if (c < 256 && (md->ctypes[c] & ctype_word) != 0) break;
                                    eptr+= len;
                                }
                                break;
                            case OP_WORDCHAR:
                                for (i = min; i < max; i++) {
                                    int len = 1;
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    GETCHARLEN(c, eptr, len);
                                    if (c >= 256 || (md->ctypes[c] & ctype_word) == 0) break;
                                    eptr+= len;
                                }
                                break;
                            default: RRETURN(PCRE_ERROR_INTERNAL);
                        }
                        if (possessive) continue;
                        for(;;) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM46);
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                            if (eptr-- == pp) break;
                            BACKCHAR(eptr);
                        }
                    } else
                #endif
                    {
                        switch(ctype) {
                            case OP_ANY:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    if (IS_NEWLINE(eptr)) break;
                                    eptr++;
                                }
                                break;
                            case OP_ALLANY: case OP_ANYBYTE:
                                c = max - min;
                                if (c > (unsigned int)(md->end_subject - eptr)) {
                                    eptr = md->end_subject;
                                    SCHECK_PARTIAL();
                                } else eptr += c;
                                break;
                            case OP_ANYNL:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    c = *eptr;
                                    if (c == 0x000d) {
                                        if (++eptr >= md->end_subject) break;
                                        if (*eptr == 0x000a) eptr++;
                                    } else {
                                        if (c != 0x000a && (md->bsr_anycrlf || (c != 0x000b && c != 0x000c && c != 0x0085))) break;
                                        eptr++;
                                    }
                                }
                                break;

                            case OP_NOT_HSPACE:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    c = *eptr;
                                    if (c == 0x09 || c == 0x20 || c == 0xa0) break;
                                    eptr++;
                                }
                                break;
                            case OP_HSPACE:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    c = *eptr;
                                    if (c != 0x09 && c != 0x20 && c != 0xa0) break;
                                    eptr++;
                                }
                                break;
                            case OP_NOT_VSPACE:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    c = *eptr;
                                    if (c == 0x0a || c == 0x0b || c == 0x0c || c == 0x0d || c == 0x85) break;
                                    eptr++;
                                }
                                break;
                            case OP_VSPACE:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    c = *eptr;
                                    if (c != 0x0a && c != 0x0b && c != 0x0c && c != 0x0d && c != 0x85)break;
                                    eptr++;
                                }
                                break;
                            case OP_NOT_DIGIT:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                      SCHECK_PARTIAL();
                                      break;
                                    }
                                    if ((md->ctypes[*eptr] & ctype_digit) != 0) break;
                                    eptr++;
                                }
                                break;
                            case OP_DIGIT:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    if ((md->ctypes[*eptr] & ctype_digit) == 0) break;
                                    eptr++;
                                }
                                break;
                            case OP_NOT_WHITESPACE:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    if ((md->ctypes[*eptr] & ctype_space) != 0) break;
                                    eptr++;
                                }
                                break;
                            case OP_WHITESPACE:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    if ((md->ctypes[*eptr] & ctype_space) == 0) break;
                                    eptr++;
                                }
                                break;
                            case OP_NOT_WORDCHAR:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    if ((md->ctypes[*eptr] & ctype_word) != 0) break;
                                    eptr++;
                                }
                                break;
                            case OP_WORDCHAR:
                                for (i = min; i < max; i++) {
                                    if (eptr >= md->end_subject) {
                                        SCHECK_PARTIAL();
                                        break;
                                    }
                                    if ((md->ctypes[*eptr] & ctype_word) == 0) break;
                                    eptr++;
                                }
                                break;
                            default: RRETURN(PCRE_ERROR_INTERNAL);
                        }
                        if (possessive) continue;
                        while(eptr >= pp) {
                            RMATCH(eptr, ecode, offset_top, md, ims, eptrb, 0, RM47);
                            eptr--;
                            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
                        }
                    }
                    MRRETURN(MATCH_NOMATCH);
                }
            default:
                DPRINTF(("Unknown opcode %d\n", *ecode));
                RRETURN(PCRE_ERROR_UNKNOWN_OPCODE);
        }
    }
#ifdef NO_RECURSE
    #define LBL(val) case val: goto L_RM##val;
    HEAP_RETURN:
    switch(frame->Xwhere) {
        LBL( 1) LBL( 2) LBL( 3) LBL( 4) LBL( 5) LBL( 6) LBL( 7) LBL( 8)
        LBL( 9) LBL(10) LBL(11) LBL(12) LBL(13) LBL(14) LBL(15) LBL(17)
        LBL(19) LBL(24) LBL(25) LBL(26) LBL(27) LBL(29) LBL(31) LBL(33)
        LBL(35) LBL(43) LBL(47) LBL(48) LBL(49) LBL(50) LBL(51) LBL(52)
        LBL(53) LBL(54) LBL(55) LBL(56) LBL(57) LBL(58)
    #ifdef SUPPORT_UTF8
        LBL(16) LBL(18) LBL(20) LBL(21) LBL(22) LBL(23) LBL(28) LBL(30)
        LBL(32) LBL(34) LBL(42) LBL(46)
    #ifdef SUPPORT_UCP
        LBL(36) LBL(37) LBL(38) LBL(39) LBL(40) LBL(41) LBL(44) LBL(45)
        LBL(59) LBL(60) LBL(61) LBL(62)
    #endif
    #endif
        default:
            DPRINTF(("jump error in pcre match: label %d non-existent\n", frame->Xwhere));
            return PCRE_ERROR_INTERNAL;
    }
#undef LBL
#endif
}
#ifdef NO_RECURSE
#undef eptr
#undef ecode
#undef mstart
#undef offset_top
#undef ims
#undef eptrb
#undef flags
#undef callpat
#undef charptr
#undef data
#undef next
#undef pp
#undef prev
#undef saved_eptr
#undef new_recursive
#undef cur_is_word
#undef condition
#undef prev_is_word
#undef original_ims
#undef ctype
#undef length
#undef max
#undef min
#undef number
#undef offset
#undef op
#undef save_capture_last
#undef save_offset1
#undef save_offset2
#undef save_offset3
#undef stacksave
#undef newptrb
#endif
#undef fc
#undef fi
PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_exec(const pcre *argument_re, const pcre_extra *extra_data, PCRE_SPTR subject, int length, int start_offset,
                                                 int options, int *offsets, int offsetcount) {
    int rc, resetcount, ocount;
    int first_byte = -1;
    int req_byte = -1;
    int req_byte2 = -1;
    int newline;
    unsigned long int ims;
    BOOL using_temporary_offsets = FALSE;
    BOOL anchored;
    BOOL startline;
    BOOL firstline;
    BOOL first_byte_caseless = FALSE;
    BOOL req_byte_caseless = FALSE;
    BOOL utf8;
    match_data match_block;
    match_data *md = &match_block;
    const uschar *tables;
    const uschar *start_bits = NULL;
    USPTR start_match = (USPTR)subject + start_offset;
    USPTR end_subject;
    USPTR start_partial = NULL;
    USPTR req_byte_ptr = start_match - 1;
    pcre_study_data internal_study;
    const pcre_study_data *study;
    real_pcre internal_re;
    const real_pcre *external_re = (const real_pcre *)argument_re;
    const real_pcre *re = external_re;
    if ((options & ~PUBLIC_EXEC_OPTIONS) != 0) return PCRE_ERROR_BADOPTION;
    if (re == NULL || subject == NULL || (offsets == NULL && offsetcount > 0)) return PCRE_ERROR_NULL;
    if (offsetcount < 0) return PCRE_ERROR_BADCOUNT;
    if (start_offset < 0 || start_offset > length) return PCRE_ERROR_BADOFFSET;
    md->name_table = (uschar *)re + re->name_table_offset;
    md->name_count = re->name_count;
    md->name_entry_size = re->name_entry_size;
    study = NULL;
    //md->match_limit = MATCH_LIMIT;
    //md->match_limit_recursion = MATCH_LIMIT_RECURSION;
    md->callout_data = NULL;
    tables = external_re->tables;
    if (extra_data != NULL) {
        register unsigned int flags = extra_data->flags;
        if ((flags & PCRE_EXTRA_STUDY_DATA) != 0) study = (const pcre_study_data *)extra_data->study_data;
        if ((flags & PCRE_EXTRA_MATCH_LIMIT) != 0) md->match_limit = extra_data->match_limit;
        if ((flags & PCRE_EXTRA_MATCH_LIMIT_RECURSION) != 0) md->match_limit_recursion = extra_data->match_limit_recursion;
        if ((flags & PCRE_EXTRA_CALLOUT_DATA) != 0) md->callout_data = extra_data->callout_data;
        if ((flags & PCRE_EXTRA_TABLES) != 0) tables = extra_data->tables;
    }
    if (tables == NULL) tables = _pcre_default_tables;
    if (re->magic_number != MAGIC_NUMBER) {
        re = _pcre_try_flipped(re, &internal_re, study, &internal_study);
        if (re == NULL) return PCRE_ERROR_BADMAGIC;
        if (study != NULL) study = &internal_study;
    }
    anchored = ((re->options | options) & PCRE_ANCHORED) != 0;
    startline = (re->flags & PCRE_STARTLINE) != 0;
    firstline = (re->options & PCRE_FIRSTLINE) != 0;
    md->start_code = (const uschar *)external_re + re->name_table_offset + re->name_count * re->name_entry_size;
    md->start_subject = (USPTR)subject;
    md->start_offset = start_offset;
    md->end_subject = md->start_subject + length;
    end_subject = md->end_subject;
    md->endonly = (re->options & PCRE_DOLLAR_ENDONLY) != 0;
    utf8 = md->utf8 = (re->options & PCRE_UTF8) != 0;
    md->use_ucp = (re->options & PCRE_UCP) != 0;
    md->jscript_compat = (re->options & PCRE_JAVASCRIPT_COMPAT) != 0;
    md->notbol = (options & PCRE_NOTBOL) != 0;
    md->noteol = (options & PCRE_NOTEOL) != 0;
    md->notempty = (options & PCRE_NOTEMPTY) != 0;
    md->notempty_atstart = (options & PCRE_NOTEMPTY_ATSTART) != 0;
    md->partial = ((options & PCRE_PARTIAL_HARD) != 0)? 2 : ((options & PCRE_PARTIAL_SOFT) != 0)? 1 : 0;
    md->hitend = FALSE;
    md->mark = NULL;
    md->recursive = NULL;
    md->lcc = tables + lcc_offset;
    md->ctypes = tables + ctypes_offset;
    switch (options & (PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE)) {
        case 0:
            if ((re->options & (PCRE_BSR_ANYCRLF|PCRE_BSR_UNICODE)) != 0) md->bsr_anycrlf = (re->options & PCRE_BSR_ANYCRLF) != 0;
            else
        #ifdef BSR_ANYCRLF
            md->bsr_anycrlf = TRUE;
        #else
            md->bsr_anycrlf = FALSE;
        #endif
            break;
        case PCRE_BSR_ANYCRLF: md->bsr_anycrlf = TRUE; break;
        case PCRE_BSR_UNICODE: md->bsr_anycrlf = FALSE; break;
        default: return PCRE_ERROR_BADNEWLINE;
    }
    switch((((options & PCRE_NEWLINE_BITS) == 0) ? re->options : (pcre_uint32)options) & PCRE_NEWLINE_BITS) {
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
    if (md->partial && (re->flags & PCRE_NOPARTIAL) != 0) return PCRE_ERROR_BADPARTIAL;
#ifdef SUPPORT_UTF8
    if (utf8 && (options & PCRE_NO_UTF8_CHECK) == 0) {
        int tb;
        if ((tb = _pcre_valid_utf8((USPTR)subject, length)) >= 0) return (tb == length && md->partial > 1) ? PCRE_ERROR_SHORTUTF8 : PCRE_ERROR_BADUTF8;
        if (start_offset > 0 && start_offset < length) {
            tb = ((USPTR)subject)[start_offset] & 0xc0;
            if (tb == 0x80) return PCRE_ERROR_BADUTF8_OFFSET;
        }
    }
#endif
    ims = re->options & (PCRE_CASELESS|PCRE_MULTILINE|PCRE_DOTALL);
    ocount = offsetcount - (offsetcount % 3);
    if (re->top_backref > 0 && re->top_backref >= ocount/3) {
        ocount = re->top_backref * 3 + 3;
        md->offset_vector = (int *)(pcre_malloc)(ocount * sizeof(int));
        if (md->offset_vector == NULL) return PCRE_ERROR_NOMEMORY;
        using_temporary_offsets = TRUE;
        DPRINTF(("Got memory to hold back references\n"));
    } else md->offset_vector = offsets;
    md->offset_end = ocount;
    md->offset_max = (2*ocount)/3;
    md->offset_overflow = FALSE;
    md->capture_last = -1;
    resetcount = 2 + re->top_bracket * 2;
    if (resetcount > offsetcount) resetcount = ocount;
    if (md->offset_vector != NULL) {
        register int *iptr = md->offset_vector + ocount;
        register int *iend = iptr - resetcount/2 + 1;
        while (--iptr >= iend) *iptr = -1;
    }
    if (!anchored) {
        if ((re->flags & PCRE_FIRSTSET) != 0) {
            first_byte = re->first_byte & 255;
            if ((first_byte_caseless = ((re->first_byte & REQ_CASELESS) != 0)) == TRUE) first_byte = md->lcc[first_byte];
        } else if (!startline && study != NULL && (study->flags & PCRE_STUDY_MAPPED) != 0) start_bits = study->start_bits;
    }
    if ((re->flags & PCRE_REQCHSET) != 0) {
        req_byte = re->req_byte & 255;
        req_byte_caseless = (re->req_byte & REQ_CASELESS) != 0;
        req_byte2 = (tables + fcc_offset)[req_byte];  /* case flipped */
    }
    for(;;) {
        USPTR save_end_subject = end_subject;
        USPTR new_start_match;
        if (md->offset_vector != NULL) {
            register int *iptr = md->offset_vector;
            register int *iend = iptr + resetcount;
            while (iptr < iend) *iptr++ = -1;
        }
        if (firstline) {
            USPTR t = start_match;
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
                    while(start_match < end_subject && md->lcc[*start_match] != first_byte) start_match++;
                } else {
                    while(start_match < end_subject && *start_match != first_byte) start_match++;
                }
            } else if (startline) {
                if (start_match > md->start_subject + start_offset) {
                #ifdef SUPPORT_UTF8
                    if (utf8) {
                        while (start_match < end_subject && !WAS_NEWLINE(start_match)) {
                        start_match++;
                        while(start_match < end_subject && (*start_match & 0xc0) == 0x80) start_match++;
                        }
                    } else
                #endif
                    while(start_match < end_subject && !WAS_NEWLINE(start_match)) start_match++;
                    if (start_match[-1] == CHAR_CR && (md->nltype == NLTYPE_ANY || md->nltype == NLTYPE_ANYCRLF) && start_match < end_subject &&
                        *start_match == CHAR_NL)
                        start_match++;
                }
            } else if (start_bits != NULL) {
                while (start_match < end_subject) {
                    register unsigned int c = *start_match;
                    if ((start_bits[c/8] & (1 << (c&7))) == 0) {
                        start_match++;
                    #ifdef SUPPORT_UTF8
                        if (utf8) while(start_match < end_subject && (*start_match & 0xc0) == 0x80) start_match++;
                    #endif
                    } else break;
                }
            }
        }
        end_subject = save_end_subject;
        if ((options & PCRE_NO_START_OPTIMIZE) == 0 && !md->partial) {
            if (study != NULL && (study->flags & PCRE_STUDY_MINLEN) != 0 && (pcre_uint32)(end_subject - start_match) < study->minlength) {
                rc = MATCH_NOMATCH;
                break;
            }
            if (req_byte >= 0 && end_subject - start_match < REQ_BYTE_MAX) {
                register USPTR p = start_match + ((first_byte >= 0)? 1 : 0);
                if (p > req_byte_ptr) {
                    if (req_byte_caseless) {
                        while(p < end_subject) {
                            register int pp = *p++;
                            if (pp == req_byte || pp == req_byte2) { p--; break; }
                        }
                    } else {
                        while(p < end_subject) {
                            if (*p++ == req_byte) { p--; break; }
                        }
                    }
                    if (p >= end_subject) {
                        rc = MATCH_NOMATCH;
                        break;
                    }
                    req_byte_ptr = p;
                }
            }
        }
    #ifdef PCRE_DEBUG
        printf(">>>> Match against: ");
        pchars(start_match, end_subject - start_match, TRUE, md);
        printf("\n");
    #endif
        md->start_match_ptr = start_match;
        md->start_used_ptr = start_match;
        md->match_call_count = 0;
        rc = match(start_match, md->start_code, start_match, NULL, 2, md, ims, NULL,0, 0);
        if (md->hitend && start_partial == NULL) start_partial = md->start_used_ptr;
        switch(rc) {
            case MATCH_SKIP:
                if (md->start_match_ptr != start_match) {
                    new_start_match = md->start_match_ptr;
                    break;
                }
            case MATCH_SKIP_ARG: case MATCH_NOMATCH: case MATCH_PRUNE: case MATCH_THEN:
                new_start_match = start_match + 1;
            #ifdef SUPPORT_UTF8
                if (utf8) while(new_start_match < end_subject && (*new_start_match & 0xc0) == 0x80) new_start_match++;
            #endif
                break;
            case MATCH_COMMIT:
                rc = MATCH_NOMATCH;
                goto ENDLOOP;
            default: goto ENDLOOP;
        }
        rc = MATCH_NOMATCH;
        if (firstline && IS_NEWLINE(start_match)) break;
        start_match = new_start_match;
        if (anchored || start_match > end_subject) break;
        if (start_match[-1] == CHAR_CR && start_match < end_subject && *start_match == CHAR_NL && (re->flags & PCRE_HASCRORLF) == 0 &&
            (md->nltype == NLTYPE_ANY || md->nltype == NLTYPE_ANYCRLF || md->nllen == 2))
            start_match++;
        md->mark = NULL;
    }
    ENDLOOP:
    if (rc == MATCH_MATCH || rc == MATCH_ACCEPT) {
        if (using_temporary_offsets) {
            if (offsetcount >= 4) {
                memcpy(offsets + 2, md->offset_vector + 2,(offsetcount - 2) * sizeof(int));
                DPRINTF(("Copied offsets from temporary memory\n"));
            }
            if (md->end_offset_top > offsetcount) md->offset_overflow = TRUE;
            DPRINTF(("Freeing temporary memory\n"));
            (pcre_free)(md->offset_vector);
        }
        rc = md->offset_overflow? 0 : md->end_offset_top/2;
        if (offsetcount < 2) rc = 0;
        else {
            offsets[0] = (int)(md->start_match_ptr - md->start_subject);
            offsets[1] = (int)(md->end_match_ptr - md->start_subject);
        }
        DPRINTF((">>>> returning %d\n", rc));
        goto RETURN_MARK;
    }
    if (using_temporary_offsets) {
      DPRINTF(("Freeing temporary memory\n"));
      (pcre_free)(md->offset_vector);
    }
    if (rc != MATCH_NOMATCH && rc != PCRE_ERROR_PARTIAL) {
        DPRINTF((">>>> error: returning %d\n", rc));
        return rc;
    }
    if (start_partial != NULL)  {
        DPRINTF((">>>> returning PCRE_ERROR_PARTIAL\n"));
        md->mark = NULL;
        if (offsetcount > 1)
        {
        offsets[0] = (int)(start_partial - (USPTR)subject);
        offsets[1] = (int)(end_subject - (USPTR)subject);
        }
        rc = PCRE_ERROR_PARTIAL;
    } else {
        DPRINTF((">>>> returning PCRE_ERROR_NOMATCH\n"));
        rc = PCRE_ERROR_NOMATCH;
    }
    RETURN_MARK:
    if (extra_data != NULL && (extra_data->flags & PCRE_EXTRA_MARK) != 0) *(extra_data->mark) = (unsigned char *)(md->mark);
    return rc;
}