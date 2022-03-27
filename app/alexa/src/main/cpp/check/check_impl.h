#ifndef CHECK_IMPL_H
#define CHECK_IMPL_H

#define US_PER_SEC 1000000
#define NANOS_PER_SECONDS 1000000000
#define DIFF_IN_USEC(begin, end)  ((((end).tv_sec - (begin).tv_sec) * US_PER_SEC) + ((end).tv_nsec/1000) - ((begin).tv_nsec/1000))
typedef struct TF {
    const TTest *ttest;
    int loop_start;
    int loop_end;
    int signal;
    signed char allowed_exit_value;
} TF;
struct Suite {
    const char *name;
    List *tclst;
};
typedef struct Fixture {
    int ischecked;
    SFun fun;
} Fixture;
struct TCase {
    const char *name;
    struct timespec timeout;
    List *tflst;
    List *unch_sflst;
    List *unch_tflst;
    List *ch_sflst;
    List *ch_tflst;
    List *tags;
};
typedef struct TestStats {
    int n_checked;
    int n_failed;
    int n_errors;
} TestStats;
struct TestResult {
    enum test_result rtype;
    enum ck_result_ctx ctx;
    char *file;
    int line;
    int iter;
    int duration;
    const char *tcname;
    const char *tname;
    char *msg;
};
TestResult *tr_create(void);
void tr_reset(TestResult *tr);
void tr_free(TestResult *tr);
enum cl_event {
    CLINITLOG_SR,
    CLENDLOG_SR,
    CLSTART_SR,
    CLSTART_S,
    CLEND_SR,
    CLEND_S,
    CLSTART_T,
    CLEND_T
};
typedef void (*LFun)(SRunner *, FILE *, enum print_output, void *, enum cl_event);
typedef struct Log {
    FILE *lfile;
    LFun lfun;
    int close;
    enum print_output mode;
} Log;
struct SRunner {
    List *slst;
    TestStats *stats;
    List *resultlst;
    const char *log_fname;
    const char *xml_fname;
    const char *tap_fname;
    List *loglst;
    enum fork_status fstat;
};
void set_fork_status(enum fork_status fstat);
enum fork_status cur_fork_status(void);
clockid_t check_get_clockid(void);
unsigned int tcase_matching_tag(TCase *tc, List *check_for);
List *tag_string_to_list(const char *tags_string);

#endif