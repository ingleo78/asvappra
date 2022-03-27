#include "libcompat.h"

#ifdef HAVE_WIN32_INIT_ONCE
static int mutex_init(pthread_mutex_t *mutex) {
    BOOL pending;
    int ret = 0;
    if (!InitOnceBeginInitialize(&mutex->init, 0, &pending, NULL)) return -1;
    if (pending) {
        mutex->mutex = CreateMutexW(NULL, FALSE, NULL);
        if (!mutex->mutex) ret = -1;
    }
    InitOnceComplete(&mutex->init, 0, NULL);
    return ret;
}
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr) {
    InitOnceInitialize(&mutex->init);
    return mutex_init(mutex);
}
int pthread_mutex_destroy(pthread_mutex_t *mutex) {
    return CloseHandle(mutex->mutex) ? 0 : -1;
}
int pthread_mutex_lock(pthread_mutex_t *mutex) {
    if (mutex_init(mutex) != 0) return -1;
    return WaitForSingleObject(mutex->mutex, INFINITE) != WAIT_OBJECT_0 ? -1 : 0;
}
int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    return ReleaseMutex(mutex->mutex) ? 0 : -1;
}
#endif