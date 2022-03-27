#include "tool_setup.h"
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>

#ifdef WIN32
#  include <direct.h>
#endif
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "tool_dirhie.h"
#include "memdebug.h"
#ifdef NETWARE
#ifndef __NOVELL_LIBC__
#define mkdir mkdir_510
#endif
#endif

#if defined(WIN32) || (defined(MSDOS) && !defined(__DJGPP__))
#define mkdir(x,y) (mkdir)((x))
#ifndef F_OK
#define F_OK 0
#endif
#endif

static void show_dir_errno(FILE *errors, const char *name) {
    switch(errno) {
    #ifdef EACCES
        case EACCES: fprintf(errors, "You don't have permission to create %s.\n", name); break;
    #endif
    #ifdef ENAMETOOLONG
        case ENAMETOOLONG: fprintf(errors, "The directory name %s is too long.\n", name); break;
    #endif
    #ifdef EROFS
        case EROFS: fprintf(errors, "%s resides on a read-only file system.\n", name); break;
    #endif
    #ifdef ENOSPC
        case ENOSPC: fprintf(errors, "No space left on the file system that will contain the directory %s.\n", name); break;
    #endif
    #ifdef EDQUOT
        case EDQUOT: fprintf(errors, "Cannot create directory %s because you exceeded your quota.\n", name); break;
    #endif
        default: fprintf(errors, "Error creating directory %s.\n", name);
    }
}
#if defined(WIN32) || defined(__DJGPP__)
#define PATH_DELIMITERS "\\/"
#else
#define DIR_CHAR "/"
#define PATH_DELIMITERS DIR_CHAR
#endif
CURLcode create_dir_hierarchy(const char *outfile, FILE *errors) {
    char *tempdir;
    char *tempdir2;
    char *outdup;
    char *dirbuildup;
    CURLcode result = CURLE_OK;
    size_t outlen;
    outlen = strlen(outfile);
    outdup = strdup(outfile);
    if(!outdup) return CURLE_OUT_OF_MEMORY;
    dirbuildup = malloc(outlen + 1);
    if(!dirbuildup) {
        Curl_safefree(outdup);
        return CURLE_OUT_OF_MEMORY;
    }
    dirbuildup[0] = '\0';
    tempdir = strtok(outdup, PATH_DELIMITERS);
    while(tempdir != NULL) {
        bool skip = false;
        tempdir2 = strtok(NULL, PATH_DELIMITERS);
        if(tempdir2 != NULL) {
            size_t dlen = strlen(dirbuildup);
            if(dlen) msnprintf(&dirbuildup[dlen], outlen - dlen, "%s%s", DIR_CHAR, tempdir);
            else {
                if(outdup == tempdir) {
                #if defined(MSDOS) || defined(WIN32)
                    char *p = strchr(tempdir, ':');
                    if(p && !p[1]) skip = true;
                #endif
                    strcpy(dirbuildup, tempdir);
                } else msnprintf(dirbuildup, outlen, "%s%s", DIR_CHAR, tempdir);
            }
            if(!skip && (-1 == mkdir(dirbuildup, (mode_t)0000750)) && (errno != EACCES) && (errno != EEXIST)) {
                show_dir_errno(errors, dirbuildup);
                result = CURLE_WRITE_ERROR;
                break;
            }
        }
        tempdir = tempdir2;
    }
    Curl_safefree(dirbuildup);
    Curl_safefree(outdup);
    return result;
}