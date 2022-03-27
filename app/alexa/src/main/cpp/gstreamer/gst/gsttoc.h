#ifndef __GST_TOC_H__
#define __GST_TOC_H__

#include "gstconfig.h"
#include "gstminiobject.h"
#include "gststructure.h"
#include "gsttaglist.h"
#include "gstformat.h"

G_BEGIN_DECLS
GST_EXPORT GType _gst_toc_type;
GST_EXPORT GType _gst_toc_entry_type;
#define GST_TYPE_TOC (_gst_toc_type)
#define GST_TYPE_TOC_ENTRY (_gst_toc_entry_type)
typedef struct _GstTocEntry GstTocEntry;
typedef struct _GstToc GstToc;
typedef enum {
  GST_TOC_SCOPE_GLOBAL = 1,
  GST_TOC_SCOPE_CURRENT = 2
} GstTocScope;
typedef enum {
  GST_TOC_ENTRY_TYPE_ANGLE       = -3,
  GST_TOC_ENTRY_TYPE_VERSION     = -2,
  GST_TOC_ENTRY_TYPE_EDITION     = -1,
  GST_TOC_ENTRY_TYPE_INVALID     = 0,
  GST_TOC_ENTRY_TYPE_TITLE       = 1,
  GST_TOC_ENTRY_TYPE_TRACK       = 2,
  GST_TOC_ENTRY_TYPE_CHAPTER     = 3,
} GstTocEntryType;
#define GST_TOC_ENTRY_TYPE_IS_ALTERNATIVE(entry_type)  (entry_type < 0)
#define GST_TOC_ENTRY_TYPE_IS_SEQUENCE(entry_type)     (entry_type > 0)
typedef enum {
  GST_TOC_LOOP_NONE = 0,
  GST_TOC_LOOP_FORWARD,
  GST_TOC_LOOP_REVERSE,
  GST_TOC_LOOP_PING_PONG
} GstTocLoopType;
#define GST_TOC_REPEAT_COUNT_INFINITE (-1)
GType           gst_toc_get_type                (void);
GType           gst_toc_entry_get_type          (void);
GstToc *           gst_toc_new                     (GstTocScope scope);
GstTocScope        gst_toc_get_scope               (const GstToc *toc);
void               gst_toc_set_tags                (GstToc *toc, GstTagList * tags);
void               gst_toc_merge_tags              (GstToc *toc, GstTagList *tags, GstTagMergeMode mode);
GstTagList *       gst_toc_get_tags                (const GstToc *toc);
void               gst_toc_append_entry               (GstToc *toc, GstTocEntry *entry);
GList *            gst_toc_get_entries             (const GstToc *toc);
void               gst_toc_dump                    (GstToc *toc);
#define gst_toc_ref(toc)            (GstToc*)gst_mini_object_ref(GST_MINI_OBJECT_CAST(toc))
#define gst_toc_unref(toc)          gst_mini_object_unref(GST_MINI_OBJECT_CAST(toc))
#define gst_toc_copy(toc)           (GstToc*)gst_mini_object_copy(GST_MINI_OBJECT_CAST(toc))
#define gst_toc_make_writable(toc)  (GstToc*)gst_mini_object_make_writable(GST_MINI_OBJECT_CAST(toc))
GstTocEntry *   gst_toc_entry_new               (GstTocEntryType type, const gchar *uid);
#define gst_toc_entry_ref(entry)            (GstTocEntry*)gst_mini_object_ref(GST_MINI_OBJECT_CAST(entry))
#define gst_toc_entry_unref(entry)          gst_mini_object_unref(GST_MINI_OBJECT_CAST(entry))
#define gst_toc_entry_copy(entry)           (GstTocEntry*)gst_mini_object_copy(GST_MINI_OBJECT_CAST(entry))
#define gst_toc_entry_make_writable(entry)  (GstTocEntry*)gst_mini_object_make_writable(GST_MINI_OBJECT_CAST(entry))
GstTocEntry *      gst_toc_find_entry                    (const GstToc *toc, const gchar *uid);
GstTocEntryType    gst_toc_entry_get_entry_type          (const GstTocEntry *entry);
const gchar *      gst_toc_entry_get_uid                 (const GstTocEntry *entry);
void               gst_toc_entry_append_sub_entry           (GstTocEntry *entry, GstTocEntry *subentry);
GList *            gst_toc_entry_get_sub_entries         (const GstTocEntry *entry);
void               gst_toc_entry_set_tags                (GstTocEntry *entry, GstTagList *tags);
void               gst_toc_entry_merge_tags              (GstTocEntry *entry, GstTagList *tags, GstTagMergeMode mode);
GstTagList *       gst_toc_entry_get_tags                (const GstTocEntry *entry);
gboolean           gst_toc_entry_is_alternative          (const GstTocEntry *entry);
gboolean           gst_toc_entry_is_sequence             (const GstTocEntry *entry);
void               gst_toc_entry_set_start_stop_times    (GstTocEntry *entry, gint64 start, gint64 stop);
gboolean           gst_toc_entry_get_start_stop_times    (const GstTocEntry *entry, gint64 *start, gint64 *stop);
void               gst_toc_entry_set_loop                (GstTocEntry *entry, GstTocLoopType loop_type, gint repeat_count);
gboolean           gst_toc_entry_get_loop                (const GstTocEntry *entry, GstTocLoopType *loop_type, gint *repeat_count);
GstToc *           gst_toc_entry_get_toc                 (GstTocEntry *entry);
GstTocEntry *      gst_toc_entry_get_parent              (GstTocEntry *entry);
const gchar *      gst_toc_entry_type_get_nick     (GstTocEntryType type);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
static inline void _gst_autoptr_toc_unref (GstToc *toc) {
  gst_toc_unref (toc);
}
static inline void _gst_autoptr_toc_entry_unref (GstTocEntry *entry) {
  gst_toc_entry_unref (entry);
}
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstToc, _gst_autoptr_toc_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTocEntry, _gst_autoptr_toc_entry_unref)
#endif
G_END_DECLS

#endif