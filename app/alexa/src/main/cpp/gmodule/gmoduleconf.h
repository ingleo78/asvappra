#ifndef __G_MODULE_CONF_H__
#define __G_MODULE_CONF_H__

#ifdef __cplusplus
//extern "C" {
#endif
#define	G_MODULE_IMPL_NONE	0
#define	G_MODULE_IMPL_DL	1
#define	G_MODULE_IMPL_DLD	2
#define	G_MODULE_IMPL_WIN32	3
#define	G_MODULE_IMPL_OS2	4
#define	G_MODULE_IMPL_BEOS	5
#define	G_MODULE_IMPL_DYLD	6
#define	G_MODULE_IMPL_AR	7
//#define	G_MODULE_IMPL		@G_MODULE_IMPL@
#undef	G_MODULE_HAVE_DLERROR
/*
#if	(@G_MODULE_HAVE_DLERROR@)
#define	G_MODULE_HAVE_DLERROR
#endif
#if	(@G_MODULE_NEED_USCORE@) || defined (hp9000s300) || defined (__hp9000s300) || defined (__hp9000s300__)
#define	G_MODULE_NEED_USCORE
#endif
#if	(@G_MODULE_BROKEN_RTLD_GLOBAL@)
#define G_MODULE_BROKEN_RTLD_GLOBAL
#endif
*/
#ifdef __cplusplus
//}
#endif
#endif