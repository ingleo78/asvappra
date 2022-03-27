#ifndef HEADER_CURL_TOOL_METALINK_H
#define HEADER_CURL_TOOL_METALINK_H

#include "tool_setup.h"
#include "tool_sdecls.h"

struct GlobalConfig;
struct OperationConfig;
typedef int (*digest_init_func)(void *context);
typedef void (*digest_update_func)(void *context, const unsigned char *data, unsigned int len);
typedef void (*digest_final_func)(unsigned char *result, void *context);
struct digest_params {
    digest_init_func     digest_init;
    digest_update_func   digest_update;
    digest_final_func    digest_final;
    unsigned int         digest_ctxtsize;
    unsigned int         digest_resultlen;
};
struct digest_context {
  const struct digest_params *digest_hash;
  void                  *digest_hashctx;
};
struct metalink_digest_def {
    const char *hash_name;
    const struct digest_params *dparams;
};
struct metalink_digest_alias {
    const char *alias_name;
    const struct metalink_digest_def *digest_def;
};

struct metalink_checksum {
    const struct metalink_digest_def *digest_def;
    unsigned char *digest;
};
struct metalink_resource {
    struct metalink_resource *next;
    char *url;
};
struct metalinkfile {
    struct metalinkfile *next;
    char *filename;
    struct metalink_checksum *checksum;
    struct metalink_resource *resource;
};
#ifdef USE_METALINK
#define CURL_REQ_LIBMETALINK_MAJOR  0
#define CURL_REQ_LIBMETALINK_MINOR  1
#define CURL_REQ_LIBMETALINK_PATCH  0
#define CURL_REQ_LIBMETALINK_VERS  ((CURL_REQ_LIBMETALINK_MAJOR * 10000) + (CURL_REQ_LIBMETALINK_MINOR * 100) + CURL_REQ_LIBMETALINK_PATCH)
extern const struct digest_params MD5_DIGEST_PARAMS[1];
extern const struct digest_params SHA1_DIGEST_PARAMS[1];
extern const struct digest_params SHA256_DIGEST_PARAMS[1];

#include <metalink/metalink.h>

int count_next_metalink_resource(struct metalinkfile *mlfile);
void delete_metalinkfile(struct metalinkfile *mlfile);
void clean_metalink(struct OperationConfig *config);
int parse_metalink(struct OperationConfig *config, struct OutStruct *outs, const char *metalink_url);
size_t metalink_write_cb(void *buffer, size_t sz, size_t nmemb, void *userdata);
int check_metalink_content_type(const char *content_type);
int metalink_check_hash(struct GlobalConfig *config, struct metalinkfile *mlfile, const char *filename);
void metalink_cleanup(void);
#else
#define count_next_metalink_resource(x)  0
#define delete_metalinkfile(x)  (void)x
#define clean_metalink(x)  (void)x
#define metalink_cleanup() Curl_nop_stmt
#endif
#endif