#include "tool_setup.h"

#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "tool_cfgable.h"
#include "tool_cb_see.h"
#include "memdebug.h"

#define OUR_MAX_SEEK_L  2147483647L - 1L
#define OUR_MAX_SEEK_O  CURL_OFF_T_C(0x7FFFFFFF) - CURL_OFF_T_C(0x1)

int tool_seek_cb(void *userdata, curl_off_t offset, int whence) {
    struct InStruct *in = userdata;
#if(CURL_SIZEOF_CURL_OFF_T > SIZEOF_OFF_T) && !defined(USE_WIN32_LARGE_FILES)
    if(offset > OUR_MAX_SEEK_O) {
        curl_off_t left = offset;
        if(whence != SEEK_SET) return CURL_SEEKFUNC_FAIL;
        if(LSEEK_ERROR == lseek(in->fd, 0, SEEK_SET)) return CURL_SEEKFUNC_FAIL;
        while(left) {
            long step = (left > OUR_MAX_SEEK_O) ? OUR_MAX_SEEK_L : (long)left;
            if(LSEEK_ERROR == lseek(in->fd, step, SEEK_CUR)) return CURL_SEEKFUNC_FAIL;
            left -= step;
        }
        return CURL_SEEKFUNC_OK;
    }
#endif
    if(LSEEK_ERROR == lseek(in->fd, offset, whence)) return CURL_SEEKFUNC_CANTSEEK;
    return CURL_SEEKFUNC_OK;
}
#ifdef USE_TOOL_FTRUNCATE
#ifdef __BORLANDC__
#define _lseeki64(hnd,ofs,whence) lseek(hnd,ofs,whence)
#endif
#ifdef __POCC__
#if(__POCC__ < 450)
#define _lseeki64(hnd,ofs,whence) _lseek(hnd,ofs,whence)
#else
#define _lseeki64(hnd,ofs,whence) _lseek64(hnd,ofs,whence)
#endif
#endif
#ifdef _WIN32_WCE
#undef _lseeki64
#define _lseeki64(hnd,ofs,whence) lseek(hnd,ofs,whence)
#undef _get_osfhandle
#define _get_osfhandle(fd) (fd)
#endif

int tool_ftruncate64(int fd, curl_off_t where) {
    intptr_t handle = _get_osfhandle(fd);
    if(_lseeki64(fd, where, SEEK_SET) < 0) return -1;
    if(!SetEndOfFile((HANDLE)handle)) return -1;
    return 0;
}
#endif