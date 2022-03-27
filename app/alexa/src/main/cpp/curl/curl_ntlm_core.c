#include "curl_setup.h"

#if defined(USE_NTLM)
#if !defined(USE_WINDOWS_SSPI) || defined(USE_WIN32_CRYPTO)
#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
#ifdef USE_WOLFSSL
#include <wolfssl/options.h>
#endif
#include <openssl/des.h>
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#if (defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x00907001L)) && !defined(USE_WOLFSSL)
#define DES_key_schedule des_key_schedule
#define DES_cblock des_cblock
#define DES_set_odd_parity des_set_odd_parity
#define DES_set_key des_set_key
#define DES_ecb_encrypt des_ecb_encrypt
#define DESKEY(x) x
#define DESKEYARG(x) x
#else
#define DESKEYARG(x) *x
#define DESKEY(x) &x
#endif
#elif defined(USE_GNUTLS_NETTLE)
#include <nettle/des.h>
#elif defined(USE_GNUTLS)
#include <gcrypt.h>
#elif defined(USE_NSS)
#include <nss.h>
#include <pk11pub.h>
#include <hasht.h>
#elif defined(USE_MBEDTLS)
#include <mbedtls/des.h>
#include "curl_md4.h"
#elif defined(USE_SECTRANSP)
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#elif defined(USE_OS400CRYPTO)
#include "cipher.mih"  /* mih/cipher */
#elif defined(USE_WIN32_CRYPTO)
#include <wincrypt.h>
#else
#error "Can't compile NTLM support without a crypto library."
#endif
#include "urldata.h"
#include "non-ascii.h"
#include "strcase.h"
#include "curl_ntlm_core.h"
#include "curl_md5.h"
#include "curl_hmac.h"
#include "warnless.h"
#include "curl_endian.h"
#include "curl_des.h"
#include "curl_md4.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#define NTLMv2_BLOB_SIGNATURE "\x01\x01\x00\x00"
#define NTLMv2_BLOB_LEN       (44 -16 + ntlm->target_info_len + 4)
static void extend_key_56_to_64(const unsigned char *key_56, char *key) {
    key[0] = key_56[0];
    key[1] = (unsigned char)(((key_56[0] << 7) & 0xFF) | (key_56[1] >> 1));
    key[2] = (unsigned char)(((key_56[1] << 6) & 0xFF) | (key_56[2] >> 2));
    key[3] = (unsigned char)(((key_56[2] << 5) & 0xFF) | (key_56[3] >> 3));
    key[4] = (unsigned char)(((key_56[3] << 4) & 0xFF) | (key_56[4] >> 4));
    key[5] = (unsigned char)(((key_56[4] << 3) & 0xFF) | (key_56[5] >> 5));
    key[6] = (unsigned char)(((key_56[5] << 2) & 0xFF) | (key_56[6] >> 6));
    key[7] = (unsigned char) ((key_56[6] << 1) & 0xFF);
}
#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
static void setup_des_key(const unsigned char *key_56, DES_key_schedule DESKEYARG(ks)) {
    DES_cblock key;
    extend_key_56_to_64(key_56, (char *) &key);
    DES_set_odd_parity(&key);
    DES_set_key(&key, ks);
}
#elif defined(USE_GNUTLS_NETTLE)
static void setup_des_key(const unsigned char *key_56, struct des_ctx *des) {
    char key[8];
    extend_key_56_to_64(key_56, key);
    Curl_des_set_odd_parity((unsigned char *) key, sizeof(key));
    des_set_key(des, (const uint8_t *) key);
}
#elif defined(USE_GNUTLS)
static void setup_des_key(const unsigned char *key_56, gcry_cipher_hd_t *des) {
    char key[8];
    extend_key_56_to_64(key_56, key);
    Curl_des_set_odd_parity((unsigned char *) key, sizeof(key));
    gcry_cipher_setkey(*des, key, sizeof(key));
}
#elif defined(USE_NSS)
static bool encrypt_des(const unsigned char *in, unsigned char *out,const unsigned char *key_56) {
    const CK_MECHANISM_TYPE mech = CKM_DES_ECB;
    char key[8];
    SECItem key_item;
    PK11SymKey *symkey = NULL;
    SECItem *param = NULL;
    PK11Context *ctx = NULL;
    int out_len;
    bool rv = FALSE;
    PK11SlotInfo *slot = PK11_GetInternalKeySlot();
    if(!slot) return FALSE;
    extend_key_56_to_64(key_56, key);
    Curl_des_set_odd_parity((unsigned char *) key, sizeof(key));
    key_item.data = (unsigned char *)key;
    key_item.len = sizeof(key);
    symkey = PK11_ImportSymKey(slot, mech, PK11_OriginUnwrap, CKA_ENCRYPT, &key_item, NULL);
    if(!symkey) goto fail;
    param = PK11_ParamFromIV(mech, NULL);
    if(!param) goto fail;
    ctx = PK11_CreateContextBySymKey(mech, CKA_ENCRYPT, symkey, param);
    if(!ctx) goto fail;
    if(SECSuccess == PK11_CipherOp(ctx, out, &out_len, 8, (unsigned char *)in, 8) && SECSuccess == PK11_Finalize(ctx)) rv = TRUE;
    fail:
    if(ctx) PK11_DestroyContext(ctx, PR_TRUE);
    if(symkey) PK11_FreeSymKey(symkey);
    if(param) SECITEM_FreeItem(param, PR_TRUE);
    PK11_FreeSlot(slot);
    return rv;
}
#elif defined(USE_MBEDTLS)
static bool encrypt_des(const unsigned char *in, unsigned char *out, const unsigned char *key_56) {
    mbedtls_des_context ctx;
    char key[8];
    extend_key_56_to_64(key_56, key);
    mbedtls_des_key_set_parity((unsigned char *) key);
    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_enc(&ctx, (unsigned char *) key);
    return mbedtls_des_crypt_ecb(&ctx, in, out) == 0;
}
#elif defined(USE_SECTRANSP)
static bool encrypt_des(const unsigned char *in, unsigned char *out, const unsigned char *key_56) {
    char key[8];
    size_t out_len;
    CCCryptorStatus err;
    extend_key_56_to_64(key_56, key);
    Curl_des_set_odd_parity((unsigned char *) key, sizeof(key));
    err = CCCrypt(kCCEncrypt, kCCAlgorithmDES, kCCOptionECBMode, key, kCCKeySizeDES, NULL, in, 8, out, 8, &out_len);
    return err == kCCSuccess;
}
#elif defined(USE_OS400CRYPTO)
static bool encrypt_des(const unsigned char *in, unsigned char *out, const unsigned char *key_56) {
    char key[8];
    _CIPHER_Control_T ctl;
    ctl.Func_ID = ENCRYPT_ONLY;
    ctl.Data_Len = sizeof(key);
    extend_key_56_to_64(key_56, ctl.Crypto_Key);
    Curl_des_set_odd_parity((unsigned char *) ctl.Crypto_Key, ctl.Data_Len);
    _CIPHER((_SPCPTR *) &out, &ctl, (_SPCPTR *) &in);
    return TRUE;
}
#elif defined(USE_WIN32_CRYPTO)
static bool encrypt_des(const unsigned char *in, unsigned char *out, const unsigned char *key_56) {
    HCRYPTPROV hprov;
    HCRYPTKEY hkey;
    struct {
        BLOBHEADER hdr;
        unsigned int len;
        char key[8];
    } blob;
    DWORD len = 8;
    if (!CryptAcquireContext(&hprov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) return FALSE;
    memset(&blob, 0, sizeof(blob));
    blob.hdr.bType = PLAINTEXTKEYBLOB;
    blob.hdr.bVersion = 2;
    blob.hdr.aiKeyAlg = CALG_DES;
    blob.len = sizeof(blob.key);
    extend_key_56_to_64(key_56, blob.key);
    Curl_des_set_odd_parity((unsigned char *) blob.key, sizeof(blob.key));
    if (!CryptImportKey(hprov, (BYTE *) &blob, sizeof(blob), 0, 0, &hkey)) {
        CryptReleaseContext(hprov, 0);
        return FALSE;
    }
    memcpy(out, in, 8);
    CryptEncrypt(hkey, 0, FALSE, 0, out, &len, len);
    CryptDestroyKey(hkey);
    CryptReleaseContext(hprov, 0);
    return TRUE;
}
#endif
void Curl_ntlm_core_lm_resp(const unsigned char *keys, const unsigned char *plaintext, unsigned char *results) {
#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
    DES_key_schedule ks;
    setup_des_key(keys, DESKEY(ks));
    DES_ecb_encrypt((DES_cblock*) plaintext, (DES_cblock*) results, DESKEY(ks), DES_ENCRYPT);
    setup_des_key(keys + 7, DESKEY(ks));
    DES_ecb_encrypt((DES_cblock*) plaintext, (DES_cblock*) (results + 8), DESKEY(ks), DES_ENCRYPT);
    setup_des_key(keys + 14, DESKEY(ks));
    DES_ecb_encrypt((DES_cblock*) plaintext, (DES_cblock*) (results + 16), DESKEY(ks), DES_ENCRYPT);
#elif defined(USE_GNUTLS_NETTLE)
    struct des_ctx des;
    setup_des_key(keys, &des);
    des_encrypt(&des, 8, results, plaintext);
    setup_des_key(keys + 7, &des);
    des_encrypt(&des, 8, results + 8, plaintext);
    setup_des_key(keys + 14, &des);
    des_encrypt(&des, 8, results + 16, plaintext);
#elif defined(USE_GNUTLS)
    gcry_cipher_hd_t des;
    gcry_cipher_open(&des, GCRY_CIPHER_DES, GCRY_CIPHER_MODE_ECB, 0);
    setup_des_key(keys, &des);
    gcry_cipher_encrypt(des, results, 8, plaintext, 8);
    gcry_cipher_close(des);
    gcry_cipher_open(&des, GCRY_CIPHER_DES, GCRY_CIPHER_MODE_ECB, 0);
    setup_des_key(keys + 7, &des);
    gcry_cipher_encrypt(des, results + 8, 8, plaintext, 8);
    gcry_cipher_close(des);
    gcry_cipher_open(&des, GCRY_CIPHER_DES, GCRY_CIPHER_MODE_ECB, 0);
    setup_des_key(keys + 14, &des);
    gcry_cipher_encrypt(des, results + 16, 8, plaintext, 8);
    gcry_cipher_close(des);
#elif defined(USE_NSS) || defined(USE_MBEDTLS) || defined(USE_SECTRANSP) || defined(USE_OS400CRYPTO) || defined(USE_WIN32_CRYPTO)
    encrypt_des(plaintext, results, keys);
    encrypt_des(plaintext, results + 8, keys + 7);
    encrypt_des(plaintext, results + 16, keys + 14);
#endif
}
CURLcode Curl_ntlm_core_mk_lm_hash(struct Curl_easy *data, const char *password, unsigned char *lmbuffer) {
    CURLcode result;
    unsigned char pw[14];
    static const unsigned char magic[] = { 0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25 };
    size_t len = CURLMIN(strlen(password), 14);
    Curl_strntoupper((char *)pw, password, len);
    memset(&pw[len], 0, 14 - len);
    result = Curl_convert_to_network(data, (char *)pw, 14);
    if (result) return result;
#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
    DES_key_schedule ks;
    setup_des_key(pw, DESKEY(ks));
    DES_ecb_encrypt((DES_cblock *)magic, (DES_cblock *)lmbuffer, DESKEY(ks), DES_ENCRYPT);
    setup_des_key(pw + 7, DESKEY(ks));
    DES_ecb_encrypt((DES_cblock *)magic, (DES_cblock *)(lmbuffer + 8), DESKEY(ks), DES_ENCRYPT);
#elif defined(USE_GNUTLS_NETTLE)
    struct des_ctx des;
    setup_des_key(pw, &des);
    des_encrypt(&des, 8, lmbuffer, magic);
    setup_des_key(pw + 7, &des);
    des_encrypt(&des, 8, lmbuffer + 8, magic);
#elif defined(USE_GNUTLS)
    gcry_cipher_hd_t des;
    gcry_cipher_open(&des, GCRY_CIPHER_DES, GCRY_CIPHER_MODE_ECB, 0);
    setup_des_key(pw, &des);
    gcry_cipher_encrypt(des, lmbuffer, 8, magic, 8);
    gcry_cipher_close(des);
    gcry_cipher_open(&des, GCRY_CIPHER_DES, GCRY_CIPHER_MODE_ECB, 0);
    setup_des_key(pw + 7, &des);
    gcry_cipher_encrypt(des, lmbuffer + 8, 8, magic, 8);
    gcry_cipher_close(des);
#elif defined(USE_NSS) || defined(USE_MBEDTLS) || defined(USE_SECTRANSP) || defined(USE_OS400CRYPTO) || defined(USE_WIN32_CRYPTO)
    encrypt_des(magic, lmbuffer, pw);
    encrypt_des(magic, lmbuffer + 8, pw + 7);
#endif
    memset(lmbuffer + 16, 0, 21 - 16);
    return CURLE_OK;
}
#ifdef USE_NTRESPONSES
static void ascii_to_unicode_le(unsigned char *dest, const char *src, size_t srclen) {
    size_t i;
    for(i = 0; i < srclen; i++) {
        dest[2 * i] = (unsigned char)src[i];
        dest[2 * i + 1] = '\0';
    }
}
#if defined(USE_NTLM_V2) && !defined(USE_WINDOWS_SSPI)
static void ascii_uppercase_to_unicode_le(unsigned char *dest, const char *src, size_t srclen) {
    size_t i;
    for(i = 0; i < srclen; i++) {
        dest[2 * i] = (unsigned char)(Curl_raw_toupper(src[i]));
        dest[2 * i + 1] = '\0';
    }
}
#endif
CURLcode Curl_ntlm_core_mk_nt_hash(struct Curl_easy *data,const char *password, unsigned char *ntbuffer) {
    size_t len = strlen(password);
    unsigned char *pw;
    CURLcode result;
    if (len > SIZE_T_MAX/2) return CURLE_OUT_OF_MEMORY;
    pw = len ? malloc(len * 2) : (unsigned char *)strdup("");
    if (!pw) return CURLE_OUT_OF_MEMORY;
    ascii_to_unicode_le(pw, password, len);
    result = Curl_convert_to_network(data, (char *)pw, len * 2);
    if (result) return result;
    Curl_md4it(ntbuffer, pw, 2 * len);
    memset(ntbuffer + 16, 0, 21 - 16);
    free(pw);
    return CURLE_OK;
}
#if defined(USE_NTLM_V2) && !defined(USE_WINDOWS_SSPI)
CURLcode Curl_ntlm_core_mk_ntlmv2_hash(const char *user, size_t userlen, const char *domain, size_t domlen, unsigned char *ntlmhash,
                                       unsigned char *ntlmv2hash) {
    size_t identity_len;
    unsigned char *identity;
    CURLcode result = CURLE_OK;
    if((userlen > SIZE_T_MAX/2) || (domlen > SIZE_T_MAX/2) || ((userlen + domlen) > SIZE_T_MAX/2)) return CURLE_OUT_OF_MEMORY;
    identity_len = (userlen + domlen) * 2;
    identity = malloc(identity_len);
    if (!identity) return CURLE_OUT_OF_MEMORY;
    ascii_uppercase_to_unicode_le(identity, user, userlen);
    ascii_to_unicode_le(identity + (userlen << 1), domain, domlen);
    result = Curl_hmacit(Curl_HMAC_MD5, ntlmhash, 16, identity, identity_len, ntlmv2hash);
    free(identity);
    return result;
}
CURLcode Curl_ntlm_core_mk_ntlmv2_resp(unsigned char *ntlmv2hash, unsigned char *challenge_client, struct ntlmdata *ntlm, unsigned char **ntresp,
                                       unsigned int *ntresp_len) {
    unsigned int len = 0;
    unsigned char *ptr = NULL;
    unsigned char hmac_output[HMAC_MD5_LENGTH];
    curl_off_t tw;
    CURLcode result = CURLE_OK;
#if CURL_SIZEOF_CURL_OFF_T < 8
    #error "this section needs 64bit support to work"
#endif
#ifdef DEBUGBUILD
    char *force_timestamp = getenv("CURL_FORCETIME");
    if (force_timestamp) tw = CURL_OFF_T_C(11644473600) * 10000000;
    else tw = ((curl_off_t)time(NULL) + CURL_OFF_T_C(11644473600)) * 10000000;
#endif
    len = HMAC_MD5_LENGTH + NTLMv2_BLOB_LEN;
    ptr = calloc(1, len);
    if (!ptr) return CURLE_OUT_OF_MEMORY;
    msnprintf((char *)ptr + HMAC_MD5_LENGTH, NTLMv2_BLOB_LEN, "%c%c%c%c%c%c%c%c", NTLMv2_BLOB_SIGNATURE[0], NTLMv2_BLOB_SIGNATURE[1],
              NTLMv2_BLOB_SIGNATURE[2], NTLMv2_BLOB_SIGNATURE[3], 0, 0, 0, 0);
    Curl_write64_le(tw, ptr + 24);
    memcpy(ptr + 32, challenge_client, 8);
    memcpy(ptr + 44, ntlm->target_info, ntlm->target_info_len);
    memcpy(ptr + 8, &ntlm->nonce[0], 8);
    result = Curl_hmacit(Curl_HMAC_MD5, ntlmv2hash, HMAC_MD5_LENGTH, ptr + 8, NTLMv2_BLOB_LEN + 8, hmac_output);
    if (result) {
        free(ptr);
        return result;
    }
    memcpy(ptr, hmac_output, HMAC_MD5_LENGTH);
    *ntresp = ptr;
    *ntresp_len = len;
    return result;
}
CURLcode  Curl_ntlm_core_mk_lmv2_resp(unsigned char *ntlmv2hash, unsigned char *challenge_client, unsigned char *challenge_server, unsigned char *lmresp) {
    unsigned char data[16];
    unsigned char hmac_output[16];
    CURLcode result = CURLE_OK;
    memcpy(&data[0], challenge_server, 8);
    memcpy(&data[8], challenge_client, 8);
    result = Curl_hmacit(Curl_HMAC_MD5, ntlmv2hash, 16, &data[0], 16, hmac_output);
    if (result) return result;
    memcpy(lmresp, hmac_output, 16);
    memcpy(lmresp + 16, challenge_client, 8);
    return result;
}
#endif
#endif
#endif
#endif