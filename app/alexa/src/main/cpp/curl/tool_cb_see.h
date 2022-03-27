#ifndef HEADER_CURL_TOOL_CB_SEE_H
#define HEADER_CURL_TOOL_CB_SEE_H

#include "tool_setup.h"

#if defined(WIN32) && !defined(HAVE_FTRUNCATE)
int tool_ftruncate64(int fd, curl_off_t where);
#undef  ftruncate
#define ftruncate(fd,where) tool_ftruncate64(fd,where)
#define HAVE_FTRUNCATE 1
#define USE_TOOL_FTRUNCATE 1
#endif
int tool_seek_cb(void *userdata, curl_off_t offset, int whence);

#endif