#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "gtimezone.h"
#include "gmappedfile.h"
#include "gtestutils.h"
#include "gfileutils.h"
#include "gstrfuncs.h"
#include "ghash.h"
#include "gthread.h"
#include "gbuffer.h"

typedef struct { gchar bytes[8]; } gint64_be;
typedef struct { gchar bytes[4]; } gint32_be;
typedef struct { gchar bytes[4]; } guint32_be;
static inline gint64 gint64_from_be(const gint64_be be) {
  gint64 tmp; memcpy(&tmp, &be, sizeof tmp); return GINT64_FROM_BE(tmp);
}
static inline gint32 gint32_from_be (const gint32_be be) {
  gint32 tmp; memcpy(&tmp, &be, sizeof tmp); return GINT32_FROM_BE(tmp);
}
static inline guint32 guint32_from_be (const guint32_be be) {
  guint32 tmp; memcpy(&tmp, &be, sizeof tmp); return GUINT32_FROM_BE(tmp);
}
struct tzhead {
  gchar tzh_magic[4];
  gchar tzh_version;
  guchar tzh_reserved[15];
  guint32_be tzh_ttisgmtcnt;
  guint32_be tzh_ttisstdcnt;
  guint32_be tzh_leapcnt;
  guint32_be tzh_timecnt;
  guint32_be tzh_typecnt;
  guint32_be tzh_charcnt;
};
struct ttinfo {
  gint32_be tt_gmtoff;
  guint8 tt_isdst;
  guint8 tt_abbrind;
};
struct _GTimeZone {
  gchar *name;
  GBuffer *zoneinfo;
  const struct tzhead *header;
  const struct ttinfo *infos;
  const gint64_be *trans;
  const guint8 *indices;
  const gchar *abbrs;
  gint timecnt;
  gint ref_count;
};
G_LOCK_DEFINE_STATIC(time_zones);
static GHashTable *time_zones;
static guint g_str_hash0(gconstpointer data) {
  return data ? g_str_hash (data) : 0;
}
static gboolean g_str_equal0(gconstpointer a, gconstpointer b) {
  if (a == b) return TRUE;
  if (!a || !b) return FALSE;
  return g_str_equal(a, b);
}
void g_time_zone_unref(GTimeZone *tz) {
  g_assert(tz->ref_count > 0);
  if (g_atomic_int_dec_and_test(&tz->ref_count)) {
      G_LOCK(time_zones);
      g_hash_table_remove(time_zones, tz->name);
      G_UNLOCK(time_zones);
      if (tz->zoneinfo) g_buffer_unref(tz->zoneinfo);
      g_free(tz->name);
      g_slice_free(GTimeZone, tz);
  }
}
GTimeZone *g_time_zone_ref(GTimeZone *tz) {
  g_assert(tz->ref_count > 0);
  g_atomic_int_inc(&tz->ref_count);
  return tz;
}
static gboolean parse_time(const gchar *time_, gint32 *offset) {
  if (*time_ < '0' || '2' < *time_) return FALSE;
  *offset = 10 * 60 * 60 * (*time_++ - '0');
  if (*time_ < '0' || '9' < *time_) return FALSE;
  *offset += 60 * 60 * (*time_++ - '0');
  if (*offset > 23 * 60 * 60) return FALSE;
  if (*time_ == '\0') return TRUE;
  if (*time_ == ':') time_++;
  if (*time_ < '0' || '5' < *time_) return FALSE;
  *offset += 10 * 60 * (*time_++ - '0');
  if (*time_ < '0' || '9' < *time_) return FALSE;
  *offset += 60 * (*time_++ - '0');
  return *time_ == '\0';
}
static gboolean parse_constant_offset(const gchar *name, gint32 *offset) {
  switch (*name++) {
      case 'Z':
          *offset = 0;
          return !*name;
      case '+': return parse_time (name, offset);
      case '-':
          if (parse_time (name, offset)) {
              *offset = -*offset;
              return TRUE;
          }
      default: return FALSE;
  }
}
static GBuffer* zone_for_constant_offset(const gchar *name) {
  const gchar fake_zoneinfo_headers[] = "TZif" "2..." "...." "...." "....\0\0\0\0" "\0\0\0\0" "\0\0\0\0" "\0\0\0\0" "\0\0\0\0" "\0\0\0\0"
                                        "TZif" "2..." "...." "...." "....\0\0\0\0" "\0\0\0\0" "\0\0\0\0" "\0\0\0\0" "\0\0\0\1" "\0\0\0\7";
  struct {
    struct tzhead headers[2];
    struct ttinfo info;
    gchar abbr[8];
  } *fake;
  gint32 offset;
  if (name == NULL || !parse_constant_offset(name, &offset)) return NULL;
  offset = GINT32_TO_BE(offset);
  fake = g_malloc(sizeof *fake);
  memcpy(fake, fake_zoneinfo_headers, sizeof fake_zoneinfo_headers);
  memcpy(&fake->info.tt_gmtoff, &offset, sizeof offset);
  fake->info.tt_isdst = FALSE;
  fake->info.tt_abbrind = 0;
  strcpy(fake->abbr, name);
  return g_buffer_new_take_data(fake, sizeof *fake);
}
GTimeZone* g_time_zone_new(const gchar *identifier) {
  GTimeZone *tz;
  G_LOCK(time_zones);
  if (time_zones == NULL) time_zones = g_hash_table_new(g_str_hash0, g_str_equal0);
  tz = g_hash_table_lookup(time_zones, identifier);
  if (tz == NULL) {
      tz = g_slice_new0(GTimeZone);
      tz->name = g_strdup(identifier);
      tz->ref_count = 0;
      tz->zoneinfo = zone_for_constant_offset(identifier);
      if (tz->zoneinfo == NULL) {
          gchar *filename;
          if (identifier != NULL) {
              const gchar *tzdir;
              tzdir = getenv ("TZDIR");
              if (tzdir == NULL) tzdir = "/usr/share/zoneinfo";
              filename = g_build_filename(tzdir, identifier, NULL);
          } else filename = g_strdup("/etc/localtime");
          tz->zoneinfo = (GBuffer*)g_mapped_file_new(filename, FALSE, NULL);
          g_free(filename);
      }
      if (tz->zoneinfo != NULL) {
          const struct tzhead *header = tz->zoneinfo->data;
          gsize size = tz->zoneinfo->size;
          if (size < sizeof (struct tzhead) || memcmp (header, "TZif2", 5)) {
              g_buffer_unref (tz->zoneinfo);
              tz->zoneinfo = NULL;
          } else {
              gint typecnt;
              tz->header = (const struct tzhead*)(((const gchar*)(header + 1)) + guint32_from_be(header->tzh_ttisgmtcnt) + guint32_from_be(header->tzh_ttisstdcnt) +
                           8 * guint32_from_be(header->tzh_leapcnt) + 5 * guint32_from_be(header->tzh_timecnt) + 6 * guint32_from_be(header->tzh_typecnt) +
                           guint32_from_be(header->tzh_charcnt));
              typecnt = guint32_from_be(tz->header->tzh_typecnt);
              tz->timecnt = guint32_from_be(tz->header->tzh_timecnt);
              tz->trans = (gconstpointer)(tz->header + 1);
              tz->indices = (gconstpointer)(tz->trans + tz->timecnt);
              tz->infos = (gconstpointer)(tz->indices + tz->timecnt);
              tz->abbrs = (gconstpointer)(tz->infos + typecnt);
          }
      }
      g_hash_table_insert (time_zones, tz->name, tz);
  }
  g_atomic_int_inc(&tz->ref_count);
  G_UNLOCK(time_zones);
  return tz;
}
GTimeZone* g_time_zone_new_utc(void) {
  return g_time_zone_new("UTC");
}
GTimeZone* g_time_zone_new_local(void) {
  return g_time_zone_new(getenv("TZ"));
}
inline static const struct ttinfo* interval_info(GTimeZone *tz, gint interval) {
  if (interval) return tz->infos + tz->indices[interval - 1];
  return tz->infos;
}
inline static gint64 interval_start(GTimeZone *tz, gint interval) {
  if (interval) return gint64_from_be(tz->trans[interval - 1]);
  return G_MININT64;
}
inline static gint64 interval_end(GTimeZone *tz, gint interval) {
  if (interval < tz->timecnt) return gint64_from_be(tz->trans[interval]) - 1;
  return G_MAXINT64;
}
inline static gint32 interval_offset(GTimeZone *tz, gint interval) {
  return gint32_from_be(interval_info(tz, interval)->tt_gmtoff);
}
inline static gboolean interval_isdst(GTimeZone *tz, gint interval) {
  return interval_info(tz, interval)->tt_isdst;
}
inline static guint8 interval_abbrind(GTimeZone *tz, gint interval) {
  return interval_info(tz, interval)->tt_abbrind;
}
inline static gint64 interval_local_start(GTimeZone *tz, gint interval) {
  if (interval) return interval_start(tz, interval) + interval_offset(tz, interval);
  return G_MININT64;
}
inline static gint64 interval_local_end(GTimeZone *tz, gint interval) {
  if (interval < tz->timecnt) return interval_end(tz, interval) + interval_offset(tz, interval);
  return G_MAXINT64;
}
static gboolean interval_valid(GTimeZone *tz, gint interval) {
  return interval <= tz->timecnt;
}
gint g_time_zone_adjust_time(GTimeZone *tz, GTimeType type, gint64 *time_) {
  gint i;
  if (tz->zoneinfo == NULL) return 0;
  for (i = 0; i < tz->timecnt; i++)
      if (*time_ <= interval_end(tz, i)) break;
  g_assert(interval_start(tz, i) <= *time_ && *time_ <= interval_end(tz, i));
  if (type != G_TIME_TYPE_UNIVERSAL) {
      if (*time_ < interval_local_start(tz, i)) {
          i--;
          if (*time_ > interval_local_end(tz, i)) {
              i++;
              *time_ = interval_local_start(tz, i);
          }
      } else if (*time_ > interval_local_end(tz, i)) {
          i++;
          if (*time_ < interval_local_start(tz, i)) *time_ = interval_local_start(tz, i);
      } else if (interval_isdst(tz, i) != type) {
          if (i && *time_ <= interval_local_end(tz, i - 1)) i--;
          else if (i < tz->timecnt && *time_ >= interval_local_start(tz, i + 1)) i++;
      }
  }
  return i;
}
gint g_time_zone_find_interval(GTimeZone *tz, GTimeType  type, gint64 time_) {
  gint i;
  if (tz->zoneinfo == NULL) return 0;
  for (i = 0; i < tz->timecnt; i++) if (time_ <= interval_end(tz, i)) break;
  if (type == G_TIME_TYPE_UNIVERSAL) return i;
  if (time_ < interval_local_start(tz, i)) {
      if (time_ > interval_local_end(tz, --i)) return -1;
  } else if (time_ > interval_local_end(tz, i)) {
      if (time_ < interval_local_start(tz, ++i)) return -1;
  } else if (interval_isdst(tz, i) != type) {
      if (i && time_ <= interval_local_end(tz, i - 1)) i--;
      else if (i < tz->timecnt && time_ >= interval_local_start(tz, i + 1)) i++;
  }
  return i;
}
const gchar* g_time_zone_get_abbreviation(GTimeZone *tz, gint interval) {
  g_return_val_if_fail(interval_valid (tz, interval), NULL);
  if (tz->header == NULL) return "UTC";
  return tz->abbrs + interval_abbrind(tz, interval);
}
gint32 g_time_zone_get_offset(GTimeZone *tz, gint interval) {
  g_return_val_if_fail(interval_valid (tz, interval), 0);
  if (tz->header == NULL) return 0;
  return interval_offset(tz, interval);
}
gboolean g_time_zone_is_dst(GTimeZone *tz, gint interval) {
  g_return_val_if_fail(interval_valid(tz, interval), FALSE);
  if (tz->header == NULL) return FALSE;
  return interval_isdst(tz, interval);
}