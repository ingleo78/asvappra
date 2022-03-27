#include <expat/expat_external.h>

#ifdef XML_UNICODE
#ifdef XML_UNICODE_WCHAR_T
#error xmlwf requires a 16-bit Unicode-compatible wchar_t
#endif
#define _PREPEND_BIG_L(x) L##x
#define T(x) _PREPEND_BIG_L(x)
#define ftprintf fwprintf
#define tfopen _wfopen
#define fputts fputws
#define puttc putwc
#define tcscmp wcscmp
#define tcscpy wcscpy
#define tcscat wcscat
#define tcschr wcschr
#define tcsrchr wcsrchr
#define tcslen wcslen
#define tperror _wperror
#define topen _wopen
#define tmain wmain
#define tremove _wremove
#define tchar wchar_t
#else
#define T(x) x
#define ftprintf fprintf
#define tfopen fopen
#define fputts fputs
#define puttc putc
#define tcscmp strcmp
#define tcscpy strcpy
#define tcscat strcat
#define tcschr strchr
#define tcsrchr strrchr
#define tcslen strlen
#define tperror perror
#define topen open
#define tmain main
#define tremove remove
#define tchar char
#endif
