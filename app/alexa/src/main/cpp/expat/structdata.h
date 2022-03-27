#ifdef __cplusplus
extern "C" {
#endif
#ifndef XML_STRUCTDATA_H
#define XML_STRUCTDATA_H 1
#include "expat.h"
typedef struct {
  const XML_Char *str;
  int data0;
  int data1;
  int data2;
} StructDataEntry;
typedef struct {
  int count;
  int max_count;
  StructDataEntry *entries;
} StructData;
void StructData_Init(StructData *storage);
void StructData_AddItem(StructData *storage, const XML_Char *s, int data0, int data1, int data2);
void StructData_CheckItems(StructData *storage, const StructDataEntry *expected, int count);
void StructData_Dispose(StructData *storage);
#endif
#ifdef __cplusplus
}
#endif