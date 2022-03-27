#ifndef CHECK_MSG_NEW_H
#define CHECK_MSG_NEW_H

void send_failure_info(const char *msg);
void send_loc_info(const char *file, int line);
void send_ctx_info(enum ck_result_ctx ctx);
void send_duration_info(int duration);
TestResult *receive_test_result(int waserror);
void setup_messaging(void);
void teardown_messaging(void);
FILE *open_tmp_file(char **name);

#endif