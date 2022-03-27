#include "tool_setup.h"

#ifdef __VMS
#if defined(__DECC) && !defined(__VAX) && defined(__CRTL_VER) && (__CRTL_VER >= 70301000)
#include <unixlib.h>
#endif
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "curlmsg_vms.h"
#include "tool_vms.h"
#include "memdebug.h"

void decc$__posix_exit(int __status);
void decc$exit(int __status);
static int vms_shell = -1;
int is_vms_shell(void) {
    char *shell;
    if(vms_shell >= 0) return vms_shell;
    shell = getenv("SHELL");
    if(shell == NULL) {
        vms_shell = 1;
        return 1;
    }
    if(strcmp(shell, "DCL") == 0) {
        vms_shell = 1;
        return 1;
    }
    vms_shell = 0;
    return 0;
}
void vms_special_exit(int code, int vms_show) {
    int vms_code;
#if __CRTL_VER >= 70000000
    if(is_vms_shell() == 0) decc$__posix_exit(code);
#endif
    if(code > CURL_LAST)  vms_code = CURL_LAST;
    else  vms_code = vms_cond[code] | vms_show;
    decc$exit(vms_code);
}
#if defined(__DECC) && !defined(__VAX) && defined(__CRTL_VER) && (__CRTL_VER >= 70301000)
struct decc_feat_t {
    char *name;
    int value;
};
static struct decc_feat_t decc_feat_array[] = {
    { "DECC$ARGV_PARSE_STYLE", 1 },
    { "DECC$EFS_CASE_PRESERVE", 1 },
    { "DECC$EFS_CHARSET", 1 },
    { (char *)NULL, 0 }
};
static int decc_init_done = -1;
static void decc_init(void) {
    int feat_index;
    int feat_value;
    int feat_value_max;
    int feat_value_min;
    int i;
    int sts;
    decc_init_done = 1;
    for(i = 0; decc_feat_array[i].name != NULL; i++) {
        feat_index = decc$feature_get_index(decc_feat_array[i].name);
        if(feat_index >= 0) {
            feat_value = decc$feature_get_value(feat_index, 1);
            feat_value_min = decc$feature_get_value(feat_index, 2);
            feat_value_max = decc$feature_get_value(feat_index, 3);
            if((decc_feat_array[i].value >= feat_value_min) && (decc_feat_array[i].value <= feat_value_max)) {
                if(feat_value != decc_feat_array[i].value) sts = decc$feature_set_value(feat_index, 1, decc_feat_array[i].value);
            } else printf(" INVALID DECC FEATURE VALUE, %d: %d <= %s <= %d.\n", feat_value, feat_value_min, decc_feat_array[i].name, feat_value_max);
        } else printf(" UNKNOWN DECC FEATURE: %s.\n", decc_feat_array[i].name);
    }
}
#pragma nostandard
#pragma extern_model save
#pragma extern_model strict_refdef "LIB$INITIALIZ" 2, nopic, nowrt
const int spare[8] = {0};
#pragma extern_model strict_refdef "LIB$INITIALIZE" 2, nopic, nowrt
void (*const x_decc_init)() = decc_init;
#pragma extern_model restore
#pragma extern_model save
int LIB$INITIALIZE(void);
#pragma extern_model strict_refdef
int dmy_lib$initialize = (int) LIB$INITIALIZE;
#pragma extern_model restore
#pragma standard
#endif
#endif