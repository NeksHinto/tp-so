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
#include "utils.h"
#include <sys/select.h>
#include <limits.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <math.h>

#define FILE_OUTPUT "./output_application"
#define SHARED_MEMORY_OBJ_NAME "/shm_obj"
#define BUFFER_SIZE 256
#define SIZEOF_RESPONSE 1024

#define SEMAPHORE_NAME "my_semaphore"

#define PROCESSES_COUNT 5
#define INITIAL_FILES_COUNT 2
#define SEPARATOR "--------------------------------------------------\n"
#define FILE_NAME "application.c"

int fd_works[PROCESSES_COUNT][2];   // PIPE from application to worker
int fd_results[PROCESSES_COUNT][2]; // PIPE from worker to application
int flags_fd_work_open[PROCESSES_COUNT];
int processes[PROCESSES_COUNT];
int offset_args = 1;
int files_count_to_send = 0;
int solved_files_count = 0;
char *pointer_sh_mem;
char *aux_pointer_sh_mem;
int solved_per_process[PROCESSES_COUNT];

// Validacion del tipo de archivo
void validate_format(int files_count, const char *files[], char *format);

// Resource: https://github.com/WhileTrueThenDream/ExamplesCLinuxUserSpace
int shm_create(size_t size);

void create_pipes();

void create_workers();

void concat_files(int files_count, const char *files[], char str[]);

void initialize_fd_set(int *nfds, fd_set *fd_workers);

#endif