#include <stdio.h>
#include "config.h"
#include "dbus-nonce.h"
#include "dbus-internals.h"
#include "dbus-protocol.h"
#include "dbus-sysdeps.h"

struct DBusNonceFile {
  DBusString path;
  DBusString dir;
};
static dbus_bool_t do_check_nonce(DBusSocket fd, const DBusString *nonce, DBusError *error) {
  DBusString buffer;
  DBusString p;
  size_t nleft;
  dbus_bool_t result;
  int n;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  nleft = 16;
  _dbus_string_init_const(&buffer, "");
  _dbus_string_init_const(&p, "");
  if (!_dbus_string_init(&buffer) || !_dbus_string_init(&p) ) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      _dbus_string_free(&p);
      _dbus_string_free(&buffer);
      return FALSE;
  }
  while(nleft) {
      int saved_errno;
      n = _dbus_read_socket(fd, &p, nleft);
      saved_errno = _dbus_save_socket_errno();
      if (n == -1 && _dbus_get_is_errno_eintr(saved_errno));
      else if (n == -1 && _dbus_get_is_errno_eagain_or_ewouldblock(saved_errno)) _dbus_sleep_milliseconds(100);
      else if (n==-1) {
          dbus_set_error(error, DBUS_ERROR_IO_ERROR, "Could not read nonce from socket (fd=%" DBUS_SOCKET_FORMAT ")", _dbus_socket_printable(fd));
          _dbus_string_free(&p);
          _dbus_string_free(&buffer);
          return FALSE;
      } else if (!n) {
          _dbus_string_free(&p);
          _dbus_string_free(&buffer);
          dbus_set_error(error, DBUS_ERROR_IO_ERROR, "Could not read nonce from socket (fd=%" DBUS_SOCKET_FORMAT ")", _dbus_socket_printable(fd));
          return FALSE;
      } else {
          if (!_dbus_string_append_len(&buffer, _dbus_string_get_const_data(&p), n)) {
              dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
              _dbus_string_free(&p);
              _dbus_string_free(&buffer);
              return FALSE;
          }
          nleft -= n;
      }
  }
  result =  _dbus_string_equal_len(&buffer, nonce, 16);
  if (!result)
      dbus_set_error(error, DBUS_ERROR_ACCESS_DENIED, "Nonces do not match, access denied (fd=%" DBUS_SOCKET_FORMAT ")", _dbus_socket_printable(fd));
  _dbus_string_free(&p);
  _dbus_string_free(&buffer);
  return result;
}
dbus_bool_t _dbus_read_nonce(const DBusString *fname, DBusString *nonce, DBusError* error) {
  FILE *fp;
  char buffer[17];
  size_t nread;
  buffer[sizeof buffer - 1] = '\0';
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _dbus_verbose("reading nonce from file: %s\n", _dbus_string_get_const_data(fname));
  fp = fopen(_dbus_string_get_const_data(fname), "rb");
  if (!fp) {
      dbus_set_error(error, _dbus_error_from_system_errno(),"Failed to open %s for read: %s", _dbus_string_get_const_data(fname),
		             _dbus_strerror_from_errno());
      return FALSE;
  }
  nread = fread(buffer, 1, sizeof buffer - 1, fp);
  fclose(fp);
  if (!nread) {
      dbus_set_error(error, DBUS_ERROR_FILE_NOT_FOUND, "Could not read nonce from file %s", _dbus_string_get_const_data(fname));
      return FALSE;
  }
  if (!_dbus_string_append_len(nonce, buffer, sizeof buffer - 1 )) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return FALSE;
  }
  return TRUE;
}
DBusSocket _dbus_accept_with_noncefile(DBusSocket listen_fd, const DBusNonceFile *noncefile) {
  DBusSocket fd = _dbus_socket_get_invalid();
  DBusString nonce;
  _dbus_assert(noncefile != NULL);
  _dbus_string_init_const(&nonce, "");
  if (!_dbus_string_init(&nonce)) goto out;
  if (_dbus_read_nonce(_dbus_noncefile_get_path(noncefile), &nonce, NULL) != TRUE) goto out;
  fd = _dbus_accept(listen_fd);
  if (!_dbus_socket_is_valid(fd)) goto out;
  if (do_check_nonce(fd, &nonce, NULL) != TRUE) {
    _dbus_verbose("nonce check failed. Closing socket.\n");
    _dbus_close_socket(fd, NULL);
    _dbus_socket_invalidate(&fd);
    goto out;
  }
out:
  _dbus_string_free(&nonce);
  return fd;
}
static dbus_bool_t generate_and_write_nonce(const DBusString *filename, DBusError *error) {
  DBusString nonce;
  dbus_bool_t ret;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (!_dbus_string_init(&nonce)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return FALSE;
  }
  if (!_dbus_generate_random_bytes(&nonce, 16, error)) {
      _dbus_string_free(&nonce);
      return FALSE;
  }
  ret = _dbus_string_save_to_file(&nonce, filename, FALSE, error);
  _dbus_string_free(&nonce);
  return ret;
}
dbus_bool_t _dbus_send_nonce(DBusSocket fd, const DBusString *noncefile, DBusError *error) {
  dbus_bool_t read_result;
  int send_result;
  DBusString nonce;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (_dbus_string_get_length(noncefile) == 0) return FALSE;
  if (!_dbus_string_init(&nonce)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return FALSE;
  }
  read_result = _dbus_read_nonce(noncefile, &nonce, error);
  if (!read_result) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      _dbus_string_free(&nonce);
      return FALSE;
  }
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  send_result = _dbus_write_socket(fd, &nonce, 0, _dbus_string_get_length(&nonce));
  _dbus_string_free(&nonce);
  if (send_result == -1) {
      dbus_set_error(error, _dbus_error_from_system_errno(),"Failed to send nonce (fd=%" DBUS_SOCKET_FORMAT "): %s", _dbus_socket_printable(fd),
                     _dbus_strerror_from_errno());
      return FALSE;
  }
  return TRUE;
}
static dbus_bool_t do_noncefile_create(DBusNonceFile **noncefile_out, DBusError *error, dbus_bool_t use_subdir) {
    DBusNonceFile *noncefile = NULL;
    DBusString randomStr;
    const char *tmp;
    _DBUS_ASSERT_ERROR_IS_CLEAR(error);
    _dbus_assert(noncefile_out != NULL);
    _dbus_assert(*noncefile_out == NULL);
    noncefile = dbus_new0(DBusNonceFile, 1);
    if (noncefile == NULL) {
        dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
        return FALSE;
    }
    _dbus_string_init_const(&randomStr, "");
    _dbus_string_init_const(&noncefile->dir, "");
    _dbus_string_init_const(&noncefile->path, "");
    if (!_dbus_string_init(&randomStr)) {
        dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
        goto on_error;
    }
    if (!_dbus_generate_random_ascii(&randomStr, 8, error)) goto on_error;
    tmp = _dbus_get_tmpdir();
    if (!_dbus_string_init(&noncefile->dir) || tmp == NULL || !_dbus_string_append(&noncefile->dir, tmp)) {
        dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
        goto on_error;
    }
    if (use_subdir) {
        if (!_dbus_string_append(&noncefile->dir, "/dbus_nonce-") || !_dbus_string_append(&noncefile->dir, _dbus_string_get_const_data(&randomStr))) {
            dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
            goto on_error;
        }
        if (!_dbus_string_init(&noncefile->path) || !_dbus_string_copy(&noncefile->dir, 0, &noncefile->path, 0) ||
            !_dbus_string_append(&noncefile->path, "/nonce")) {
            dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
            goto on_error;
        }
        if (!_dbus_create_directory(&noncefile->dir, error)) {
            _DBUS_ASSERT_ERROR_IS_SET(error);
            goto on_error;
        }
        _DBUS_ASSERT_ERROR_IS_CLEAR(error);
    } else {
        if (!_dbus_string_init(&noncefile->path) || !_dbus_string_copy(&noncefile->dir, 0, &noncefile->path, 0) ||
            !_dbus_string_append(&noncefile->path, "/dbus_nonce-") || !_dbus_string_append(&noncefile->path, _dbus_string_get_const_data(&randomStr))) {
            dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
            goto on_error;
        }
    }
    if (!generate_and_write_nonce(&noncefile->path, error)) {
        _DBUS_ASSERT_ERROR_IS_SET(error);
        if (use_subdir) _dbus_delete_directory(&noncefile->dir, NULL);
        goto on_error;
    }
    _DBUS_ASSERT_ERROR_IS_CLEAR(error);
    *noncefile_out = noncefile;
    _dbus_string_free(&randomStr);
    return TRUE;
on_error:
    if (use_subdir) _dbus_delete_directory(&noncefile->dir, NULL);
    _dbus_string_free(&noncefile->dir);
    _dbus_string_free(&noncefile->path);
    dbus_free(noncefile);
    _dbus_string_free(&randomStr);
    return FALSE;
}
#ifdef DBUS_WIN
dbus_bool_t _dbus_noncefile_create(DBusNonceFile **noncefile_out, DBusError *error) {
    return do_noncefile_create(noncefile_out, error, /*use_subdir=*/FALSE);
}
dbus_bool_t _dbus_noncefile_delete(DBusNonceFile **noncefile_location, DBusError *error) {
    DBusNonceFile *noncefile;
    _DBUS_ASSERT_ERROR_IS_CLEAR(error);
    _dbus_assert(noncefile_location != NULL);
    noncefile = *noncefile_location;
    *noncefile_location = NULL;
    if (noncefile == NULL) return TRUE;
    _dbus_delete_file(&noncefile->path, error);
    _dbus_string_free(&noncefile->dir);
    _dbus_string_free(&noncefile->path);
    dbus_free(noncefile);
    return TRUE;
}
#else
dbus_bool_t _dbus_noncefile_create(DBusNonceFile **noncefile_out, DBusError *error) {
    return do_noncefile_create(noncefile_out, error, TRUE);
}
dbus_bool_t _dbus_noncefile_delete(DBusNonceFile **noncefile_location, DBusError *error) {
    DBusNonceFile *noncefile;
    _DBUS_ASSERT_ERROR_IS_CLEAR(error);
    _dbus_assert(noncefile_location != NULL);
    noncefile = *noncefile_location;
    *noncefile_location = NULL;
    if (noncefile == NULL) return TRUE;
    _dbus_delete_directory(&noncefile->dir, error);
    _dbus_string_free(&noncefile->dir);
    _dbus_string_free(&noncefile->path);
    dbus_free(noncefile);
    return TRUE;
}
#endif
const DBusString* _dbus_noncefile_get_path(const DBusNonceFile *noncefile) {
    _dbus_assert(noncefile);
    return &noncefile->path;
}
dbus_bool_t _dbus_noncefile_check_nonce(DBusSocket fd, const DBusNonceFile *noncefile, DBusError* error) {
    return do_check_nonce(fd, _dbus_noncefile_get_path(noncefile), error);
}