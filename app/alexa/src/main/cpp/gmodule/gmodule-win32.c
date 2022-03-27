#include <stdio.h>
#include <string.h>
#include "../gio/config.h"
#include "../glib/glib.h"
#include "../glib/gwin32.h"
#ifdef G_WITH_CYGWIN
#include <sys/cygwin.h>
#endif

static void set_error(const gchar *format, ...) {
    gchar *error;
    gchar *detail;
    gchar *message;
    va_list args;
    //error = g_win32_error_message(GetLastError());
    va_start(args, format);
    detail = g_strdup_vprintf(format, args);
    va_end(args);
    message = g_strconcat(detail, error, NULL);
    //g_module_set_error(message);
    g_free(message);
    g_free(detail);
    g_free(error);
}
static gpointer _g_module_open(const gchar *file_name, gboolean     bind_lazy, gboolean     bind_local) {
    /*HINSTANCE handle;
    wchar_t *wfilename;
#ifdef G_WITH_CYGWIN
    gchar tmp[MAX_PATH];
    cygwin_conv_to_win32_path(file_name, tmp);
    file_name = tmp;
#endif
    wfilename = g_utf8_to_utf16(file_name, -1, NULL, NULL, NULL);
    handle = LoadLibraryW(wfilename);
    g_free(wfilename);
    if (!handle) set_error("`%s': ", file_name);
    return handle;*/
    return NULL;
}
static gint dummy;
static gpointer null_module_handle = &dummy;
static gpointer _g_module_self(void) {
    return null_module_handle;
}
static void _g_module_close(gpointer handle, gboolean is_unref) {
    /*if (handle != null_module_handle)
        if (!FreeLibrary(handle)) set_error("");*/
}
static gpointer find_in_any_module_using_toolhelp(const gchar *symbol_name) {
    /*HANDLE snapshot;
    MODULEENTRY32 me32;
    gpointer p;
    if ((snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0)) == (HANDLE) -1) return NULL;
    me32.dwSize = sizeof(me32);
    p = NULL;
    if (Module32First(snapshot, &me32)) {
        do {
            if ((p = GetProcAddress(me32.hModule, symbol_name)) != NULL) break;
        } while (Module32Next(snapshot, &me32));
    }
    CloseHandle(snapshot);
    return p;*/
    return NULL;
}
static gpointer find_in_any_module(const gchar *symbol_name) {
    gpointer result;
    if ((result = find_in_any_module_using_toolhelp(symbol_name)) == NULL) return NULL;
    else return result;
}
static gpointer _g_module_symbol(gpointer handle, const gchar *symbol_name) {
    /*gpointer p;
    if (handle == null_module_handle) {
        if ((p = GetProcAddress(GetModuleHandle(NULL), symbol_name)) == NULL) p = find_in_any_module (symbol_name);
    } else p = GetProcAddress (handle, symbol_name);
    if (!p) set_error ("");
    return p;*/
    return NULL;
}
static gchar* _g_module_build_path (const gchar *directory, const gchar *module_name) {
    gint k;
    k = strlen (module_name);
    if (directory && *directory)
    if (k > 4 && g_ascii_strcasecmp (module_name + k - 4, ".dll") == 0) return g_strconcat (directory, G_DIR_SEPARATOR_S, module_name, NULL);
#ifdef G_WITH_CYGWIN
    else if (strncmp (module_name, "lib", 3) == 0 || strncmp (module_name, "cyg", 3) == 0) {
      return g_strconcat (directory, G_DIR_SEPARATOR_S, module_name, ".dll", NULL);
    } else return g_strconcat (directory, G_DIR_SEPARATOR_S, "cyg", module_name, ".dll", NULL);
#else
    else if (strncmp (module_name, "lib", 3) == 0) return g_strconcat (directory, G_DIR_SEPARATOR_S, module_name, ".dll", NULL);
    else return g_strconcat (directory, G_DIR_SEPARATOR_S, "lib", module_name, ".dll", NULL);
#endif
    else if (k > 4 && g_ascii_strcasecmp (module_name + k - 4, ".dll") == 0) return g_strdup (module_name);
#ifdef G_WITH_CYGWIN
    else if (strncmp (module_name, "lib", 3) == 0 || strncmp (module_name, "cyg", 3) == 0) return g_strconcat (module_name, ".dll", NULL);
    else return g_strconcat ("cyg", module_name, ".dll", NULL);
#else
    else if (strncmp (module_name, "lib", 3) == 0) return g_strconcat (module_name, ".dll", NULL);
    else return g_strconcat ("lib", module_name, ".dll", NULL);
#endif
}