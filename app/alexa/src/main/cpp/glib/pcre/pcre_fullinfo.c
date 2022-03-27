#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_fullinfo(const pcre *argument_re, const pcre_extra *extra_data, int what, void *where) {
    real_pcre internal_re;
    pcre_study_data internal_study;
    const real_pcre *re = (const real_pcre *)argument_re;
    const pcre_study_data *study = NULL;
    if (re == NULL || where == NULL) return PCRE_ERROR_NULL;
    if (extra_data != NULL && (extra_data->flags & PCRE_EXTRA_STUDY_DATA) != 0) study = (const pcre_study_data *)extra_data->study_data;
    if (re->magic_number != MAGIC_NUMBER)  {
        re = _pcre_try_flipped(re, &internal_re, study, &internal_study);
        if (re == NULL) return PCRE_ERROR_BADMAGIC;
        if (study != NULL) study = &internal_study;
    }
    switch (what) {
        case PCRE_INFO_OPTIONS: *((unsigned long int *)where) = re->options & PUBLIC_COMPILE_OPTIONS; break;
        case PCRE_INFO_SIZE: *((size_t*)where) = re->size; break;
        case PCRE_INFO_STUDYSIZE: *((size_t*)where) = (study == NULL)? 0 : study->size; break;
        case PCRE_INFO_CAPTURECOUNT: *((int*)where) = re->top_bracket; break;
        case PCRE_INFO_BACKREFMAX: *((int*)where) = re->top_backref; break;
        case PCRE_INFO_FIRSTBYTE: *((int*)where) = ((re->flags & PCRE_FIRSTSET) != 0)? re->first_byte : ((re->flags & PCRE_STARTLINE) != 0)? -1 : -2; break;
        case PCRE_INFO_FIRSTTABLE:
            *((const uschar**)where) = (study != NULL && (study->flags & PCRE_STUDY_MAPPED)!=0) ? ((const pcre_study_data *)extra_data->study_data)->start_bits : NULL;
            break;
        case PCRE_INFO_MINLENGTH: *((int*)where) = (study != NULL && (study->flags & PCRE_STUDY_MINLEN) != 0) ? study->minlength : -1; break;
        case PCRE_INFO_LASTLITERAL: *((int*)where) = ((re->flags & PCRE_REQCHSET) != 0)? re->req_byte : -1; break;
        case PCRE_INFO_NAMEENTRYSIZE: *((int*)where) = re->name_entry_size; break;
        case PCRE_INFO_NAMECOUNT: *((int*)where) = re->name_count; break;
        case PCRE_INFO_NAMETABLE: *((const uschar**)where) = (const uschar *)re + re->name_table_offset; break;
        case PCRE_INFO_DEFAULT_TABLES: *((const uschar**)where) = (const uschar *)(_pcre_default_tables); break;
        case PCRE_INFO_OKPARTIAL: *((int*)where) = (re->flags & PCRE_NOPARTIAL) == 0; break;
        case PCRE_INFO_JCHANGED: *((int*)where) = (re->flags & PCRE_JCHANGED) != 0; break;
        case PCRE_INFO_HASCRORLF: *((int*)where) = (re->flags & PCRE_HASCRORLF) != 0; break;
        default: return PCRE_ERROR_BADOPTION;
    }
    return 0;
}
