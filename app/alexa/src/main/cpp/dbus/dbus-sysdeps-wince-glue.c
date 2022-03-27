#include "../gio/gregistrysettingsbackend.h"
#include "config.h"
#include "dbus-internals.h"
#include "dbus-sysdeps.h"
#include "dbus-sysdeps-win.h"

#ifndef CSIDL_PERSONAL
#define CSIDL_PERSONAL 5
#endif
static char *get_verbose_setting() {
  /*const wchar_t dir[] = L"Software\\freedesktop\\DBus";
  const wchar_t name[] = L"Verbose";
  HKEY root_key;
  HKEY key_handle;
  DWORD nbytes;
  DWORD n1;
  DWORD type;
  wchar_t *result_w = NULL;
  char *result;
  int len;
  root_key = HKEY_LOCAL_MACHINE;
  if (RegOpenKeyExW(root_key, dir, 0, KEY_READ, &key_handle)) return NULL;
  nbytes = 1;
  if (RegQueryValueExW(key_handle, name, 0, NULL, NULL, &nbytes)) {
      RegCloseKey(key_handle);
      return NULL;
  }
  n1 = ((nbytes + sizeof(wchar_t) - 1) / sizeof(wchar_t)) + 1;
  result_w = malloc(n1 * sizeof(wchar_t));
  if (!result_w) {
      RegCloseKey(key_handle);
      return NULL;
  }
  if (RegQueryValueExW(key_handle, name, 0, &type, (LPBYTE) result_w, &nbytes)) {
      RegCloseKey(key_handle);
      free(result_w);
      return NULL;
  }
  RegCloseKey(key_handle);
  result_w[n1 - 1] = 0;
  if (type != REG_SZ) {
      free(result_w);
      return NULL;
  }
  len = WideCharToMultiByte(CP_UTF8, 0, result_w, -1, NULL, 0, NULL, NULL);
  if (len < 0) {
      free(result_w);
      return NULL;
  }
  result = malloc(len + 1);
  if (!result) {
      free(result_w);
      return NULL;
  }
  len = WideCharToMultiByte(CP_UTF8, 0, result_w, -1, result, len, NULL, NULL);
  free(result_w);
  if (len < 0) {
      free(result);
      return NULL;
  }
  return result;*/
  return NULL;
}
static char *read_w32_registry_string(const char *root, const char *dir, const char *name) {
  /*HKEY root_key, key_handle;
  DWORD n1, nbytes, type;
  char *result = NULL;
  if (!root) root_key = HKEY_CURRENT_USER;
  else if (!strcmp(root, "HKEY_CLASSES_ROOT")) root_key = HKEY_CLASSES_ROOT;
  else if (!strcmp(root, "HKEY_CURRENT_USER")) root_key = HKEY_CURRENT_USER;
  else if (!strcmp(root, "HKEY_LOCAL_MACHINE")) root_key = HKEY_LOCAL_MACHINE;
  else if (!strcmp(root, "HKEY_USERS")) root_key = HKEY_USERS;
  else return NULL;
  if (RegOpenKeyExA(root_key, dir, 0, KEY_READ, &key_handle)) {
      if (root) return NULL;
      if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, dir, 0, KEY_READ, &key_handle)) return NULL;
  }
  nbytes = 1;
  if (RegQueryValueExA(key_handle, name, 0, NULL, NULL, &nbytes)) {
      if (root) goto out;
      RegCloseKey(key_handle);
      if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, dir, 0, KEY_READ, &key_handle)) return NULL;
      if (RegQueryValueExA(key_handle, name, 0, NULL, NULL, &nbytes)) goto out;
  }
  n1 = nbytes + 1;
  result = malloc (n1);
  if (!result) goto out;
  if (RegQueryValueExA(key_handle, name, 0, &type, result, &n1)) {
      free(result);
      result = NULL;
      goto out;
    }
  result[nbytes] = 0;
out:
  RegCloseKey(key_handle);
  return result;*/
  return NULL;
}
static char *find_inst_dir() {
  return read_w32_registry_string("HKEY_LOCAL_MACHINE","Software\\freedesktop\\DBus","Install Directory");
}
static char *find_env_in_registry(const char *name) {
  return read_w32_registry_string ("HKEY_LOCAL_MACHINE","Software\\freedesktop\\DBus", name);
}
static char *find_program_in_inst_dir(const char *name) {
  char *result = NULL;
  char *tmp;
  tmp = find_inst_dir();
  if (!tmp) return NULL;
  result = malloc(strlen(tmp) + 5 + strlen(name) + 1);
  if (!result) {
      free(tmp);
      return NULL;
  }
  strcpy(stpcpy(stpcpy(result, tmp), "\\bin\\"), name);
  free(tmp);
  return result;
}
static char *find_inst_subdir(const char *name) {
  char *result = NULL;
  char *tmp;
  tmp = find_inst_dir();
  if (!tmp) return NULL;
  result = malloc(strlen(tmp) + 1 + strlen(name) + 1);
  if (!result) {
      free(tmp);
      return NULL;
  }
  strcpy(stpcpy(stpcpy(result, tmp), "\\"), name);
  free(tmp);
  return result;
}
static char *find_my_documents_folder () {
  /*char dir[MAX_PATH + 1];
  char *result;
  dir[0] = '\0';
  SHGetSpecialFolderPathA(0, dir, CSIDL_PERSONAL, 0);
  if (dir[0] == '\0') return NULL;
  result = malloc(strlen(dir) + 1);
  if (!result) return NULL;
  strcpy(result, dir);
  return result;*/
  return NULL;
}
#define MAX_ENV 30
char *environ[MAX_ENV + 1];
char *getenv(const char *name) {
  static char *past_result;
  char **envp;
  int idx;
  if (past_result) {
      free(past_result);
      past_result = NULL;
  }
  if (!strcmp(name, "DBUS_VERBOSE")) return past_result = get_verbose_setting();
  else if (!strcmp(name, "HOMEPATH")) return past_result = find_my_documents_folder();
  else if (!strcmp(name, "DBUS_DATADIR")) return past_result = find_inst_subdir("share");
  for (envp = environ; *envp != 0; envp++) {
      const char *varp = name;
      char *ep = *envp;
      int same_name = 0;
      while(*varp == *ep && *varp != '\0') {
          ++ep;
          ++varp;
	  };
      if (*varp == '\0' && *ep == '=') return ep + 1;
  }
  return NULL;
}
int putenv(char *str) {
  /*char **envp;
  int idx;
  for (envp = environ; *envp != 0; envp++) {
      char *varp = str;
      char *ep = *envp;
      int same_name = 0;
      while(*varp == *ep && *varp != '\0') {
	  if (*varp == '=') same_name = 1;
          ++ep;
          ++varp;
	  };
      if (*varp == *ep && *varp == '\0') return 0;
      if (same_name) {
          *envp = str;
          return 0;
	  }
  }
  idx = envp - environ;
  if (idx > MAX_ENV) {
      _dbus_win_set_errno(ENOMEM);
      return -1;
  }
  environ[idx] = str;*/
  return 0;
}
clock_t clock(void) {
  //return GetTickCount();
  return 0;
}
void abort(void) {
  //exit(3);
}
void GetSystemTimeAsFileTime(LPFILETIME ftp) {
  /*SYSTEMTIME st;
  GetSystemTime(&st);
  SystemTimeToFileTime(&st, ftp);*/
}
unsigned char* _mbsrchr(const unsigned char* str, unsigned int ch) {
  return strrchr(str, ch);
}
HANDLE OpenFileMappingA(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName) {
  /*DWORD flProtect = 0;
  HANDLE hMapping;
  if (dwDesiredAccess & FILE_MAP_READ) flProtect |= PAGE_READONLY;
  if (dwDesiredAccess & FILE_MAP_WRITE) flProtect |= PAGE_READWRITE;
  SetLastError (0);
  hMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, flProtect, 0, 0, lpName);
  if (hMapping != INVALID_HANDLE_VALUE) {
      if (GetLastError() != ERROR_ALREADY_EXISTS) {
          CloseHandle(hMapping);
          hMapping = INVALID_HANDLE_VALUE;
      }
  }
  return hMapping;*/
  return NULL;
}
BOOL MoveFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, DWORD dwFlags) {
  /*_dbus_assert(dwFlags == MOVEFILE_REPLACE_EXISTING);
  if (_dbus_file_exists(lpNewFileName)) {
      BOOL result = DeleteFileA(lpNewFileName);
      if (result == 0) return FALSE;
  }
  return MoveFileA(lpExistingFileName, lpNewFileName);*/
  return FALSE;
}
BOOL SetHandleInformation(HANDLE hObject, DWORD dwMask, DWORD dwFlags) {
  _dbus_assert(dwMask == (HANDLE_FLAG_INHERIT | HANDLE_FLAG_PROTECT_FROM_CLOSE));
  _dbus_assert(dwFlags == 0);
  return 1;
}
DWORD SearchPathA(LPCSTR lpPath, LPCSTR lpFileName, LPCSTR lpExtension, DWORD nBufferLength, LPSTR lpBuffer, LPSTR* lpFilePart) {
  /*char *filename;
  char *filepart;
  int filename_len;
  _dbus_assert(lpPath == NULL);
  _dbus_assert(lpExtension == NULL);
  filename = find_program_in_inst_dir(lpFileName);
  if (!filename) {
      SetLastError(ERROR_FILE_NOT_FOUND);
      return 0;
  }
  filename_len = strlen (filename) + 1;
  if (filename_len > nBufferLength) {
      free(filename);
      return filename_len;
  }
  strcpy(lpBuffer, filename);
  free(filename);
  filepart = _mbsrchr(lpBuffer, '\\');
  if (!filepart) filepart = lpBuffer;
  *lpFilePart = filepart;
  return filename_len - 1;*/
  return 0;
}
dbus_bool_t _dbus_getsid(char **sid, dbus_pid_t process_id) {
  /*tatic const char asid[] = "S-1-5-21-515967899-920026266-1708537768-1000";
  char *buf = LocalAlloc(LMEM_FIXED, sizeof (asid));
  if (!buf) {
      _dbus_win_warn_win_error("LocalAlloc failed", GetLastError ());
      return FALSE;
  }
  memcpy(buf, asid, sizeof(asid));
  *sid = buf;*/
  return TRUE;
}
BOOL LookupAccountNameW(LPCWSTR lpSystemName, LPCWSTR lpAccountName, PSID Sid, PDWORD cbSid, LPWSTR ReferencedDomainName, PDWORD cchReferencedDomainName,
                        PSID_NAME_USE peUse) {
  return FALSE;
}
BOOL IsValidSid(PSID psid) {
  return FALSE;
}
HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwSharedMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	               DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
  /*wchar_t *filename;
  HANDLE result;
  int err;
  filename = _dbus_win_utf8_to_utf16 (lpFileName, NULL);
  if (!filename) return INVALID_HANDLE_VALUE;
  result = CreateFileW (filename, dwDesiredAccess, dwSharedMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
  err = GetLastError ();
  dbus_free (filename);
  SetLastError (err);
  return result;
}
BOOL DeleteFileA(LPCSTR lpFileName) {
  wchar_t *filename;
  BOOL result;
  int err;
  filename = _dbus_win_utf8_to_utf16 (lpFileName, NULL);
  if (!filename) return FALSE;
  result = DeleteFileW (filename);
  err = GetLastError ();
  dbus_free (filename);
  SetLastError (err);
  return result;*/
  return NULL;
}
BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) {
  /*wchar_t *existing_filename;
  wchar_t *new_filename;
  BOOL result;
  int err;
  existing_filename = _dbus_win_utf8_to_utf16(lpExistingFileName, NULL);
  if (! existing_filename) return FALSE;
  new_filename = _dbus_win_utf8_to_utf16(lpNewFileName, NULL);
  if (! new_filename) {
      dbus_free(existing_filename);
      return FALSE;
  }
  result = MoveFileW(existing_filename, new_filename);
  err = GetLastError();
  dbus_free(existing_filename);
  dbus_free(new_filename);
  SetLastError(err);
  return result;*/
  return FALSE;
}
DWORD GetFileAttributesA(LPCSTR lpFileName) {
  /*wchar_t *filename;
  DWORD result;
  int err;
  filename = _dbus_win_utf8_to_utf16(lpFileName, NULL);
  if (!filename) return INVALID_FILE_ATTRIBUTES;
  result = GetFileAttributesW(filename);
  err = GetLastError();
  dbus_free(filename);
  SetLastError(err);
  return result;*/
  return 0;
}
BOOL GetFileAttributesExA(LPCSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, PVOID lpFileInformation) {
  /*wchar_t *filename;
  DWORD result;
  int err;
  filename = _dbus_win_utf8_to_utf16(lpFileName, NULL);
  if (!filename) return INVALID_FILE_ATTRIBUTES;
  result = GetFileAttributesExW(filename, fInfoLevelId, lpFileInformation);
  err = GetLastError();
  dbus_free(filename);
  SetLastError(err);
  return result;*/
  return FALSE;
}
HANDLE CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName) {
  /*wchar_t *name;
  HANDLE result;
  int err;
  if (lpName) {
      name = _dbus_win_utf8_to_utf16(lpName, NULL);
      if (!name) return INVALID_HANDLE_VALUE;
  } else name = NULL;
  result = CreateFileMappingW(hFile, lpAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, name);
  err = GetLastError();
  dbus_free(name);
  SetLastError(err);
  return result;*/
  return NULL;
}
BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
  /*wchar_t *pathname;
  BOOL result;
  int err;
  pathname = _dbus_win_utf8_to_utf16(lpPathName, NULL);
  if (!pathname) return FALSE;
  result = CreateDirectoryW(pathname, lpSecurityAttributes);
  err = GetLastError();
  dbus_free(pathname);
  SetLastError(err);
  return result;*/
  return FALSE;
}
BOOL RemoveDirectoryA(LPCSTR lpPathName) {
  /*wchar_t *pathname;
  BOOL result;
  int err;
  pathname = _dbus_win_utf8_to_utf16(lpPathName, NULL);
  if (!pathname) return FALSE;
  result = RemoveDirectoryW(pathname);
  err = GetLastError();
  dbus_free(pathname);
  SetLastError(err);
  return result;*/
  return FALSE;
}
static BOOL convert_find_data(LPWIN32_FIND_DATAW fdw, LPWIN32_FIND_DATAA fda) {
  char *filename;
  int len;
  fda->dwFileAttributes = fdw->dwFileAttributes;
  fda->ftCreationTime = fdw->ftCreationTime;
  fda->ftLastAccessTime = fdw->ftLastAccessTime;
  fda->ftLastWriteTime = fdw->ftLastWriteTime;
  fda->nFileSizeHigh = fdw->nFileSizeHigh;
  fda->nFileSizeLow = fdw->nFileSizeLow;
  filename = _dbus_win_utf16_to_utf8(fdw->cFileName, NULL);
  if (!filename) return FALSE;
  len = sizeof(fda->cFileName);
  strncpy(fda->cFileName, filename, len);
  fda->cFileName[len - 1] = '\0';
  return TRUE;
}
HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) {
  /*wchar_t *pathname;
  WIN32_FIND_DATAW find_file_data;
  HANDLE result;
  int err;
  pathname = _dbus_win_utf8_to_utf16(lpFileName, NULL);
  if (!pathname) return INVALID_HANDLE_VALUE;
  result = FindFirstFileW(pathname, &find_file_data);
  if (result != INVALID_HANDLE_VALUE) {
      BOOL res = convert_find_data(&find_file_data, lpFindFileData);
      if (! res) {
          err = GetLastError();
          FindClose(result);
          SetLastError(err);
          result = INVALID_HANDLE_VALUE;
      }
  }
  err = GetLastError();
  dbus_free(pathname);
  SetLastError(err);
  return result;*/
  return NULL;
}
BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) {
  /*WIN32_FIND_DATAW find_file_data;
  BOOL result;
  int err;
  result = FindNextFileW(hFindFile, &find_file_data);
  if (result) result = convert_find_data(&find_file_data, lpFindFileData);
  return result;*/
  return FALSE;
}
HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) {
  /*wchar_t *name;
  HANDLE result;
  int err;
  if (lpName) {
      name = _dbus_win_utf8_to_utf16(lpName, NULL);
      if (!name) return INVALID_HANDLE_VALUE;
  } else name = NULL;
  result = CreateMutexW(lpMutexAttributes, bInitialOwner, name);
  err = GetLastError();
  dbus_free(name);
  SetLastError(err);
  return result;*/
  return NULL;
}
BOOL CreateProcessA (LPCSTR pszImageName, LPSTR pszCmdLine, LPSECURITY_ATTRIBUTES psaProcess, LPSECURITY_ATTRIBUTES psaThread, BOOL fInheritHandles,
                    DWORD fdwCreate, PVOID pvEnvironment, LPCSTR pszCurDir, LPSTARTUPINFOA psiStartInfo, LPPROCESS_INFORMATION pProcInfo) {
  /*wchar_t *image_name = NULL;
  wchar_t *cmd_line = NULL;
  BOOL result;
  int err;
  _dbus_assert(psaProcess == NULL);
  _dbus_assert(psaThread == NULL);
  _dbus_assert(fInheritHandles == FALSE);
  _dbus_assert(pvEnvironment == NULL);
  _dbus_assert(pszCurDir == NULL);
  if (pszImageName) {
      image_name = _dbus_win_utf8_to_utf16(pszImageName, NULL);
      if (!image_name) return 0;
  }
  if (pszCmdLine) {
      cmd_line = _dbus_win_utf8_to_utf16(pszCmdLine, NULL);
      if (!cmd_line) {
          if (image_name) dbus_free(image_name);
          return 0;
      }
  }
  result = CreateProcessW(image_name, cmd_line, NULL, NULL, FALSE, fdwCreate, NULL, NULL, NULL, pProcInfo);
  err = GetLastError();
  dbus_free(image_name);
  dbus_free(cmd_line);
  SetLastError(err);
  return result;*/
  return FALSE;
}
LONG RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) {
  /*wchar_t *subkey;
  LONG result;
  int err;
  if (lpSubKey) {
      subkey = _dbus_win_utf8_to_utf16(lpSubKey, NULL);
      if (!subkey) return 0;
  } else subkey = NULL;
  result = RegOpenKeyEx(hKey, subkey, ulOptions, samDesired, phkResult);
  err = GetLastError();
  dbus_free(subkey);
  SetLastError(err);
  return result;*/
  return 0;
}
LONG RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
  /*wchar_t *name;
  LONG err;
  BYTE *data;
  DWORD data_len;
  DWORD type;
  if (lpValueName) {
      name = _dbus_win_utf8_to_utf16(lpValueName, NULL);
      if (!name) return GetLastError();
  } else name = NULL;
  data_len = 0;
  err = RegQueryValueExW(hKey, name, lpReserved, lpType, NULL, &data_len);
  if (err || !lpcbData) {
      dbus_free(name);
      return err;
  }
  data = malloc(data_len + sizeof (wchar_t));
  if (!data) {
      dbus_free(name);
      return ERROR_NOT_ENOUGH_MEMORY;
  }
  err = RegQueryValueExW(hKey, name, lpReserved, &type, data, &data_len);
  if (lpType) *lpType = type;
  dbus_free (name);
  if (err) {
      free(data);
      return err;
  }
  if (type == REG_SZ) {
      char *data_c;
      int data_c_len;
      data[data_len] = '\0';
      data[data_len + 1] = '\0';
      data_c = _dbus_win_utf16_to_utf8((wchar_t*)data, NULL);
      if (!data_c) {
          free(data);
          return GetLastError();
      }
      data_c_len = strlen(data_c) + 1;
      _dbus_assert(data_c_len <= data_len + sizeof(wchar_t));
      memcpy(data, data_c, data_c_len);
      data_len = data_c_len;
      dbus_free(data_c);
  }
  if (lpData) {
      if (data_len > *lpcbData) err = ERROR_MORE_DATA;
      else memcpy(lpData, data, data_len);
  }
  free(data);
  *lpcbData = data_len;
  return err;*/
  return 0;
}
DWORD FormatMessageA(DWORD dwFlags, PCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments) {
  /*LPWSTR buffer_w = NULL;
  LPSTR buffer_c;
  DWORD len;
  char *buffer_new;
  DWORD buffer_new_len;
  BOOL buffer_w_free;
  len = FormatMessageW(dwFlags | FORMAT_MESSAGE_ALLOCATE_BUFFER, lpSource, dwMessageId, dwLanguageId, (LPWSTR)&buffer_w, 0, Arguments);
  if (len == 0) return 0;
  buffer_c = _dbus_win_utf16_to_utf8(buffer_w, NULL);
  if (! buffer_c) {
      LocalFree(buffer_w);
      return 0;
  }
  if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
      buffer_new = (char*)buffer_w;
      buffer_new_len = sizeof(wchar_t) * (len + 1);
      buffer_w_free = FALSE;
      memcpy(lpBuffer, &buffer_new, sizeof(buffer_new));
  } else {
      buffer_new = lpBuffer;
      buffer_new_len = nSize;
      buffer_w_free = TRUE;
  }
  strncpy (buffer_new, buffer_c, buffer_new_len);
  dbus_free(buffer_c);
  buffer_new[buffer_new_len - 1] = '\0';
  if (buffer_w_free) LocalFree(buffer_w);
  return strlen(buffer_new);*/
  return 0;
}
DWORD GetModuleFileNameA(HINSTANCE hModule, LPSTR lpFilename, DWORD nSize) {
  /*wchar_t *filename_w;
  char *filename_c;
  DWORD len;
  if (nSize == 0) {
      SetLastError(0);
      return 0;
  }
  filename_w = malloc(sizeof(wchar_t) * nSize);
  if (! filename_w) return 0;
  len = GetModuleFileNameW(hModule, filename_w, nSize);
  if (len == 0) {
      free(filename_w);
      return 0;
  }
  filename_w[nSize - 1] = '\0';
  filename_c = _dbus_win_utf16_to_utf8(filename_w, NULL);
  free(filename_w);
  if (!filename_c) return 0;
  strncpy(lpFilename, filename_c, nSize);
  dbus_free(filename_c);
  lpFilename[nSize - 1] = '\0';
  return strlen(lpFilename);*/
  return 0;
}
DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer) {
  /*wchar_t dummy[1];
  DWORD len;
  len = GetTempPathW(0, dummy);
  if (len == 0) return 0;
  _dbus_assert(len <= MAX_PATH);
  len++;
  {
      wchar_t *buffer_w;
      DWORD len_w;
      char *buffer_c;
      DWORD len_c;
      buffer_w = malloc(sizeof (wchar_t) * len);
      if (! buffer_w) return 0;
      len_w = GetTempPathW(len, buffer_w);
      if (len_w == 0 || len_w >= len) {
          free(buffer_w);
          return 0;
      }
      buffer_w[len_w] = '\0';
      buffer_c = _dbus_win_utf16_to_utf8(buffer_w, NULL);
      free(buffer_w);
      if (! buffer_c) return 0;
      len_c = strlen(buffer_c) + 1;
      if (len_c > nBufferLength) return len_c;
      strcpy(lpBuffer, buffer_c);
      dbus_free(buffer_c);
      return len_c - 1;
  }*/
  return 0;
}
BOOL SHGetSpecialFolderPathA(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) {
  /*wchar_t path[MAX_PATH];
  char *path_c;
  BOOL result;
  path[0] = (wchar_t)0;
  result = SHGetSpecialFolderPathW(hwndOwner, path, nFolder, fCreate);
  path[MAX_PATH - 1] = (wchar_t)0;
  path_c = _dbus_win_utf16_to_utf8(path, NULL);
  if (! path_c) return 0;
  strncpy(lpszPath, path_c, MAX_PATH);
  dbus_free(path_c);
  lpszPath[MAX_PATH - 1] = '\0';
  return result;*/
  return FALSE;
}
void OutputDebugStringA(LPCSTR lpOutputString) {
  /*wchar_t *str;
  HANDLE result;
  int err;
  str = _dbus_win_utf8_to_utf16(lpOutputString, NULL);
  if (!str) return;
  OutputDebugStringW(str);
  err = GetLastError();
  dbus_free(str);
  SetLastError(err);*/
}