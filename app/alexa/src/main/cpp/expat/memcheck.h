#ifdef __cplusplus
extern "C" {
#endif
#ifndef XML_MEMCHECK_H
#define XML_MEMCHECK_H 1
typedef unsigned int size_t;
void *tracking_malloc(size_t size);
void tracking_free(void *ptr);
void *tracking_realloc(void *ptr, size_t size);
int tracking_report(void);
#endif
#ifdef __cplusplus
}
#endif