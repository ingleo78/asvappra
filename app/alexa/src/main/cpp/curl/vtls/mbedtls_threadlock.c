#include "mbedtls_threadlock.h"

#if defined(USE_MBEDTLS) && ((defined(USE_THREADS_POSIX) && defined(HAVE_PTHREAD_H)) || (defined(USE_THREADS_WIN32) && defined(HAVE_PROCESS_H)))
#if defined(USE_THREADS_POSIX) && defined(HAVE_PTHREAD_H)
#include <pthread.h>
#define MBEDTLS_MUTEX_T pthread_mutex_t
#elif defined(USE_THREADS_WIN32) && defined(HAVE_PROCESS_H)
#include <process.h>
#define MBEDTLS_MUTEX_T HANDLE
#endif
#include "mbedtls_threadlock.h"
#include "../curl_printf.h"
#include "../curl_memory.h"
#include "../memdebug.h"

#define NUMT                    2

static MBEDTLS_MUTEX_T *mutex_buf = NULL;
int Curl_mbedtlsthreadlock_thread_setup(void) {
    int i;
    mutex_buf = calloc(NUMT * sizeof(MBEDTLS_MUTEX_T), 1);
    if(!mutex_buf) return 0;
    for(i = 0;  i < NUMT;  i++) {
        int ret;
    #if defined(USE_THREADS_POSIX) && defined(HAVE_PTHREAD_H)
        ret = pthread_mutex_init(&mutex_buf[i], NULL);
        if(ret) return 0;
    #elif defined(USE_THREADS_WIN32) && defined(HAVE_PROCESS_H)
        mutex_buf[i] = CreateMutex(0, FALSE, 0);
        if(mutex_buf[i] == 0) return 0;
    #endif
    }
    return 1;
}
int Curl_mbedtlsthreadlock_thread_cleanup(void) {
    int i;
    if(!mutex_buf) return 0;
    for(i = 0; i < NUMT; i++) {
        int ret;
    #if defined(USE_THREADS_POSIX) && defined(HAVE_PTHREAD_H)
        ret = pthread_mutex_destroy(&mutex_buf[i]);
        if(ret) return 0;
    #elif defined(USE_THREADS_WIN32) && defined(HAVE_PROCESS_H)
        ret = CloseHandle(mutex_buf[i]);
        if(!ret) return 0;
    #endif
    }
    free(mutex_buf);
    mutex_buf = NULL;
    return 1;
}
int Curl_mbedtlsthreadlock_lock_function(int n) {
    if(n < NUMT) {
        int ret;
    #if defined(USE_THREADS_POSIX) && defined(HAVE_PTHREAD_H)
        ret = pthread_mutex_lock(&mutex_buf[n]);
        if(ret) {
            DEBUGF(fprintf(stderr, "Error: mbedtlsthreadlock_lock_function failed\n"));
            return 0;
        }
    #elif defined(USE_THREADS_WIN32) && defined(HAVE_PROCESS_H)
        ret = (WaitForSingleObject(mutex_buf[n], INFINITE) == WAIT_FAILED?1:0);
        if(ret) {
            DEBUGF(fprintf(stderr, "Error: mbedtlsthreadlock_lock_function failed\n"));
            return 0;
        }
    #endif
    }
    return 1;
}
int Curl_mbedtlsthreadlock_unlock_function(int n) {
    if(n < NUMT) {
        int ret;
    #if defined(USE_THREADS_POSIX) && defined(HAVE_PTHREAD_H)
        ret = pthread_mutex_unlock(&mutex_buf[n]);
        if(ret) {
            DEBUGF(fprintf(stderr, "Error: mbedtlsthreadlock_unlock_function failed\n"));
            return 0;
        }
    #elif defined(USE_THREADS_WIN32) && defined(HAVE_PROCESS_H)
        ret = ReleaseMutex(mutex_buf[n]);
        if(!ret) {
            DEBUGF(fprintf(stderr, "Error: mbedtlsthreadlock_unlock_function failed\n"));
            return 0;
        }
    #endif
    }
    return 1;
}
#endif