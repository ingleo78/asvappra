#include <expat/expat.h>

#define XML_MAP_FILE 01
#define XML_EXTERNAL_ENTITIES 02

#ifdef XML_LARGE_SIZE
#define XML_FMT_INT_MOD "ll"
#else
#define XML_FMT_INT_MOD "l"
#endif

extern int XML_ProcessFile(XML_Parser parser, const XML_Char *filename, unsigned flags);