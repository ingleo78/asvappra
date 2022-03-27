#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gbookmarkfile.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "gconvert.h"
#include "gdataset.h"
#include "gerror.h"
#include "gfileutils.h"
#include "ghash.h"
#include "glibintl.h"
#include "glist.h"
#include "gslist.h"
#include "gmain.h"
#include "gmarkup.h"
#include "gmem.h"
#include "gmessages.h"
#include "gshell.h"
#include "gslice.h"
#include "gstdio.h"
#include "gstring.h"
#include "gstrfuncs.h"
#include "gtimer.h"
#include "gutils.h"

#define XBEL_VERSION  "1.0"
#define XBEL_DTD_NICK  "xbel"
#define XBEL_DTD_SYSTEM	 "+//IDN python.org//DTD XML Bookmark Exchange Language 1.0//EN//XML"
#define XBEL_DTD_URI  "http://www.python.org/topics/xml/dtds/xbel-1.0.dtd"
#define XBEL_ROOT_ELEMENT  "xbel"
#define XBEL_FOLDER_ELEMENT	"folder"
#define XBEL_BOOKMARK_ELEMENT  "bookmark"
#define XBEL_ALIAS_ELEMENT  "alias"
#define XBEL_SEPARATOR_ELEMENT	"separator"
#define XBEL_TITLE_ELEMENT	"title"
#define XBEL_DESC_ELEMENT  "desc"
#define XBEL_INFO_ELEMENT  "info"
#define XBEL_METADATA_ELEMENT  "metadata"
#define XBEL_VERSION_ATTRIBUTE	"version"
#define XBEL_FOLDED_ATTRIBUTE  "folded"
#define XBEL_OWNER_ATTRIBUTE  "owner"
#define XBEL_ADDED_ATTRIBUTE  "added"
#define XBEL_VISITED_ATTRIBUTE	"visited"
#define XBEL_MODIFIED_ATTRIBUTE  "modified"
#define XBEL_ID_ATTRIBUTE  "id"
#define XBEL_HREF_ATTRIBUTE	 "href"
#define XBEL_REF_ATTRIBUTE	"ref"
#define XBEL_YES_VALUE	"yes"
#define XBEL_NO_VALUE  "no"
#define BOOKMARK_METADATA_OWNER   "http://freedesktop.org"
#define BOOKMARK_NAMESPACE_NAME  "bookmark"
#define BOOKMARK_NAMESPACE_URI	"http://www.freedesktop.org/standards/desktop-bookmarks"
#define BOOKMARK_GROUPS_ELEMENT	 "groups"
#define BOOKMARK_GROUP_ELEMENT	"group"
#define BOOKMARK_APPLICATIONS_ELEMENT  "applications"
#define BOOKMARK_APPLICATION_ELEMENT  "application"
#define BOOKMARK_ICON_ELEMENT  "icon"
#define BOOKMARK_PRIVATE_ELEMENT  "private"
#define BOOKMARK_NAME_ATTRIBUTE	 "name"
#define BOOKMARK_EXEC_ATTRIBUTE	 "exec"
#define BOOKMARK_COUNT_ATTRIBUTE  "count"
#define BOOKMARK_TIMESTAMP_ATTRIBUTE  "timestamp"
#define BOOKMARK_MODIFIED_ATTRIBUTE  "modified"
#define BOOKMARK_HREF_ATTRIBUTE  "href"
#define BOOKMARK_TYPE_ATTRIBUTE  "type"
#define MIME_NAMESPACE_NAME  "mime"
#define MIME_NAMESPACE_URI 	"http://www.freedesktop.org/standards/shared-mime-info"
#define MIME_TYPE_ELEMENT  "mime-type"
#define MIME_TYPE_ATTRIBUTE  "type"
typedef struct _BookmarkAppInfo BookmarkAppInfo;
typedef struct _BookmarkMetadata BookmarkMetadata;
typedef struct _BookmarkItem BookmarkItem;
typedef struct _ParseData ParseData;
struct _BookmarkAppInfo {
  gchar *name;
  gchar *exec;
  guint count;
  time_t stamp;
};
struct _BookmarkMetadata {
  gchar *mime_type;
  GList *groups;
  GList *applications;
  GHashTable *apps_by_name;
  gchar *icon_href;
  gchar *icon_mime;
  guint is_private : 1;
};
struct _BookmarkItem {
  gchar *uri;
  gchar *title;
  gchar *description;
  time_t added;
  time_t modified;
  time_t visited;
  BookmarkMetadata *metadata;
};
struct _GBookmarkFile {
  gchar *title;
  gchar *description;
  GList *items;
  GHashTable *items_by_uri;
};
enum {
  STATE_STARTED = 0,
  STATE_ROOT,
  STATE_BOOKMARK,
  STATE_TITLE,
  STATE_DESC,
  STATE_INFO,
  STATE_METADATA,
  STATE_APPLICATIONS,
  STATE_APPLICATION,
  STATE_GROUPS,
  STATE_GROUP,
  STATE_MIME,
  STATE_ICON,
  STATE_FINISHED
};
static void g_bookmark_file_init(GBookmarkFile *bookmark);
static void g_bookmark_file_clear(GBookmarkFile *bookmark);
static gboolean g_bookmark_file_parse(GBookmarkFile *bookmark, const gchar *buffer, gsize length, GError **error);
static gchar* g_bookmark_file_dump(GBookmarkFile *bookmark, gsize *length, GError **error);
static BookmarkItem *g_bookmark_file_lookup_item(GBookmarkFile *bookmark, const gchar *uri);
static void g_bookmark_file_add_item(GBookmarkFile *bookmark, BookmarkItem *item, GError **error);
static time_t timestamp_from_iso8601(const gchar *iso_date);
static gchar* timestamp_to_iso8601(time_t timestamp);
static BookmarkAppInfo * bookmark_app_info_new(const gchar *name) {
  BookmarkAppInfo *retval;
  g_warn_if_fail(name != NULL);
  retval = g_slice_new(BookmarkAppInfo);
  retval->name = g_strdup(name);
  retval->exec = NULL;
  retval->count = 0;
  retval->stamp = 0;
  return retval;
}
static void bookmark_app_info_free(BookmarkAppInfo *app_info) {
  if (!app_info) return;
  g_free(app_info->name);
  g_free(app_info->exec);
  g_slice_free(BookmarkAppInfo, app_info);
}
static gchar* bookmark_app_info_dump(BookmarkAppInfo *app_info) {
  gchar *retval;
  gchar *name, *exec, *modified, *count;
  g_warn_if_fail(app_info != NULL);
  if (app_info->count == 0) return NULL;
  name = g_markup_escape_text(app_info->name, -1);
  exec = g_markup_escape_text(app_info->exec, -1);
  modified = timestamp_to_iso8601(app_info->stamp);
  count = g_strdup_printf("%u", app_info->count);
  retval = g_strconcat("          <" BOOKMARK_NAMESPACE_NAME ":" BOOKMARK_APPLICATION_ELEMENT " " BOOKMARK_NAME_ATTRIBUTE "=\"", name, "\""
                        " " BOOKMARK_EXEC_ATTRIBUTE "=\"", exec, "\" " BOOKMARK_MODIFIED_ATTRIBUTE "=\"", modified, "\""
                        " " BOOKMARK_COUNT_ATTRIBUTE "=\"", count, "\"/>\n", NULL);
  g_free(name);
  g_free(exec);
  g_free(modified);
  g_free(count);
  return retval;
}
static BookmarkMetadata* bookmark_metadata_new(void) {
  BookmarkMetadata *retval;
  retval = g_slice_new(BookmarkMetadata);
  retval->mime_type = NULL;
  retval->groups = NULL;
  retval->applications = NULL;
  retval->apps_by_name = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
  retval->is_private = FALSE;
  retval->icon_href = NULL;
  retval->icon_mime = NULL;
  return retval;
}
static void bookmark_metadata_free(BookmarkMetadata *metadata) {
  if (!metadata) return;
  g_free(metadata->mime_type);
  if (metadata->groups) {
      g_list_foreach(metadata->groups, (GFunc)g_free,NULL);
      g_list_free (metadata->groups);
  }
  if (metadata->applications) {
      g_list_foreach(metadata->applications, (GFunc)bookmark_app_info_free,NULL);
      g_list_free(metadata->applications);
  }
  g_hash_table_destroy(metadata->apps_by_name);
  g_free(metadata->icon_href);
  g_free(metadata->icon_mime);
  g_slice_free(BookmarkMetadata, metadata);
}
static gchar* bookmark_metadata_dump(BookmarkMetadata *metadata) {
  GString *retval;
  gchar *buffer;
  if (!metadata->applications) return NULL;
  retval = g_string_sized_new(1024);
  g_string_append(retval,"      <" XBEL_METADATA_ELEMENT " " XBEL_OWNER_ATTRIBUTE "=\"" BOOKMARK_METADATA_OWNER "\">\n");
  if (metadata->mime_type) {
      buffer = g_strconcat("        <" MIME_NAMESPACE_NAME ":" MIME_TYPE_ELEMENT " " MIME_TYPE_ATTRIBUTE "=\"", metadata->mime_type, "\"/>\n", NULL);
      g_string_append(retval, buffer);
      g_free(buffer);
  }
  if (metadata->groups) {
      GList *l;
      g_string_append (retval,"        <" BOOKMARK_NAMESPACE_NAME ":" BOOKMARK_GROUPS_ELEMENT ">\n");
      for (l = g_list_last (metadata->groups); l != NULL; l = l->prev) {
          gchar *group_name;
          group_name = g_markup_escape_text ((gchar *) l->data, -1);
	      buffer = g_strconcat("          <" BOOKMARK_NAMESPACE_NAME ":" BOOKMARK_GROUP_ELEMENT ">", group_name, "</" BOOKMARK_NAMESPACE_NAME
				               ":"  BOOKMARK_GROUP_ELEMENT ">\n", NULL);
          g_string_append(retval, buffer);
          g_free(buffer);
          g_free(group_name);
      }
      g_string_append(retval,"        </" BOOKMARK_NAMESPACE_NAME ":" BOOKMARK_GROUPS_ELEMENT ">\n");
  }
  if (metadata->applications) {
      GList *l;
      g_string_append (retval,"        <" BOOKMARK_NAMESPACE_NAME ":" BOOKMARK_APPLICATIONS_ELEMENT ">\n");
      for (l = g_list_last (metadata->applications); l != NULL; l = l->prev) {
          BookmarkAppInfo *app_info = (BookmarkAppInfo*)l->data;
          gchar *app_data;
	      g_warn_if_fail(app_info != NULL);
          app_data = bookmark_app_info_dump(app_info);
	      if (app_data) {
              retval = g_string_append(retval, app_data);
	          g_free(app_data);
	      }
      }
      g_string_append (retval,"        </" BOOKMARK_NAMESPACE_NAME ":" BOOKMARK_APPLICATIONS_ELEMENT ">\n");
  }
  if (metadata->icon_href) {
      if (!metadata->icon_mime) metadata->icon_mime = g_strdup("application/octet-stream");
      buffer = g_strconcat("       <" BOOKMARK_NAMESPACE_NAME ":" BOOKMARK_ICON_ELEMENT " " BOOKMARK_HREF_ATTRIBUTE "=\"", metadata->icon_href,
			               "\" " BOOKMARK_TYPE_ATTRIBUTE "=\"", metadata->icon_mime, "\"/>\n", NULL);
      g_string_append(retval, buffer);
      g_free(buffer);
  }
  if (metadata->is_private) g_string_append(retval,"        <" BOOKMARK_NAMESPACE_NAME ":" BOOKMARK_PRIVATE_ELEMENT "/>\n");
  g_string_append(retval,"      </" XBEL_METADATA_ELEMENT ">\n");
  return g_string_free(retval, FALSE);
}
static BookmarkItem* bookmark_item_new(const gchar *uri) {
  BookmarkItem *item;
  g_warn_if_fail(uri != NULL);
  item = g_slice_new(BookmarkItem);
  item->uri = g_strdup(uri);
  item->title = NULL;
  item->description = NULL;
  item->added = (time_t) -1;
  item->modified = (time_t) -1;
  item->visited = (time_t) -1;
  item->metadata = NULL;
  return item;
}
static void bookmark_item_free(BookmarkItem *item) {
  if (!item) return;
  g_free(item->uri);
  g_free(item->title);
  g_free(item->description);
  if (item->metadata) bookmark_metadata_free(item->metadata);
  g_slice_free(BookmarkItem, item);
}
static gchar* bookmark_item_dump(BookmarkItem *item) {
  GString *retval;
  gchar *added, *visited, *modified;
  gchar *escaped_uri;
  gchar *buffer;
  if (!item->metadata || !item->metadata->applications) {
      g_warning("Item for URI '%s' has no registered applications: skipping.\n", item->uri);
      return NULL;
  }
  retval = g_string_sized_new(4096);
  added = timestamp_to_iso8601(item->added);
  modified = timestamp_to_iso8601(item->modified);
  visited = timestamp_to_iso8601(item->visited);
  escaped_uri = g_markup_escape_text(item->uri, -1);
  buffer = g_strconcat("  <" XBEL_BOOKMARK_ELEMENT " " XBEL_HREF_ATTRIBUTE "=\"", escaped_uri, "\" " XBEL_ADDED_ATTRIBUTE "=\"", added, "\" "
                        XBEL_MODIFIED_ATTRIBUTE "=\"", modified, "\" " XBEL_VISITED_ATTRIBUTE "=\"", visited, "\">\n", NULL);
  g_string_append(retval, buffer);
  g_free(escaped_uri);
  g_free(visited);
  g_free(modified);
  g_free(added);
  g_free(buffer);
  if (item->title) {
      gchar *escaped_title;
      escaped_title = g_markup_escape_text(item->title, -1);
      buffer = g_strconcat("    <" XBEL_TITLE_ELEMENT ">", escaped_title, "</" XBEL_TITLE_ELEMENT ">\n", NULL);
      g_string_append(retval, buffer);
      g_free(escaped_title);
      g_free(buffer);
  }
  if (item->description) {
      gchar *escaped_desc;
      escaped_desc = g_markup_escape_text(item->description, -1);
      buffer = g_strconcat("    <" XBEL_DESC_ELEMENT ">", escaped_desc, "</" XBEL_DESC_ELEMENT ">\n", NULL);
      g_string_append(retval, buffer);
      g_free(escaped_desc);
      g_free(buffer);
  }
  if (item->metadata) {
      gchar *metadata;
      metadata = bookmark_metadata_dump(item->metadata);
      if (metadata) {
          buffer = g_strconcat("    <" XBEL_INFO_ELEMENT ">\n", metadata, "    </" XBEL_INFO_ELEMENT ">\n", NULL);
          retval = g_string_append(retval, buffer);
          g_free(buffer);
          g_free(metadata);
	  }
  }
  g_string_append(retval, "  </" XBEL_BOOKMARK_ELEMENT ">\n");
  return g_string_free(retval, FALSE);
}
static BookmarkAppInfo* bookmark_item_lookup_app_info(BookmarkItem *item, const gchar *app_name) {
  g_warn_if_fail(item != NULL && app_name != NULL);
  if (!item->metadata) return NULL;
  return g_hash_table_lookup(item->metadata->apps_by_name, app_name);
}
static void g_bookmark_file_init(GBookmarkFile *bookmark) {
  bookmark->title = NULL;
  bookmark->description = NULL;
  bookmark->items = NULL;
  bookmark->items_by_uri = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
}
static void g_bookmark_file_clear(GBookmarkFile *bookmark) {
  g_free(bookmark->title);
  g_free(bookmark->description);
  if (bookmark->items) {
      g_list_foreach(bookmark->items, (GFunc)bookmark_item_free,NULL);
      g_list_free(bookmark->items);
      bookmark->items = NULL;
  }
  if (bookmark->items_by_uri) {
      g_hash_table_destroy(bookmark->items_by_uri);
      bookmark->items_by_uri = NULL;
  }
}
struct _ParseData {
  gint state;
  GHashTable *namespaces;
  GBookmarkFile *bookmark_file;
  BookmarkItem *current_item;
};
static ParseData* parse_data_new(void) {
  ParseData *retval;
  retval = g_new(ParseData, 1);
  retval->state = STATE_STARTED;
  retval->namespaces = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free, (GDestroyNotify)g_free);
  retval->bookmark_file = NULL;
  retval->current_item = NULL;
  return retval;
}
static void parse_data_free(ParseData *parse_data) {
  g_hash_table_destroy(parse_data->namespaces);
  g_free(parse_data);
}
#define IS_ATTRIBUTE(s,a)	((0 == strcmp ((s), (a))))
static void
parse_bookmark_element (GMarkupParseContext  *context, ParseData *parse_data, const gchar **attribute_names, const gchar **attribute_values, GError **error) {
  const gchar *uri, *added, *modified, *visited;
  const gchar *attr;
  gint i;
  BookmarkItem *item;
  GError *add_error;
  g_warn_if_fail((parse_data != NULL) && (parse_data->state == STATE_BOOKMARK));
  i = 0;
  uri = added = modified = visited = NULL;
  for (attr = attribute_names[i]; attr != NULL; attr = attribute_names[++i]) {
      if (IS_ATTRIBUTE(attr, XBEL_HREF_ATTRIBUTE)) uri = attribute_values[i];
      else if (IS_ATTRIBUTE(attr, XBEL_ADDED_ATTRIBUTE)) added = attribute_values[i];
      else if (IS_ATTRIBUTE(attr, XBEL_MODIFIED_ATTRIBUTE)) modified = attribute_values[i];
      else if (IS_ATTRIBUTE(attr, XBEL_VISITED_ATTRIBUTE)) visited = attribute_values[i];
      else {
          g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE, _("Unexpected attribute '%s' for element '%s'"), attr, XBEL_BOOKMARK_ELEMENT);
          return;
      }
  }
  if (!uri) {
      g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Attribute '%s' of element '%s' not found"), XBEL_HREF_ATTRIBUTE,
      		      XBEL_BOOKMARK_ELEMENT);
      return;
  }
  g_warn_if_fail(parse_data->current_item == NULL);
  item = bookmark_item_new(uri);
  if (added) item->added = timestamp_from_iso8601(added);
  if (modified) item->modified = timestamp_from_iso8601(modified);
  if (visited) item->visited = timestamp_from_iso8601(visited);
  add_error = NULL;
  g_bookmark_file_add_item(parse_data->bookmark_file, item, &add_error);
  if (add_error) {
      bookmark_item_free(item);
      g_propagate_error(error, add_error);
      return;
  }
  parse_data->current_item = item;
}
static void parse_application_element(GMarkupParseContext  *context, ParseData *parse_data, const gchar **attribute_names, const gchar **attribute_values,
                                      GError **error) {
  const gchar *name, *exec, *count, *stamp, *modified;
  const gchar *attr;
  gint i;
  BookmarkItem *item;
  BookmarkAppInfo *ai;
  g_warn_if_fail((parse_data != NULL) && (parse_data->state == STATE_APPLICATION));
  i = 0;
  name = exec = count = stamp = modified = NULL;
  for (attr = attribute_names[i]; attr != NULL; attr = attribute_names[++i]) {
      if (IS_ATTRIBUTE(attr, BOOKMARK_NAME_ATTRIBUTE)) name = attribute_values[i];
      else if (IS_ATTRIBUTE(attr, BOOKMARK_EXEC_ATTRIBUTE)) exec = attribute_values[i];
      else if (IS_ATTRIBUTE(attr, BOOKMARK_COUNT_ATTRIBUTE)) count = attribute_values[i];
      else if (IS_ATTRIBUTE(attr, BOOKMARK_TIMESTAMP_ATTRIBUTE)) stamp = attribute_values[i];
      else if (IS_ATTRIBUTE(attr, BOOKMARK_MODIFIED_ATTRIBUTE)) modified = attribute_values[i];
  }
  if (!name) {
      g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Attribute '%s' of element '%s' not found"), BOOKMARK_NAME_ATTRIBUTE,
      		      BOOKMARK_APPLICATION_ELEMENT);
      return;
  }
  if (!exec) {
      g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Attribute '%s' of element '%s' not found"), BOOKMARK_EXEC_ATTRIBUTE,
      		      BOOKMARK_APPLICATION_ELEMENT);
      return;
  }
  g_warn_if_fail(parse_data->current_item != NULL);
  item = parse_data->current_item;
  ai = bookmark_item_lookup_app_info(item, name);
  if (!ai) {
      ai = bookmark_app_info_new(name);
      if (!item->metadata) item->metadata = bookmark_metadata_new();
      item->metadata->applications = g_list_prepend(item->metadata->applications, ai);
      g_hash_table_replace(item->metadata->apps_by_name, ai->name, ai);
  }
  ai->exec = g_strdup(exec);
  if (count) ai->count = atoi(count);
  else ai->count = 1;
  if (modified) ai->stamp = timestamp_from_iso8601(modified);
  else {
      if (stamp) ai->stamp = (time_t)atol(stamp);
      else ai->stamp = time(NULL);
  }
}
static void parse_mime_type_element(GMarkupParseContext *context, ParseData *parse_data, const gchar **attribute_names, const gchar **attribute_values,
                                    GError **error) {
  const gchar *type;
  const gchar *attr;
  gint i;
  BookmarkItem *item;
  g_warn_if_fail((parse_data != NULL) && (parse_data->state == STATE_MIME));
  i = 0;
  type = NULL;
  for (attr = attribute_names[i]; attr != NULL; attr = attribute_names[++i]) {
      if (IS_ATTRIBUTE(attr, MIME_TYPE_ATTRIBUTE)) type = attribute_values[i];
  }
  if (!type) type = "application/octet-stream";
  g_warn_if_fail(parse_data->current_item != NULL);
  item = parse_data->current_item;
  if (!item->metadata) item->metadata = bookmark_metadata_new();
  item->metadata->mime_type = g_strdup(type);
}
static void parse_icon_element(GMarkupParseContext *context, ParseData *parse_data, const gchar **attribute_names, const gchar **attribute_values, GError **error) {
  const gchar *href;
  const gchar *type;
  const gchar *attr;
  gint i;
  BookmarkItem *item;
  g_warn_if_fail((parse_data != NULL) && (parse_data->state == STATE_ICON));
  i = 0;
  href = NULL;
  type = NULL;
  for (attr = attribute_names[i]; attr != NULL; attr = attribute_names[++i]) {
      if (IS_ATTRIBUTE(attr, BOOKMARK_HREF_ATTRIBUTE)) href = attribute_values[i];
      else if (IS_ATTRIBUTE(attr, BOOKMARK_TYPE_ATTRIBUTE)) type = attribute_values[i];
  }
  if (!href) {
      g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Attribute '%s' of element '%s' not found"), BOOKMARK_HREF_ATTRIBUTE,
      		      BOOKMARK_ICON_ELEMENT);
      return;
  }
  if (!type) type = "application/octet-stream";
  g_warn_if_fail(parse_data->current_item != NULL);
  item = parse_data->current_item;
  if (!item->metadata) item->metadata = bookmark_metadata_new();
  item->metadata->icon_href = g_strdup(href);
  item->metadata->icon_mime = g_strdup(type);
}
static void map_namespace_to_name(ParseData *parse_data, const gchar **attribute_names, const gchar **attribute_values) {
  const gchar *attr;
  gint i;
  g_warn_if_fail(parse_data != NULL);
  if (!attribute_names || !attribute_names[0]) return;
  i = 0;
  for (attr = attribute_names[i]; attr; attr = attribute_names[++i]) {
      if (g_str_has_prefix(attr, "xmlns")) {
          gchar *namespace_name, *namespace_uri;
          gchar *p;
          p = g_utf8_strchr(attr, -1, ':');
          if (p) p = g_utf8_next_char(p);
          else p = "default";
          namespace_name = g_strdup(p);
          namespace_uri = g_strdup(attribute_values[i]);
          g_hash_table_replace(parse_data->namespaces, namespace_name, namespace_uri);
      }
  }
}
static gboolean is_element_full(ParseData *parse_data, const gchar *element_full, const gchar *namespace, const gchar *element, const gchar sep) {
  gchar *ns_uri, *ns_name;
  const gchar *p, *element_name;
  gboolean retval;
  g_warn_if_fail(parse_data != NULL);
  g_warn_if_fail(element_full != NULL);
  if (!element) return FALSE;
  if (!namespace) return (0 == strcmp(element_full, element));
  p = g_utf8_strchr(element_full, -1, ':');
  if (p) {
      ns_name = g_strndup(element_full, p - element_full);
      element_name = g_utf8_next_char(p);
  } else {
      ns_name = g_strdup("default");
      element_name = element_full;
  }
  ns_uri = g_hash_table_lookup(parse_data->namespaces, ns_name);
  if (!ns_uri) {
      g_free(ns_name);
      return (0 == strcmp(element_full, element));
  }
  retval = (0 == strcmp(ns_uri, namespace) && 0 == strcmp(element_name, element));
  g_free(ns_name);
  return retval;
}
#define IS_ELEMENT(p,s,e)	(is_element_full((p), (s), NULL, (e), '\0'))
#define IS_ELEMENT_NS(p,s,n,e)	(is_element_full((p), (s), (n), (e), '|'))
static void start_element_raw_cb(GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values,
                                 gpointer user_data, GError **error) {
  ParseData *parse_data = (ParseData*)user_data;
  map_namespace_to_name(parse_data, attribute_names, attribute_values);
  switch(parse_data->state) {
    case STATE_STARTED:
      if (IS_ELEMENT(parse_data, element_name, XBEL_ROOT_ELEMENT)) {
          const gchar *attr;
          gint i;
          i = 0;
          for (attr = attribute_names[i]; attr; attr = attribute_names[++i]) {
              if ((IS_ATTRIBUTE(attr, XBEL_VERSION_ATTRIBUTE)) && (0 == strcmp(attribute_values[i], XBEL_VERSION))) parse_data->state = STATE_ROOT;
          }
	  } else g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Unexpected tag '%s', tag '%s' expected"), element_name, XBEL_ROOT_ELEMENT);
      break;
    case STATE_ROOT:
      if (IS_ELEMENT(parse_data, element_name, XBEL_TITLE_ELEMENT)) parse_data->state = STATE_TITLE;
      else if (IS_ELEMENT(parse_data, element_name, XBEL_DESC_ELEMENT)) parse_data->state = STATE_DESC;
      else if (IS_ELEMENT(parse_data, element_name, XBEL_BOOKMARK_ELEMENT)) {
          GError *inner_error = NULL;
          parse_data->state = STATE_BOOKMARK;
          parse_bookmark_element(context, parse_data, attribute_names, attribute_values, &inner_error);
          if (inner_error) g_propagate_error(error, inner_error);
      } else g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Unexpected tag '%s' inside '%s'"), element_name, XBEL_ROOT_ELEMENT);
      break;
    case STATE_BOOKMARK:
      if (IS_ELEMENT(parse_data, element_name, XBEL_TITLE_ELEMENT)) parse_data->state = STATE_TITLE;
      else if (IS_ELEMENT(parse_data, element_name, XBEL_DESC_ELEMENT)) parse_data->state = STATE_DESC;
      else if (IS_ELEMENT(parse_data, element_name, XBEL_INFO_ELEMENT)) parse_data->state = STATE_INFO;
      else g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Unexpected tag '%s' inside '%s'"), element_name, XBEL_BOOKMARK_ELEMENT);
      break;
    case STATE_INFO:
      if (IS_ELEMENT(parse_data, element_name, XBEL_METADATA_ELEMENT)) {
          const gchar *attr;
          gint i;
          i = 0;
          for (attr = attribute_names[i]; attr; attr = attribute_names[++i]) {
              if ((IS_ATTRIBUTE(attr, XBEL_OWNER_ATTRIBUTE)) && (0 == strcmp(attribute_values[i], BOOKMARK_METADATA_OWNER))) {
                  parse_data->state = STATE_METADATA;
                  if (!parse_data->current_item->metadata) parse_data->current_item->metadata = bookmark_metadata_new();
              }
          }
      } else {
          g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Unexpected tag '%s', tag '%s' expected"), element_name, XBEL_METADATA_ELEMENT);
      }
      break;
    case STATE_METADATA:
      if (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_APPLICATIONS_ELEMENT)) parse_data->state = STATE_APPLICATIONS;
      else if (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_GROUPS_ELEMENT)) parse_data->state = STATE_GROUPS;
      else if (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_PRIVATE_ELEMENT)) parse_data->current_item->metadata->is_private = TRUE;
      else if (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_ICON_ELEMENT)) {
          GError *inner_error = NULL;
          parse_data->state = STATE_ICON;
          parse_icon_element(context, parse_data, attribute_names, attribute_values, &inner_error);
          if (inner_error) g_propagate_error(error, inner_error);
      } else if (IS_ELEMENT_NS(parse_data, element_name, MIME_NAMESPACE_URI, MIME_TYPE_ELEMENT)) {
          GError *inner_error = NULL;
          parse_data->state = STATE_MIME;
          parse_mime_type_element(context, parse_data, attribute_names, attribute_values, &inner_error);
          if (inner_error) g_propagate_error(error, inner_error);
      } else g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT, _("Unexpected tag '%s' inside '%s'"), element_name, XBEL_METADATA_ELEMENT);
      break;
    case STATE_APPLICATIONS:
      if (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_APPLICATION_ELEMENT)) {
          GError *inner_error = NULL;
          parse_data->state = STATE_APPLICATION;
          parse_application_element(context, parse_data, attribute_names, attribute_values, &inner_error);
          if (inner_error) g_propagate_error(error, inner_error);
      } else {
          g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Unexpected tag '%s', tag '%s' expected"), element_name, BOOKMARK_APPLICATION_ELEMENT);
      }
      break;
    case STATE_GROUPS:
      if (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_GROUP_ELEMENT)) parse_data->state = STATE_GROUP;
      else {
          g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, _("Unexpected tag '%s', tag '%s' expected"), element_name, BOOKMARK_GROUP_ELEMENT);
      }
      break;
    case STATE_ICON:
      if (IS_ELEMENT_NS (parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_ICON_ELEMENT)) {
          GError *inner_error = NULL;
          parse_icon_element(context, parse_data, attribute_names, attribute_values, &inner_error);
          if (inner_error) g_propagate_error(error, inner_error);
      } else g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT, _("Unexpected tag '%s' inside '%s'"), element_name, XBEL_METADATA_ELEMENT);
      break;
    default:
      g_warn_if_reached();
      break;
  }
}
static void end_element_raw_cb(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error) {
  ParseData *parse_data = (ParseData*)user_data;
  if (IS_ELEMENT(parse_data, element_name, XBEL_ROOT_ELEMENT)) parse_data->state = STATE_FINISHED;
  else if (IS_ELEMENT (parse_data, element_name, XBEL_BOOKMARK_ELEMENT)) {
      parse_data->current_item = NULL;
      parse_data->state = STATE_ROOT;
  } else if ((IS_ELEMENT(parse_data, element_name, XBEL_INFO_ELEMENT)) || (IS_ELEMENT(parse_data, element_name, XBEL_TITLE_ELEMENT)) ||
             (IS_ELEMENT(parse_data, element_name, XBEL_DESC_ELEMENT))) {
      if (parse_data->current_item) parse_data->state = STATE_BOOKMARK;
      else parse_data->state = STATE_ROOT;
  } else if (IS_ELEMENT(parse_data, element_name, XBEL_METADATA_ELEMENT)) parse_data->state = STATE_INFO;
  else if (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_APPLICATION_ELEMENT)) parse_data->state = STATE_APPLICATIONS;
  else if (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_GROUP_ELEMENT)) parse_data->state = STATE_GROUPS;
  else if ((IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_APPLICATIONS_ELEMENT)) ||
           (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_GROUPS_ELEMENT)) ||
           (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_PRIVATE_ELEMENT)) ||
           (IS_ELEMENT_NS(parse_data, element_name, BOOKMARK_NAMESPACE_URI, BOOKMARK_ICON_ELEMENT)) ||
           (IS_ELEMENT_NS(parse_data, element_name, MIME_NAMESPACE_URI, MIME_TYPE_ELEMENT))) {
      parse_data->state = STATE_METADATA;
  }
}
static void text_raw_cb(GMarkupParseContext *context, const gchar *text, gsize length, gpointer user_data, GError **error) {
  ParseData *parse_data = (ParseData*)user_data;
  gchar *payload;
  payload = g_strndup(text, length);
  switch(parse_data->state) {
    case STATE_TITLE:
      if (parse_data->current_item) {
          g_free(parse_data->current_item->title);
          parse_data->current_item->title = g_strdup(payload);
      } else {
          g_free(parse_data->bookmark_file->title);
          parse_data->bookmark_file->title = g_strdup(payload);
      }
      break;
    case STATE_DESC:
      if (parse_data->current_item) {
          g_free(parse_data->current_item->description);
          parse_data->current_item->description = g_strdup(payload);
      } else {
          g_free(parse_data->bookmark_file->description);
          parse_data->bookmark_file->description = g_strdup(payload);
      }
      break;
    case STATE_GROUP: {
          GList *groups;
          g_warn_if_fail(parse_data->current_item != NULL);
          if (!parse_data->current_item->metadata) parse_data->current_item->metadata = bookmark_metadata_new();
          groups = parse_data->current_item->metadata->groups;
          parse_data->current_item->metadata->groups = g_list_prepend(groups, g_strdup (payload));
      }
      break;
    case STATE_ROOT: case STATE_BOOKMARK: case STATE_INFO: case STATE_METADATA: case STATE_APPLICATIONS: case STATE_APPLICATION: case STATE_GROUPS:
    case STATE_MIME: case STATE_ICON:
      break;
    default: g_warn_if_reached();
  }
  g_free(payload);
}
static const GMarkupParser markup_parser = { start_element_raw_cb, end_element_raw_cb, text_raw_cb,NULL, NULL };
static gboolean g_bookmark_file_parse(GBookmarkFile *bookmark, const gchar *buffer, gsize length, GError **error) {
  GMarkupParseContext *context;
  ParseData *parse_data;
  GError *parse_error, *end_error;
  gboolean retval;
  g_warn_if_fail(bookmark != NULL);
  if (!buffer) return FALSE;
  if (length == (gsize) -1) length = strlen(buffer);
  parse_data = parse_data_new();
  parse_data->bookmark_file = bookmark;
  context = g_markup_parse_context_new(&markup_parser,0, parse_data, (GDestroyNotify)parse_data_free);
  parse_error = NULL;
  retval = g_markup_parse_context_parse(context, buffer, length, &parse_error);
  if (!retval) {
      g_propagate_error(error, parse_error);
      return FALSE;
  }
  end_error = NULL;
  retval = g_markup_parse_context_end_parse(context, &end_error);
  if (!retval) {
      g_propagate_error(error, end_error);
      return FALSE;
  }
  g_markup_parse_context_free(context);
  return TRUE;
}
static gchar* g_bookmark_file_dump(GBookmarkFile *bookmark, gsize *length, GError **error) {
  GString *retval;
  gchar *buffer;
  GList *l;
  retval = g_string_sized_new(4096);
  g_string_append(retval,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                #if 0
		          "<!DOCTYPE " XBEL_DTD_NICK "\n  PUBLIC \"" XBEL_DTD_SYSTEM "\"\n         \"" XBEL_DTD_URI "\">\n"
                #endif
		          "<" XBEL_ROOT_ELEMENT " " XBEL_VERSION_ATTRIBUTE "=\"" XBEL_VERSION "\"\n      xmlns:" BOOKMARK_NAMESPACE_NAME "=\"" BOOKMARK_NAMESPACE_URI "\"\n"
		          "      xmlns:" MIME_NAMESPACE_NAME     "=\"" MIME_NAMESPACE_URI "\"\n>");
  if (bookmark->title) {
      gchar *escaped_title;
      escaped_title = g_markup_escape_text(bookmark->title, -1);
      buffer = g_strconcat ("  <" XBEL_TITLE_ELEMENT ">", escaped_title, "</" XBEL_TITLE_ELEMENT ">\n", NULL);
      g_string_append(retval, buffer);
      g_free(buffer);
      g_free(escaped_title);
  }
  if (bookmark->description) {
      gchar *escaped_desc;
      escaped_desc = g_markup_escape_text(bookmark->description, -1);
      buffer = g_strconcat("  <" XBEL_DESC_ELEMENT ">", escaped_desc, "</" XBEL_DESC_ELEMENT ">\n", NULL);
      g_string_append(retval, buffer);
      g_free(buffer);
      g_free(escaped_desc);
  }
  if (!bookmark->items) goto out;
  else retval = g_string_append(retval, "\n");
  for (l = g_list_last(bookmark->items); l != NULL; l = l->prev) {
      BookmarkItem *item = (BookmarkItem*)l->data;
      gchar *item_dump;
      item_dump = bookmark_item_dump(item);
      if (!item_dump) continue;
      retval = g_string_append(retval, item_dump);
      g_free(item_dump);
  }
  out:
  g_string_append(retval, "</" XBEL_ROOT_ELEMENT ">");
  if (length) *length = retval->len;
  return g_string_free(retval, FALSE);
}
static gchar* timestamp_to_iso8601(time_t timestamp) {
  GTimeVal stamp;
  if (timestamp == (time_t) -1) g_get_current_time(&stamp);
  else {
      stamp.tv_sec = timestamp;
      stamp.tv_usec = 0;
  }
  return g_time_val_to_iso8601(&stamp);
}
static time_t timestamp_from_iso8601(const gchar *iso_date) {
  GTimeVal stamp;
  if (!g_time_val_from_iso8601(iso_date, &stamp)) return (time_t)-1;
  return (time_t)stamp.tv_sec;
}
GQuark g_bookmark_file_error_quark (void) {
  return g_quark_from_static_string("g-bookmark-file-error-quark");
}
GBookmarkFile* g_bookmark_file_new(void) {
  GBookmarkFile *bookmark;
  bookmark = g_new(GBookmarkFile, 1);
  g_bookmark_file_init(bookmark);
  return bookmark;
}
void g_bookmark_file_free(GBookmarkFile *bookmark) {
  if (!bookmark) return;
  g_bookmark_file_clear(bookmark);
  g_free(bookmark);
}
gboolean g_bookmark_file_load_from_data(GBookmarkFile *bookmark, const gchar *data, gsize length, GError **error) {
  GError *parse_error;
  gboolean retval;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  if (length == (gsize) -1) length = strlen(data);
  if (bookmark->items) {
      g_bookmark_file_clear(bookmark);
      g_bookmark_file_init(bookmark);
  }
  parse_error = NULL;
  retval = g_bookmark_file_parse(bookmark, data, length, &parse_error);
  if (!retval) {
      g_propagate_error(error, parse_error);
      return FALSE;
  }
  return TRUE;
}
gboolean g_bookmark_file_load_from_file(GBookmarkFile *bookmark, const gchar *filename, GError **error) {
  gchar *buffer;
  gsize len;
  GError *read_error;
  gboolean retval;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(filename != NULL, FALSE);
  read_error = NULL;
  g_file_get_contents(filename, &buffer, &len, &read_error);
  if (read_error) {
      g_propagate_error(error, read_error);
      return FALSE;
  }
  read_error = NULL;
  retval = g_bookmark_file_load_from_data(bookmark, buffer, len, &read_error);
  if (read_error) {
      g_propagate_error(error, read_error);
      g_free(buffer);
      return FALSE;
  }
  g_free(buffer);
  return retval;
}
/*gchar* find_file_in_data_dirs(const gchar *file, const gchar **dirs, GError **error) {
  gchar **data_dirs, *data_dir, *path;
  path = NULL;
  if (dirs == NULL) return NULL;
  data_dirs = *dirs;
  path = NULL;
  while(data_dirs && (data_dir = *data_dirs) && !path) {
      gchar *candidate_file, *sub_dir;
      candidate_file = (gchar*)file;
      sub_dir = g_strdup("");
      while(candidate_file != NULL && !path) {
          gchar *p;
          path = g_build_filename(data_dir, sub_dir, candidate_file, NULL);
          candidate_file = strchr(candidate_file, '-');
          if (candidate_file == NULL) break;
          candidate_file++;
          g_free(sub_dir);
          sub_dir = g_strndup(file, candidate_file - file - 1);
          for (p = sub_dir; *p != '\0'; p++) {
              if (*p == '-') *p = G_DIR_SEPARATOR;
          }
      }
      g_free(sub_dir);
      data_dirs++;
  }
  *dirs = data_dirs;
  if (!path) {
      g_set_error_literal(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_FILE_NOT_FOUND, _("No valid bookmark file found in data dirs"));
      return NULL;
  }
  return path;
}*/
gboolean g_bookmark_file_load_from_data_dirs(GBookmarkFile *bookmark, const gchar *file, gchar **full_path, GError **error) {
  GError *file_error = NULL;
  gchar **all_data_dirs, **data_dirs, **output_file;
  const gchar *user_data_dir;
  const gchar* const* system_data_dirs;
  gsize i, j;
  gchar *output_path;
  gboolean found_file;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(!g_path_is_absolute (file), FALSE);
  user_data_dir = g_get_user_data_dir();
  system_data_dirs = g_get_system_data_dirs();
  all_data_dirs = g_new0(gchar*, g_strv_length((gchar**)system_data_dirs) + 2);
  i = 0;
  all_data_dirs[i++] = g_strdup(user_data_dir);
  j = 0;
  while(system_data_dirs[j] != NULL) all_data_dirs[i++] = g_strdup(system_data_dirs[j++]);
  found_file = FALSE;
  data_dirs = all_data_dirs;
  output_path = NULL;
  while(*data_dirs != NULL && !found_file) {
      g_free(output_path);
      find_file_in_data_dirs(file, &data_dirs, output_file, &file_error);
      if (file_error) {
          g_propagate_error(error, file_error);
          break;
      }
      found_file = g_bookmark_file_load_from_file(bookmark, output_file, &file_error);
      if (file_error) {
          g_propagate_error(error, file_error);
          break;
      }
  }
  if (found_file && full_path) *full_path = output_file;
  else g_free(output_path);
  g_strfreev(all_data_dirs);
  return found_file;
}
gchar* g_bookmark_file_to_data(GBookmarkFile *bookmark, gsize *length, GError **error) {
  GError *write_error = NULL;
  gchar *retval;
  g_return_val_if_fail(bookmark != NULL, NULL);
  retval = g_bookmark_file_dump(bookmark, length, &write_error);
  if (write_error) {
      g_propagate_error(error, write_error);
      return NULL;
  }
  return retval;
}
gboolean g_bookmark_file_to_file(GBookmarkFile  *bookmark, const gchar *filename, GError **error) {
  gchar *data;
  GError *data_error, *write_error;
  gsize len;
  gboolean retval;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(filename != NULL, FALSE);
  data_error = NULL;
  data = g_bookmark_file_to_data(bookmark, &len, &data_error);
  if (data_error) {
      g_propagate_error(error, data_error);
      return FALSE;
  }
  write_error = NULL;
  g_file_set_contents(filename, data, len, &write_error);
  if (write_error) {
      g_propagate_error(error, write_error);
      retval = FALSE;
  } else retval = TRUE;
  g_free(data);
  return retval;
}
static BookmarkItem* g_bookmark_file_lookup_item(GBookmarkFile *bookmark, const gchar *uri) {
  g_warn_if_fail(bookmark != NULL && uri != NULL);
  return g_hash_table_lookup(bookmark->items_by_uri, uri);
}
static void g_bookmark_file_add_item(GBookmarkFile *bookmark, BookmarkItem *item, GError **error) {
  g_warn_if_fail(bookmark != NULL);
  g_warn_if_fail(item != NULL);
  if (G_UNLIKELY(g_bookmark_file_has_item (bookmark, item->uri))) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_INVALID_URI, _("A bookmark for URI '%s' already exists"),
                  item->uri);
      return;
  }
  bookmark->items = g_list_prepend(bookmark->items, item);
  g_hash_table_replace(bookmark->items_by_uri, item->uri, item);
  if (item->added == (time_t) -1) item->added = time(NULL);
  if (item->modified == (time_t) -1) item->modified = time(NULL);
}
gboolean g_bookmark_file_remove_item (GBookmarkFile *bookmark, const gchar *uri, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return FALSE;
  }
  bookmark->items = g_list_remove(bookmark->items, item);
  g_hash_table_remove(bookmark->items_by_uri, item->uri);
  bookmark_item_free(item);
  return TRUE;
}
gboolean g_bookmark_file_has_item(GBookmarkFile *bookmark, const gchar *uri) {
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  return (NULL != g_hash_table_lookup (bookmark->items_by_uri, uri));
}
gchar** g_bookmark_file_get_uris(GBookmarkFile *bookmark, gsize *length) {
  GList *l;
  gchar **uris;
  gsize i, n_items;
  g_return_val_if_fail(bookmark != NULL, NULL);
  n_items = g_list_length (bookmark->items); 
  uris = g_new0(gchar *, n_items + 1);
  for (l = g_list_last(bookmark->items), i = 0; l != NULL; l = l->prev) {
      BookmarkItem *item = (BookmarkItem*)l->data;
      g_warn_if_fail(item != NULL);
      uris[i++] = g_strdup(item->uri);
  }
  uris[i] = NULL;
  if (length) *length = i;
  return uris;
}
void g_bookmark_file_set_title(GBookmarkFile *bookmark, const gchar *uri, const gchar *title) {
  g_return_if_fail(bookmark != NULL);
  if (!uri) {
      g_free(bookmark->title);
      bookmark->title = g_strdup(title);
  } else {
      BookmarkItem *item;
      item = g_bookmark_file_lookup_item(bookmark, uri);
      if (!item) {
          item = bookmark_item_new(uri);
          g_bookmark_file_add_item(bookmark, item, NULL);
      }
      g_free(item->title);
      item->title = g_strdup(title);
      item->modified = time(NULL);
  }
}
gchar* g_bookmark_file_get_title(GBookmarkFile *bookmark, const gchar *uri, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, NULL);
  if (!uri) return g_strdup(bookmark->title);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return NULL;
  }
  return g_strdup (item->title);
}
void g_bookmark_file_set_description(GBookmarkFile *bookmark, const gchar *uri, const gchar *description) {
  g_return_if_fail(bookmark != NULL);
  if (!uri) {
      g_free(bookmark->description);
      bookmark->description = g_strdup(description);
  } else {
      BookmarkItem *item;
      item = g_bookmark_file_lookup_item(bookmark, uri);
      if (!item) {
          item = bookmark_item_new(uri);
          g_bookmark_file_add_item(bookmark, item, NULL);
      }
      g_free(item->description);
      item->description = g_strdup(description);
      item->modified = time(NULL);
  }
}
gchar* g_bookmark_file_get_description(GBookmarkFile *bookmark, const gchar *uri, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, NULL);
  if (!uri) return g_strdup(bookmark->description);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return NULL;
  }
  return g_strdup (item->description);
}
void g_bookmark_file_set_mime_type(GBookmarkFile *bookmark, const gchar *uri, const gchar *mime_type) {
  BookmarkItem *item;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  g_return_if_fail(mime_type != NULL);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (!item->metadata) item->metadata = bookmark_metadata_new();
  g_free(item->metadata->mime_type);
  item->metadata->mime_type = g_strdup(mime_type);
  item->modified = time(NULL);
}
gchar* g_bookmark_file_get_mime_type(GBookmarkFile *bookmark, const gchar *uri, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, NULL);
  g_return_val_if_fail(uri != NULL, NULL);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return NULL;
  }
  if (!item->metadata) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_INVALID_VALUE, _("No MIME type defined in the bookmark for URI '%s'"), uri);
      return NULL;
  }
  return g_strdup(item->metadata->mime_type);
}
void g_bookmark_file_set_is_private(GBookmarkFile *bookmark, const gchar *uri, gboolean is_private) {
  BookmarkItem *item;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (!item->metadata) item->metadata = bookmark_metadata_new();
  item->metadata->is_private = (is_private == TRUE);
  item->modified = time(NULL);
}
gboolean g_bookmark_file_get_is_private(GBookmarkFile *bookmark, const gchar *uri, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return FALSE;
  }
  if (!item->metadata) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_INVALID_VALUE, _("No private flag has been defined in bookmark for URI '%s'"), uri);
      return FALSE;
  }
  return item->metadata->is_private;
}
void g_bookmark_file_set_added(GBookmarkFile *bookmark, const gchar *uri, time_t added) {
  BookmarkItem *item;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (added == (time_t) -1) time(&added);
  item->added = added;
  item->modified = added;
}
time_t g_bookmark_file_get_added(GBookmarkFile *bookmark, const gchar *uri, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, (time_t) -1);
  g_return_val_if_fail(uri != NULL, (time_t) -1);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return (time_t) -1;
  }
  return item->added;
}
void g_bookmark_file_set_modified(GBookmarkFile *bookmark, const gchar *uri, time_t modified) {
  BookmarkItem *item;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (modified == (time_t) -1) time(&modified);
  item->modified = modified;
}
time_t g_bookmark_file_get_modified(GBookmarkFile *bookmark, const gchar *uri, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, (time_t) -1);
  g_return_val_if_fail(uri != NULL, (time_t) -1);
  item = g_bookmark_file_lookup_item (bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return (time_t) -1;
  }
  return item->modified;
}
void g_bookmark_file_set_visited(GBookmarkFile *bookmark, const gchar *uri, time_t visited) {
  BookmarkItem *item;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (visited == (time_t) -1) time(&visited);
  item->visited = visited;
}
time_t g_bookmark_file_get_visited(GBookmarkFile *bookmark, const gchar *uri, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, (time_t) -1);
  g_return_val_if_fail(uri != NULL, (time_t) -1);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return (time_t) -1;
  }
  return item->visited;
}
gboolean g_bookmark_file_has_group(GBookmarkFile *bookmark, const gchar *uri, const gchar *group, GError **error) {
  BookmarkItem *item;
  GList *l;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return FALSE;
  }
  if (!item->metadata) return FALSE;
  for (l = item->metadata->groups; l != NULL; l = l->next) {
      if (strcmp (l->data, group) == 0) return TRUE;
  }
  return FALSE;
}
void g_bookmark_file_add_group(GBookmarkFile *bookmark, const gchar *uri, const gchar *group) {
  BookmarkItem *item;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  g_return_if_fail(group != NULL && group[0] != '\0');
  item = g_bookmark_file_lookup_item (bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (!item->metadata) item->metadata = bookmark_metadata_new();
  if (!g_bookmark_file_has_group (bookmark, uri, group, NULL)) {
      item->metadata->groups = g_list_prepend(item->metadata->groups, g_strdup (group));
      item->modified = time(NULL);
  }
}
gboolean g_bookmark_file_remove_group(GBookmarkFile *bookmark, const gchar *uri, const gchar *group, GError **error) {
  BookmarkItem *item;
  GList *l;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return FALSE;
  }
  if (!item->metadata) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_INVALID_VALUE, _("No groups set in bookmark for URI '%s'"), uri);
      return FALSE;
  }
  for (l = item->metadata->groups; l != NULL; l = l->next) {
      if (strcmp (l->data, group) == 0) {
          item->metadata->groups = g_list_remove_link(item->metadata->groups, l);
          g_free(l->data);
          g_list_free_1(l);
          item->modified = time(NULL);
          return TRUE;
      }
  }
  return FALSE;
}
void g_bookmark_file_set_groups(GBookmarkFile *bookmark, const gchar *uri, const gchar **groups, gsize length) {
  BookmarkItem *item;
  gsize i;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  g_return_if_fail(groups != NULL);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (!item->metadata) item->metadata = bookmark_metadata_new();
  if (item->metadata->groups != NULL) {
      g_list_foreach(item->metadata->groups, (GFunc)g_free, NULL);
      g_list_free(item->metadata->groups);
      item->metadata->groups = NULL;
  }
  if (groups) {
      for (i = 0; groups[i] != NULL && i < length; i++) item->metadata->groups = g_list_append(item->metadata->groups, g_strdup (groups[i]));
  }
  item->modified = time(NULL);
}
gchar **g_bookmark_file_get_groups (GBookmarkFile  *bookmark, const gchar *uri, gsize *length, GError **error) {
  BookmarkItem *item;
  GList *l;
  gsize len, i;
  gchar **retval;
  g_return_val_if_fail(bookmark != NULL, NULL);
  g_return_val_if_fail(uri != NULL, NULL);
  item = g_bookmark_file_lookup_item (bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return NULL;
  }
  if (!item->metadata) {
      if (length) *length = 0;
      return NULL;
  }
  len = g_list_length(item->metadata->groups);
  retval = g_new0(gchar *, len + 1);
  for (l = g_list_last(item->metadata->groups), i = 0; l != NULL; l = l->prev) {
      gchar *group_name = (gchar*)l->data;
      g_warn_if_fail(group_name != NULL);
      retval[i++] = g_strdup(group_name);
  }
  retval[i] = NULL;
  if (length) *length = len;
  return retval;
}
void g_bookmark_file_add_application(GBookmarkFile *bookmark, const gchar *uri, const gchar *name, const gchar *exec) {
  BookmarkItem *item;
  gchar *app_name, *app_exec;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  item = g_bookmark_file_lookup_item (bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (name && name[0] != '\0') app_name = g_strdup(name);
  else app_name = g_strdup(g_get_application_name());
  if (exec && exec[0] != '\0') app_exec = g_strdup(exec);
  else app_exec = g_strjoin(" ", g_get_prgname(), "%u", NULL);
  g_bookmark_file_set_app_info(bookmark, uri, app_name,app_exec,-1,(time_t)-1,NULL);
  g_free(app_exec);
  g_free(app_name);
}
gboolean g_bookmark_file_remove_application(GBookmarkFile *bookmark, const gchar *uri, const gchar *name, GError **error) {
  GError *set_error;
  gboolean retval;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);
  set_error = NULL;
  retval = g_bookmark_file_set_app_info(bookmark, uri, name,"",0,(time_t)-1, &set_error);
  if (set_error) {
      g_propagate_error(error, set_error);
      return FALSE;
  }
  return retval;
}
gboolean g_bookmark_file_has_application(GBookmarkFile *bookmark, const gchar *uri, const gchar *name, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return FALSE;
  }
  return (NULL != bookmark_item_lookup_app_info (item, name));
}
gboolean g_bookmark_file_set_app_info(GBookmarkFile  *bookmark, const gchar *uri, const gchar *name, const gchar *exec, gint count, time_t stamp, GError **error) {
  BookmarkItem *item;
  BookmarkAppInfo *ai;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);
  g_return_val_if_fail(exec != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      if (count == 0) {
          g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
          return FALSE;
      } else {
          item = bookmark_item_new (uri);
	      g_bookmark_file_add_item (bookmark, item, NULL);
      }
  }
  ai = bookmark_item_lookup_app_info(item, name);
  if (!ai) {
      if (count == 0) {
          g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_APP_NOT_REGISTERED, _("No application with name '%s' registered a bookmark for '%s'"),
                      name, uri);
          return FALSE;
      } else {
          ai = bookmark_app_info_new (name);
          item->metadata->applications = g_list_prepend (item->metadata->applications, ai);
          g_hash_table_replace (item->metadata->apps_by_name, ai->name, ai);
      }
  }
  if (count == 0) {
      item->metadata->applications = g_list_remove(item->metadata->applications, ai);
      g_hash_table_remove(item->metadata->apps_by_name, ai->name);
      bookmark_app_info_free(ai);
      item->modified = time(NULL);
      return TRUE;
  } else if (count > 0) ai->count = count;
  else ai->count += 1;
  if (stamp != (time_t) -1) ai->stamp = stamp;
  else ai->stamp = time(NULL);
  if (exec && exec[0] != '\0') {
      g_free(ai->exec);
      ai->exec = g_shell_quote(exec);
  }
  item->modified = time(NULL);
  return TRUE;
}
static gchar* expand_exec_line(const gchar *exec_fmt, const gchar *uri) {
  GString *exec;
  gchar ch;
  exec = g_string_sized_new(512);
  while ((ch = *exec_fmt++) != '\0') {
     if (ch != '%') {
         exec = g_string_append_c(exec, ch);
         continue;
     }
     ch = *exec_fmt++;
     switch (ch) {
       case '\0': goto out;
       case 'U': case 'u': g_string_append(exec, uri); break;
       case 'F': case 'f': {
           gchar *file = g_filename_from_uri(uri, NULL, NULL);
           if (file) {
	       g_string_append(exec, file);
	       g_free(file);
           } else {
               g_string_free(exec, TRUE);
               return NULL;
           }
         }
         break;
       default: exec = g_string_append_c(exec, ch);
     }
  }
  out:
  return g_string_free(exec, FALSE);
}
gboolean g_bookmark_file_get_app_info(GBookmarkFile  *bookmark, const gchar *uri, const gchar *name, gchar **exec, guint *count, time_t *stamp, GError **error) {
  BookmarkItem *item;
  BookmarkAppInfo *ai;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return FALSE;
  }
  ai = bookmark_item_lookup_app_info(item, name);
  if (!ai) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_APP_NOT_REGISTERED, _("No application with name '%s' registered a bookmark for '%s'"),
                  name, uri);
      return FALSE;
  }
  if (exec) {
      GError *unquote_error = NULL;
      gchar *command_line;
      command_line = g_shell_unquote(ai->exec, &unquote_error);
      if (unquote_error) {
          g_propagate_error(error, unquote_error);
          return FALSE;
      }
      *exec = expand_exec_line(command_line, uri);
      if (!*exec) {
          g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_INVALID_URI, _("Failed to expand exec line '%s' with URI '%s'"), ai->exec, uri);
          g_free (command_line);
          return FALSE;
      } else g_free(command_line);
  }
  if (count) *count = ai->count;
  if (stamp) *stamp = ai->stamp;
  return TRUE;
}
gchar** g_bookmark_file_get_applications(GBookmarkFile *bookmark, const gchar *uri, gsize *length, GError **error) {
  BookmarkItem *item;
  GList *l;
  gchar **apps;
  gsize i, n_apps;
  g_return_val_if_fail(bookmark != NULL, NULL);
  g_return_val_if_fail(uri != NULL, NULL);
  item = g_bookmark_file_lookup_item (bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return NULL;
  }
  if (!item->metadata) {
      if (length) *length = 0;
      return NULL;
  }
  n_apps = g_list_length(item->metadata->applications);
  apps = g_new0(gchar *, n_apps + 1);
  for (l = g_list_last(item->metadata->applications), i = 0; l != NULL; l = l->prev) {
      BookmarkAppInfo *ai;
      ai = (BookmarkAppInfo*)l->data;
      g_warn_if_fail(ai != NULL);
      g_warn_if_fail(ai->name != NULL);
      apps[i++] = g_strdup(ai->name);
  }
  apps[i] = NULL;
  if (length) *length = i;
  return apps;
}
gint g_bookmark_file_get_size(GBookmarkFile *bookmark) {
  g_return_val_if_fail(bookmark != NULL, 0);
  return g_list_length(bookmark->items);
}
gboolean g_bookmark_file_move_item(GBookmarkFile *bookmark, const gchar *old_uri, const gchar *new_uri, GError **error) {
  BookmarkItem *item;
  GError *remove_error;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(old_uri != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, old_uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), old_uri);
      return FALSE;
  }
  if (new_uri && new_uri[0] != '\0') {
      if (g_bookmark_file_has_item(bookmark, new_uri)) {
          remove_error = NULL;
          g_bookmark_file_remove_item(bookmark, new_uri, &remove_error);
          if (remove_error) {
              g_propagate_error(error, remove_error);
              return FALSE;
          }
      }
      g_hash_table_steal(bookmark->items_by_uri, item->uri);
      g_free(item->uri);
      item->uri = g_strdup(new_uri);
      item->modified = time(NULL);
      g_hash_table_replace(bookmark->items_by_uri, item->uri, item);
      return TRUE;
  } else {
      remove_error = NULL;
      g_bookmark_file_remove_item(bookmark, old_uri, &remove_error);
      if (remove_error) {
          g_propagate_error(error, remove_error);
          return FALSE;
      }
      return TRUE;
  }
}
void g_bookmark_file_set_icon(GBookmarkFile *bookmark, const gchar *uri, const gchar *href, const gchar *mime_type) {
  BookmarkItem *item;
  g_return_if_fail(bookmark != NULL);
  g_return_if_fail(uri != NULL);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      item = bookmark_item_new(uri);
      g_bookmark_file_add_item(bookmark, item, NULL);
  }
  if (!item->metadata) item->metadata = bookmark_metadata_new();
  g_free(item->metadata->icon_href);
  g_free(item->metadata->icon_mime);
  item->metadata->icon_href = g_strdup(href);
  if (mime_type && mime_type[0] != '\0') item->metadata->icon_mime = g_strdup(mime_type);
  else item->metadata->icon_mime = g_strdup("application/octet-stream");
  item->modified = time(NULL);
}
gboolean g_bookmark_file_get_icon(GBookmarkFile *bookmark, const gchar *uri, gchar **href, gchar **mime_type, GError **error) {
  BookmarkItem *item;
  g_return_val_if_fail(bookmark != NULL, FALSE);
  g_return_val_if_fail(uri != NULL, FALSE);
  item = g_bookmark_file_lookup_item(bookmark, uri);
  if (!item) {
      g_set_error(error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND, _("No bookmark found for URI '%s'"), uri);
      return FALSE;
  }
  if ((!item->metadata) || (!item->metadata->icon_href)) return FALSE;
  if (href) *href = g_strdup(item->metadata->icon_href);
  if (mime_type) *mime_type = g_strdup(item->metadata->icon_mime);
  return TRUE;
}