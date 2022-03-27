#include <stdlib.h>
#include <unistd.h>
#include "../config.h"
#ifdef DBUS_WIN
#include <windows.h>

#define sleep Sleep
#endif
int main(int argc, char **argv) {
  while (1) sleep(10000000);
  return 1;
}