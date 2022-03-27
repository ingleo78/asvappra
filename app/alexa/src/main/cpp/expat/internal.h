#if defined(__GNUC__) && defined(__i386__) && ! defined(__MINGW32__)
#define FASTCALL __attribute__((regparm(3)))
#define PTRFASTCALL __attribute__((regparm(3)))
#endif
#ifndef FASTCALL
#define FASTCALL
#endif
#ifndef PTRCALL
#define PTRCALL
#endif
#ifndef PTRFASTCALL
#define PTRFASTCALL
#endif
#ifndef XML_MIN_SIZE
#if ! defined(__cplusplus) && ! defined(inline)
#ifdef __GNUC__
#define inline __inline
#endif
#endif
#endif
#ifdef __cplusplus
#define inline inline
#else
#ifndef inline
#define inline
#endif
#endif
#ifndef UNUSED_P
#define UNUSED_P(p) (void)p
#endif
#ifdef __cplusplus
extern "C" {
#endif
#ifndef XML_ENABLE_VISIBILITY
#if !XML_ENABLE_VISIBILITY
__attribute__((visibility("default")))
#endif
#endif
void _INTERNAL_trim_to_complete_utf8_characters(const char *from, const char **fromLimRef);
#ifdef __cplusplus
}
#endif