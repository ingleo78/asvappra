#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_COMPLETION_H__
#define __G_COMPLETION_H__

#include "glist.h"
#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
typedef unsigned int gsize;
typedef struct _GCompletion     GCompletion;
typedef char* (*GCompletionFunc)(gpointer);
typedef gint (*GCompletionStrncmpFunc)(const char *s1, const char *s2, gsize n);
struct _GCompletion {
  GList* items;
  GCompletionFunc func;
  char* prefix;
  GList* cache;
  GCompletionStrncmpFunc strncmp_func;
};
#ifndef G_DISABLE_DEPRECATED
GCompletion* g_completion_new(GCompletionFunc func);
void g_completion_add_items(GCompletion* cmp, GList* items);
void g_completion_remove_items(GCompletion* cmp, GList* items);
void g_completion_clear_items(GCompletion* cmp);
GList* g_completion_complete(GCompletion* cmp, const char* prefix, char**  new_prefix);
GList* g_completion_complete_utf8(GCompletion *cmp, const char* prefix, char** new_prefix);
void g_completion_set_compare(GCompletion *cmp, GCompletionStrncmpFunc strncmp_func);
void g_completion_free(GCompletion* cmp);
#endif
G_END_DECLS

#endif