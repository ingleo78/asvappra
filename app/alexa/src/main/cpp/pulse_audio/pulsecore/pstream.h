#ifndef foopstreamhfoo
#define foopstreamhfoo

#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/sysroot/usr/include/inttypes.h"
#include "../pulse/mainloop-api.h"
#include "../pulse/def.h"
#include "packet.h"
#include "memblock.h"
#include "iochannel.h"
#include "srbchannel.h"
#include "memchunk.h"
#include "creds.h"
#include "macro.h"

typedef struct pa_pstream pa_pstream;

typedef void (*pa_pstream_packet_cb_t)(pa_pstream *p, pa_packet *packet, pa_cmsg_ancil_data *ancil_data, void *userdata);
typedef void (*pa_pstream_memblock_cb_t)(pa_pstream *p, uint32_t channel, int64_t offset, pa_seek_mode_t seek, const pa_memchunk *chunk, void *userdata);
typedef void (*pa_pstream_notify_cb_t)(pa_pstream *p, void *userdata);
typedef void (*pa_pstream_block_id_cb_t)(pa_pstream *p, uint32_t block_id, void *userdata);

pa_pstream* pa_pstream_new(pa_mainloop_api *m, pa_iochannel *io, pa_mempool *p);

pa_pstream* pa_pstream_ref(pa_pstream*p);
void pa_pstream_unref(pa_pstream*p);

void pa_pstream_unlink(pa_pstream *p);

int pa_pstream_attach_memfd_shmid(pa_pstream *p, unsigned shm_id, int memfd_fd);

void pa_pstream_send_packet(pa_pstream*p, pa_packet *packet, pa_cmsg_ancil_data *ancil_data);
void pa_pstream_send_memblock(pa_pstream*p, uint32_t channel, int64_t offset, pa_seek_mode_t seek, const pa_memchunk *chunk);
void pa_pstream_send_release(pa_pstream *p, uint32_t block_id);
void pa_pstream_send_revoke(pa_pstream *p, uint32_t block_id);

void pa_pstream_set_receive_packet_callback(pa_pstream *p, pa_pstream_packet_cb_t cb, void *userdata);
void pa_pstream_set_receive_memblock_callback(pa_pstream *p, pa_pstream_memblock_cb_t cb, void *userdata);
void pa_pstream_set_drain_callback(pa_pstream *p, pa_pstream_notify_cb_t cb, void *userdata);
void pa_pstream_set_die_callback(pa_pstream *p, pa_pstream_notify_cb_t cb, void *userdata);
void pa_pstream_set_release_callback(pa_pstream *p, pa_pstream_block_id_cb_t cb, void *userdata);
void pa_pstream_set_revoke_callback(pa_pstream *p, pa_pstream_block_id_cb_t cb, void *userdata);

bool pa_pstream_is_pending(pa_pstream *p);

void pa_pstream_enable_shm(pa_pstream *p, bool enable);
void pa_pstream_enable_memfd(pa_pstream *p);
bool pa_pstream_get_shm(pa_pstream *p);
bool pa_pstream_get_memfd(pa_pstream *p);

/* Enables shared ringbuffer channel. Note that the srbchannel is now owned by the pstream.
   Setting srb to NULL will free any existing srbchannel. */
void pa_pstream_set_srbchannel(pa_pstream *p, pa_srbchannel *srb);

#endif
