#ifndef VIEW_H
#define VIEW_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "utils.h"

#define SHARED_MEMORY_OBJ_NAME "/shm_obj"
#define SIZEOF_RESPONSE 150
#define FILE_NAME "view.c"
#define SEMAPHORE_NAME "my_semaphore"

#endif