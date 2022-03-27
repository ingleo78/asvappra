#include "curl_setup.h"
#if defined(USE_GSKIT) || defined(USE_NSS) || defined(USE_GNUTLS) || defined(USE_WOLFSSL) || defined(USE_SCHANNEL)
#include "curl.h"
#include "urldata.h"
#include "strcase.h"
#include "hostcheck.h"
#include "vtls/vtls.h"
#include "sendf.h"
#include "inet_pton.h"
#include "curl_base64.h"
#include "x509asn1.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static const char       cnOID[] = "2.5.4.3";
static const char       sanOID[] = "2.5.29.17";
static const struct Curl_OID OIDtable[] = {
    { "1.2.840.10040.4.1",        "dsa" },
    { "1.2.840.10040.4.3",        "dsa-with-sha1" },
    { "1.2.840.10045.2.1",        "ecPublicKey" },
    { "1.2.840.10045.3.0.1",      "c2pnb163v1" },
    { "1.2.840.10045.4.1",        "ecdsa-with-SHA1" },
    { "1.2.840.10046.2.1",        "dhpublicnumber" },
    { "1.2.840.113549.1.1.1",     "rsaEncryption" },
    { "1.2.840.113549.1.1.2",     "md2WithRSAEncryption" },
    { "1.2.840.113549.1.1.4",     "md5WithRSAEncryption" },
    { "1.2.840.113549.1.1.5",     "sha1WithRSAEncryption" },
    { "1.2.840.113549.1.1.10",    "RSASSA-PSS" },
    { "1.2.840.113549.1.1.14",    "sha224WithRSAEncryption" },
    { "1.2.840.113549.1.1.11",    "sha256WithRSAEncryption" },
    { "1.2.840.113549.1.1.12",    "sha384WithRSAEncryption" },
    { "1.2.840.113549.1.1.13",    "sha512WithRSAEncryption" },
    { "1.2.840.113549.2.2",       "md2" },
    { "1.2.840.113549.2.5",       "md5" },
    { "1.3.14.3.2.26",            "sha1" },
    { cnOID,                      "CN" },
    { "2.5.4.4",                  "SN" },
    { "2.5.4.5",                  "serialNumber" },
    { "2.5.4.6",                  "C" },
    { "2.5.4.7",                  "L" },
    { "2.5.4.8",                  "ST" },
    { "2.5.4.9",                  "streetAddress" },
    { "2.5.4.10",                 "O" },
    { "2.5.4.11",                 "OU" },
    { "2.5.4.12",                 "title" },
    { "2.5.4.13",                 "description" },
    { "2.5.4.17",                 "postalCode" },
    { "2.5.4.41",                 "name" },
    { "2.5.4.42",                 "givenName" },
    { "2.5.4.43",                 "initials" },
    { "2.5.4.44",                 "generationQualifier" },
    { "2.5.4.45",                 "X500UniqueIdentifier" },
    { "2.5.4.46",                 "dnQualifier" },
    { "2.5.4.65",                 "pseudonym" },
    { "1.2.840.113549.1.9.1",     "emailAddress" },
    { "2.5.4.72",                 "role" },
    { sanOID,                     "subjectAltName" },
    { "2.5.29.18",                "issuerAltName" },
    { "2.5.29.19",                "basicConstraints" },
    { "2.16.840.1.101.3.4.2.4",   "sha224" },
    { "2.16.840.1.101.3.4.2.1",   "sha256" },
    { "2.16.840.1.101.3.4.2.2",   "sha384" },
    { "2.16.840.1.101.3.4.2.3",   "sha512" },
    { (const char *) NULL,        (const char *) NULL }
};
static const char *getASN1Element(struct Curl_asn1Element *elem, const char *beg, const char *end) WARN_UNUSED_RESULT;
static const char *getASN1Element(struct Curl_asn1Element *elem, const char *beg, const char *end) {
    unsigned char b;
    unsigned long len;
    struct Curl_asn1Element lelem;
    if(!beg || !end || beg >= end || !*beg || (size_t)(end - beg) > CURL_ASN1_MAX) return NULL;
    elem->header = beg;
    b = (unsigned char) *beg++;
    elem->constructed = (b & 0x20) != 0;
    elem->class = (b >> 6) & 3;
    b &= 0x1F;
    if(b == 0x1F) return NULL;
    elem->tag = b;
    if(beg >= end) return NULL;
    b = (unsigned char) *beg++;
    if(!(b & 0x80)) len = b;
    else if(!(b &= 0x7F)) {
        if(!elem->constructed) return NULL;
        elem->beg = beg;
        while(beg < end && *beg) {
            beg = getASN1Element(&lelem, beg, end);
            if(!beg) return NULL;
        }
        if(beg >= end) return NULL;
        elem->end = beg;
        return beg + 1;
    }
    else if((unsigned)b > (size_t)(end - beg)) return NULL;
    else {
        len = 0;
        do {
            if(len & 0xFF000000L) return NULL;
            len = (len << 8) | (unsigned char) *beg++;
        } while(--b);
    }
    if(len > (size_t)(end - beg)) return NULL;
    elem->beg = beg;
    elem->end = beg + len;
    return elem->end;
}
static const struct Curl_OID *searchOID(const char *oid) {
    const struct Curl_OID *op;
    for(op = OIDtable; op->numoid; op++)
        if(!strcmp(op->numoid, oid) || strcasecompare(op->textoid, oid)) return op;
    return NULL;
}
static const char *bool2str(const char *beg, const char *end) {
    if(end - beg != 1) return NULL;
    return strdup(*beg? "TRUE": "FALSE");
}
static const char *octet2str(const char *beg, const char *end) {
    size_t n = end - beg;
    char *buf = NULL;
    if(n <= (SIZE_T_MAX - 1) / 3) {
        buf = malloc(3 * n + 1);
        if(buf)
            for(n = 0; beg < end; n += 3)
                msnprintf(buf + n, 4, "%02x:", *(const unsigned char *) beg++);
    }
    return buf;
}
static const char *bit2str(const char *beg, const char *end) {
    if(++beg > end) return NULL;
    return octet2str(beg, end);
}
static const char *int2str(const char *beg, const char *end) {
    unsigned long val = 0;
    size_t n = end - beg;
    if(!n) return NULL;
    if(n > 4) return octet2str(beg, end);
    if(*beg & 0x80) val = ~val;
    do {
        val = (val << 8) | *(const unsigned char *) beg++;
    } while(beg < end);
    return curl_maprintf("%s%lx", val >= 10? "0x": "", val);
}
static ssize_t utf8asn1str(char **to, int type, const char *from, const char *end) {
    size_t inlength = end - from;
    int size = 1;
    size_t outlength;
    char *buf;
    *to = NULL;
    switch(type) {
        case CURL_ASN1_BMP_STRING: size = 2; break;
        case CURL_ASN1_UNIVERSAL_STRING: size = 4; break;
        case CURL_ASN1_NUMERIC_STRING: case CURL_ASN1_PRINTABLE_STRING: case CURL_ASN1_TELETEX_STRING: case CURL_ASN1_IA5_STRING:
        case CURL_ASN1_VISIBLE_STRING: case CURL_ASN1_UTF8_STRING:
            break;
        default: return -1;
    }
    if(inlength % size) return -1;
    if(inlength / size > (SIZE_T_MAX - 1) / 4) return -1;
    buf = malloc(4 * (inlength / size) + 1);
    if(!buf) return -1;
    if(type == CURL_ASN1_UTF8_STRING) {
        outlength = inlength;
        if(outlength) memcpy(buf, from, outlength);
    } else {
        for(outlength = 0; from < end;) {
            int charsize;
            unsigned int wc;
            wc = 0;
            switch(size) {
                case 4:
                    wc = (wc << 8) | *(const unsigned char *) from++;
                    wc = (wc << 8) | *(const unsigned char *) from++;
                case 2:
                    wc = (wc << 8) | *(const unsigned char *) from++;
                    default: wc = (wc << 8) | *(const unsigned char *) from++;
            }
            charsize = 1;
            if(wc >= 0x00000080) {
                if(wc >= 0x00000800) {
                    if(wc >= 0x00010000) {
                    if(wc >= 0x00200000) {
                        free(buf);
                        return -1;
                    }
                    buf[outlength + 3] = (char) (0x80 | (wc & 0x3F));
                    wc = (wc >> 6) | 0x00010000;
                    charsize++;
                    }
                    buf[outlength + 2] = (char) (0x80 | (wc & 0x3F));
                    wc = (wc >> 6) | 0x00000800;
                    charsize++;
                }
                buf[outlength + 1] = (char) (0x80 | (wc & 0x3F));
                wc = (wc >> 6) | 0x000000C0;
                charsize++;
            }
            buf[outlength] = (char) wc;
            outlength += charsize;
        }
    }
    buf[outlength] = '\0';
    *to = buf;
    return outlength;
}
static const char *string2str(int type, const char *beg, const char *end) {
    char *buf;
    if(utf8asn1str(&buf, type, beg, end) < 0) return NULL;
    return buf;
}
static size_t encodeUint(char *buf, size_t buflen, unsigned int x) {
    size_t i = 0;
    unsigned int y = x / 10;
    if(y) {
        i = encodeUint(buf, buflen, y);
        x -= y * 10;
    }
    if(i < buflen) buf[i] = (char) ('0' + x);
    i++;
    if(i < buflen) buf[i] = '\0';
    return i;
}
static size_t encodeOID(char *buf, size_t buflen, const char *beg, const char *end) {
    size_t i;
    unsigned int x;
    unsigned int y;
    y = *(const unsigned char *) beg++;
    x = y / 40;
    y -= x * 40;
    i = encodeUint(buf, buflen, x);
    if(i < buflen) buf[i] = '.';
    i++;
    if(i >= buflen) i += encodeUint(NULL, 0, y);
    else i += encodeUint(buf + i, buflen - i, y);
    while(beg < end) {
        if(i < buflen) buf[i] = '.';
        i++;
        x = 0;
        do {
            if(x & 0xFF000000) return 0;
            y = *(const unsigned char *) beg++;
            x = (x << 7) | (y & 0x7F);
        } while(y & 0x80);
        if(i >= buflen) i += encodeUint(NULL, 0, x);
        else i += encodeUint(buf + i, buflen - i, x);
    }
    if(i < buflen) buf[i] = '\0';
    return i;
}
static const char *OID2str(const char *beg, const char *end, bool symbolic) {
    char *buf = NULL;
    if(beg < end) {
        size_t buflen = encodeOID(NULL, 0, beg, end);
        if(buflen) {
            buf = malloc(buflen + 1); /* one extra for the zero byte */
            if(buf) {
                encodeOID(buf, buflen, beg, end);
                buf[buflen] = '\0';
                if(symbolic) {
                    const struct Curl_OID *op = searchOID(buf);
                    if(op) {
                    free(buf);
                    buf = strdup(op->textoid);
                    }
                }
            }
        }
    }
    return buf;
}
static const char *GTime2str(const char *beg, const char *end) {
    const char *tzp;
    const char *fracp;
    char sec1, sec2;
    size_t fracl;
    size_t tzl;
    const char *sep = "";
    for(fracp = beg; fracp < end && *fracp >= '0' && *fracp <= '9'; fracp++);
    sec1 = '0';
    switch(fracp - beg - 12) {
        case 0: sec2 = '0'; break;
        case 2: sec1 = fracp[-2];
        case 1: sec2 = fracp[-1]; break;
        default: return NULL;
    }
    tzp = fracp;
    fracl = 0;
    if(fracp < end && (*fracp == '.' || *fracp == ',')) {
        fracp++;
        do {
            tzp++;
        } while(tzp < end && *tzp >= '0' && *tzp <= '9');
        for(fracl = tzp - fracp - 1; fracl && fracp[fracl - 1] == '0'; fracl--);
    }
    if(tzp >= end);
    else if(*tzp == 'Z') {
        tzp = " GMT";
        end = tzp + 4;
    } else {
        sep = " ";
        tzp++;
    }
    tzl = end - tzp;
    return curl_maprintf("%.4s-%.2s-%.2s %.2s:%.2s:%c%c%s%.*s%s%.*s", beg, beg + 4, beg + 6, beg + 8, beg + 10, sec1, sec2,
                         fracl? ".": "", fracl, fracp, sep, tzl, tzp);
}
static const char *UTime2str(const char *beg, const char *end) {
    const char *tzp;
    size_t tzl;
    const char *sec;
    for(tzp = beg; tzp < end && *tzp >= '0' && *tzp <= '9'; tzp++);
    sec = beg + 10;
    switch(tzp - sec) {
        case 0: sec = "00";
        case 2: break;
        default: return NULL;
    }
    if(tzp >= end) return NULL;
    if(*tzp == 'Z') {
        tzp = "GMT";
        end = tzp + 3;
    } else tzp++;
    tzl = end - tzp;
    return curl_maprintf("%u%.2s-%.2s-%.2s %.2s:%.2s:%.2s %.*s", 20 - (*beg >= '5'), beg, beg + 2, beg + 4, beg + 6, beg + 8, sec, tzl, tzp);
}
static const char *ASN1tostr(struct Curl_asn1Element *elem, int type) {
    if(elem->constructed) return NULL;
    if(!type) type = elem->tag;
    switch(type) {
        case CURL_ASN1_BOOLEAN: return bool2str(elem->beg, elem->end);
        case CURL_ASN1_INTEGER: case CURL_ASN1_ENUMERATED: return int2str(elem->beg, elem->end);
        case CURL_ASN1_BIT_STRING: return bit2str(elem->beg, elem->end);
        case CURL_ASN1_OCTET_STRING: return octet2str(elem->beg, elem->end);
        case CURL_ASN1_NULL: return strdup("");
        case CURL_ASN1_OBJECT_IDENTIFIER: return OID2str(elem->beg, elem->end, TRUE);
        case CURL_ASN1_UTC_TIME: return UTime2str(elem->beg, elem->end);
        case CURL_ASN1_GENERALIZED_TIME: return GTime2str(elem->beg, elem->end);
        case CURL_ASN1_UTF8_STRING: case CURL_ASN1_NUMERIC_STRING: case CURL_ASN1_PRINTABLE_STRING: case CURL_ASN1_TELETEX_STRING: case CURL_ASN1_IA5_STRING:
        case CURL_ASN1_VISIBLE_STRING: case CURL_ASN1_UNIVERSAL_STRING: case CURL_ASN1_BMP_STRING:
            return string2str(type, elem->beg, elem->end);
    }
    return NULL;
}
static ssize_t encodeDN(char *buf, size_t buflen, struct Curl_asn1Element *dn) {
    struct Curl_asn1Element rdn;
    struct Curl_asn1Element atv;
    struct Curl_asn1Element oid;
    struct Curl_asn1Element value;
    size_t l = 0;
    const char *p1;
    const char *p2;
    const char *p3;
    const char *str;
    for(p1 = dn->beg; p1 < dn->end;) {
        p1 = getASN1Element(&rdn, p1, dn->end);
        if(!p1) return -1;
        for(p2 = rdn.beg; p2 < rdn.end;) {
            p2 = getASN1Element(&atv, p2, rdn.end);
            if(!p2) return -1;
            p3 = getASN1Element(&oid, atv.beg, atv.end);
            if(!p3) return -1;
            if(!getASN1Element(&value, p3, atv.end)) return -1;
            str = ASN1tostr(&oid, 0);
            if(!str) return -1;
            if(l) {
                for(p3 = str; isupper(*p3); p3++);
                for(p3 = (*p3 || p3 - str > 2)? "/": ", "; *p3; p3++) {
                    if(l < buflen) buf[l] = *p3;
                    l++;
                }
            }
            for(p3 = str; *p3; p3++) {
                if(l < buflen) buf[l] = *p3;
                l++;
            }
            free((char*)str);
            if(l < buflen) buf[l] = '=';
            l++;
            str = ASN1tostr(&value, 0);
            if(!str) return -1;
            for(p3 = str; *p3; p3++) {
                if(l < buflen) buf[l] = *p3;
                l++;
            }
            free((char *) str);
        }
    }
    return l;
}
static const char *DNtostr(struct Curl_asn1Element *dn) {
    char *buf = NULL;
    ssize_t buflen = encodeDN(NULL, 0, dn);
    if(buflen >= 0) {
        buf = malloc(buflen + 1);
        if(buf) {
            encodeDN(buf, buflen + 1, dn);
            buf[buflen] = '\0';
        }
    }
    return buf;
}
int Curl_parseX509(struct Curl_X509certificate *cert, const char *beg, const char *end) {
    struct Curl_asn1Element elem;
    struct Curl_asn1Element tbsCertificate;
    const char *ccp;
    static const char defaultVersion = 0;
    cert->certificate.header = NULL;
    cert->certificate.beg = beg;
    cert->certificate.end = end;
    if(!getASN1Element(&elem, beg, end)) return -1;
    beg = elem.beg;
    end = elem.end;
    beg = getASN1Element(&tbsCertificate, beg, end);
    if(!beg) return -1;
    beg = getASN1Element(&cert->signatureAlgorithm, beg, end);
    if(!beg) return -1;
    if(!getASN1Element(&cert->signature, beg, end)) return -1;
    beg = tbsCertificate.beg;
    end = tbsCertificate.end;
    cert->version.header = NULL;
    cert->version.beg = &defaultVersion;
    cert->version.end = &defaultVersion + sizeof(defaultVersion);
    beg = getASN1Element(&elem, beg, end);
    if(!beg) return -1;
    if(elem.tag == 0) {
        if(!getASN1Element(&cert->version, elem.beg, elem.end)) return -1;
        beg = getASN1Element(&elem, beg, end);
        if(!beg) return -1;
    }
    cert->serialNumber = elem;
    beg = getASN1Element(&cert->signatureAlgorithm, beg, end);
    beg = getASN1Element(&cert->issuer, beg, end);
    if(!beg) return -1;
    beg = getASN1Element(&elem, beg, end);
    if(!beg) return -1;
    ccp = getASN1Element(&cert->notBefore, elem.beg, elem.end);
    if(!ccp) return -1;
    if(!getASN1Element(&cert->notAfter, ccp, elem.end)) return -1;
    beg = getASN1Element(&cert->subject, beg, end);
    if(!beg) return -1;
    beg = getASN1Element(&cert->subjectPublicKeyInfo, beg, end);
    if(!beg) return -1;
    ccp = getASN1Element(&cert->subjectPublicKeyAlgorithm, cert->subjectPublicKeyInfo.beg, cert->subjectPublicKeyInfo.end);
    if(!ccp) return -1;
    if(!getASN1Element(&cert->subjectPublicKey, ccp, cert->subjectPublicKeyInfo.end)) return -1;
    cert->issuerUniqueID.tag = cert->subjectUniqueID.tag = 0;
    cert->extensions.tag = elem.tag = 0;
    cert->issuerUniqueID.header = cert->subjectUniqueID.header = NULL;
    cert->issuerUniqueID.beg = cert->issuerUniqueID.end = "";
    cert->subjectUniqueID.beg = cert->subjectUniqueID.end = "";
    cert->extensions.header = NULL;
    cert->extensions.beg = cert->extensions.end = "";
    if(beg < end) {
        beg = getASN1Element(&elem, beg, end);
        if(!beg) return -1;
    }
    if(elem.tag == 1) {
        cert->issuerUniqueID = elem;
        if(beg < end) {
            beg = getASN1Element(&elem, beg, end);
            if(!beg) return -1;
        }
    }
    if(elem.tag == 2) {
        cert->subjectUniqueID = elem;
        if(beg < end) {
            beg = getASN1Element(&elem, beg, end);
            if(!beg) return -1;
        }
    }
    if(elem.tag == 3)
        if(!getASN1Element(&cert->extensions, elem.beg, elem.end)) return -1;
    return 0;
}
static size_t copySubstring(char *to, const char *from) {
    size_t i;
    for(i = 0; i < 64; i++) {
        to[i] = *from;
        if(!*from++) break;
    }
    to[i++] = '\n';
    return i;
}
static const char *dumpAlgo(struct Curl_asn1Element *param, const char *beg, const char *end) {
    struct Curl_asn1Element oid;
    beg = getASN1Element(&oid, beg, end);
    if(!beg) return NULL;
    param->header = NULL;
    param->tag = 0;
    param->beg = param->end = end;
    if(beg < end)
        if(!getASN1Element(param, beg, end)) return NULL;
    return OID2str(oid.beg, oid.end, TRUE);
}
static void do_pubkey_field(struct Curl_easy *data, int certnum, const char *label, struct Curl_asn1Element *elem) {
    const char *output;
    output = ASN1tostr(elem, 0);
    if(output) {
        if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, label, output);
        if(!certnum) infof(data, "   %s: %s\n", label, output);
        free((char *) output);
    }
}
static void do_pubkey(struct Curl_easy *data, int certnum, const char *algo, struct Curl_asn1Element *param, struct Curl_asn1Element *pubkey) {
    struct Curl_asn1Element elem;
    struct Curl_asn1Element pk;
    const char *p;
    if(!getASN1Element(&pk, pubkey->beg + 1, pubkey->end)) return;
    if(strcasecompare(algo, "rsaEncryption")) {
        const char *q;
        unsigned long len;
        p = getASN1Element(&elem, pk.beg, pk.end);
        if(!p) return;
        for(q = elem.beg; !*q && q < elem.end; q++);
        len = (unsigned long)((elem.end - q) * 8);
        if(len) {
            unsigned int i;
            for(i = *(unsigned char *) q; !(i & 0x80); i <<= 1) len--;
        }
        if(len > 32) elem.beg = q;
        if(!certnum) infof(data, "   RSA Public Key (%lu bits)\n", len);
        if(data->set.ssl.certinfo) {
            q = curl_maprintf("%lu", len);
            if(q) {
                Curl_ssl_push_certinfo(data, certnum, "RSA Public Key", q);
                free((char *) q);
            }
        }
        do_pubkey_field(data, certnum, "rsa(n)", &elem);
        if(!getASN1Element(&elem, p, pk.end)) return;
        do_pubkey_field(data, certnum, "rsa(e)", &elem);
    } else if(strcasecompare(algo, "dsa")) {
        p = getASN1Element(&elem, param->beg, param->end);
        if(p) {
            do_pubkey_field(data, certnum, "dsa(p)", &elem);
            p = getASN1Element(&elem, p, param->end);
            if(p) {
                do_pubkey_field(data, certnum, "dsa(q)", &elem);
                if(getASN1Element(&elem, p, param->end)) {
                    do_pubkey_field(data, certnum, "dsa(g)", &elem);
                    do_pubkey_field(data, certnum, "dsa(pub_key)", &pk);
                }
            }
        }
    } else if(strcasecompare(algo, "dhpublicnumber")) {
        p = getASN1Element(&elem, param->beg, param->end);
        if(p) {
            do_pubkey_field(data, certnum, "dh(p)", &elem);
            if(getASN1Element(&elem, param->beg, param->end)) {
                do_pubkey_field(data, certnum, "dh(g)", &elem);
                do_pubkey_field(data, certnum, "dh(pub_key)", &pk);
            }
        }
    }
}
CURLcode Curl_extract_certinfo(struct connectdata *conn, int certnum, const char *beg, const char *end) {
    struct Curl_X509certificate cert;
    struct Curl_easy *data = conn->data;
    struct Curl_asn1Element param;
    const char *ccp;
    char *cp1;
    size_t cl1;
    char *cp2;
    CURLcode result;
    unsigned long version;
    size_t i;
    size_t j;
    if(!data->set.ssl.certinfo) if(certnum) return CURLE_OK;
    if(Curl_parseX509(&cert, beg, end)) return CURLE_PEER_FAILED_VERIFICATION;
    ccp = DNtostr(&cert.subject);
    if(!ccp) return CURLE_OUT_OF_MEMORY;
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Subject", ccp);
    if(!certnum) infof(data, "%2d Subject: %s\n", certnum, ccp);
    free((char *) ccp);
    ccp = DNtostr(&cert.issuer);
    if(!ccp) return CURLE_OUT_OF_MEMORY;
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Issuer", ccp);
    if(!certnum) infof(data, "   Issuer: %s\n", ccp);
    free((char *) ccp);
    version = 0;
    for(ccp = cert.version.beg; ccp < cert.version.end; ccp++) version = (version << 8) | *(const unsigned char *) ccp;
    if(data->set.ssl.certinfo) {
        ccp = curl_maprintf("%lx", version);
        if(!ccp) return CURLE_OUT_OF_MEMORY;
        Curl_ssl_push_certinfo(data, certnum, "Version", ccp);
        free((char *) ccp);
    }
    if(!certnum) infof(data, "   Version: %lu (0x%lx)\n", version + 1, version);
    ccp = ASN1tostr(&cert.serialNumber, 0);
    if(!ccp) return CURLE_OUT_OF_MEMORY;
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Serial Number", ccp);
    if(!certnum) infof(data, "   Serial Number: %s\n", ccp);
    free((char *) ccp);
    ccp = dumpAlgo(&param, cert.signatureAlgorithm.beg, cert.signatureAlgorithm.end);
    if(!ccp) return CURLE_OUT_OF_MEMORY;
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Signature Algorithm", ccp);
    if(!certnum) infof(data, "   Signature Algorithm: %s\n", ccp);
    free((char *) ccp);
    ccp = ASN1tostr(&cert.notBefore, 0);
    if(!ccp) return CURLE_OUT_OF_MEMORY;
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Start Date", ccp);
    if(!certnum) infof(data, "   Start Date: %s\n", ccp);
    free((char *) ccp);
    ccp = ASN1tostr(&cert.notAfter, 0);
    if(!ccp) return CURLE_OUT_OF_MEMORY;
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Expire Date", ccp);
    if(!certnum) infof(data, "   Expire Date: %s\n", ccp);
    free((char *) ccp);
    ccp = dumpAlgo(&param, cert.subjectPublicKeyAlgorithm.beg, cert.subjectPublicKeyAlgorithm.end);
    if(!ccp) return CURLE_OUT_OF_MEMORY;
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Public Key Algorithm", ccp);
    if(!certnum) infof(data, "   Public Key Algorithm: %s\n", ccp);
    do_pubkey(data, certnum, ccp, &param, &cert.subjectPublicKey);
    free((char *) ccp);
    ccp = ASN1tostr(&cert.signature, 0);
    if(!ccp) return CURLE_OUT_OF_MEMORY;
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Signature", ccp);
    if(!certnum) infof(data, "   Signature: %s\n", ccp);
    free((char *) ccp);
    result = Curl_base64_encode(data, cert.certificate.beg, cert.certificate.end - cert.certificate.beg, &cp1, &cl1);
    if(result) return result;
    i = 28 + cl1 + (cl1 + 64 - 1) / 64 + 26;
    cp2 = malloc(i + 1);
    if(!cp2) {
        free(cp1);
        return CURLE_OUT_OF_MEMORY;
    }
    i = copySubstring(cp2, "-----BEGIN CERTIFICATE-----");
    for(j = 0; j < cl1; j += 64) i += copySubstring(cp2 + i, cp1 + j);
    i += copySubstring(cp2 + i, "-----END CERTIFICATE-----");
    cp2[i] = '\0';
    free(cp1);
    if(data->set.ssl.certinfo) Curl_ssl_push_certinfo(data, certnum, "Cert", cp2);
    if(!certnum) infof(data, "%s\n", cp2);
    free(cp2);
    return CURLE_OK;
}
#endif
#if defined(USE_GSKIT)
static const char *checkOID(const char *beg, const char *end, const char *oid) {
    struct Curl_asn1Element e;
    const char *ccp;
    const char *p;
    bool matched;
    ccp = getASN1Element(&e, beg, end);
    if(!ccp || e.tag != CURL_ASN1_OBJECT_IDENTIFIER) return NULL;
    p = OID2str(e.beg, e.end, FALSE);
    if(!p) return NULL;
    matched = !strcmp(p, oid);
    free((char *) p);
    return matched? ccp: NULL;
}
CURLcode Curl_verifyhost(struct connectdata *conn, const char *beg, const char *end) {
    struct Curl_easy *data = conn->data;
    struct Curl_X509certificate cert;
    struct Curl_asn1Element dn;
    struct Curl_asn1Element elem;
    struct Curl_asn1Element ext;
    struct Curl_asn1Element name;
    const char *p;
    const char *q;
    char *dnsname;
    int matched = -1;
    size_t addrlen = (size_t) -1;
    ssize_t len;
    const char *const hostname = SSL_IS_PROXY() ? conn->http_proxy.host.name : conn->host.name;
    const char *const dispname = SSL_IS_PROXY() ? conn->http_proxy.host.dispname : conn->host.dispname;
#ifdef ENABLE_IPV6
    struct in6_addr addr;
#else
    struct in_addr addr;
#endif
    if(!SSL_CONN_CONFIG(verifyhost)) return CURLE_OK;
    if(Curl_parseX509(&cert, beg, end)) return CURLE_PEER_FAILED_VERIFICATION;
#ifdef ENABLE_IPV6
    if(conn->bits.ipv6_ip && Curl_inet_pton(AF_INET6, hostname, &addr)) addrlen = sizeof(struct in6_addr);
    else
#endif
    if(Curl_inet_pton(AF_INET, hostname, &addr)) addrlen = sizeof(struct in_addr);
    for(p = cert.extensions.beg; p < cert.extensions.end && matched != 1;) {
        p = getASN1Element(&ext, p, cert.extensions.end);
        if(!p) return CURLE_PEER_FAILED_VERIFICATION;
        ext.beg = checkOID(ext.beg, ext.end, sanOID);
        if(ext.beg) {
            ext.beg = getASN1Element(&elem, ext.beg, ext.end);
            if(!ext.beg) return CURLE_PEER_FAILED_VERIFICATION;
            if(elem.tag == CURL_ASN1_BOOLEAN) {
                ext.beg = getASN1Element(&elem, ext.beg, ext.end);
                if(!ext.beg) return CURLE_PEER_FAILED_VERIFICATION;
            }
            if(!getASN1Element(&elem, elem.beg, elem.end)) return CURLE_PEER_FAILED_VERIFICATION;
            for(q = elem.beg; matched != 1 && q < elem.end;) {
                q = getASN1Element(&name, q, elem.end);
                if(!q) break;
                switch(name.tag) {
                case 2:
                    len = utf8asn1str(&dnsname, CURL_ASN1_IA5_STRING, name.beg, name.end);
                    if(len > 0 && (size_t)len == strlen(dnsname)) matched = Curl_cert_hostcheck(dnsname, hostname);
                    else matched = 0;
                    free(dnsname);
                    break;
                case 7: matched = (size_t) (name.end - name.beg) == addrlen && !memcmp(&addr, name.beg, addrlen); break;
                }
            }
        }
    }
    switch(matched) {
        case 1:
            infof(data, "\t subjectAltName: %s matched\n", dispname);
            return CURLE_OK;
        case 0:
            infof(data, "\t subjectAltName does not match %s\n", dispname);
            return CURLE_PEER_FAILED_VERIFICATION;
    }
    name.header = NULL;
    name.beg = name.end = "";
    q = cert.subject.beg;
    while(q < cert.subject.end) {
        q = getASN1Element(&dn, q, cert.subject.end);
        if(!q) break;
        for(p = dn.beg; p < dn.end;) {
            p = getASN1Element(&elem, p, dn.end);
            if(!p) return CURLE_PEER_FAILED_VERIFICATION;
            elem.beg = checkOID(elem.beg, elem.end, cnOID);
            if(elem.beg) name = elem;
        }
    }
    if(!getASN1Element(&elem, name.beg, name.end)) failf(data, "SSL: unable to obtain common name from peer certificate");
    else {
        len = utf8asn1str(&dnsname, elem.tag, elem.beg, elem.end);
        if(len < 0) {
            free(dnsname);
            return CURLE_OUT_OF_MEMORY;
        }
        if(strlen(dnsname) != (size_t)len) failf(data, "SSL: illegal cert name field");
        else if(Curl_cert_hostcheck((const char *) dnsname, hostname)) {
            infof(data, "\t common name: %s (matched)\n", dnsname);
            free(dnsname);
            return CURLE_OK;
        } else failf(data, "SSL: certificate subject name '%s' does not match target host name '%s'", dnsname, dispname);
        free(dnsname);
    }
    return CURLE_PEER_FAILED_VERIFICATION;
}
#endif