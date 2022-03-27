#include <errno.h>
#include "config.h"
#include "gioerror.h"

GQuark g_io_error_quark(void) {
  return g_quark_from_static_string ("g-io-error-quark");
}
GIOErrorEnum g_io_error_from_errno(gint err_no) {
  switch(err_no) {
  #ifdef EEXIST
      case EEXIST: return G_IO_ERROR_EXISTS;
  #endif
  #ifdef EISDIR
      case EISDIR: return G_IO_ERROR_IS_DIRECTORY;
  #endif
  #ifdef EACCES
      case EACCES: return G_IO_ERROR_PERMISSION_DENIED;
  #endif
  #ifdef ENAMETOOLONG
      case ENAMETOOLONG: return G_IO_ERROR_FILENAME_TOO_LONG;
  #endif
  #ifdef ENOENT
      case ENOENT: return G_IO_ERROR_NOT_FOUND;
  #endif
  #ifdef ENOTDIR
      case ENOTDIR: return G_IO_ERROR_NOT_DIRECTORY;
  #endif
  #ifdef EROFS
      case EROFS: return G_IO_ERROR_READ_ONLY;
  #endif
  #ifdef ELOOP
      case ELOOP: return G_IO_ERROR_TOO_MANY_LINKS;
  #endif
  #ifdef ENOSPC
      case ENOSPC: return G_IO_ERROR_NO_SPACE;
  #endif
  #ifdef ENOMEM
      case ENOMEM: return G_IO_ERROR_NO_SPACE;
  #endif
  #ifdef EINVAL
      case EINVAL: return G_IO_ERROR_INVALID_ARGUMENT;
  #endif
  #ifdef EPERM
      case EPERM: return G_IO_ERROR_PERMISSION_DENIED;
  #endif
  #ifdef ECANCELED
      case ECANCELED: return G_IO_ERROR_CANCELLED;
  #endif
  #if defined(ENOTEMPTY) && (!defined (EEXIST) || (ENOTEMPTY != EEXIST))
      case ENOTEMPTY: return G_IO_ERROR_NOT_EMPTY;
  #endif
  #ifdef ENOTSUP
      case ENOTSUP: return G_IO_ERROR_NOT_SUPPORTED;
  #endif
  #ifdef ETIMEDOUT
      case ETIMEDOUT: return G_IO_ERROR_TIMED_OUT;
  #endif
  #ifdef EBUSY
      case EBUSY: return G_IO_ERROR_BUSY;
  #endif
  #if defined(EWOULDBLOCK) && defined(EAGAIN) && EWOULDBLOCK == EAGAIN
      case EAGAIN: return G_IO_ERROR_WOULD_BLOCK;
  #else
  #ifdef EAGAIN
      case EAGAIN: return G_IO_ERROR_WOULD_BLOCK;
  #endif
  #ifdef EWOULDBLOCK
      case EWOULDBLOCK: return G_IO_ERROR_WOULD_BLOCK;
  #endif
  #endif
  #ifdef EMFILE
      case EMFILE: return G_IO_ERROR_TOO_MANY_OPEN_FILES;
  #endif
  #ifdef EADDRINUSE
      case EADDRINUSE: return G_IO_ERROR_ADDRESS_IN_USE;
  #endif
  #ifdef EHOSTUNREACH
      case EHOSTUNREACH: return G_IO_ERROR_HOST_UNREACHABLE;
  #endif
  #ifdef ENETUNREACH
      case ENETUNREACH: return G_IO_ERROR_NETWORK_UNREACHABLE;
  #endif
  #ifdef ECONNREFUSED
      case ECONNREFUSED: return G_IO_ERROR_CONNECTION_REFUSED;
  #endif
      default: return G_IO_ERROR_FAILED;
  }
}
#ifdef G_OS_WIN32
GIOErrorEnum g_io_error_from_win32_error(gint error_code) {
  switch(error_code) {
      default: return G_IO_ERROR_FAILED;
  }
}
#endif