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
#define ERROR_TEXT "ERROR, in Application - "

int fd_work[PROCESSES_COUNT][2]; // PIPE master --> slave
int fd_results[PROCESSES_COUNT][2]; // PIPE slave --> master
int flags_fd_work_open[PROCESSES_COUNT];
int processes[PROCESSES_COUNT];
int offset_args = 1;
int files_count_to_send = 0;
int files_count_resolved = 0;
char *pointer_sh_mem;
char *aux_pointer_sh_mem;


// Validacion del tipo de archivo
void check_format(int files_count, const char *files[], char *format){
        int i = 0;
        for (; i < files_count; i++)
        {
                if (strstr(files[i], format) == NULL)
                {
                    printf(ERROR_TEXT);
                    printf("Send only .cnf files\n");
                    exit(1);
                }
        }
}

// Resource: https://github.com/WhileTrueThenDream/ExamplesCLinuxUserSpace
int shm_create(size_t size){
        int fd;
        fd = shm_open(SHARED_MEMORY_OBJ_NAME, O_CREAT | O_RDWR, 00700);
        if (-1 == fd)
        {
            printf(ERROR_TEXT);
            perror("File descriptor");
            exit(1);
        }
        if (-1 == ftruncate(fd, size))
        {
            printf(ERROR_TEXT);
            perror("Share memory cannot be resized");
            exit(1);
        }
        return fd;
        
}

void clean_buffer(char *buffer){
        int j = 0;
        while (buffer[j] != '\0')
        {
                buffer[j++] = '\0';
        }
}

void create_pipes(){
        int i;
        for (i = 0; i < PROCESSES_COUNT; i++)
        {
                if (pipe(fd_work[i]) != 0)
                {
                    printf(ERROR_TEXT);
                    perror("PIPE Work");
                    exit(1);
                }
                flags_fd_work_open[i] = 1;
                if (pipe(fd_results[i]) != 0)
                {
                        printf(ERROR_TEXT);
                        perror("PIPE results");
                        exit(1);
                }
        }
}

void create_slaves(){
        int i;
        for (i = 0; i < PROCESSES_COUNT; i++)
        {
                if (0 == (processes[i] = fork()))
                {
                        int j;
                        for ( j = 0; j < PROCESSES_COUNT; j++)
                        {
                                // Cerramos los pipes ajenos a este hijo
                                if (j != i){
                                     close(fd_work[j][0]);
                                     close(fd_work[j][1]);
                                     close(fd_results[j][0]);
                                     close(fd_results[j][1]);
                             }
                        }
                        close(fd_work[i][1]);
                        close(fd_results[i][0]);

                        // Redireccionamos la entrada del hijo al nuevo pipe
                        if (dup2(fd_work[i][0], STDIN_FILENO) < 0)
                        {
                            printf(ERROR_TEXT);
                            perror("Dup work");
                            exit(1);
                        }
                        
                        if (dup2(fd_results[i][1], STDOUT_FILENO) < 0)
                        {
                            printf(ERROR_TEXT);
                            perror("Dup results");
                            exit(1);
                        }

                        char *const params[] = {"worker", NULL};
                        int res_execv = execv(params[0], params);
                        if (res_execv < 0)
                        {
                            printf(ERROR_TEXT);
                            perror("Execv");
                            exit(1);
                        }
                }
                else if (processes[i] < 0){
                    printf(ERROR_TEXT);
                    perror("Fork");
                }
        }
}

void concat_files(int files_count, const char *files[], char concat[]){
    strcpy(concat, files[0]);
    strcat(concat, "\n");
    int i = 1;
    for (; i < files_count; i++)
    {
        strcat(concat, files[i]);
        strcat(concat, "\n");
    }
    
}


#endif