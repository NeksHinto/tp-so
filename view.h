#ifndef VIEW
#define VIEW

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "errors.h"

#define SHARED_MEMORY_OBJ_NAME "/shm_obj"
#define SIZEOF_RESPONSE 150
#define FILE_NAME "view.c"
#define SEMAPHORE_NAME "semaphore1"

void clean_buffer(char *buffer);

#endif