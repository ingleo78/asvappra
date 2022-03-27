#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "dbus-internals.h"
#include "dbus-sysdeps.h"
#include "dbus-threads.h"

#ifndef HAVE_MONOTONIC_CLOCK
static dbus_bool_t have_monotonic_clock = 0;
#endif
struct DBusRMutex {
  pthread_mutex_t lock;
};
struct DBusCMutex {
  pthread_mutex_t lock;
};
struct DBusCondVar {
  pthread_cond_t cond;
};
#define DBUS_MUTEX(m)  ((DBusMutex*) m)
#define DBUS_MUTEX_PTHREAD(m)  ((DBusMutexPThread*) m)
#define DBUS_COND_VAR(c)  ((DBusCondVar*) c)
#define DBUS_COND_VAR_PTHREAD(c)  ((DBusCondVarPThread*) c)
#ifdef DBUS_DISABLE_ASSERT
#define PTHREAD_CHECK(func_name, result_or_call)   do { int tmp = (result_or_call); if (tmp != 0) {;} } while(0);
#else
#define PTHREAD_CHECK(func_name, result_or_call) \
  do {                                  \
      int tmp = (result_or_call); \
      if (tmp != 0) _dbus_warn_check_failed("pthread function %s failed with %d %s in %s", func_name, tmp, strerror(tmp), _DBUS_FUNCTION_NAME); \
  } while(0);
#endif
DBusCMutex *_dbus_platform_cmutex_new(void) {
  DBusCMutex *pmutex;
  int result;
  pmutex = dbus_new(DBusCMutex, 1);
  if (pmutex == NULL) return NULL;
  result = pthread_mutex_init(&pmutex->lock, NULL);
  if (result == ENOMEM || result == EAGAIN) {
      dbus_free(pmutex);
      return NULL;
  } else { PTHREAD_CHECK("pthread_mutex_init", result); }
  return pmutex;
}
DBusRMutex *_dbus_platform_rmutex_new(void) {
  DBusRMutex *pmutex;
  pthread_mutexattr_t mutexattr;
  int result;
  pmutex = dbus_new (DBusRMutex, 1);
  if (pmutex == NULL) return NULL;
  pthread_mutexattr_init(&mutexattr);
  pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
  result = pthread_mutex_init(&pmutex->lock, &mutexattr);
  pthread_mutexattr_destroy(&mutexattr);
  if (result == ENOMEM || result == EAGAIN) {
      dbus_free(pmutex);
      return NULL;
  } else { PTHREAD_CHECK("pthread_mutex_init", result); }
  return pmutex;
}
void _dbus_platform_cmutex_free(DBusCMutex *mutex) {
  PTHREAD_CHECK("pthread_mutex_destroy", pthread_mutex_destroy(&mutex->lock));
  dbus_free(mutex);
}
void _dbus_platform_rmutex_free(DBusRMutex *mutex) {
  PTHREAD_CHECK("pthread_mutex_destroy", pthread_mutex_destroy(&mutex->lock));
  dbus_free(mutex);
}
void _dbus_platform_cmutex_lock(DBusCMutex *mutex) {
  PTHREAD_CHECK("pthread_mutex_lock", pthread_mutex_lock(&mutex->lock));
}
void _dbus_platform_rmutex_lock(DBusRMutex *mutex) {
  PTHREAD_CHECK("pthread_mutex_lock", pthread_mutex_lock(&mutex->lock));
}
void _dbus_platform_cmutex_unlock(DBusCMutex *mutex) {
  PTHREAD_CHECK("pthread_mutex_unlock", pthread_mutex_unlock(&mutex->lock));
}
void _dbus_platform_rmutex_unlock(DBusRMutex *mutex) {
  PTHREAD_CHECK("pthread_mutex_unlock", pthread_mutex_unlock(&mutex->lock));
}
DBusCondVar *_dbus_platform_condvar_new(void) {
  DBusCondVar *pcond;
  pthread_condattr_t attr;
  int result;
  pcond = dbus_new(DBusCondVar, 1);
  if (pcond == NULL) return NULL;
  pthread_condattr_init(&attr);
#ifdef HAVE_MONOTONIC_CLOCK
  if (have_monotonic_clock) pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
#endif
  result = pthread_cond_init(&pcond->cond, &attr);
  pthread_condattr_destroy(&attr);
  if (result == EAGAIN || result == ENOMEM) {
      dbus_free(pcond);
      return NULL;
  } else { PTHREAD_CHECK("pthread_cond_init", result); }
  return pcond;
}
void _dbus_platform_condvar_free(DBusCondVar *cond) {
  PTHREAD_CHECK("pthread_cond_destroy", pthread_cond_destroy(&cond->cond));
  dbus_free(cond);
}
void _dbus_platform_condvar_wait(DBusCondVar *cond, DBusCMutex *mutex) {
  PTHREAD_CHECK("pthread_cond_wait", pthread_cond_wait(&cond->cond, &mutex->lock));
}
dbus_bool_t _dbus_platform_condvar_wait_timeout(DBusCondVar *cond, DBusCMutex *mutex, int timeout_milliseconds) {
  struct timeval time_now;
  struct timespec end_time;
  int result;
#ifdef HAVE_MONOTONIC_CLOCK
  if (have_monotonic_clock) {
      struct timespec monotonic_timer;
      clock_gettime(CLOCK_MONOTONIC, &monotonic_timer);
      time_now.tv_sec = monotonic_timer.tv_sec;
      time_now.tv_usec = monotonic_timer.tv_nsec / 1000;
  }  else
#endif
  gettimeofday(&time_now, NULL);
  end_time.tv_sec = time_now.tv_sec + timeout_milliseconds / 1000;
  end_time.tv_nsec = (time_now.tv_usec + (timeout_milliseconds % 1000) * 1000) * 1000;
  if (end_time.tv_nsec > 1000*1000*1000) {
      end_time.tv_sec += 1;
      end_time.tv_nsec -= 1000*1000*1000;
  }
  result = pthread_cond_timedwait(&cond->cond, &mutex->lock, &end_time);
  if (result != ETIMEDOUT) { PTHREAD_CHECK("pthread_cond_timedwait", result); }
  return result != ETIMEDOUT;
}
void _dbus_platform_condvar_wake_one(DBusCondVar *cond) {
  PTHREAD_CHECK("pthread_cond_signal", pthread_cond_signal(&cond->cond));
}
static void check_monotonic_clock(void) {
#ifdef HAVE_MONOTONIC_CLOCK
  struct timespec dummy;
  if (clock_getres(CLOCK_MONOTONIC, &dummy) == 0) have_monotonic_clock = TRUE;
#endif
}
dbus_bool_t _dbus_threads_init_platform_specific(void) {
  check_monotonic_clock();
  (void)_dbus_check_setuid();
  return TRUE;
}
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
void _dbus_threads_lock_platform_specific(void) {
  pthread_mutex_lock(&init_mutex);
}
void _dbus_threads_unlock_platform_specific(void) {
  pthread_mutex_unlock(&init_mutex);
}
#ifdef DBUS_ENABLE_VERBOSE_MODE
void _dbus_print_thread(void) {
#ifdef __linux__
  fprintf(stderr, "%lu: 0x%lx: ", _dbus_pid_for_log(), (unsigned long)pthread_self());
#else
  fprintf(stderr, "%lu: ", _dbus_pid_for_log());
#endif
}
#endif