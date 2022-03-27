#ifndef CHECK_STR_H
#define CHECK_STR_H

#include "libcompat.h"

char *tr_str(TestResult *tr);
char *tr_short_str(TestResult *tr);
char *sr_stat_str(SRunner *sr);
char *ck_strdup_printf(const char *fmt, ...) CK_ATTRIBUTE_FORMAT(printf, 1, 2);

#endif