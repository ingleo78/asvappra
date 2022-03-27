#ifndef CHECK_PACK_H
#define CHECK_PACK_H

enum ck_msg_type {
    CK_MSG_CTX,
    CK_MSG_FAIL,
    CK_MSG_LOC,
    CK_MSG_DURATION,
    CK_MSG_LAST
};
typedef struct CtxMsg {
    enum ck_result_ctx ctx;
} CtxMsg;
typedef struct LocMsg {
    int line;
    char *file;
} LocMsg;
typedef struct FailMsg {
    char *msg;
} FailMsg;
typedef struct DurationMsg {
    int duration;
} DurationMsg;
typedef union {
    CtxMsg ctx_msg;
    FailMsg fail_msg;
    LocMsg loc_msg;
    DurationMsg duration_msg;
} CheckMsg;
typedef struct RcvMsg {
    enum ck_result_ctx lastctx;
    enum ck_result_ctx failctx;
    char *fixture_file;
    int fixture_line;
    char *test_file;
    int test_line;
    char *msg;
    int duration;
} RcvMsg;
void rcvmsg_free(RcvMsg *rmsg);
int pack(enum ck_msg_type type, char **buf, CheckMsg *msg);
int upack(char *buf, CheckMsg *msg, enum ck_msg_type *type);
void ppack(FILE *fdes, enum ck_msg_type type, CheckMsg *msg);
RcvMsg *punpack(FILE *fdes);

#endif