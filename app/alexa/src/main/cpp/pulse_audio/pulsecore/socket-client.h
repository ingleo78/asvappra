#ifndef foosocketclienthfoo
#define foosocketclienthfoo

#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/sysroot/usr/include/inttypes.h"
#include "../pulse/mainloop-api.h"
#include "iochannel.h"

struct sockaddr;
typedef unsigned int size_t;
typedef struct pa_socket_client pa_socket_client;

typedef void (*pa_socket_client_cb_t)(pa_socket_client *c, pa_iochannel*io, void *userdata);

pa_socket_client* pa_socket_client_new_ipv4(pa_mainloop_api *m, uint32_t address, uint16_t port);
pa_socket_client* pa_socket_client_new_ipv6(pa_mainloop_api *m, uint8_t address[16], uint16_t port);
pa_socket_client* pa_socket_client_new_unix(pa_mainloop_api *m, const char *filename);
pa_socket_client* pa_socket_client_new_sockaddr(pa_mainloop_api *m, const struct sockaddr *sa, size_t salen);
pa_socket_client* pa_socket_client_new_string(pa_mainloop_api *m, bool use_rtclock, const char *a, uint16_t default_port);

pa_socket_client* pa_socket_client_ref(pa_socket_client *c);
void pa_socket_client_unref(pa_socket_client *c);

void pa_socket_client_set_callback(pa_socket_client *c, pa_socket_client_cb_t on_connection, void *userdata);

bool pa_socket_client_is_local(pa_socket_client *c);

#endif
