#include <stdio.h>
#include "../config.h"
#include "../dbus-server-socket.h"

int main(int argc, char **argv) {
    DBusServer *server;
    DBusError error;
    int result = 0;
    int i;
    char *family = NULL;
    if (argc == 2) family = argv[1];
    for (i = 0; i < 1000; i++) {
        dbus_error_init(&error);
        server = _dbus_server_new_for_tcp_socket("localhost", "localhost", "0", family, &error, FALSE);
        if (server == NULL) {
            printf("%d: %s %s\n",i, error.name, error.message);
            dbus_error_free(&error);
            result = -1;
        } else {
            printf("%d: %s \n",i, dbus_server_get_address(server));
            dbus_server_disconnect(server);
            dbus_server_unref(server);
        }
    }
    return result;
}