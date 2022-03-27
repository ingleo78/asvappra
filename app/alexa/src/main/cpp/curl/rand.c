#include "rand.h"
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <stdbool.h>
#include <assert.h>


static CURLcode randit(struct Curl_easy *data, unsigned int *rnd) {
    unsigned int r;
    CURLcode result = CURLE_OK;
    static unsigned int randseed;
    static bool seeded = FALSE;
#ifdef CURLDEBUG
    char *force_entropy = getenv("CURL_ENTROPY");
    if(force_entropy) {
        if(!seeded) {
            unsigned int seed = 0;
            size_t elen = strlen(force_entropy);
            size_t clen = sizeof(seed);
            size_t min = elen < clen ? elen : clen;
            memcpy((char *)&seed, force_entropy, min);
            randseed = ntohl(seed);
            seeded = TRUE;
        } else randseed++;
        *rnd = randseed;
        return CURLE_OK;
    }
#endif
    result = Curl_ssl_random(data, (unsigned char *)rnd, sizeof(*rnd));
    if(result != CURLE_NOT_BUILT_IN) return result;
#ifdef RANDOM_FILE
    if(!seeded) {
        int fd = open(RANDOM_FILE, O_RDONLY);
        if(fd > -1) {
            ssize_t nread = read(fd, &randseed, sizeof(randseed));
            if(nread == sizeof(randseed)) seeded = TRUE;
            close(fd);
        }
    }
#endif
    if(!seeded) {
        struct curltime now = Curl_now();
        infof(data, "WARNING: Using weak random seed\n");
        randseed += (unsigned int)now.tv_usec + (unsigned int)now.tv_sec;
        randseed = randseed * 1103515245 + 12345;
        randseed = randseed * 1103515245 + 12345;
        randseed = randseed * 1103515245 + 12345;
        seeded = TRUE;
    }
    r = randseed = randseed * 1103515245 + 12345;
    *rnd = (r << 16) | ((r >> 16) & 0xFFFF);
    return CURLE_OK;
}