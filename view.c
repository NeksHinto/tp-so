// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "view.h"

//Codigo prestado: https://github.com/WhileTrueThenDream/ExamplesCLinuxUserSpace
int main(void)
{ 
        int fd_share_memory;
        char *pointer;
        struct stat share_memory_obj_st;
        char *aux_pointer;

        sem_t *sem_r_share_memory = sem_open(SEMAPHORE_NAME, O_CREAT);
        if (SEM_FAILED == sem_r_share_memory)
        {
                print_error(FILE_NAME, "main: sem_open", errno);
                exit(EXIT_FAILURE);
        }
        int ans = sem_wait(sem_r_share_memory);
        if (ans < 0)
        {
                print_error(FILE_NAME, "main: sem_wait", errno);
                exit(EXIT_FAILURE);
        }

        // open s.m object
        while (-1 == (fd_share_memory = shm_open(SHARED_MEMORY_OBJ_NAME, O_RDONLY, 00400)))
                ;

        if (-1 == fstat(fd_share_memory, &share_memory_obj_st))
        {
                print_error(FILE_NAME, "main: fstat", errno);
                exit(EXIT_FAILURE);
        }
        pointer = mmap(NULL, share_memory_obj_st.st_size, PROT_READ, MAP_SHARED, fd_share_memory, 0);
        aux_pointer = mmap(NULL, share_memory_obj_st.st_size, PROT_READ, MAP_SHARED, fd_share_memory, 0);
        if (MAP_FAILED == pointer || MAP_FAILED == aux_pointer)
        {
                print_error(FILE_NAME, "main: mmap", errno);
                exit(EXIT_FAILURE);
        }
        int files = (int)atoi(aux_pointer);
        aux_pointer += sizeof(files);
        printf("\n");
        printf("VIEW Process: %d solutions for printing\n", files);
        printf("\n ///////////////////////////////////////////////////////////////////////\n\n");
        ans = 0;

        while (files > 0)
        {
                char buffer[SIZEOF_RESPONSE] = {'\0'};
                ans = sem_wait(sem_r_share_memory);
                if (ans < 0)
                {
                      print_error(FILE_NAME, "main: sem_wait", errno);
                      exit(EXIT_FAILURE);
                }

                files--;
                memcpy(buffer, aux_pointer, SIZEOF_RESPONSE);
                aux_pointer += SIZEOF_RESPONSE;

                write(STDOUT_FILENO, buffer, SIZEOF_RESPONSE);
                clean_buffer(buffer);
        }
        printf("\n ///////////////////////////////////////////////////////////////////////\n\n");
        printf("VIEW Process: all solutions were printed correctly\n\n");

        if (sem_close(sem_r_share_memory) < 0)
        {
                print_error(FILE_NAME, "main: sem_close", errno);
                exit(EXIT_FAILURE);
        }

        if (munmap(pointer, share_memory_obj_st.st_size) < 0)
        {
                print_error(FILE_NAME, "main: munmap", errno);
                exit(EXIT_FAILURE);
        }
        close(fd_share_memory);
        return 0;
}

void clean_buffer(char *buffer)
{
        int j = 0;
        while (buffer[j] != '\0')
        {
                buffer[j++] = '\0';
        }
}