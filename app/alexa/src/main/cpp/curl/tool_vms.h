#ifndef HEADER_CURL_TOOL_VMS_H
#define HEADER_CURL_TOOL_VMS_H

#include "tool_setup.h"

#ifdef __VMS
extern int vms_show;
int is_vms_shell(void);
void vms_special_exit(int code, int vms_show);
#undef exit
#define exit(__code) vms_special_exit((__code), (0))
#define  VMS_STS(c,f,e,s) (((c&0xF)<<28)|((f&0xFFF)<<16)|((e&0x1FFF)<3)|(s&7))
#define  VMSSTS_HIDE  VMS_STS(1,0,0,0)
#endif
#endif