#include "curl_setup.h"
#ifndef CURL_DISABLE_NETRC

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <stdbool.h>
#include <assert.h>
#include "curl.h"
#include "netrc.h"
#include "strtok.h"
#include "strcase.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

enum host_lookup_state {
    NOTHING,
    HOSTFOUND,
    HOSTVALID
};
#define NETRC_FILE_MISSING 1
#define NETRC_FAILED -1
#define NETRC_SUCCESS 0

static int parsenetrc(const char *host, char **loginp, char **passwordp, bool *login_changed, bool *password_changed, char *netrcfile) {
    FILE *file;
    int retcode = NETRC_FILE_MISSING;
    char *login = *loginp;
    char *password = *passwordp;
    bool specific_login = (login && *login != 0);
    bool login_alloc = FALSE;
    bool password_alloc = FALSE;
    enum host_lookup_state state = NOTHING;
    char state_login = 0;
    char state_password = 0;
    int state_our_login = FALSE;
    DEBUGASSERT(netrcfile);
    file = fopen(netrcfile, FOPEN_READTEXT);
    if (file) {
        char *tok;
        char *tok_buf;
        bool done = FALSE;
        char netrcbuffer[4096];
        int  netrcbuffsize = (int)sizeof(netrcbuffer);
        while(!done && fgets(netrcbuffer, netrcbuffsize, file)) {
            tok = strtok_r(netrcbuffer, " \t\n", &tok_buf);
            if (tok && *tok == '#') continue;
            while(tok) {
                if ((login && *login) && (password && *password)) {
                    done = TRUE;
                    break;
                }
                switch(state) {
                    case NOTHING:
                        if (strcasecompare("machine", tok)) state = HOSTFOUND;
                        else if (strcasecompare("default", tok)) {
                            state = HOSTVALID;
                            retcode = NETRC_SUCCESS;
                        }
                        break;
                    case HOSTFOUND:
                        if (strcasecompare(host, tok)) {
                            state = HOSTVALID;
                            retcode = NETRC_SUCCESS;
                        } else state = NOTHING;
                        break;
                    case HOSTVALID:
                        if (state_login) {
                            if (specific_login) state_our_login = strcasecompare(login, tok);
                            else if (!login || strcmp(login, tok)) {
                                if (login_alloc) {
                                    free(login);
                                    login_alloc = FALSE;
                                }
                                login = strdup(tok);
                                if (!login) {
                                    retcode = NETRC_FAILED;
                                    goto out;
                                }
                                login_alloc = TRUE;
                            }
                            state_login = 0;
                        } else if (state_password) {
                            if ((state_our_login || !specific_login) && (!password || strcmp(password, tok))) {
                                if (password_alloc) {
                                    free(password);
                                    password_alloc = FALSE;
                                }
                                password = strdup(tok);
                                if (!password) {
                                    retcode = NETRC_FAILED;
                                    goto out;
                                }
                                password_alloc = TRUE;
                            }
                            state_password = 0;
                        } else if(strcasecompare("login", tok)) state_login = 1;
                        else if(strcasecompare("password", tok)) state_password = 1;
                        else if(strcasecompare("machine", tok)) {
                            state = HOSTFOUND;
                            state_our_login = FALSE;
                        }
                        break;
                }
                tok = strtok_r(NULL, " \t\n", &tok_buf);
            }
        }
        out:
        if (!retcode) {
            *login_changed = FALSE;
            *password_changed = FALSE;
            if (login_alloc) {
                if (*loginp) free(*loginp);
                *loginp = login;
                *login_changed = TRUE;
            }
            if (password_alloc) {
            if (*passwordp) free(*passwordp);
                *passwordp = password;
                *password_changed = TRUE;
            }
        } else {
            if (login_alloc) free(login);
            if (password_alloc) free(password);
        }
        fclose(file);
    }
    return retcode;
}
int Curl_parsenetrc(const char *host, char **loginp, char **passwordp, bool *login_changed, bool *password_changed, char *netrcfile) {
    int retcode = 1;
    char *filealloc = NULL;
    if (!netrcfile) {
        char *home = NULL;
        char *homea = curl_getenv("HOME");
        if (homea) {
            home = homea;
    #if defined(HAVE_GETPWUID_R) && defined(HAVE_GETEUID)
        } else {
            struct passwd pw, *pw_res;
            char pwbuf[1024];
            if (!getpwuid_r(geteuid(), &pw, pwbuf, sizeof(pwbuf), &pw_res) && pw_res) home = pw.pw_dir;
    #elif defined(HAVE_GETPWUID) && defined(HAVE_GETEUID)
        } else {
            struct passwd *pw;
            pw = getpwuid(geteuid());
            if (pw) home = pw->pw_dir;
    #endif
        }
        if (!home) return retcode;
        filealloc = curl_maprintf("%s%s.netrc", home, "/");
        if (!filealloc) {
            free(homea);
            return -1;
        }
        retcode = parsenetrc(host, loginp, passwordp, login_changed, password_changed, filealloc);
        free(filealloc);
    #ifdef WIN32
        if (retcode == NETRC_FILE_MISSING) {
            filealloc = curl_maprintf("%s%s_netrc", home, DIR_CHAR);
            if (!filealloc) {
                free(homea);
                return -1;
            }
            retcode = parsenetrc(host, loginp, passwordp, login_changed, password_changed, filealloc);
            free(filealloc);
        }
    #endif
        free(homea);
    } else retcode = parsenetrc(host, loginp, passwordp, login_changed, password_changed, netrcfile);
    return retcode;
}
#endif