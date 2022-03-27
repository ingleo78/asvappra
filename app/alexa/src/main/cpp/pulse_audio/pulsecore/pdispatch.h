#ifndef foopdispatchhfoo
#define foopdispatchhfoo

#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/sysroot/usr/include/inttypes.h"
#include "../pulse/mainloop-api.h"
#include "../pulse/def.h"
#include "tagstruct.h"
#include "packet.h"
#include "creds.h"

typedef struct pa_pdispatch pa_pdispatch;

typedef void (*pa_pdispatch_cb_t)(pa_pdispatch *pd, uint32_t command, uint32_t tag, pa_tagstruct *t, void *userdata);
typedef void (*pa_pdispatch_drain_cb_t)(pa_pdispatch *pd, void *userdata);

pa_pdispatch* pa_pdispatch_new(pa_mainloop_api *m, bool use_rtclock, const pa_pdispatch_cb_t *table, unsigned entries);
void pa_pdispatch_unref(pa_pdispatch *pd);
pa_pdispatch* pa_pdispatch_ref(pa_pdispatch *pd);

int pa_pdispatch_run(pa_pdispatch *pd, pa_packet *p, pa_cmsg_ancil_data *ancil_data, void *userdata);

void pa_pdispatch_register_reply(pa_pdispatch *pd, uint32_t tag, int timeout, pa_pdispatch_cb_t callback, void *userdata, pa_free_cb_t free_cb);

int pa_pdispatch_is_pending(pa_pdispatch *pd);

void pa_pdispatch_set_drain_callback(pa_pdispatch *pd, pa_pdispatch_drain_cb_t callback, void *userdata);

/* Remove all reply slots with the give userdata parameter */
void pa_pdispatch_unregister_reply(pa_pdispatch *pd, void *userdata);

const pa_creds * pa_pdispatch_creds(pa_pdispatch *pd);
pa_cmsg_ancil_data *pa_pdispatch_take_ancil_data(pa_pdispatch *pd);

#endif
