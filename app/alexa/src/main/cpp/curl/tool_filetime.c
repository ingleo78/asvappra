#include <errno.h>
#include <sys/stat.h>
#include "tool_filetime.h"
#ifdef HAVE_UTIME_H
#include <utime.h>
#elif defined(HAVE_SYS_UTIME_H)
#include <sys/utime.h>
#endif

curl_off_t getfiletime(const char *filename, FILE *error_stream) {
    curl_off_t result = -1;
#if defined(WIN32) && (SIZEOF_CURL_OFF_T >= 8)
    HANDLE hfile;
    hfile = CreateFileA(filename, FILE_READ_ATTRIBUTES, (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_EXISTING, 0, NULL);
    if(hfile != INVALID_HANDLE_VALUE) {
        FILETIME ft;
        if(GetFileTime(hfile, NULL, NULL, &ft)) {
            curl_off_t converted = (curl_off_t)ft.dwLowDateTime | ((curl_off_t)ft.dwHighDateTime) << 32;
            if(converted < CURL_OFF_T_C(116444736000000000)) fprintf(error_stream, "Failed to get filetime: underflow\n");
            else result = (converted - CURL_OFF_T_C(116444736000000000)) / 10000000;
        } else fprintf(error_stream, "Failed to get filetime: GetFileTime failed: GetLastError %u\n", (unsigned int)GetLastError());
        CloseHandle(hfile);
    } else if(GetLastError() != ERROR_FILE_NOT_FOUND) {
        fprintf(error_stream, "Failed to get filetime: CreateFile failed: GetLastError %u\n", (unsigned int)GetLastError());
    }
#else
    struct_stat statbuf;
    if(-1 != stat(filename, &statbuf)) result = (curl_off_t)statbuf.st_mtime;
    else if(errno != ENOENT) fprintf(error_stream,"Failed to get filetime: %s\n", strerror(errno));
#endif
    return result;
}
#if defined(HAVE_UTIME) || defined(HAVE_UTIMES) || (defined(WIN32) && (SIZEOF_CURL_OFF_T >= 8))
void setfiletime(curl_off_t filetime, const char *filename, FILE *error_stream) {
    if(filetime >= 0) {
    #if defined(WIN32) && (SIZEOF_CURL_OFF_T >= 8)
        HANDLE hfile;
        if(filetime > CURL_OFF_T_C(910670515199)) {
            fprintf(error_stream, "Failed to set filetime % CURL_FORMAT_CURL_OFF_T on outfile: overflow\n", filetime);
            return;
        }
        hfile = CreateFileA(filename, FILE_WRITE_ATTRIBUTES, (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_EXISTING, 0, NULL);
        if(hfile != INVALID_HANDLE_VALUE) {
            curl_off_t converted = ((curl_off_t)filetime * 10000000) + CURL_OFF_T_C(116444736000000000);
            FILETIME ft;
            ft.dwLowDateTime = (DWORD)(converted & 0xFFFFFFFF);
            ft.dwHighDateTime = (DWORD)(converted >> 32);
            if(!SetFileTime(hfile, NULL, &ft, &ft)) {
                fprintf(error_stream, "Failed to set filetime % CURL_FORMAT_CURL_OFF_T on outfile: SetFileTime failed: GetLastError %u\n",
                        filetime, (unsigned int)GetLastError());
            }
            CloseHandle(hfile);
        } else {
            fprintf(error_stream, "Failed to set filetime % CURL_FORMAT_CURL_OFF_T on outfile: CreateFile failed: GetLastError %u\n",
                    filetime, (unsigned int)GetLastError());
        }
    #elif defined(HAVE_UTIMES)
        struct timeval times[2];
        times[0].tv_sec = times[1].tv_sec = (time_t)filetime;
        times[0].tv_usec = times[1].tv_usec = 0;
        if(utimes(filename, times)) fprintf(error_stream, "Failed to set filetime % CURL_FORMAT_CURL_OFF_T on outfile: %s\n", filetime, strerror(errno));
    #elif defined(HAVE_UTIME)
        struct utimbuf times;
        times.actime = (time_t)filetime;
        times.modtime = (time_t)filetime;
        if(utime(filename, &times)) fprintf(error_stream, "Failed to set filetime % CURL_FORMAT_CURL_OFF_T on outfile: %s\n", filetime, strerror(errno));
    #endif
    }
}
#endif