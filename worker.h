#ifndef WORKER
#define WORKER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errors.h>

#define MINISAT "minisat "
#define GREP_AND_FLAGS " | grep -o -e 'Number of.*[0-9]\\+' -e 'CPU time.*' -e '.*SATISFIABLE' | xargs | tr -s [:space:]"
#define BUFFER_SIZE 256
#define FILE_NAME "worker.c"
void clean_buffer(char *buffer);

#endif