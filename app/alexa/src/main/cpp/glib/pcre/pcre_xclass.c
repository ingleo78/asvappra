#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

BOOL _pcre_xclass(int c, const uschar *data) {
    int t;
    BOOL negated = (*data & XCL_NOT) != 0;
    if (c < 256) {
        if ((*data & XCL_MAP) != 0 && (data[1 + c/8] & (1 << (c&7))) != 0) return !negated;
    }
    if ((*data++ & XCL_MAP) != 0) data += 32;
    while ((t = *data++) != XCL_END) {
        int x, y;
        if (t == XCL_SINGLE) {
            GETCHARINC(x, data);
            if (c == x) return !negated;
        } else if (t == XCL_RANGE) {
            GETCHARINC(x, data);
            GETCHARINC(y, data);
            if (c >= x && c <= y) return !negated;
        }
    #ifdef SUPPORT_UCP
        else {
            int chartype = UCD_CHARTYPE(c);
            switch(*data) {
                case PT_ANY: if (t == XCL_PROP) return !negated; break;
                case PT_LAMP: if ((chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt) == (t == XCL_PROP)) return !negated; break;
                case PT_GC: if ((data[1] == _pcre_ucp_gentype[chartype]) == (t == XCL_PROP)) return !negated;break;
                case PT_PC: if ((data[1] == chartype) == (t == XCL_PROP)) return !negated; break;
                case PT_SC: if ((data[1] == UCD_SCRIPT(c)) == (t == XCL_PROP)) return !negated; break;
                case PT_ALNUM:
                    if ((_pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N) == (t == XCL_PROP)) return !negated;
                    break;
                case PT_SPACE:
                    if ((_pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_FF || c == CHAR_CR) == (t == XCL_PROP)) return !negated;
                    break;
                case PT_PXSPACE:
                    if ((_pcre_ucp_gentype[chartype] == ucp_Z || c == CHAR_HT || c == CHAR_NL || c == CHAR_VT || c == CHAR_FF || c == CHAR_CR) == (t == XCL_PROP))
                        return !negated;
                    break;
                case PT_WORD:
                    if ((_pcre_ucp_gentype[chartype] == ucp_L || _pcre_ucp_gentype[chartype] == ucp_N || c == CHAR_UNDERSCORE) == (t == XCL_PROP))return !negated;
                    break;
                default: return FALSE;
            }
            data += 2;
        }
    #endif
    }
    return negated;
}