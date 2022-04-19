// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "view.h"

// Resource: https://github.com/WhileTrueThenDream/ExamplesCLinuxUserSpace
int main(void)
{
        int shm_fd;
        char *pointer_shm;
        struct stat share_memory_obj_st;
        char *aux_pointer_shm;

        sem_t *sem_r_share_memory = sem_open(SEMAPHORE_NAME, O_CREAT);
        if (SEM_FAILED == sem_r_share_memory)
        {
                print_error(FILE_NAME, "main: sem_open", errno);
        }
        int ans = sem_wait(sem_r_share_memory);
        if (ans < 0)
        {
                print_error(FILE_NAME, "main: sem_wait", errno);
        }

        // open s.m object
        while (-1 == (shm_fd = shm_open(SHARED_MEMORY_OBJ_NAME, O_RDONLY, 00400)))
                ;

        if (-1 == fstat(shm_fd, &share_memory_obj_st))
        {
                print_error(FILE_NAME, "main: fstat", errno);
        }
        pointer_shm = mmap(NULL, share_memory_obj_st.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
        aux_pointer_shm = mmap(NULL, share_memory_obj_st.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
        if (MAP_FAILED == pointer_shm || MAP_FAILED == aux_pointer_shm)
        {
                print_error(FILE_NAME, "main: mmap", errno);
        }
        int files = (int)atoi(aux_pointer_shm);
        aux_pointer_shm += sizeof(files);
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
                }

                files--;
                memcpy(buffer, aux_pointer_shm, SIZEOF_RESPONSE);
                aux_pointer_shm += SIZEOF_RESPONSE;

                write(STDOUT_FILENO, buffer, SIZEOF_RESPONSE);
                clean_buffer(buffer);
        }
        printf("\n ///////////////////////////////////////////////////////////////////////\n\n");
        printf("VIEW Process: all solutions were printed correctly\n\n");

        if (sem_close(sem_r_share_memory) < 0)
        {
                print_error(FILE_NAME, "main: sem_close", errno);
        }

        if (munmap(pointer_shm, share_memory_obj_st.st_size) < 0)
        {
                print_error(FILE_NAME, "main: munmap", errno);
        }
        close(shm_fd);
        return 0;
}
