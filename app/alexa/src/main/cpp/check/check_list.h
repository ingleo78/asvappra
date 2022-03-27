#ifndef CHECK_LIST_H
#define CHECK_LIST_H

typedef struct List List;
List *check_list_create(void);
int check_list_at_end(List *lp);
void check_list_front(List *lp);
void check_list_add_front(List *lp, void *val);
void check_list_add_end(List *lp, void *val);
void *check_list_val(List *lp);
void check_list_advance(List *lp);
void check_list_free(List *lp);
void check_list_apply(List *lp, void (*fp) (void *));
int check_list_contains(List *lp, void *val);

#endif