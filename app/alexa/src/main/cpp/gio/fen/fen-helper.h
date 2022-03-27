#ifndef _FEN_HELPER_H_
#define _FEN_HELPER_H_

#include "../../glib/glib.h"

void fen_add(const gchar *filename, gpointer sub, gboolean is_mondir);
void fen_remove(const gchar *filename, gpointer sub, gboolean is_mondir);
gboolean fen_init();

#endif