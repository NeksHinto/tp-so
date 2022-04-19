#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define ERROR_TEXT "[ERROR] "

// static void print_error(char *file_name, char *fn_name, int errnum)
// {
//         if (file_name != NULL)
//         {
//                 fprintf(stderr, "%s %s (%s): %s", ERROR_TEXT, file_name, fn_name, strerror(errnum));
//         }
// }

// static void clean_buffer(char *buffer)
// {
//         int j = 0;
//         while (buffer[j] != '\0')
//         {
//                 buffer[j++] = '\0';
//         }
// }
void clean_buffer(char *buffer);
void print_error(char *file_name, char *fn_name, int errnum);
#endif