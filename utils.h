#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define ERROR_TEXT "[ERROR] "

void clean_buffer(char *buffer);
void print_error(char *file_name, char *fn_name, int errnum);
#endif