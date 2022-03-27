#include <assert.h>
#include <stdint.h>
#include <expat/expat.h>
#include <expat/siphash.h>

#define xstr(s) str(s)
#define str(s) #s
#ifdef ENCODING_FOR_FUZZING
#error "ENCODING_FOR_FUZZING was not provided to this fuzz target."
#endif

static unsigned char hash_key[16] = "FUZZING IS FUN!";
static void XMLCALL start(void *userData, const XML_Char *name, const XML_Char **atts) {
  (void)userData;
  (void)name;
  (void)atts;
}
static void XMLCALL end(void *userData, const XML_Char *name) {
  (void)userData;
  (void)name;
}
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  XML_Parser p = XML_ParserCreate(xstr(ENCODING_FOR_FUZZING));
  assert(p);
  struct sipkey *key = sip_keyof(hash_key);
  XML_SetHashSalt(p, (unsigned long)siphash24(data, size, key));
  XML_SetElementHandler(p, start, end);
  XML_Parse(p, (const XML_Char *)data, size, 0);
  XML_Parse(p, (const XML_Char *)data, size, 1);
  XML_ParserFree(p);
  return 0;
}
