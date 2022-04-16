#ifndef ERRORS
#define ERRORS

#include <stdio.h>
#include <errno.h>

#define ERROR_TEXT "[ERROR] "

print_error(char* file_name, char* fn_name, int errnum);

#endif