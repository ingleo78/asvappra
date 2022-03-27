#ifndef __GST_CONFIG_H__
#define __GST_CONFIG_H__

#if 0
#define GST_DISABLE_GST_DEBUG 1
#define GST_DISABLE_PARSE 1
#define GST_DISABLE_TRACE 1
#define GST_DISABLE_ALLOC_TRACE 1
#define GST_DISABLE_REGISTRY 1
#define GST_DISABLE_PLUGIN 1
#define GST_HAVE_GLIB_2_8 1
#endif
#define GST_PADDING		4
#define GST_PADDING_INIT	{ NULL }

#define GST_PADDING_LARGE	20
#ifdef _MSC_VER
#define GST_PLUGIN_EXPORT __declspec(dllexport) extern
#ifdef GST_EXPORTS
#define GST_EXPORT __declspec(dllexport) extern
#else
#define GST_EXPORT __declspec(dllimport) extern
#endif
#else
#define GST_PLUGIN_EXPORT
#if (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
#define GST_EXPORT extern __attribute__ ((visibility ("default")))
#else
#define GST_EXPORT extern
#endif
#endif

#endif