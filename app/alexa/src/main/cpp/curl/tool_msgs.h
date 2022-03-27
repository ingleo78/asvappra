#ifndef HEADER_CURL_TOOL_MSGS_H
#define HEADER_CURL_TOOL_MSGS_H

#include "tool_setup.h"

void warnf(struct GlobalConfig *config, const char *fmt, ...);
void notef(struct GlobalConfig *config, const char *fmt, ...);
void helpf(FILE *errors, const char *fmt, ...);
void errorf(struct GlobalConfig *config, const char *fmt, ...);

#endif