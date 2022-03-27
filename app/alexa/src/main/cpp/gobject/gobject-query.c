#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../gio/config.h"
#include "../glib/glib-object.h"
#include "../glib/gprintf.h"

static gchar *indent_inc = NULL;
static guint spacing = 1;
static FILE *f_out = NULL;
static GType root = 0;
static gboolean recursion = TRUE;
#if 0
#define	O_SPACE	"\\as"
#define	O_ESPACE " "
#define	O_BRANCH "\\aE"
#define	O_VLINE "\\al"
#define	O_LLEAF	"\\aL"
#define	O_KEY_FILL "_"
#else
#define	O_SPACE	" "
#define	O_ESPACE ""
#define	O_BRANCH "+"
#define	O_VLINE "|"
#define	O_LLEAF	"`"
#define	O_KEY_FILL "_"
#endif
static void show_nodes(GType type, GType sibling, const gchar *indent) {
    GType   *children;
    guint i;
    if (!type) return;
    children = g_type_children (type,NULL);
    if (type != root)
        for (i = 0; i < spacing; i++) g_fprintf(f_out,"%s%s\n", indent, O_VLINE);
    g_fprintf(f_out,"%s%s%s%s", indent, sibling ? O_BRANCH : (type != root ? O_LLEAF : O_SPACE), O_ESPACE, g_type_name(type));
    for (i = strlen(g_type_name(type)); i <= strlen (indent_inc); i++) fputs(O_KEY_FILL, f_out);
    fputc('\n', f_out);
    if (children && recursion) {
        gchar *new_indent;
        GType *child;
        if (sibling) new_indent = g_strconcat(indent, O_VLINE, indent_inc, NULL);
        else new_indent = g_strconcat(indent, O_SPACE, indent_inc, NULL);
        for (child = children; *child; child++) show_nodes(child[0],child[1], new_indent);
        g_free (new_indent);
    }
    g_free(children);
}
static gint help (gchar *arg) {
    g_fprintf(stderr, "usage: gobject-query <qualifier> [-r <type>] [-{i|b} \"\"] [-s #] [-{h|x|y}]\n");
    g_fprintf(stderr, "       -r       specifiy root type\n");
    g_fprintf(stderr, "       -n       don't descend type tree\n");
    g_fprintf(stderr, "       -h       guess what ;)\n");
    g_fprintf(stderr, "       -b       specify indent string\n");
    g_fprintf(stderr, "       -i       specify incremental indent string\n");
    g_fprintf(stderr, "       -s       specify line spacing\n");
    g_fprintf(stderr, "qualifiers:\n");
    g_fprintf(stderr, "       froots   iterate over fundamental roots\n");
    g_fprintf(stderr, "       tree     print type tree\n");
    return arg != NULL;
}
int main(gint argc, gchar *argv[]) {
    GLogLevelFlags fatal_mask;
    gboolean gen_froots = 0;
    gboolean gen_tree = 0;
    gint i;
    gchar *iindent = "";
    f_out = stdout;
    fatal_mask = g_log_set_always_fatal(G_LOG_FATAL_MASK);
    fatal_mask |= G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
    g_log_set_always_fatal(fatal_mask);
    root = G_TYPE_OBJECT;
    g_type_init();
    for (i = 1; i < argc; i++) {
        if (strcmp("-s", argv[i]) == 0) {
            i++;
            if (i < argc)spacing = atoi(argv[i]);
	    } else if (strcmp("-i", argv[i]) == 0) {
            i++;
            if (i < argc) {
                char *p;
                guint n;
                p = argv[i];
                while(*p) p++;
                n = p - argv[i];
                indent_inc = g_new(gchar, n * strlen(O_SPACE) + 1);
                *indent_inc = 0;
                while(n) {
                    n--;
                    strcpy(indent_inc, O_SPACE);
                }
            }
	    } else if (strcmp("-b", argv[i]) == 0) {
            i++;
            if (i < argc) iindent = argv[i];
	    } else if (strcmp("-r", argv[i]) == 0) {
            i++;
            if (i < argc) root = g_type_from_name(argv[i]);
        } else if (strcmp("-n", argv[i]) == 0) {
            recursion = FALSE;
        } else if (strcmp("froots", argv[i]) == 0) {
            gen_froots = 1;
        } else if (strcmp("tree", argv[i]) == 0) {
            gen_tree = 1;
        } else if (strcmp("-h", argv[i]) == 0) {
            return help(NULL);
        } else if (strcmp("--help", argv[i]) == 0) {
            return help(NULL);
        } else return help(argv[i]);
    }
    if (!gen_froots && !gen_tree) return help(argv[i-1]);
    if (!indent_inc) {
        indent_inc = g_new(gchar, strlen(O_SPACE) + 1);
        *indent_inc = 0;
        strcpy(indent_inc, O_SPACE);
    }
    if (gen_tree) show_nodes(root, 0, iindent);
    if (gen_froots) {
        root = ~0;
        for (i = 0; i <= G_TYPE_FUNDAMENTAL_MAX; i += G_TYPE_MAKE_FUNDAMENTAL(1)) {
            const gchar *name = g_type_name(i);
            if (name) show_nodes(i, 0, iindent);
        }
    }
    return 0;
}