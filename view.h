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

#define SHARED_MEMORY_OBJ_NAME "/shm_obj"
#define SIZEOF_RESPONSE 150
#define ERROR_TEXT "ERROR, in View - "
#define SEMAPHORE_NAME "semaphore1" 

void clean_buffer(char *buffer){
        int j = 0;
        while (buffer[j] != '\0')
        {
                buffer[j++] = '\0';
        }
}

char *aux_pointer;

#endif