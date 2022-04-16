#include "errors.h"

print_error(char* file_name, char* fn_name, int errnum){
  if(file_name != NULL){
    fprintf(stderr, "%s %s (%s): %s", ERROR_TEXT, file_name, fn_name, strerror(errnum));
  }
}