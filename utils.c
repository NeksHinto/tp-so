#include "utils.h"

void print_error(char *file_name, char *fn_name, int errnum)
{
    if (file_name != NULL)
    {
        fprintf(stderr, "%s %s (%s): %s", ERROR_TEXT, file_name, fn_name, strerror(errnum));
    }
}

void clean_buffer(char *buffer)
{
    int j = 0;
    while (buffer[j] != '\0')
    {
        buffer[j++] = '\0';
    }
}