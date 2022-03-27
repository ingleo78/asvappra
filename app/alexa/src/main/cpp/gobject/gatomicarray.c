#include <string.h>
#include "../gio/config.h"
#include "gatomicarray.h"

G_LOCK_DEFINE_STATIC (array);
typedef struct _FreeListNode FreeListNode;
struct _FreeListNode {
    FreeListNode *next;
};
static FreeListNode *freelist = NULL;
static gpointer freelist_alloc(gsize size, gboolean reuse) {
    gpointer mem;
    FreeListNode *free, **prev;
    gsize real_size;
    if (reuse) {
        for (free = freelist, prev = &freelist; free != NULL; prev = &free->next, free = free->next) {
            if (G_ATOMIC_ARRAY_DATA_SIZE(free) == size) {
                *prev = free->next;
                return (gpointer)free;
            }
	    }
    }
    real_size = sizeof(gsize) + MAX(size, sizeof(FreeListNode));
    mem = g_slice_alloc(real_size);
    mem = ((char*)mem) + sizeof(gsize);
    G_ATOMIC_ARRAY_DATA_SIZE(mem) = size;
    return mem;
}
static void freelist_free(gpointer mem) {
    FreeListNode *free;
    free = mem;
    free->next = freelist;
    freelist = free;
}
void _g_atomic_array_init(GAtomicArray *array) {
    array->data = NULL;
}
gpointer _g_atomic_array_copy (GAtomicArray *array, gsize header_size, gsize additional_element_size) {
    guint8 *new, *old;
    gsize old_size, new_size;
    g_assert(additional_element_size >= 0);
    G_LOCK(array);
    old = g_atomic_pointer_get(&array->data);
    if (old) {
        old_size = G_ATOMIC_ARRAY_DATA_SIZE (old);
        new_size = old_size + additional_element_size;
        new = freelist_alloc(new_size, additional_element_size != 0);
        memcpy(new, old, old_size);
    } else if (additional_element_size != 0) {
        new_size = header_size + additional_element_size;
        new = freelist_alloc(new_size, TRUE);
    } else new = NULL;
    G_UNLOCK(array);
    return new;
}
void _g_atomic_array_update(GAtomicArray *array, gpointer new_data) {
    guint8 *old;
    G_LOCK (array);
    old = g_atomic_pointer_get(&array->data);
    g_assert(old == NULL || G_ATOMIC_ARRAY_DATA_SIZE (old) <= G_ATOMIC_ARRAY_DATA_SIZE(new_data));
    g_atomic_pointer_set(&array->data, new_data);
    if (old) freelist_free(old);
    G_UNLOCK(array);
}