#ifndef CHECK_LOG_H
#define CHECK_LOG_H

void log_srunner_start(SRunner *sr);
void log_srunner_end(SRunner *sr);
void log_suite_start(SRunner *sr, Suite *s);
void log_suite_end(SRunner *sr, Suite *s);
void log_test_end(SRunner *sr, TestResult *tr);
void log_test_start(SRunner *sr, TCase *tc, TF *tfun);
void stdout_lfun(SRunner *sr, FILE *file, enum print_output, void *obj, enum cl_event evt);
void lfile_lfun(SRunner *sr, FILE *file, enum print_output, void *obj, enum cl_event evt);
void xml_lfun(SRunner *sr, FILE *file, enum print_output, void *obj, enum cl_event evt);
void tap_lfun(SRunner *sr, FILE *file, enum print_output, void *obj, enum cl_event evt);
void subunit_lfun(SRunner *sr, FILE *file, enum print_output, void *obj, enum cl_event evt);
void srunner_register_lfun(SRunner *sr, FILE *lfile, int close, LFun lfun, enum print_output);
FILE *srunner_open_lfile(SRunner *sr);
FILE *srunner_open_xmlfile(SRunner *sr);
FILE *srunner_open_tapfile(SRunner *sr);
void srunner_init_logging(SRunner *sr, enum print_output print_mode);
void srunner_end_logging(SRunner *sr);

#endif