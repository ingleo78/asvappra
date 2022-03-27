#ifndef __GST_PARAMSPECS_H__
#define __GST_PARAMSPECS_H__

#include "gstvalue.h"

G_BEGIN_DECLS
#define	GST_PARAM_CONTROLLABLE	(1 << (G_PARAM_USER_SHIFT + 1))
#define GST_PARAM_MUTABLE_READY  (1 << (G_PARAM_USER_SHIFT + 2))
#define GST_PARAM_MUTABLE_PAUSED  (1 << (G_PARAM_USER_SHIFT + 3))
#define GST_PARAM_MUTABLE_PLAYING  (1 << (G_PARAM_USER_SHIFT + 4))
#define	GST_PARAM_USER_SHIFT	(1 << (G_PARAM_USER_SHIFT + 8))
#define GST_TYPE_PARAM_FRACTION           (gst_param_spec_fraction_get_type ())
#define GST_IS_PARAM_SPEC_FRACTION(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), GST_TYPE_PARAM_FRACTION))
#define GST_PARAM_SPEC_FRACTION(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), GST_TYPE_PARAM_FRACTION, GstParamSpecFraction))
GType  gst_param_spec_fraction_get_type (void);
typedef struct _GstParamSpecFraction GstParamSpecFraction;
struct _GstParamSpecFraction {
  GParamSpec    parent_instance;
  gint          min_num, min_den;
  gint          max_num, max_den;
  gint          def_num, def_den;
};
GParamSpec *gst_param_spec_fraction(const gchar *name, const gchar *nick, const gchar *blurb, gint min_num, gint min_denom, gint max_num,
                                    gint max_denom, gint default_num, gint default_denom, GParamFlags flags) G_GNUC_MALLOC;
G_END_DECLS

#endif