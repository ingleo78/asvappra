#ifndef __XDG_MIME_GLOB_H__
#define __XDG_MIME_GLOB_H__

#include "xdgmime.h"

typedef struct XdgGlobHash XdgGlobHash;
typedef enum {
  XDG_GLOB_LITERAL,
  XDG_GLOB_SIMPLE,
  XDG_GLOB_FULL
} XdgGlobType;
#ifdef XDG_PREFIX
#define _xdg_mime_glob_read_from_file  XDG_RESERVED_ENTRY(glob_read_from_file)
#define _xdg_glob_hash_new  XDG_RESERVED_ENTRY(hash_new)
#define _xdg_glob_hash_free  XDG_RESERVED_ENTRY(hash_free)
#define _xdg_glob_hash_lookup_file_name  XDG_RESERVED_ENTRY(hash_lookup_file_name)
#define _xdg_glob_hash_append_glob  XDG_RESERVED_ENTRY(hash_append_glob)
#define _xdg_glob_determine_type  XDG_RESERVED_ENTRY(determine_type)
#define _xdg_glob_hash_dump  XDG_RESERVED_ENTRY(hash_dump)
#endif
void _xdg_mime_glob_read_from_file(XdgGlobHash *glob_hash, const char *file_name, int version_two);
XdgGlobHash *_xdg_glob_hash_new(void);
void _xdg_glob_hash_free(XdgGlobHash *glob_hash);
int _xdg_glob_hash_lookup_file_name(XdgGlobHash *glob_hash, const char *text, const char *mime_types[], int n_mime_types);
void _xdg_glob_hash_append_glob(XdgGlobHash *glob_hash, const char *glob, const char *mime_type, int weight, int case_sensitive);
XdgGlobType _xdg_glob_determine_type(const char *glob);
#ifdef NOT_USED_IN_GIO
void _xdg_glob_hash_dump(XdgGlobHash *glob_hash);
#endif

#endif