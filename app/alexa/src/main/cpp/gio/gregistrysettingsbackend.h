#ifndef __G_REGISTRY_SETTINGS_BACKEND_H__
#define __G_REGISTRY_SETTINGS_BACKEND_H__

#include "../glib/gmacros.h"
#include "../glib/glib-object.h"
#include "../gobject/gtype.h"

#define REG_NONE (0)
#define REG_SZ (1)
#define REG_EXPAND_SZ (2)
#define REG_BINARY (3)
#define REG_DWORD (4)
#define REG_DWORD_LITTLE_ENDIAN (4)
#define REG_DWORD_BIG_ENDIAN (5)
#define REG_LINK (6)
#define REG_MULTI_SZ (7)
#define REG_RESOURCE_LIST (8)
#define REG_QWORD (11)
#define REG_QWORD_LITTLE_ENDIAN (11)
#define ERROR_SUCCESS (0)
#define ERROR_INVALID_FUNCTION (1)
#define ERROR_FILE_NOT_FOUND (2)
#define ERROR_PATH_NOT_FOUND (3)
#define ERROR_TOO_MAY_OPEN_FILES (4)
#define ERROR_ACCESS_DENIED (5)
#define ERROR_INVALID_HANDLE (6)
#define ERROR_ARENA_TRASHED (7)
#define ERROR_NOT_ENOUGH_MEMORY (8)
#define ERROR_INVALID_BLOCK (9)
#define ERROR_BAD_ENVIRONMENT (10)
#define ERROR_BAD_FORMAT (11)
#define ERROR_INVALID_ACCESS (12)
#define ERROR_INVALID_DATA (13)
#define ERROR_OUTOFMEMORY (14)
#define ERROR_INVALID_DRIVE (15)
#define ERROR_CURRENT_DIRECTORY (16)
#define ERROR_NOT_SAME_DEVICE (17)
#define ERROR_NO_MORE_FILES (18)
#define ERROR_WRITE_PROTECT (19)
#define ERROR_BAD_UNIT (20)
#define ERROR_NOT_READY (21)
#define ERROR_BAD_COMMAND (22)
#define ERROR_CRC (23)
#define ERROR_BAD_LENGTH (24)
#define ERROR_SEEK (25)
#define ERROR_NOT_DOS_DISK (26)
#define ERROR_SECTOR_NOT_FOUND (27)
#define ERROR_OUT_OF_PAPER (28)
#define ERROR_WRITE_FAULT (29)
#define ERROR_READ_FAULT (30)
#define ERROR_GEN_FAILURE (31)
#define ERROR_SHARING_VIOLATION (32)
#define ERROR_LOCK_VIOLATION (33)
#define ERROR_WRONG_DISK (34)
#define ERROR_SHARING_BUFFER_EXCEEDED (36)
#define ERROR_HANDLE_EOF (38)
#define ERROR_HANDLE_DISK_FULL (39)
#define ERROR_NOT_SUPPORTED (50)
#define ERROR_REM_NOT_LIST (51)
#define ERROR_DUP_NAME (52)
#define ERROR_BAD_NETPATH (53)
#define ERROR_NETWORK_BUSY (54)
#define ERROR_DEV_NOT_EXIST (55)
#define ERROR_TOO_MANY_CMDS (56)
#define ERROR_ADAP_HDW_ERR (57)
#define ERROR_BAD_NE_RESP (58)
#define ERROR_UNEXP_NET_ERR (59)
#define ERROR_BAD_REM_ADAP (60)
#define ERROR_PRINTQ_FULL (61)
#define ERROR_NO_SPOOL_SPACE (62)
#define ERROR_PRINT_CANCELLED (63)
#define ERROR_NETNAME_DELETED (64)
#define ERROR_NETWORK_ACCESS_DENIED (65)
#define ERROR_BAD_DEV_TYPE (66)
#define ERROR_BAD_NET_NAME (67)
#define ERROR_TOO_MANY_NAMES (68)
#define ERROR_TOO_MANY_SESS (69)
#define ERROR_SHARING_PAUSED (70)
#define ERROR_REQ_NOT_ACCEP (71)
#define ERROR_REDIR_PAUSED (72)
#define ERROR_FILE_EXIST (80)
#define ERROR_CANNOT_MAKE (82)
#define ERROR_FAIL_I24 (83)
#define ERROR_OUT_OF_STRUCTURES (84)
#define ERROR_ALREADY_ASSIGNED (85)
#define ERROR_INVALID_PASSWORD (86)
#define ERROR_INVALID_PARAMETER (87)
#define ERROR_NET_WRITE_FAULT (88)
#define ERROR_NO_PROC_SLOTS (89)
#define ERROR_TOO_MANY_SEMAPHORES (100)
#define ERROR_EXCL_SEM_ALREADY_OWNED (101)
#define ERROR_SEM_IS_SET (102)
#define ERROR_TOO_MANY_SEM_REQUEST (103)
#define ERROR_INVALID_AT_INTERRUPT_TIME (104)
#define ERROR_SEM_OWNER_DIED (105)
#define ERROR_SEM_USER_LIMIT (106)
#define ERROR_DISK_CHANGE (107)
#define ERROR_DRIVE_LOCKED (108)
#define ERROR_BROKEN_PIPE (109)
#define ERROR_OPEN_FAILED (110)
#define ERROR_BUFFER_OVERFLOW (111)
#define ERROR_DISK_FULL (112)
#define ERROR_NO_MORE_SEARCH_HANDLES (113)
#define ERROR_INVALID_TARGET_HANDLE (114)
#define ERROR_INVALID_CATEGORY (117)
#define ERROR_INVALID_VERIFY_SWITCH (118)
#define ERROR_BAD_DRIVER_LEVEL (119)
#define ERROR_CALL_NOT_IMPLEMENTED (120)
#define ERROR_SEM_TIMEOUT (121)
#define ERROR_INSUFFICIENT_BUFFER (122)
#define ERROR_INVALID_NAME (123)
#define ERROR_INVALID_LEVEL (124)
#define ERROR_NO_VOLUME_LABEL (125)
#define ERROR_MOD_NOT_FOUND (126)
#define ERROR_PROC_NOT_FOUND (127)
#define ERROR_WAIT_NO_CHILDREN (128)
#define ERROR_CHILD_NOT_COMPLETE (129)
#define ERROR_DIRECT_ACCESS_HANDLE (130)
#define ERROR_NEGATIVE_SEEK (131)
#define ERROR_SEEK_ON_DEVICE (132)
#define ERROR_IS_JOIN_TARGET (133)
#define ERROR_IS_JOINED (134)
#define ERROR_IS_SUBSTED (135)
#define ERROR_NOT_JOINED (136)
#define ERROR_NOT_SUBSTED (137)
#define ERROR_JOIN_TO_JOIN (138)
#define ERROR_SUBST_TO_SUBST (140)
#define ERROR_SUBST_TO_JOIN (140)
#define ERROR_BUSY_DRIVE (142)
#define ERROR_SAME_DRIVE (143)
#define ERROR_DIR_NOT_ROOT (144)
#define ERROR_DIR_NOT_EMPTY (145)
#define ERROR_IS_SUBST_PATH (146)
#define ERROR_IS_JOIN_PATH (147)
#define ERROR_PATH_BUSY (148)
#define ERROR_IS_SUBST_TARGET (149)
#define ERROR_SYSTEM_TRACE (150)
#define ERROR_INVALID_EVENT_COUNT (151)
#define ERROR_TOO_MANY_MUXWAITERS (152)
#define ERROR_INVALID_LIST_FORMAT (153)
#define ERROR_LABEL_TOO_LONG (154)
#define ERROR_TOO_MANY_TCBS (155)
#define ERROR_SIGNAL_REFUSED (156)
#define ERROR_DISCARDED (157)
#define ERROR_NOT_LOCKED (158)
#define ERROR_BAD_THREADID_ADDR (159)
#define ERROR_BAD_ARGUMENTS (160)
#define ERROR_BAD_PATHNAME (161)
#define ERROR_SIGNAL_PENDING (162)
#define ERROR_MAX_THRDS_REACHED (164)
#define ERROR_LOCK_FAILED (167)
#define ERROR_BUSY (170)
#define ERROR_DEVICE_SUPPORT_IN_PROGRESS (171)
#define ERROR_CANCEL_VIOLATION (172)
#define ERROR_ATOMIC_LOCKS_NOT_SUPPORTED (174)
#define ERROR_INVALID_SEGMENT_NUMBER (180)
#define ERROR_INVALID_ORDINAL (181)
#define ERROR_ALREADY_EXISTS (183)
#define ERROR_INVALID_FLAGS_NUMBER (186)
#define ERROR_SEM_NOT_FOUND (187)
#define ERROR_INVALID_STARTING_CODESEG (188)
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef long HRESULT;
typedef uint64_t ULONGLONG;
typedef uint64_t ULONG64;
typedef unsigned int ULONG32;
typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef ULONG_PTR SIZE_T;
typedef long LONG;
typedef unsigned short WORD;
typedef unsigned char UCHAR;
typedef unsigned char* PUCHAR;
typedef UCHAR* STRING;
typedef STRING UNC;
typedef unsigned char BYTE;
typedef int HALF_PTR;
typedef int REGSAM;
typedef BYTE BOOLEAN;
typedef BYTE* PBOOLEAN;
typedef BYTE* LPBYTE;
typedef LONG_PTR LRESULT;
typedef LONG_PTR LPARAM;
typedef DWORD ATOM;
typedef DWORD LCID;
typedef DWORD LCTYPE;
typedef DWORD LGRPID;
typedef DWORD LANGID;
typedef DWORD* LPDWORD;
typedef DWORD* DWORD_PTR;
typedef int32_t DWORD32;
typedef int64_t DWORD64;
typedef uint64_t DWORDLONG;
typedef DWORD* PDWORD;
typedef int BOOL;
typedef int* PBOOL;
typedef int* LPBOOL;
typedef char TCHAR;
typedef char CHAR;
typedef char CCHAR;
typedef char* LPCSTR;
typedef char* LPSTR;
typedef wchar_t* LPCWSTR;
typedef wchar_t* LPWSTR;
typedef wchar_t* PWSTR;
typedef wchar_t* LMSTR;
typedef wchar_t WCHAR;
typedef WCHAR* BSTR;
typedef wchar_t* PWCHAR;
typedef wchar_t UNICODE;
typedef uint64_t QWORD;
typedef WORD* LPWORD;
typedef float FLOAT;
typedef double DOUBLE;
typedef void* PVOID;
typedef void* PCVOID;
typedef void* LPCVOID;
typedef PVOID ADCONNECTION_HANDLE;
typedef PVOID LDAP_UDP_HANDLE;
typedef PVOID LPVOID;
typedef PVOID HANDLE;
typedef HANDLE HWND;
typedef HANDLE HKEY;
typedef HANDLE HINSTANCE;
typedef HANDLE WINSTA;
typedef HANDLE HSZ;
typedef HANDLE HRSRC;
typedef HANDLE HRGN;
typedef HANDLE HMONITOR;
typedef HANDLE HPALETTE;
typedef HANDLE HLOCAL;
typedef HANDLE HHOOK;
typedef HANDLE HDWP;
typedef HANDLE HDROP;
typedef HANDLE HDESK;
typedef HANDLE HGLOBAL;
typedef HINSTANCE HMODULE;
typedef HINSTANCE HKEY;
typedef HKEY* PHKEY;
typedef struct _RTL_CRITICAL_SECTION_DEBUG RTL_CRITICAL_SECTION_DEBUG;
typedef struct _RTL_CRITICAL_SECTION_DEBUG *PRTL_CRITICAL_SECTION_DEBUG;
typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION;
typedef struct _RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef struct _RTL_CRITICAL_SECTION* PRTL_CRITICAL_SECTION;
typedef struct _WIN32_FIND_DATA WIN32_FIND_DATA;
typedef struct _FILETIME FILETIME;
typedef struct _FILETIME* PFILETIME;
typedef struct _FILETIME* LPFILETIME;
typedef struct _SID SID;
typedef struct _SID* PSID;
typedef struct _SID* PISID;
typedef struct _MIB_TCPTABLE_OWNER_PID* PMIB_TCPTABLE_OWNER_PID;
typedef struct _MIB_TCPTABLE_OWNER_PID MIB_TCPTABLE_OWNER_PID;
typedef PMIB_TCPTABLE_OWNER_PID PMIB_TCPTABLE_EX;
typedef DWORD (*ProcAllocateAndGetTcpExtTableFromStack)(PMIB_TCPTABLE_EX*,BOOL,HANDLE,DWORD,DWORD);
G_GNUC_INTERNAL GType g_registry_backend_get_type(void);
typedef struct _PROCESS_INFORMATION {
  HANDLE hProcess;
  HANDLE hThread;
  DWORD dwProcessId;
  DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;
typedef struct _STARTUPINFOA {
  DWORD cb;
  LPSTR lpReserved;
  LPSTR lpDesktop;
  LPSTR lpTitle;
  DWORD dwX;
  DWORD dwY;
  DWORD dwXSize;
  DWORD dwYSize;
  DWORD dwXCountChars;
  DWORD dwYCountChars;
  DWORD dwFillAttribute;
  DWORD dwFlags;
  WORD wShowWindow;
  WORD cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;
typedef enum _GET_FILEEX_INFO_LEVELS {
  GetFileExInfoStandard,
  GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;
typedef enum _SID_NAME_USE {
  SidTypeUser,
  SidTypeGroup,
  SidTypeDomain,
  SidTypeAlias,
  SidTypeWellKnownGroup,
  SidTypeDeletedAccount,
  SidTypeInvalid,
  SidTypeUnknown,
  SidTypeComputer,
  SidTypeLabel,
  SidTypeLogonSession
} SID_NAME_USE, *PSID_NAME_USE;
typedef struct _SECURITY_ATTRIBUTES {
  DWORD nLength;
  LPVOID lpSecurityDescriptor;
  BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
typedef struct _MIB_TCPROW_OWNER_PID {
  DWORD dwState;
  DWORD dwLocalAddr;
  DWORD dwLocalPort;
  DWORD dwRemoteAddr;
  DWORD dwRemotePort;
  DWORD dwOwningPid;
} MIB_TCPROW_OWNER_PID;
struct _MIB_TCPTABLE_OWNER_PID {
  DWORD dwNumEntries;
  MIB_TCPROW_OWNER_PID table[5];
};
typedef struct _SID_IDENTIFIER_AUTHORITY {
  BYTE Value[6];
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;
struct _SID {
  BYTE Revision;
  BYTE SubAuthorityCount;
  SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
  DWORD SubAuthority[5];
};
typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
};
typedef struct _WIN32_FIND_DATAA {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  DWORD dwReserved0;
  DWORD dwReserved1;
  CHAR cFileName[255];
  CHAR cAlternateFileName[14];
  DWORD dwFileType;
  DWORD dwCreatorType;
  WORD wFinderFlags;
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef struct _WIN32_FIND_DATA {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  DWORD dwReserved0;
  DWORD dwReserved1;
  TCHAR cFileName[255];
  TCHAR cAlternateFileName[14];
};
typedef struct _WIN32_FIND_DATAW {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    WCHAR    cFileName[255];
    WCHAR    cAlternateFileName[14];
    DWORD    dwFileType;
    DWORD    dwCreatorType;
    WORD     wFinderFlags;
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;
typedef struct _LIST_ENTRY {
  struct _LIST_ENTRY *Flink;
  struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, PRLIST_ENTRY;
struct _RTL_CRITICAL_SECTION {
  PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
  LONG LockCount;
  LONG RecursionCount;
  HANDLE OwningThread;
  HANDLE LockSemaphore;
  ULONG_PTR SpinCount;
};
struct _RTL_CRITICAL_SECTION_DEBUG {
  WORD Type;
  WORD CreatorBackTraceIndex;
  PRTL_CRITICAL_SECTION CriticalSection;
  LIST_ENTRY ProcessLocksList;
  ULONG EntryCount;
  ULONG ContentionCount;
  ULONG Flags;
  WORD CreatorBackTraceIndexHigh;
  WORD SpareUSHORT;
};
#endif