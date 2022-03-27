#ifndef CHECK_PRINT_H
#define CHECK_PRINT_H

void fprint_xml_esc(FILE *file, const char *str);
void tr_fprint(FILE *file, TestResult *tr, enum print_output print_mode);
void tr_xmlprint(FILE *file, TestResult *tr, enum print_output print_mode);
void srunner_fprint(FILE *file, SRunner *sr, enum print_output print_mode);
enum print_output get_env_printmode(void);

#endif