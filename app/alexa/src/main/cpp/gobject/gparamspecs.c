#include <string.h>
#include "../gio/config.h"
#include "../glib/glib-object.h"
#include "gparamspecs.h"
#include "gvaluecollector.h"
#include "gvaluearray.h"

#define	G_FLOAT_EPSILON	 (1e-30)
#define	G_DOUBLE_EPSILON  (1e-90)
static void param_char_init(GParamSpec *pspec) {
    GParamSpecChar *cspec = G_PARAM_SPEC_CHAR(pspec);
    cspec->minimum = 0x7f;
    cspec->maximum = 0x80;
    cspec->default_value = 0;
}
static void param_char_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_int = G_PARAM_SPEC_CHAR(pspec)->default_value;
}
static gboolean param_char_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecChar *cspec = G_PARAM_SPEC_CHAR(pspec);
    gint oval = value->data[0].v_int;
    value->data[0].v_int = CLAMP(value->data[0].v_int, cspec->minimum, cspec->maximum);
    return value->data[0].v_int != oval;
}
static void param_uchar_init(GParamSpec *pspec) {
    GParamSpecUChar *uspec = G_PARAM_SPEC_UCHAR(pspec);
    uspec->minimum = 0;
    uspec->maximum = 0xff;
    uspec->default_value = 0;
}
static void param_uchar_set_default(GParamSpec *pspec, GValue	*value) {
    value->data[0].v_uint = G_PARAM_SPEC_UCHAR(pspec)->default_value;
}
static gboolean param_uchar_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecUChar *uspec = G_PARAM_SPEC_UCHAR(pspec);
    guint oval = value->data[0].v_uint;
    value->data[0].v_uint = CLAMP(value->data[0].v_uint, uspec->minimum, uspec->maximum);
    return value->data[0].v_uint != oval;
}
static void param_boolean_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_int = G_PARAM_SPEC_BOOLEAN(pspec)->default_value;
}
static gboolean param_boolean_validate(GParamSpec *pspec, GValue *value) {
    gint oval = value->data[0].v_int;
    value->data[0].v_int = value->data[0].v_int != FALSE;
    return value->data[0].v_int != oval;
}
static void param_int_init (GParamSpec *pspec) {
    GParamSpecInt *ispec = G_PARAM_SPEC_INT(pspec);
    ispec->minimum = 0x7fffffff;
    ispec->maximum = 0x80000000;
    ispec->default_value = 0;
}
static void param_int_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_int = G_PARAM_SPEC_INT(pspec)->default_value;
}
static gboolean param_int_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecInt *ispec = G_PARAM_SPEC_INT(pspec);
    gint oval = value->data[0].v_int;
    value->data[0].v_int = CLAMP(value->data[0].v_int, ispec->minimum, ispec->maximum);
    return value->data[0].v_int != oval;
}
static gint param_int_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    if (value1->data[0].v_int < value2->data[0].v_int) return -1;
    else return value1->data[0].v_int > value2->data[0].v_int;
}
static void param_uint_init(GParamSpec *pspec) {
    GParamSpecUInt *uspec = G_PARAM_SPEC_UINT(pspec);
    uspec->minimum = 0;
    uspec->maximum = 0xffffffff;
    uspec->default_value = 0;
}
static void param_uint_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_uint = G_PARAM_SPEC_UINT(pspec)->default_value;
}
static gboolean param_uint_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecUInt *uspec = G_PARAM_SPEC_UINT(pspec);
    guint oval = value->data[0].v_uint;
    value->data[0].v_uint = CLAMP(value->data[0].v_uint, uspec->minimum, uspec->maximum);
    return value->data[0].v_uint != oval;
}
static gint param_uint_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    if (value1->data[0].v_uint < value2->data[0].v_uint) return -1;
    else return value1->data[0].v_uint > value2->data[0].v_uint;
}
static void param_long_init(GParamSpec *pspec) {
    GParamSpecLong *lspec = G_PARAM_SPEC_LONG(pspec);
#if SIZEOF_LONG == 4
    lspec->minimum = 0x7fffffff;
    lspec->maximum = 0x80000000;
#else
    lspec->minimum = 0x7fffffffffffffff;
    lspec->maximum = 0x8000000000000000;
#endif
    lspec->default_value = 0;
}
static void param_long_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_long = G_PARAM_SPEC_LONG(pspec)->default_value;
}
static gboolean param_long_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecLong *lspec = G_PARAM_SPEC_LONG(pspec);
    glong oval = value->data[0].v_long;
    value->data[0].v_long = CLAMP(value->data[0].v_long, lspec->minimum, lspec->maximum);
    return value->data[0].v_long != oval;
}
static gint param_long_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    if (value1->data[0].v_long < value2->data[0].v_long) return -1;
    else return value1->data[0].v_long > value2->data[0].v_long;
}
static void param_ulong_init(GParamSpec *pspec) {
    GParamSpecULong *uspec = G_PARAM_SPEC_ULONG(pspec);
    uspec->minimum = 0;
#if SIZEOF_LONG == 4
    uspec->maximum = 0xffffffff;
#else
    uspec->maximum = 0xffffffffffffffff;
#endif
    uspec->default_value = 0;
}
static void param_ulong_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_ulong = G_PARAM_SPEC_ULONG(pspec)->default_value;
}
static gboolean param_ulong_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecULong *uspec = G_PARAM_SPEC_ULONG(pspec);
    gulong oval = value->data[0].v_ulong;
    value->data[0].v_ulong = CLAMP(value->data[0].v_ulong, uspec->minimum, uspec->maximum);
    return value->data[0].v_ulong != oval;
}
static gint param_ulong_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    if (value1->data[0].v_ulong < value2->data[0].v_ulong) return -1;
    else return value1->data[0].v_ulong > value2->data[0].v_ulong;
}
static void param_int64_init(GParamSpec *pspec) {
    GParamSpecInt64 *lspec = G_PARAM_SPEC_INT64(pspec);
    lspec->minimum = G_MININT64;
    lspec->maximum = G_MAXINT64;
    lspec->default_value = 0;
}
static void param_int64_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_int64 = G_PARAM_SPEC_INT64(pspec)->default_value;
}
static gboolean param_int64_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecInt64 *lspec = G_PARAM_SPEC_INT64(pspec);
    gint64 oval = value->data[0].v_int64;
    value->data[0].v_int64 = CLAMP(value->data[0].v_int64, lspec->minimum, lspec->maximum);
    return value->data[0].v_int64 != oval;
}
static gint param_int64_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    if (value1->data[0].v_int64 < value2->data[0].v_int64) return -1;
    else return value1->data[0].v_int64 > value2->data[0].v_int64;
}
static void param_uint64_init(GParamSpec *pspec) {
    GParamSpecUInt64 *uspec = G_PARAM_SPEC_UINT64(pspec);
    uspec->minimum = 0;
    uspec->maximum = G_MAXUINT64;
    uspec->default_value = 0;
}
static void param_uint64_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_uint64 = G_PARAM_SPEC_UINT64(pspec)->default_value;
}
static gboolean param_uint64_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecUInt64 *uspec = G_PARAM_SPEC_UINT64(pspec);
    guint64 oval = value->data[0].v_uint64;
    value->data[0].v_uint64 = CLAMP(value->data[0].v_uint64, uspec->minimum, uspec->maximum);
    return value->data[0].v_uint64 != oval;
}
static gint param_uint64_values_cmp(GParamSpec   *pspec, const GValue *value1, const GValue *value2) {
    if (value1->data[0].v_uint64 < value2->data[0].v_uint64) return -1;
    else return value1->data[0].v_uint64 > value2->data[0].v_uint64;
}
static void param_unichar_init (GParamSpec *pspec) {
    GParamSpecUnichar *uspec = G_PARAM_SPEC_UNICHAR(pspec);
    uspec->default_value = 0;
}
static void param_unichar_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_uint = G_PARAM_SPEC_UNICHAR(pspec)->default_value;
}
static gboolean param_unichar_validate(GParamSpec *pspec, GValue *value) {
    gunichar oval = value->data[0].v_uint;
    gboolean changed = FALSE;
    if (!g_unichar_validate(oval)) {
        value->data[0].v_uint = 0;
        changed = TRUE;
    }
    return changed;
}
static gint param_unichar_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    if (value1->data[0].v_uint < value2->data[0].v_uint) return -1;
    else return value1->data[0].v_uint > value2->data[0].v_uint;
}
static void param_enum_init(GParamSpec *pspec) {
    GParamSpecEnum *espec = G_PARAM_SPEC_ENUM(pspec);
    espec->enum_class = NULL;
    espec->default_value = 0;
}
static void param_enum_finalize(GParamSpec *pspec) {
    GParamSpecEnum *espec = G_PARAM_SPEC_ENUM(pspec);
    GParamSpecClass *parent_class = g_type_class_peek(g_type_parent(G_TYPE_PARAM_ENUM));
    if (espec->enum_class) {
        g_type_class_unref(espec->enum_class);
        espec->enum_class = NULL;
    }
    parent_class->finalize(pspec);
}
static void param_enum_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_long = G_PARAM_SPEC_ENUM(pspec)->default_value;
}
static gboolean param_enum_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecEnum *espec = G_PARAM_SPEC_ENUM(pspec);
    glong oval = value->data[0].v_long;
    if (!espec->enum_class || !g_enum_get_value(espec->enum_class, value->data[0].v_long)) value->data[0].v_long = espec->default_value;
    return value->data[0].v_long != oval;
}
static void param_flags_init(GParamSpec *pspec) {
    GParamSpecFlags *fspec = G_PARAM_SPEC_FLAGS(pspec);
    fspec->flags_class = NULL;
    fspec->default_value = 0;
}
static void param_flags_finalize(GParamSpec *pspec) {
    GParamSpecFlags *fspec = G_PARAM_SPEC_FLAGS(pspec);
    GParamSpecClass *parent_class = g_type_class_peek(g_type_parent(G_TYPE_PARAM_FLAGS));
    if (fspec->flags_class) {
        g_type_class_unref(fspec->flags_class);
        fspec->flags_class = NULL;
    }
    parent_class->finalize(pspec);
}
static void param_flags_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_ulong = G_PARAM_SPEC_FLAGS(pspec)->default_value;
}
static gboolean param_flags_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecFlags *fspec = G_PARAM_SPEC_FLAGS(pspec);
    gulong oval = value->data[0].v_ulong;
    if (fspec->flags_class) value->data[0].v_ulong &= fspec->flags_class->mask;
    else value->data[0].v_ulong = fspec->default_value;
    return value->data[0].v_ulong != oval;
}
static void param_float_init(GParamSpec *pspec) {
    GParamSpecFloat *fspec = G_PARAM_SPEC_FLOAT(pspec);
    fspec->minimum = -G_MAXFLOAT;
    fspec->maximum = G_MAXFLOAT;
    fspec->default_value = 0;
    fspec->epsilon = G_FLOAT_EPSILON;
}
static void param_float_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_float = G_PARAM_SPEC_FLOAT(pspec)->default_value;
}
static gboolean param_float_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecFloat *fspec = G_PARAM_SPEC_FLOAT(pspec);
    gfloat oval = value->data[0].v_float;
    value->data[0].v_float = CLAMP(value->data[0].v_float, fspec->minimum, fspec->maximum);
    return value->data[0].v_float != oval;
}
static gint param_float_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    gfloat epsilon = G_PARAM_SPEC_FLOAT(pspec)->epsilon;
    if (value1->data[0].v_float < value2->data[0].v_float) return -(value2->data[0].v_float - value1->data[0].v_float > epsilon);
    else return value1->data[0].v_float - value2->data[0].v_float > epsilon;
}
static void param_double_init(GParamSpec *pspec) {
    GParamSpecDouble *dspec = G_PARAM_SPEC_DOUBLE(pspec);
    dspec->minimum = -G_MAXDOUBLE;
    dspec->maximum = G_MAXDOUBLE;
    dspec->default_value = 0;
    dspec->epsilon = G_DOUBLE_EPSILON;
}
static void param_double_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_double = G_PARAM_SPEC_DOUBLE(pspec)->default_value;
}
static gboolean param_double_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecDouble *dspec = G_PARAM_SPEC_DOUBLE(pspec);
    gdouble oval = value->data[0].v_double;
    value->data[0].v_double = CLAMP(value->data[0].v_double, dspec->minimum, dspec->maximum);
    return value->data[0].v_double != oval;
}
static gint param_double_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    gdouble epsilon = G_PARAM_SPEC_DOUBLE (pspec)->epsilon;
    if (value1->data[0].v_double < value2->data[0].v_double) return -(value2->data[0].v_double - value1->data[0].v_double > epsilon);
    else return value1->data[0].v_double - value2->data[0].v_double > epsilon;
}
static void param_string_init(GParamSpec *pspec) {
    GParamSpecString *sspec = G_PARAM_SPEC_STRING(pspec);
    sspec->default_value = NULL;
    sspec->cset_first = NULL;
    sspec->cset_nth = NULL;
    sspec->substitutor = '_';
    sspec->null_fold_if_empty = FALSE;
    sspec->ensure_non_null = FALSE;
}
static void param_string_finalize(GParamSpec *pspec) {
    GParamSpecString *sspec = G_PARAM_SPEC_STRING(pspec);
    GParamSpecClass *parent_class = g_type_class_peek(g_type_parent(G_TYPE_PARAM_STRING));
    g_free(sspec->default_value);
    g_free(sspec->cset_first);
    g_free(sspec->cset_nth);
    sspec->default_value = NULL;
    sspec->cset_first = NULL;
    sspec->cset_nth = NULL;
    parent_class->finalize(pspec);
}
static void param_string_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_pointer = g_strdup(G_PARAM_SPEC_STRING (pspec)->default_value);
}
static gboolean param_string_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecString *sspec = G_PARAM_SPEC_STRING (pspec);
    gchar *string = value->data[0].v_pointer;
    guint changed = 0;
    if (string && string[0]) {
        gchar *s;
        if (sspec->cset_first && !strchr(sspec->cset_first, string[0])) {
            if (value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS) {
                value->data[0].v_pointer = g_strdup(string);
                string = value->data[0].v_pointer;
                value->data[1].v_uint &= ~G_VALUE_NOCOPY_CONTENTS;
            }
            string[0] = sspec->substitutor;
            changed++;
	    }
        if (sspec->cset_nth)
	        for (s = string + 1; *s; s++)
	            if (!strchr(sspec->cset_nth, *s)) {
                    if (value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS) {
                        value->data[0].v_pointer = g_strdup (string);
                        s = (gchar*) value->data[0].v_pointer + (s - string);
                        string = value->data[0].v_pointer;
                        value->data[1].v_uint &= ~G_VALUE_NOCOPY_CONTENTS;
                    }
                    *s = sspec->substitutor;
                    changed++;
                }
    }
    if (sspec->null_fold_if_empty && string && string[0] == 0) {
        if (!(value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS)) g_free(value->data[0].v_pointer);
        else value->data[1].v_uint &= ~G_VALUE_NOCOPY_CONTENTS;
        value->data[0].v_pointer = NULL;
        changed++;
        string = value->data[0].v_pointer;
    }
    if (sspec->ensure_non_null && !string) {
        value->data[1].v_uint &= ~G_VALUE_NOCOPY_CONTENTS;
        value->data[0].v_pointer = g_strdup("");
        changed++;
        string = value->data[0].v_pointer;
    }
    return changed;
}
static gint param_string_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    if (!value1->data[0].v_pointer) return value2->data[0].v_pointer != NULL ? -1 : 0;
    else if (!value2->data[0].v_pointer) return value1->data[0].v_pointer != NULL;
    else return strcmp (value1->data[0].v_pointer, value2->data[0].v_pointer);
}
static void param_param_init(GParamSpec *pspec) {}
static void param_param_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_pointer = NULL;
}
static gboolean param_param_validate(GParamSpec *pspec, GValue *value) {
    GParamSpec *param = value->data[0].v_pointer;
    guint changed = 0;
    if (param && !g_value_type_compatible(G_PARAM_SPEC_TYPE(param), G_PARAM_SPEC_VALUE_TYPE(pspec))) {
        g_param_spec_unref(param);
        value->data[0].v_pointer = NULL;
        changed++;
    }
    return changed;
}
static void param_boxed_init(GParamSpec *pspec) {}
static void param_boxed_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_pointer = NULL;
}
static gboolean param_boxed_validate(GParamSpec *pspec, GValue *value) {
    guint changed = 0;
    return changed;
}
static gint param_boxed_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    guint8 *p1 = value1->data[0].v_pointer;
    guint8 *p2 = value2->data[0].v_pointer;
    return p1 < p2 ? -1 : p1 > p2;
}
static void param_pointer_init(GParamSpec *pspec) {}
static void param_pointer_set_default(GParamSpec *pspec, GValue *value) {
  value->data[0].v_pointer = NULL;
}
static gboolean param_pointer_validate(GParamSpec *pspec, GValue *value) {
  guint changed = 0;
  return changed;
}
static gint param_pointer_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
  guint8 *p1 = value1->data[0].v_pointer;
  guint8 *p2 = value2->data[0].v_pointer;
  return p1 < p2 ? -1 : p1 > p2;
}
static void param_value_array_init (GParamSpec *pspec) {
    GParamSpecValueArray *aspec = G_PARAM_SPEC_VALUE_ARRAY(pspec);
    aspec->element_spec = NULL;
    aspec->fixed_n_elements = 0;
}
static inline guint value_array_ensure_size(GValueArray *value_array, guint fixed_n_elements) {
    guint changed = 0;
    if (fixed_n_elements) {
        while (value_array->n_values < fixed_n_elements) {
            g_value_array_append(value_array, NULL);
            changed++;
	    }
        while (value_array->n_values > fixed_n_elements) {
            g_value_array_remove(value_array, value_array->n_values - 1);
            changed++;
	    }
    }
    return changed;
}
static void param_value_array_finalize(GParamSpec *pspec) {
    GParamSpecValueArray *aspec = G_PARAM_SPEC_VALUE_ARRAY(pspec);
    GParamSpecClass *parent_class = g_type_class_peek(g_type_parent(G_TYPE_PARAM_VALUE_ARRAY));
    if (aspec->element_spec) {
        g_param_spec_unref(aspec->element_spec);
        aspec->element_spec = NULL;
    }
    parent_class->finalize(pspec);
}
static void param_value_array_set_default(GParamSpec *pspec, GValue *value) {
    GParamSpecValueArray *aspec = G_PARAM_SPEC_VALUE_ARRAY(pspec);
    if (!value->data[0].v_pointer && aspec->fixed_n_elements) value->data[0].v_pointer = g_value_array_new(aspec->fixed_n_elements);
    if (value->data[0].v_pointer) value_array_ensure_size(value->data[0].v_pointer, aspec->fixed_n_elements);
}
static gboolean param_value_array_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecValueArray *aspec = G_PARAM_SPEC_VALUE_ARRAY(pspec);
    GValueArray *value_array = value->data[0].v_pointer;
    guint changed = 0;
    if (!value->data[0].v_pointer && aspec->fixed_n_elements) value->data[0].v_pointer = g_value_array_new(aspec->fixed_n_elements);
    if (value->data[0].v_pointer) {
        changed += value_array_ensure_size(value_array, aspec->fixed_n_elements);
        if (aspec->element_spec) {
            GParamSpec *element_spec = aspec->element_spec;
            guint i;
            for (i = 0; i < value_array->n_values; i++) {
                GValue *element = value_array->values + i;
                if (!g_value_type_compatible(G_VALUE_TYPE(element), G_PARAM_SPEC_VALUE_TYPE(element_spec))) {
                    if (G_VALUE_TYPE(element) != 0) g_value_unset(element);
                    g_value_init(element, G_PARAM_SPEC_VALUE_TYPE(element_spec));
                    g_param_value_set_default(element_spec, element);
                    changed++;
                }
                changed += g_param_value_validate(element_spec, element);
            }
	    }
    }
    return changed;
}
static gint param_value_array_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    GParamSpecValueArray *aspec = G_PARAM_SPEC_VALUE_ARRAY(pspec);
    GValueArray *value_array1 = value1->data[0].v_pointer;
    GValueArray *value_array2 = value2->data[0].v_pointer;
    if (!value_array1 || !value_array2) return value_array2 ? -1 : value_array1 != value_array2;
    if (value_array1->n_values != value_array2->n_values) return value_array1->n_values < value_array2->n_values ? -1 : 1;
    else if (!aspec->element_spec) {
        return value_array1->n_values < value_array2->n_values ? -1 : value_array1->n_values > value_array2->n_values;
    } else  {
        guint i;
        for (i = 0; i < value_array1->n_values; i++) {
            GValue *element1 = value_array1->values + i;
            GValue *element2 = value_array2->values + i;
            gint cmp;
            if (G_VALUE_TYPE(element1) != G_VALUE_TYPE(element2)) return G_VALUE_TYPE(element1) < G_VALUE_TYPE(element2) ? -1 : 1;
            cmp = g_param_values_cmp(aspec->element_spec, element1, element2);
            if (cmp) return cmp;
        }
        return 0;
    }
}
static void param_object_init(GParamSpec *pspec) {}
static void param_object_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_pointer = NULL;
}
static gboolean param_object_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecObject *ospec = G_PARAM_SPEC_OBJECT(pspec);
    GObject *object = value->data[0].v_pointer;
    guint changed = 0;
    if (object && !g_value_type_compatible(G_OBJECT_TYPE(object), G_PARAM_SPEC_VALUE_TYPE(ospec))) {
        g_object_unref(object);
        value->data[0].v_pointer = NULL;
        changed++;
    }
    return changed;
}
static gint param_object_values_cmp(GParamSpec *pspec, const GValue *value1, const GValue *value2) {
    guint8 *p1 = value1->data[0].v_pointer;
    guint8 *p2 = value2->data[0].v_pointer;
    return p1 < p2 ? -1 : p1 > p2;
}
static void param_override_init(GParamSpec *pspec) {}
static void param_override_finalize(GParamSpec *pspec) {
    GParamSpecOverride *ospec = G_PARAM_SPEC_OVERRIDE(pspec);
    GParamSpecClass *parent_class = g_type_class_peek(g_type_parent (G_TYPE_PARAM_OVERRIDE));
    if (ospec->overridden) {
        g_param_spec_unref(ospec->overridden);
        ospec->overridden = NULL;
    }
    parent_class->finalize(pspec);
}
static void param_override_set_default(GParamSpec *pspec, GValue *value) {
    GParamSpecOverride *ospec = G_PARAM_SPEC_OVERRIDE(pspec);
    g_param_value_set_default(ospec->overridden, value);
}
static gboolean param_override_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecOverride *ospec = G_PARAM_SPEC_OVERRIDE(pspec);
    return g_param_value_validate(ospec->overridden, value);
}
static gint param_override_values_cmp (GParamSpec   *pspec, const GValue *value1, const GValue *value2) {
    GParamSpecOverride *ospec = G_PARAM_SPEC_OVERRIDE(pspec);
    return g_param_values_cmp(ospec->overridden, value1, value2);
}
static void param_gtype_init(GParamSpec *pspec) {}
static void param_gtype_set_default(GParamSpec *pspec, GValue *value) {
    GParamSpecGType *tspec = G_PARAM_SPEC_GTYPE(pspec);
    value->data[0].v_long = tspec->is_a_type;
}
static gboolean param_gtype_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecGType *tspec = G_PARAM_SPEC_GTYPE(pspec);
    GType gtype = value->data[0].v_long;
    guint changed = 0;
    if (tspec->is_a_type != G_TYPE_NONE && !g_type_is_a(gtype, tspec->is_a_type)) {
        value->data[0].v_long = tspec->is_a_type;
        changed++;
    }
    return changed;
}
static gint param_gtype_values_cmp(GParamSpec   *pspec, const GValue *value1, const GValue *value2) {
    GType p1 = value1->data[0].v_long;
    GType p2 = value2->data[0].v_long;
    return p1 < p2 ? -1 : p1 > p2;
}
static void param_variant_init(GParamSpec *pspec) {
    GParamSpecVariant *vspec = G_PARAM_SPEC_VARIANT(pspec);
    vspec->type = NULL;
    vspec->default_value = NULL;
}
static void param_variant_finalize(GParamSpec *pspec) {
    GParamSpecVariant *vspec = G_PARAM_SPEC_VARIANT(pspec);
    GParamSpecClass *parent_class = g_type_class_peek(g_type_parent(G_TYPE_PARAM_VARIANT));
    if (vspec->default_value) g_variant_unref(vspec->default_value);
    g_variant_type_free(vspec->type);
    parent_class->finalize(pspec);
}
static void param_variant_set_default(GParamSpec *pspec, GValue *value) {
    value->data[0].v_pointer = G_PARAM_SPEC_VARIANT(pspec)->default_value;
    value->data[1].v_uint |= G_VALUE_NOCOPY_CONTENTS;
}
static gboolean param_variant_validate(GParamSpec *pspec, GValue *value) {
    GParamSpecVariant *vspec = G_PARAM_SPEC_VARIANT(pspec);
    GVariant *variant = value->data[0].v_pointer;
    if ((variant == NULL && vspec->default_value != NULL) || (variant != NULL && !g_variant_is_of_type(variant, vspec->type))) {
        g_param_value_set_default(pspec, value);
        return TRUE;
    }
    return FALSE;
}
static gint param_variant_values_cmp(GParamSpec   *pspec, const GValue *value1, const GValue *value2) {
    GVariant *v1 = value1->data[0].v_pointer;
    GVariant *v2 = value2->data[0].v_pointer;
    return v1 < v2 ? -1 : v2 > v1;
}
GType *g_param_spec_types = NULL;
void g_param_spec_types_init (void) {
    const guint n_types = 23;
    GType type, *spec_types, *spec_types_bound;
    g_param_spec_types = g_new0(GType, n_types);
    spec_types = g_param_spec_types;
    spec_types_bound = g_param_spec_types + n_types;
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecChar),
            16,
            param_char_init,
            G_TYPE_CHAR,
            NULL,
            param_char_set_default,
            param_char_validate,
            param_int_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamChar"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_CHAR);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecUChar),
            16,
            param_uchar_init,
            G_TYPE_UCHAR,
            NULL,
            param_uchar_set_default,
            param_uchar_validate,
            param_uint_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamUChar"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_UCHAR);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecBoolean),
            16,
            NULL,
            G_TYPE_BOOLEAN,
            NULL,
            param_boolean_set_default,
            param_boolean_validate,
            param_int_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamBoolean"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_BOOLEAN);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecInt),
            16,
            param_int_init,
            G_TYPE_INT,
            NULL,
            param_int_set_default,
            param_int_validate,
            param_int_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamInt"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_INT);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof (GParamSpecUInt),
            16,
            param_uint_init,
            G_TYPE_UINT,
            NULL,
            param_uint_set_default,
            param_uint_validate,
            param_uint_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamUInt"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_UINT);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecLong),
            16,
            param_long_init,
            G_TYPE_LONG,
            NULL,
            param_long_set_default,
            param_long_validate,
            param_long_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamLong"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_LONG);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
          sizeof(GParamSpecULong),
          16,
          param_ulong_init,
          G_TYPE_ULONG,
          NULL,
          param_ulong_set_default,
          param_ulong_validate,
          param_ulong_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string ("GParamULong"), &pspec_info);
        *spec_types++ = type;
        g_assert (type == G_TYPE_PARAM_ULONG);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecInt64),
            16,
            param_int64_init,
            G_TYPE_INT64,
            NULL,
            param_int64_set_default,
            param_int64_validate,
            param_int64_values_cmp,
        };
        type = g_param_type_register_static(g_intern_static_string("GParamInt64"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_INT64);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof (GParamSpecUInt64),
            16,
            param_uint64_init,
            G_TYPE_UINT64,
            NULL,
            param_uint64_set_default,
            param_uint64_validate,
            param_uint64_values_cmp
        };
        type = g_param_type_register_static (g_intern_static_string ("GParamUInt64"), &pspec_info);
        *spec_types++ = type;
        g_assert (type == G_TYPE_PARAM_UINT64);
        }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof (GParamSpecUnichar),
            16,
            param_unichar_init,
            G_TYPE_UINT,
            NULL,
            param_unichar_set_default,
            param_unichar_validate,
            param_unichar_values_cmp
        };
        type = g_param_type_register_static (g_intern_static_string ("GParamUnichar"), &pspec_info);
        *spec_types++ = type;
        g_assert (type == G_TYPE_PARAM_UNICHAR);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecEnum),
            16,
            param_enum_init,
            G_TYPE_ENUM,
            param_enum_finalize,
            param_enum_set_default,
            param_enum_validate,
            param_long_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamEnum"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_ENUM);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecFlags),
            16,
            param_flags_init,
            G_TYPE_FLAGS,
            param_flags_finalize,
            param_flags_set_default,
            param_flags_validate,
            param_ulong_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamFlags"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_FLAGS);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecFloat),
            16,
            param_float_init,
            G_TYPE_FLOAT,
            NULL,
            param_float_set_default,
            param_float_validate,
            param_float_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamFloat"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_FLOAT);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecDouble),
            16,
            param_double_init,
            G_TYPE_DOUBLE,
            NULL,
            param_double_set_default,
            param_double_validate,
            param_double_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamDouble"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_DOUBLE);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecString),
            16,
            param_string_init,
            G_TYPE_STRING,
            param_string_finalize,
            param_string_set_default,
            param_string_validate,
            param_string_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamString"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_STRING);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecParam),
            16,
            param_param_init,
            G_TYPE_PARAM,
            NULL,
            param_param_set_default,
            param_param_validate,
            param_pointer_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamParam"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_PARAM);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecBoxed),
            4,
            param_boxed_init,
            G_TYPE_BOXED,
            NULL,
            param_boxed_set_default,
            param_boxed_validate,
            param_boxed_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamBoxed"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_BOXED);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecPointer),
            0,
            param_pointer_init,
            G_TYPE_POINTER,
            NULL,
            param_pointer_set_default,
            param_pointer_validate,
            param_pointer_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamPointer"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_POINTER);
    }
    {
        static GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecValueArray),
            0,
            param_value_array_init,
            0xdeadbeef,
            param_value_array_finalize,
            param_value_array_set_default,
            param_value_array_validate,
            param_value_array_values_cmp
        };
        pspec_info.value_type = G_TYPE_VALUE_ARRAY;
        type = g_param_type_register_static(g_intern_static_string("GParamValueArray"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_VALUE_ARRAY);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecObject),
            16,
            param_object_init,
            G_TYPE_OBJECT,
            NULL,
            param_object_set_default,
            param_object_validate,
            param_object_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamObject"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_OBJECT);
    }
    {
        static const GParamSpecTypeInfo pspec_info = {
            sizeof (GParamSpecOverride),
            16,
            param_override_init,
            G_TYPE_NONE,
            param_override_finalize,
            param_override_set_default,
            param_override_validate,
            param_override_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamOverride"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_OVERRIDE);
    }
    {
        GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecGType),
            0,
            param_gtype_init,
            0xdeadbeef,
            NULL,
            param_gtype_set_default,
            param_gtype_validate,
            param_gtype_values_cmp
        };
        pspec_info.value_type = G_TYPE_GTYPE;
        type = g_param_type_register_static(g_intern_static_string("GParamGType"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_GTYPE);
    }
    {
        const GParamSpecTypeInfo pspec_info = {
            sizeof(GParamSpecVariant),
            0,
            param_variant_init,
            G_TYPE_VARIANT,
            param_variant_finalize,
            param_variant_set_default,
            param_variant_validate,
            param_variant_values_cmp
        };
        type = g_param_type_register_static(g_intern_static_string("GParamVariant"), &pspec_info);
        *spec_types++ = type;
        g_assert(type == G_TYPE_PARAM_VARIANT);
    }
    g_assert(spec_types == spec_types_bound);
}
GParamSpec* g_param_spec_char(const gchar *name, const gchar *nick, const gchar *blurb, gint8 minimum, gint8 maximum, gint8 default_value, GParamFlags flags) {
    GParamSpecChar *cspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    cspec = g_param_spec_internal(G_TYPE_PARAM_CHAR, name, nick, blurb, flags);
    cspec->minimum = minimum;
    cspec->maximum = maximum;
    cspec->default_value = default_value;
    return G_PARAM_SPEC(cspec);
}
GParamSpec* g_param_spec_uchar(const gchar *name, const gchar *nick, const gchar *blurb, guint8 minimum, guint8 maximum, guint8 default_value, GParamFlags flags) {
    GParamSpecUChar *uspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    uspec = g_param_spec_internal(G_TYPE_PARAM_UCHAR, name, nick, blurb, flags);
    uspec->minimum = minimum;
    uspec->maximum = maximum;
    uspec->default_value = default_value;
    return G_PARAM_SPEC(uspec);
}
GParamSpec* g_param_spec_boolean(const gchar *name, const gchar *nick, const gchar *blurb, gboolean default_value, GParamFlags flags) {
    GParamSpecBoolean *bspec;
    g_return_val_if_fail(default_value == TRUE || default_value == FALSE, NULL);
    bspec = g_param_spec_internal(G_TYPE_PARAM_BOOLEAN, name, nick, blurb, flags);
    bspec->default_value = default_value;
    return G_PARAM_SPEC(bspec);
}
GParamSpec* g_param_spec_int(const gchar *name, const gchar *nick, const gchar *blurb, gint minimum, gint maximum, gint default_value, GParamFlags flags) {
    GParamSpecInt *ispec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    ispec = g_param_spec_internal(G_TYPE_PARAM_INT, name, nick, blurb, flags);
    ispec->minimum = minimum;
    ispec->maximum = maximum;
    ispec->default_value = default_value;
    return G_PARAM_SPEC(ispec);
}
GParamSpec* g_param_spec_uint(const gchar *name, const gchar *nick, const gchar *blurb, guint minimum, guint maximum, guint default_value, GParamFlags flags) {
    GParamSpecUInt *uspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    uspec = g_param_spec_internal(G_TYPE_PARAM_UINT, name, nick, blurb, flags);
    uspec->minimum = minimum;
    uspec->maximum = maximum;
    uspec->default_value = default_value;
    return G_PARAM_SPEC(uspec);
}
GParamSpec* g_param_spec_long(const gchar *name, const gchar *nick, const gchar *blurb, glong minimum, glong maximum, glong default_value, GParamFlags flags) {
    GParamSpecLong *lspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    lspec = g_param_spec_internal(G_TYPE_PARAM_LONG, name, nick, blurb, flags);
    lspec->minimum = minimum;
    lspec->maximum = maximum;
    lspec->default_value = default_value;
    return G_PARAM_SPEC(lspec);
}
GParamSpec* g_param_spec_ulong(const gchar *name, const gchar *nick, const gchar *blurb, gulong minimum, gulong maximum, gulong default_value, GParamFlags flags) {
    GParamSpecULong *uspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    uspec = g_param_spec_internal(G_TYPE_PARAM_ULONG, name, nick, blurb, flags);
    uspec->minimum = minimum;
    uspec->maximum = maximum;
    uspec->default_value = default_value;
    return G_PARAM_SPEC(uspec);
}
GParamSpec* g_param_spec_int64(const gchar *name, const gchar *nick, const gchar *blurb, gint64 minimum, gint64 maximum, gint64 default_value, GParamFlags flags) {
    GParamSpecInt64 *lspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    lspec = g_param_spec_internal(G_TYPE_PARAM_INT64, name, nick, blurb, flags);
    lspec->minimum = minimum;
    lspec->maximum = maximum;
    lspec->default_value = default_value;
    return G_PARAM_SPEC(lspec);
}
GParamSpec* g_param_spec_uint64(const gchar *name, const gchar *nick, const gchar *blurb, guint64 minimum, guint64 maximum, guint64 default_value,
                                GParamFlags flags) {
    GParamSpecUInt64 *uspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    uspec = g_param_spec_internal(G_TYPE_PARAM_UINT64, name, nick, blurb, flags);
    uspec->minimum = minimum;
    uspec->maximum = maximum;
    uspec->default_value = default_value;
    return G_PARAM_SPEC(uspec);
}
GParamSpec* g_param_spec_unichar(const gchar *name, const gchar *nick, const gchar *blurb, gunichar default_value, GParamFlags flags) {
    GParamSpecUnichar *uspec;
    uspec = g_param_spec_internal(G_TYPE_PARAM_UNICHAR, name, nick, blurb, flags);
    uspec->default_value = default_value;
    return G_PARAM_SPEC(uspec);
}
GParamSpec* g_param_spec_enum(const gchar *name, const gchar *nick, const gchar *blurb, GType enum_type, gint default_value, GParamFlags flags) {
    GParamSpecEnum *espec;
    GEnumClass *enum_class;
    g_return_val_if_fail(G_TYPE_IS_ENUM (enum_type), NULL);
    enum_class = g_type_class_ref(enum_type);
    g_return_val_if_fail(g_enum_get_value(enum_class, default_value) != NULL, NULL);
    espec = g_param_spec_internal(G_TYPE_PARAM_ENUM, name, nick, blurb, flags);
    espec->enum_class = enum_class;
    espec->default_value = default_value;
    G_PARAM_SPEC(espec)->value_type = enum_type;
    return G_PARAM_SPEC(espec);
}
GParamSpec* g_param_spec_flags(const gchar *name, const gchar *nick, const gchar *blurb, GType flags_type, guint default_value, GParamFlags	flags) {
    GParamSpecFlags *fspec;
    GFlagsClass *flags_class;
    g_return_val_if_fail(G_TYPE_IS_FLAGS(flags_type), NULL);
    flags_class = g_type_class_ref(flags_type);
    g_return_val_if_fail((default_value & flags_class->mask) == default_value, NULL);
    fspec = g_param_spec_internal(G_TYPE_PARAM_FLAGS, name, nick, blurb, flags);
    fspec->flags_class = flags_class;
    fspec->default_value = default_value;
    G_PARAM_SPEC(fspec)->value_type = flags_type;
    return G_PARAM_SPEC(fspec);
}
GParamSpec* g_param_spec_float(const gchar *name, const gchar *nick, const gchar *blurb, gfloat minimum, gfloat maximum, gfloat default_value, GParamFlags flags) {
    GParamSpecFloat *fspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    fspec = g_param_spec_internal(G_TYPE_PARAM_FLOAT, name, nick, blurb, flags);
    fspec->minimum = minimum;
    fspec->maximum = maximum;
    fspec->default_value = default_value;
    return G_PARAM_SPEC(fspec);
}
GParamSpec* g_param_spec_double(const gchar *name, const gchar *nick, const gchar *blurb, gdouble minimum, gdouble maximum, gdouble default_value,
		                        GParamFlags flags) {
    GParamSpecDouble *dspec;
    g_return_val_if_fail(default_value >= minimum && default_value <= maximum, NULL);
    dspec = g_param_spec_internal(G_TYPE_PARAM_DOUBLE, name, nick, blurb, flags);
    dspec->minimum = minimum;
    dspec->maximum = maximum;
    dspec->default_value = default_value;
    return G_PARAM_SPEC(dspec);
}
GParamSpec* g_param_spec_string(const gchar *name, const gchar *nick, const gchar *blurb, const gchar *default_value, GParamFlags flags) {
    GParamSpecString *sspec = g_param_spec_internal(G_TYPE_PARAM_STRING, name, nick, blurb, flags);
    g_free(sspec->default_value);
    sspec->default_value = g_strdup(default_value);
    return G_PARAM_SPEC(sspec);
}
GParamSpec* g_param_spec_param(const gchar *name, const gchar *nick, const gchar *blurb, GType param_type, GParamFlags flags) {
    GParamSpecParam *pspec;
    g_return_val_if_fail(G_TYPE_IS_PARAM (param_type), NULL);
    pspec = g_param_spec_internal(G_TYPE_PARAM_PARAM, name, nick, blurb, flags);
    G_PARAM_SPEC(pspec)->value_type = param_type;
    return G_PARAM_SPEC(pspec);
}
GParamSpec* g_param_spec_boxed(const gchar *name, const gchar *nick, const gchar *blurb, GType boxed_type, GParamFlags flags) {
    GParamSpecBoxed *bspec;
    g_return_val_if_fail(G_TYPE_IS_BOXED (boxed_type), NULL);
    g_return_val_if_fail(G_TYPE_IS_VALUE_TYPE (boxed_type), NULL);
    bspec = g_param_spec_internal(G_TYPE_PARAM_BOXED, name, nick, blurb, flags);
    G_PARAM_SPEC(bspec)->value_type = boxed_type;
    return G_PARAM_SPEC(bspec);
}
GParamSpec* g_param_spec_pointer(const gchar *name, const gchar *nick, const gchar *blurb, GParamFlags flags) {
    GParamSpecPointer *pspec;
    pspec = g_param_spec_internal(G_TYPE_PARAM_POINTER, name, nick, blurb, flags);
    return G_PARAM_SPEC(pspec);
}
GParamSpec* g_param_spec_gtype(const gchar *name, const gchar *nick, const gchar *blurb, GType is_a_type, GParamFlags flags) {
    GParamSpecGType *tspec;
    tspec = g_param_spec_internal(G_TYPE_PARAM_GTYPE, name, nick, blurb, flags);
    tspec->is_a_type = is_a_type;
    return G_PARAM_SPEC(tspec);
}
GParamSpec* g_param_spec_value_array(const gchar *name, const gchar *nick, const gchar *blurb, GParamSpec *element_spec, GParamFlags flags) {
    GParamSpecValueArray *aspec;
    if (element_spec) g_return_val_if_fail(G_IS_PARAM_SPEC(element_spec), NULL);
    aspec = g_param_spec_internal(G_TYPE_PARAM_VALUE_ARRAY, name, nick, blurb, flags);
    if (element_spec) {
        aspec->element_spec = g_param_spec_ref(element_spec);
        g_param_spec_sink(element_spec);
    }
    return G_PARAM_SPEC(aspec);
}
GParamSpec* g_param_spec_object(const gchar *name, const gchar *nick, const gchar *blurb, GType object_type, GParamFlags flags) {
    GParamSpecObject *ospec;
    g_return_val_if_fail(g_type_is_a(object_type, G_TYPE_OBJECT), NULL);
    ospec = g_param_spec_internal(G_TYPE_PARAM_OBJECT, name, nick, blurb, flags);
    G_PARAM_SPEC(ospec)->value_type = object_type;
    return G_PARAM_SPEC(ospec);
}
GParamSpec* g_param_spec_override(const gchar *name, GParamSpec *overridden) {
    GParamSpec *pspec;
    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(G_IS_PARAM_SPEC (overridden), NULL);
    while(TRUE) {
        GParamSpec *indirect = g_param_spec_get_redirect_target(overridden);
        if (indirect) overridden = indirect;
        else break;
    }
    pspec = g_param_spec_internal(G_TYPE_PARAM_OVERRIDE, name, NULL, NULL, overridden->flags);
    pspec->value_type = G_PARAM_SPEC_VALUE_TYPE(overridden);
    G_PARAM_SPEC_OVERRIDE(pspec)->overridden = g_param_spec_ref (overridden);
    return pspec;
}
GParamSpec* g_param_spec_variant(const gchar *name, const gchar *nick, const gchar *blurb, const GVariantType *type,GVariant *default_value, GParamFlags flags) {
    GParamSpecVariant *vspec;
    g_return_val_if_fail(type != NULL, NULL);
    g_return_val_if_fail(default_value == NULL || g_variant_is_of_type(default_value, type), NULL);
    vspec = g_param_spec_internal(G_TYPE_PARAM_VARIANT, name, nick, blurb, flags);
    vspec->type = g_variant_type_copy(type);
    if (default_value) vspec->default_value = g_variant_ref_sink(default_value);
    return G_PARAM_SPEC(vspec);
}