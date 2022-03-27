#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

static unsigned long int byteflip(unsigned long int value, int n) {
    if (n == 2) return ((value & 0x00ff) << 8) | ((value & 0xff00) >> 8);
    return ((value & 0x000000ff) << 24) | ((value & 0x0000ff00) <<  8) | ((value & 0x00ff0000) >>  8) | ((value & 0xff000000) >> 24);
}
real_pcre *_pcre_try_flipped(const real_pcre *re, real_pcre *internal_re, const pcre_study_data *study, pcre_study_data *internal_study) {
    if (byteflip(re->magic_number, sizeof(re->magic_number)) != MAGIC_NUMBER) return NULL;
    *internal_re = *re;
    internal_re->size = byteflip(re->size, sizeof(re->size));
    internal_re->options = byteflip(re->options, sizeof(re->options));
    internal_re->flags = (pcre_uint16)byteflip(re->flags, sizeof(re->flags));
    internal_re->top_bracket = (pcre_uint16)byteflip(re->top_bracket, sizeof(re->top_bracket));
    internal_re->top_backref = (pcre_uint16)byteflip(re->top_backref, sizeof(re->top_backref));
    internal_re->first_byte = (pcre_uint16)byteflip(re->first_byte, sizeof(re->first_byte));
    internal_re->req_byte = (pcre_uint16)byteflip(re->req_byte, sizeof(re->req_byte));
    internal_re->name_table_offset = (pcre_uint16)byteflip(re->name_table_offset, sizeof(re->name_table_offset));
    internal_re->name_entry_size = (pcre_uint16)byteflip(re->name_entry_size, sizeof(re->name_entry_size));
    internal_re->name_count = (pcre_uint16)byteflip(re->name_count, sizeof(re->name_count));
    if (study != NULL) {
        *internal_study = *study;
        internal_study->size = byteflip(study->size, sizeof(study->size));
        internal_study->flags = byteflip(study->flags, sizeof(study->flags));
        internal_study->minlength = byteflip(study->minlength, sizeof(study->minlength));
    }
    return internal_re;
}