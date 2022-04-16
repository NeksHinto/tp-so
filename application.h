#ifndef APPLICATION
#define APPLICATION

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <limits.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <math.h>

#define FILE_OUTPUT "./output_application"
#define SHARED_MEMORY_OBJ_NAME "/shm_obj"
#define BUFFER_SIZE 256
#define SIZEOF_RESPONSE 150

#define SEMAPHORE_NAME "semaphore1"

#define PROCESSES_COUNT 5
#define INITIAL_FILES_COUNT 2
#define ERROR_TEXT "ERROR in Application: "

int fd_works[PROCESSES_COUNT][2];    // PIPE master --> slave
int fd_results[PROCESSES_COUNT][2]; // PIPE slave --> master
int flags_fd_work_open[PROCESSES_COUNT];
int processes[PROCESSES_COUNT];
int offset_args = 1;
int files_count_to_send = 0;
int files_count_resolved = 0;
char *pointer_sh_mem;
char *aux_pointer_sh_mem;
int res_processes[PROCESSES_COUNT];

// Validacion del tipo de archivo
void check_format(int files_count, const char *files[], char *format);

// Resource: https://github.com/WhileTrueThenDream/ExamplesCLinuxUserSpace
int shm_create(size_t size);

void clean_buffer(char *buffer);

void create_pipes();

void create_slaves();

void concat_files(int files_count, const char *files[], char concat[]);

#endif