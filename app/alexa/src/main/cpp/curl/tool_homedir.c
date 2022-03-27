#include "tool_setup.h"
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include "mprintf.h"
#include "tool_homedir.h"
#include "memdebug.h"

static char *GetEnv(const char *variable) {
    char *dupe, *env;
    env = curl_getenv(variable);
    if(!env) return NULL;
    dupe = strdup(env);
    curl_free(env);
    return dupe;
}
char *homedir(void) {
    char *home;
    home = GetEnv("CURL_HOME");
    if(home) return home;
    home = GetEnv("HOME");
    if(home) return home;
#if defined(HAVE_GETPWUID) && defined(HAVE_GETEUID)
   struct passwd *pw = getpwuid(geteuid());
   if(pw) {
       home = pw->pw_dir;
       if(home && home[0]) home = strdup(home);
       else home = NULL;
   }
#endif
#ifdef WIN32
    home = GetEnv("APPDATA");
    if(!home) {
        char *env = GetEnv("USERPROFILE");
        if(env) {
            char *path = curl_maprintf("%s\\Application Data", env);
            if(path) {
                home = strdup(path);
                curl_free(path);
            }
            free(env);
        }
    }
#endif
    return home;
}