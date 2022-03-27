#define _GNU_SOURCE
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "gqsort.h"
#include "gtestutils.h"

#ifdef HAVE_QSORT_R
void g_qsort_with_data(gconstpointer pbase, gint total_elems, gsize size, GCompareDataFunc compare_func, gpointer user_data) {
  qsort_r(pbase, total_elems, size, compare_func, user_data);
}
#else
#define SWAP(a, b, size)				    \
  do {									    \
      register size_t __size = (size);		\
      register char *__a = (a), *__b = (b); \
      do {								    \
          char __tmp = *__a;				\
          *__a++ = *__b;					\
          *__b++ = __tmp;					\
	  } while(--__size > 0);			    \
  } while(0);
#define MAX_THRESH 4
typedef struct {
    char *lo;
    char *hi;
} stack_node;
#define STACK_SIZE	(CHAR_BIT * sizeof(size_t))
#define PUSH(low, high)	((void)((top->lo = (low)), (top->hi = (high)), ++top))
#define	POP(low, high)	((void)(--top, (low = top->lo), (high = top->hi)))
#define	STACK_NOT_EMPTY	(stack < top)
void g_qsort_with_data(gconstpointer pbase, gint total_elems, gsize size, GCompareDataFunc compare_func, gpointer user_data) {
  register char *base_ptr = (char*)pbase;
  const size_t max_thresh = MAX_THRESH * size;
  g_return_if_fail(total_elems >= 0);
  g_return_if_fail(pbase != NULL || total_elems == 0);
  g_return_if_fail(compare_func != NULL);
  if (total_elems == 0) return;
  if (total_elems > MAX_THRESH) {
      char *lo = base_ptr;
      char *hi = &lo[size * (total_elems - 1)];
      stack_node stack[STACK_SIZE];
      stack_node *top = stack;
      PUSH(NULL, NULL);
      while(STACK_NOT_EMPTY) {
          char *left_ptr;
          char *right_ptr;
	  char *mid = lo + size * ((hi - lo) / size >> 1);
	  if ((*compare_func)((void*)mid, (void*)lo, user_data) < 0) {
	      SWAP(mid, lo, size);
	  }
	  if ((*compare_func)((void*)hi, (void*)mid, user_data) < 0) {
	      SWAP(mid, hi, size);
	  } else goto jump_over;
	  if ((*compare_func)((void*)mid, (void*)lo, user_data) < 0) SWAP (mid, lo, size);
	  jump_over:;
	  left_ptr  = lo + size;
	  right_ptr = hi - size;
	  do {
	      while((*compare_func)((void*)left_ptr, (void*)mid, user_data) < 0) left_ptr += size;
	      while((*compare_func)((void*)mid, (void*)right_ptr, user_data) < 0) right_ptr -= size;
	      if (left_ptr < right_ptr) {
              SWAP(left_ptr, right_ptr, size);
              if (mid == left_ptr) mid = right_ptr;
              else if (mid == right_ptr) mid = left_ptr;
              left_ptr += size;
              right_ptr -= size;
		  } else if (left_ptr == right_ptr) {
              left_ptr += size;
              right_ptr -= size;
              break;
		  }
	  } while(left_ptr <= right_ptr);
          if ((size_t) (right_ptr - lo) <= max_thresh) {
              if ((size_t) (hi - left_ptr) <= max_thresh) {
                  POP(lo, hi);
              } else lo = left_ptr;
          } else if ((size_t) (hi - left_ptr) <= max_thresh) hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr)) {
              PUSH (lo, right_ptr);
              lo = left_ptr;
          } else {
              PUSH (left_ptr, hi);
              hi = right_ptr;
          }
      }
  }
  #define min(x, y) ((x) < (y) ? (x) : (y))
  {
      char *const end_ptr = &base_ptr[size * (total_elems - 1)];
      char *tmp_ptr = base_ptr;
      char *thresh = min(end_ptr, base_ptr + max_thresh);
      register char *run_ptr;
      for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
      if ((*compare_func) ((void *) run_ptr, (void *) tmp_ptr, user_data) < 0) tmp_ptr = run_ptr;
      if (tmp_ptr != base_ptr) SWAP (tmp_ptr, base_ptr, size);
      run_ptr = base_ptr + size;
      while ((run_ptr += size) <= end_ptr) {
          tmp_ptr = run_ptr - size;
          while((*compare_func)((void*)run_ptr, (void*)tmp_ptr, user_data) < 0) tmp_ptr -= size;
          tmp_ptr += size;
          if (tmp_ptr != run_ptr) {
              char *trav;
              trav = run_ptr + size;
              while(--trav >= run_ptr) {
                  char c = *trav;
                  char *hi, *lo;
                  for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo) *hi = *lo;
                  *hi = c;
              }
          }
      }
  }
}
#endif