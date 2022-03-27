#include "curl_setup.h"

#ifdef NETWARE
#ifdef __NOVELL_LIBC__
#include <library.h>
#include <netware.h>
#include <screen.h>
#include <nks/thread.h>
#include <nks/synch.h>
#include "curl_memory.h"
#include "memdebug.h"

struct libthreaddata {
    int     _errno;
    void    *twentybytes;
};
struct libdata {
    int         x;
    int         y;
    int         z;
    void        *tenbytes;
    NXKey_t     perthreadkey;
    NXMutex_t   *lock;
};
int         gLibId      = -1;
void        *gLibHandle = (void *) NULL;
rtag_t      gAllocTag   = (rtag_t) NULL;
NXMutex_t   *gLibLock   = (NXMutex_t *) NULL;
int  DisposeLibraryData(void *);
void DisposeThreadData(void *);
int  GetOrSetUpData(int id, struct libdata **data, struct libthreaddata **threaddata);
int _NonAppStart(void *NLMHandle, void *errorScreen, const char *cmdLine, const char *loadDirPath, size_t uninitializedDataLength, void *NLMFileHandle,
                 int (*readRoutineP)(int conn, void *fileHandle, size_t offset, size_t nbytes, size_t *bytesRead, void *buffer),
                 size_t customDataOffset, size_t customDataSize, int messageCount, const char **messages) {
    NX_LOCK_INFO_ALLOC(liblock, "Per-Application Data Lock", 0);
    #ifndef __GNUC__
    #pragma unused(cmdLine)
    #pragma unused(loadDirPath)
    #pragma unused(uninitializedDataLength)
    #pragma unused(NLMFileHandle)
    #pragma unused(readRoutineP)
    #pragma unused(customDataOffset)
    #pragma unused(customDataSize)
    #pragma unused(messageCount)
    #pragma unused(messages)
    #endif
    gAllocTag = AllocateResourceTag(NLMHandle, "<library-name> memory allocations", AllocSignature);
    if(!gAllocTag) {
        OutputToScreen(errorScreen, "Unable to allocate resource tag for " "library memory allocations.\n");
        return -1;
    }
    gLibId = register_library(DisposeLibraryData);
    if(gLibId < -1) {
        OutputToScreen(errorScreen, "Unable to register library with kernel.\n");
        return -1;
    }
    gLibHandle = NLMHandle;
    gLibLock = NXMutexAlloc(0, 0, &liblock);
    if(!gLibLock) {
        OutputToScreen(errorScreen, "Unable to allocate library data lock.\n");
        return -1;
    }
    return 0;
}
void _NonAppStop(void) {
    (void)unregister_library(gLibId);
    NXMutexFree(gLibLock);
}
int _NonAppCheckUnload(void) { return 0; }
int GetOrSetUpData(int id, struct libdata **appData, struct libthreaddata **threadData) {
    int                 err;
    struct libdata      *app_data;
    struct libthreaddata *thread_data;
    NXKey_t             key;
    NX_LOCK_INFO_ALLOC(liblock, "Application Data Lock", 0);
    err         = 0;
    thread_data = (struct libthreaddata_t*) NULL;
    app_data = (struct libdata*)get_app_data(id);
    if(!app_data) {
        NXLock(gLibLock);
        app_data = (struct libdata*)get_app_data(id);
        if(!app_data) {
            app_data = calloc(1, sizeof(struct libdata));
            if(app_data) {
                app_data->tenbytes = malloc(10);
                app_data->lock     = NXMutexAlloc(0, 0, &liblock);
                if(!app_data->tenbytes || !app_data->lock) {
                    if(app_data->lock) NXMutexFree(app_data->lock);
                    free(app_data->tenbytes);
                    free(app_data);
                    app_data = (libdata_t *) NULL;
                    err      = ENOMEM;
                }
                if(app_data) {
                    err = set_app_data(gLibId, app_data);
                    if(err) {
                        if(app_data->lock) NXMutexFree(app_data->lock);
                        free(app_data->tenbytes);
                        free(app_data);
                        app_data = (libdata_t*) NULL;
                        err = ENOMEM;
                    } else {
                        err = NXKeyCreate(DisposeThreadData, (void *) NULL, &key);
                        if(err) key = -1;
                        app_data->perthreadkey = key;
                    }
                }
            }
        }
        NXUnlock(gLibLock);
    }
    if(app_data) {
        key = app_data->perthreadkey;
        if(key != -1 && !(err = NXKeyGetValue(key, (void **) &thread_data)) && !thread_data) {
            thread_data = malloc(sizeof(struct libthreaddata));
            if(thread_data) {
                thread_data->_errno      = 0;
                thread_data->twentybytes = malloc(20);
                if(!thread_data->twentybytes) {
                    free(thread_data);
                    thread_data = (struct libthreaddata *) NULL;
                    err = ENOMEM;
                }
                err = NXKeySetValue(key, thread_data);
                if(err) {
                    free(thread_data->twentybytes);
                    free(thread_data);
                    thread_data = (struct libthreaddata *) NULL;
                }
            }
        }
    }
    if(appData) *appData = app_data;
    if(threadData) *threadData = thread_data;
    return err;
}
int DisposeLibraryData(void *data) {
    if(data) {
        void *tenbytes = ((libdata_t*)data)->tenbytes;
        free(tenbytes);
        free(data);
    }
    return 0;
}
void DisposeThreadData(void *data) {
    if(data) {
        void *twentybytes = ((struct libthreaddata *) data)->twentybytes;
        free(twentybytes);
        free(data);
    }
}
#else
#include <nwthread.h>
int main(void) {
    ExitThread(TSR_THREAD, 0);
    return 0;
}
#endif
#else
#ifdef __POCC__
#pragma warn(disable:2024)
#endif
#endif