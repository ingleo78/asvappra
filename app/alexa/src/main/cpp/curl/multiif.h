#ifndef HEADER_CURL_MULTIIF_H
#define HEADER_CURL_MULTIIF_H

void Curl_updatesocket(struct Curl_easy *data);
void Curl_expire(struct Curl_easy *data, timediff_t milli, expire_id);
void Curl_expire_clear(struct Curl_easy *data);
void Curl_expire_done(struct Curl_easy *data, expire_id id);
void Curl_update_timer(struct Curl_multi *multi);
void Curl_attach_connnection(struct Curl_easy *data, struct connectdata *conn);
void Curl_detach_connnection(struct Curl_easy *data);
bool Curl_multiplex_wanted(const struct Curl_multi *multi);
void Curl_set_in_callback(struct Curl_easy *data, bool value);
bool Curl_is_in_callback(struct Curl_easy *easy);
CURLcode Curl_preconnect(struct Curl_easy *data);
struct Curl_multi *Curl_multi_handle(int hashsize, int chashsize);
#define GETSOCK_WRITEBITSTART 16
#define GETSOCK_BLANK 0
#define GETSOCK_WRITESOCK(x) (1 << (GETSOCK_WRITEBITSTART + (x)))
#define GETSOCK_READSOCK(x) (1 << (x))
#ifdef DEBUGBUILD
void Curl_multi_dump(struct Curl_multi *multi);
#endif
size_t Curl_multi_max_host_connections(struct Curl_multi *multi);
size_t Curl_multi_max_total_connections(struct Curl_multi *multi);
void Curl_multiuse_state(struct connectdata *conn, int bundlestate);
void Curl_multi_closed(struct Curl_easy *data, curl_socket_t s);
CURLMcode Curl_multi_add_perform(struct Curl_multi *multi, struct Curl_easy *data, struct connectdata *conn);
unsigned int Curl_multi_max_concurrent_streams(struct Curl_multi *multi);

#endif