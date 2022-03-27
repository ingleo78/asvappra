#include "curl_setup.h"

#if defined(WIN32)
#include "curl.h"
#include "system_win32.h"
#include "curl_sspi.h"
#include "warnless.h"
#include "curl_memory.h"
#include "memdebug.h"

LARGE_INTEGER Curl_freq;
bool Curl_isVistaOrGreater;
static HMODULE s_hIpHlpApiDll = NULL;
IF_NAMETOINDEX_FN Curl_if_nametoindex = NULL;
CURLcode Curl_win32_init(long flags) {
    if(flags & CURL_GLOBAL_WIN32) {
    #ifdef USE_WINSOCK
        WORD wVersionRequested;
        WSADATA wsaData;
        int res;
    #if defined(ENABLE_IPV6) && (USE_WINSOCK < 2)
        #error IPV6_requires_winsock2
    #endif
        wVersionRequested = MAKEWORD(USE_WINSOCK, USE_WINSOCK);
        res = WSAStartup(wVersionRequested, &wsaData);
        if(res != 0) return CURLE_FAILED_INIT;
        if(LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) || HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested) ) {
            WSACleanup();
            return CURLE_FAILED_INIT;
        }
    #elif defined(USE_LWIPSOCK)
        lwip_init();
    #endif
    }
#ifdef USE_WINDOWS_SSPI
    CURLcode result = Curl_sspi_global_init();
    if(result) return result;
#endif
    s_hIpHlpApiDll = Curl_load_library(TEXT("iphlpapi.dll"));
    if(s_hIpHlpApiDll) {
        IF_NAMETOINDEX_FN pIfNameToIndex = CURLX_FUNCTION_CAST(IF_NAMETOINDEX_FN, (GetProcAddress(s_hIpHlpApiDll, "if_nametoindex")));
        if(pIfNameToIndex) Curl_if_nametoindex = pIfNameToIndex;
    }
    if(Curl_verify_windows_version(6, 0, PLATFORM_WINNT, VERSION_GREATER_THAN_EQUAL)) Curl_isVistaOrGreater = TRUE;
    else Curl_isVistaOrGreater = FALSE;
    QueryPerformanceFrequency(&Curl_freq);
    return CURLE_OK;
}
void Curl_win32_cleanup(long init_flags) {
    if(s_hIpHlpApiDll) {
        FreeLibrary(s_hIpHlpApiDll);
        s_hIpHlpApiDll = NULL;
        Curl_if_nametoindex = NULL;
    }
#ifdef USE_WINDOWS_SSPI
    Curl_sspi_global_cleanup();
#endif
    if(init_flags & CURL_GLOBAL_WIN32) {
    #ifdef USE_WINSOCK
        WSACleanup();
    #endif
    }
}
#if !defined(LOAD_WITH_ALTERED_SEARCH_PATH)
#define LOAD_WITH_ALTERED_SEARCH_PATH  0x00000008
#endif
#if !defined(LOAD_LIBRARY_SEARCH_SYSTEM32)
#define LOAD_LIBRARY_SEARCH_SYSTEM32   0x00000800
#endif
typedef HMODULE (APIENTRY *LOADLIBRARYEX_FN)(LPCTSTR, HANDLE, DWORD);
#ifdef UNICODE
#ifdef _WIN32_WCE
#define LOADLIBARYEX  L"LoadLibraryExW"
#else
#define LOADLIBARYEX  "LoadLibraryExW"
#endif
#else
#define LOADLIBARYEX    "LoadLibraryExA"
#endif
bool Curl_verify_windows_version(const unsigned int majorVersion, const unsigned int minorVersion, const PlatformIdentifier platform,
                                 const VersionCondition condition) {
    bool matched = FALSE;
#if defined(CURL_WINDOWS_APP)
    const WORD fullVersion = MAKEWORD(minorVersion, majorVersion);
    const WORD targetVersion = (WORD)_WIN32_WINNT;
    switch(condition) {
        case VERSION_LESS_THAN: matched = targetVersion < fullVersion; break;
        case VERSION_LESS_THAN_EQUAL: matched = targetVersion <= fullVersion; break;
        case VERSION_EQUAL: matched = targetVersion == fullVersion; break;
        case VERSION_GREATER_THAN_EQUAL: matched = targetVersion >= fullVersion; break;
        case VERSION_GREATER_THAN: matched = targetVersion > fullVersion; break;
    }
    if(matched && (platform == PLATFORM_WINDOWS)) matched = FALSE;
#elif !defined(_WIN32_WINNT) || !defined(_WIN32_WINNT_WIN2K) || (_WIN32_WINNT < _WIN32_WINNT_WIN2K)
    OSVERSIONINFO osver;
    memset(&osver, 0, sizeof(osver));
    osver.dwOSVersionInfoSize = sizeof(osver);
    if(GetVersionEx(&osver)) {
        switch(condition) {
            case VERSION_LESS_THAN:
                if(osver.dwMajorVersion < majorVersion || (osver.dwMajorVersion == majorVersion && osver.dwMinorVersion < minorVersion)) matched = TRUE;
                break;
            case VERSION_LESS_THAN_EQUAL:
                if(osver.dwMajorVersion < majorVersion || (osver.dwMajorVersion == majorVersion && osver.dwMinorVersion <= minorVersion)) matched = TRUE;
                break;
            case VERSION_EQUAL:
                if(osver.dwMajorVersion == majorVersion && osver.dwMinorVersion == minorVersion) matched = TRUE;
                break;
            case VERSION_GREATER_THAN_EQUAL:
                if(osver.dwMajorVersion > majorVersion || (osver.dwMajorVersion == majorVersion && osver.dwMinorVersion >= minorVersion)) matched = TRUE;
                break;
            case VERSION_GREATER_THAN:
                if(osver.dwMajorVersion > majorVersion || (osver.dwMajorVersion == majorVersion && osver.dwMinorVersion > minorVersion)) matched = TRUE;
                break;
        }
        if(matched) {
            switch(platform) {
                case PLATFORM_WINDOWS:
                    if(osver.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) matched = FALSE;
                    break;
                case PLATFORM_WINNT: if(osver.dwPlatformId != VER_PLATFORM_WIN32_NT) matched = FALSE;
                default: break;
            }
        }
    }
#else
    ULONGLONG cm = 0;
    OSVERSIONINFOEX osver;
    BYTE majorCondition;
    BYTE minorCondition;
    BYTE spMajorCondition;
    BYTE spMinorCondition;
    switch(condition) {
        case VERSION_LESS_THAN:
            majorCondition = VER_LESS;
            minorCondition = VER_LESS;
            spMajorCondition = VER_LESS_EQUAL;
            spMinorCondition = VER_LESS_EQUAL;
            break;
        case VERSION_LESS_THAN_EQUAL:
            majorCondition = VER_LESS_EQUAL;
            minorCondition = VER_LESS_EQUAL;
            spMajorCondition = VER_LESS_EQUAL;
            spMinorCondition = VER_LESS_EQUAL;
            break;
        case VERSION_EQUAL:
            majorCondition = VER_EQUAL;
            minorCondition = VER_EQUAL;
            spMajorCondition = VER_GREATER_EQUAL;
            spMinorCondition = VER_GREATER_EQUAL;
            break;
        case VERSION_GREATER_THAN_EQUAL:
            majorCondition = VER_GREATER_EQUAL;
            minorCondition = VER_GREATER_EQUAL;
            spMajorCondition = VER_GREATER_EQUAL;
            spMinorCondition = VER_GREATER_EQUAL;
            break;
        case VERSION_GREATER_THAN:
            majorCondition = VER_GREATER;
            minorCondition = VER_GREATER;
            spMajorCondition = VER_GREATER_EQUAL;
            spMinorCondition = VER_GREATER_EQUAL;
            break;
        default: return FALSE;
    }
    memset(&osver, 0, sizeof(osver));
    osver.dwOSVersionInfoSize = sizeof(osver);
    osver.dwMajorVersion = majorVersion;
    osver.dwMinorVersion = minorVersion;
    if(platform == PLATFORM_WINDOWS) osver.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;
    else if(platform == PLATFORM_WINNT) osver.dwPlatformId = VER_PLATFORM_WIN32_NT;
    cm = VerSetConditionMask(cm, VER_MAJORVERSION, majorCondition);
    cm = VerSetConditionMask(cm, VER_MINORVERSION, minorCondition);
    cm = VerSetConditionMask(cm, VER_SERVICEPACKMAJOR, spMajorCondition);
    cm = VerSetConditionMask(cm, VER_SERVICEPACKMINOR, spMinorCondition);
    if(platform != PLATFORM_DONT_CARE) cm = VerSetConditionMask(cm, VER_PLATFORMID, VER_EQUAL);
    if(VerifyVersionInfo(&osver, (VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR), cm)) matched = TRUE;
#endif
    return matched;
}
HMODULE Curl_load_library(LPCTSTR filename) {
#ifndef CURL_WINDOWS_APP
    HMODULE hModule = NULL;
    LOADLIBRARYEX_FN pLoadLibraryEx = NULL;
    HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32"));
    if(!hKernel32) return NULL;
    pLoadLibraryEx = CURLX_FUNCTION_CAST(LOADLIBRARYEX_FN, (GetProcAddress(hKernel32, LOADLIBARYEX)));
    if(_tcspbrk(filename, TEXT("\\/"))) hModule = pLoadLibraryEx ? pLoadLibraryEx(filename, NULL, LOAD_WITH_ALTERED_SEARCH_PATH) : LoadLibrary(filename);
    else if(pLoadLibraryEx && GetProcAddress(hKernel32, "AddDllDirectory")) {
        hModule = pLoadLibraryEx(filename, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    } else {
        UINT systemdirlen = GetSystemDirectory(NULL, 0);
        if(systemdirlen) {
            size_t filenamelen = _tcslen(filename);
            TCHAR *path = malloc(sizeof(TCHAR) * (systemdirlen + 1 + filenamelen));
            if(path && GetSystemDirectory(path, systemdirlen)) {
                _tcscpy(path + _tcslen(path), TEXT("\\"));
                _tcscpy(path + _tcslen(path), filename);
                hModule = pLoadLibraryEx ? pLoadLibraryEx(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH) : LoadLibrary(path);
            }
            free(path);
        }
    }
    return hModule;
#else
    (void)filename;
    return NULL;
#endif
}
#endif