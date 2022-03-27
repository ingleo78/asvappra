#ifndef HEADER_CURL_SECURITY_H
#define HEADER_CURL_SECURITY_H

struct Curl_sec_client_mech {
    const char *name;
    size_t size;
    int (*init)(void *);
    int (*auth)(void *, struct connectdata *);
    void (*end)(void *);
    int (*check_prot)(void *, int);
    int (*overhead)(void *, int, int);
    int (*encode)(void *, const void *, int, int, void **);
    int (*decode)(void *, void *, int, int, struct connectdata *);
};
#define AUTH_OK         0
#define AUTH_CONTINUE   1
#define AUTH_ERROR      2
#ifdef HAVE_GSSAPI
int Curl_sec_read_msg(struct connectdata *conn, char *, enum protection_level);
void Curl_sec_end(struct connectdata *);
CURLcode Curl_sec_login(struct connectdata *);
int Curl_sec_request_prot(struct connectdata *conn, const char *level);
extern struct Curl_sec_client_mech Curl_krb5_client_mech;
#endif
#endif