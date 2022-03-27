#include "keylog.h"

#define KEYLOG_LABEL_MAXLEN (sizeof("CLIENT_HANDSHAKE_TRAFFIC_SECRET") - 1)
#define CLIENT_RANDOM_SIZE  32
#define SECRET_MAXLEN       48

static FILE *keylog_file_fp;

void Curl_tls_keylog_open(void) {
    char *keylog_file_name;
    if(!keylog_file_fp) {
        keylog_file_name = curl_getenv("SSLKEYLOGFILE");
        if(keylog_file_name) {
            keylog_file_fp = fopen(keylog_file_name, FOPEN_APPENDTEXT);
            if(keylog_file_fp) {
            #ifdef WIN32
                if(setvbuf(keylog_file_fp, NULL, _IONBF, 0))
            #else
                if(setvbuf(keylog_file_fp, NULL, _IOLBF, 4096))
            #endif
                {
                    fclose(keylog_file_fp);
                    keylog_file_fp = NULL;
                }
            }
            Curl_safefree(keylog_file_name);
        }
    }
}
void Curl_tls_keylog_close(void) {
    if(keylog_file_fp) {
        fclose(keylog_file_fp);
        keylog_file_fp = NULL;
    }
}
bool Curl_tls_keylog_enabled(void) {
    return keylog_file_fp != NULL;
}
bool Curl_tls_keylog_write_line(const char *line) {
    size_t linelen;
    char buf[256];
    if(!keylog_file_fp || !line) return false;
    linelen = strlen(line);
    if(linelen == 0 || linelen > sizeof(buf) - 2) return false;
    memcpy(buf, line, linelen);
    if(line[linelen - 1] != '\n') buf[linelen++] = '\n';
    buf[linelen] = '\0';
    fputs(buf, keylog_file_fp);
    return true;
}
bool Curl_tls_keylog_write(const char *label, const unsigned char client_random[CLIENT_RANDOM_SIZE], const unsigned char *secret, size_t secretlen) {
    const char *hex = "0123456789ABCDEF";
    size_t pos, i;
    char line[KEYLOG_LABEL_MAXLEN + 1 + 2 * CLIENT_RANDOM_SIZE + 1 + 2 * SECRET_MAXLEN + 1 + 1];
    if(!keylog_file_fp) return false;
    pos = strlen(label);
    if(pos > KEYLOG_LABEL_MAXLEN || !secretlen || secretlen > SECRET_MAXLEN) return false;
    memcpy(line, label, pos);
    line[pos++] = ' ';
    for(i = 0; i < CLIENT_RANDOM_SIZE; i++) {
        line[pos++] = hex[client_random[i] >> 4];
        line[pos++] = hex[client_random[i] & 0xF];
    }
    line[pos++] = ' ';
    for(i = 0; i < secretlen; i++) {
        line[pos++] = hex[secret[i] >> 4];
        line[pos++] = hex[secret[i] & 0xF];
    }
    line[pos++] = '\n';
    line[pos] = '\0';
    fputs(line, keylog_file_fp);
    return true;
}