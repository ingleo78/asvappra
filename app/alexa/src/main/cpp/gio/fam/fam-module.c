#include "../giomodule.h"
#include "gfamdirectorymonitor.h"
#include "gfamfilemonitor.h"
#include "fam-helper.h"

void g_io_module_load(GIOModule *module) {
  g_fam_file_monitor_register (module);
  g_fam_directory_monitor_register (module);
}
void g_io_module_unload(GIOModule *module) {
  _fam_sub_shutdown();
}
char ** g_io_module_query(void) {
  char *eps[] = {
    G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
    G_LOCAL_FILE_MONITOR_EXTENSION_POINT_NAME,
    NULL
  };
  return g_strdupv(eps);
}

