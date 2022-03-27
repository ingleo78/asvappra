#ifdef __cplusplus
extern "C" {
#endif
#ifndef XML_CHARDATA_H
#define XML_CHARDATA_H 1
#ifndef XML_VERSION
#include "expat.h"
#endif
typedef struct {
    int count;
    XML_Char data[2048];
} CharData;
void CharData_Init(CharData *storage);
void CharData_AppendXMLChars(CharData *storage, const XML_Char *s, int len);
int CharData_CheckXMLChars(CharData *storage, const XML_Char *s);
#endif
#ifdef __cplusplus
}
#endif