#ifndef __G_MARSHAL_H__
#define __G_MARSHAL_H__

#include <stdarg.h>
#include "gtype.h"
#include "gclosure.h"

G_BEGIN_DECLS
#define GLIB_AVAILABLE_IN_ALL extern
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__VOID(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                         gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__VOIDv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                          int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__BOOLEAN(GClosure *closure, GValue *return_value, guint  n_param_values, const GValue *param_values,
                                                            gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__BOOLEANv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                             int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__CHAR(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                         gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__CHARv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                          int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__UCHAR(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                          gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__UCHARv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                           int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__INT(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                        gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__INTv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                         int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__UINT(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                         gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__UINTv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                          int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__LONG(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                         gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__LONGv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                          int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__ULONG(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                          gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__ULONGv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                           int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__ENUM(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                         gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__ENUMv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                          int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__FLAGS(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                          gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__FLAGSv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                           int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__FLOAT(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                          gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__FLOATv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                           int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__DOUBLE(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                           gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__DOUBLEv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                            int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__STRING(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                           gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__STRINGv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                            int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__PARAM(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                          gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__PARAMv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                           int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__BOXED(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                          gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__BOXEDv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                           int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__POINTER(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                            gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__POINTERv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                             int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__OBJECT(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                           gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__OBJECTv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                            int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__VARIANT(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                            gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__VARIANTv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                             int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__UINT_POINTER(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                                 gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_VOID__UINT_POINTERv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                                  int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_BOOLEAN__FLAGS(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                             gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_BOOLEAN__FLAGSv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                              int n_params, GType *param_types);
#define g_cclosure_marshal_BOOL__FLAGS  g_cclosure_marshal_BOOLEAN__FLAGS
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_STRING__OBJECT_POINTER(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                                     gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_STRING__OBJECT_POINTERv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                                      int n_params, GType *param_types);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_BOOLEAN__BOXED_BOXED(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values,
                                                                   gpointer invocation_hint, gpointer marshal_data);
GLIB_AVAILABLE_IN_ALL void g_cclosure_marshal_BOOLEAN__BOXED_BOXEDv(GClosure *closure, GValue *return_value, gpointer instance, va_list args, gpointer marshal_data,
                                                                    int n_params, GType *param_types);
#define g_cclosure_marshal_BOOL__BOXED_BOXED   g_cclosure_marshal_BOOLEAN__BOXED_BOXED
G_END_DECLS
#endif