#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"
#include "ucp.h"

#ifdef NOT_USED_IN_GLIB
static int ucp_gentype[] = {
  ucp_C, ucp_C, ucp_C, ucp_C, ucp_C,  /* Cc, Cf, Cn, Co, Cs */
  ucp_L, ucp_L, ucp_L, ucp_L, ucp_L,  /* Ll, Lu, Lm, Lo, Lt */
  ucp_M, ucp_M, ucp_M,                /* Mc, Me, Mn */
  ucp_N, ucp_N, ucp_N,                /* Nd, Nl, No */
  ucp_P, ucp_P, ucp_P, ucp_P, ucp_P,  /* Pc, Pd, Pe, Pf, Pi */
  ucp_P, ucp_P,                       /* Ps, Po */
  ucp_S, ucp_S, ucp_S, ucp_S,         /* Sc, Sk, Sm, So */
  ucp_Z, ucp_Z, ucp_Z                 /* Zl, Zp, Zs */
};
int _pcre_ucp_findprop(const unsigned int c, int *type_ptr, int *script_ptr) {
    *type_ptr = g_unichar_type(c);
    *script_ptr = g_unichar_get_script(c);
    return ucp_gentype[*type_ptr];
}
#endif
unsigned int _pcre_ucp_othercase(const unsigned int c) {
    int other_case = NOTACHAR;
    if (g_unichar_islower(c)) other_case = g_unichar_toupper(c);
    else if (g_unichar_isupper(c)) other_case = g_unichar_tolower(c);
    if (other_case == c) other_case = NOTACHAR;
    return other_case;
}