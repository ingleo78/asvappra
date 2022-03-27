#include "rename.h"
#include "curl_setup.h"

#if (!defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)) || defined(USE_ALTSVC)

#include "timeval.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

int Curl_rename(const char *oldpath, const char *newpath) {
#ifdef WIN32
    const int max_wait_ms = 1000;
    struct curltime start = Curl_now();
    for(;;) {
        timediff_t diff;
        if(MoveFileExA(oldpath, newpath, MOVEFILE_REPLACE_EXISTING)) break;
        diff = Curl_timediff(Curl_now(), start);
        if(diff < 0 || diff > max_wait_ms) return 1;
        Sleep(1);
    }
#else
  if(rename(oldpath, newpath)) return 1;
#endif
  return 0;
}

#endif