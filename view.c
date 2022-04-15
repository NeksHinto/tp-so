// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "view.h"

int main(void){ // Idea de: https://github.com/WhileTrueThenDream/ExamplesCLinuxUserSpace
        int fd_share_memory;
        char *pointer;
        struct stat share_memory_obj_st;

        sem_t *sem_r_share_memory = sem_open(SEMAPHORE_NAME, O_CREAT);
        if (SEM_FAILED == sem_r_share_memory)
        {
                printf(ERROR_TEXT);
                perror("sem_open");
                // abort();
                exit(-1);
        }
        int ans = sem_wait(sem_r_share_memory);
        if (ans < 0)
        {
                printf(ERROR_TEXT);
                perror("sem_wait");
                // abort();
                exit(-1);
        }
        
        // open s.m object
        while (-1 == (fd_share_memory = shm_open(SHARED_MEMORY_OBJ_NAME, O_RDONLY, 00400)));

        if (-1 == fstat(fd_share_memory, &share_memory_obj_st))
        {
                printf(ERROR_TEXT);
                perror("fstat");
                // abort();
                exit(-1);
        }
        pointer = mmap(NULL, share_memory_obj_st.st_size, PROT_READ, MAP_SHARED, fd_share_memory, 0);
        aux_pointer = mmap(NULL, share_memory_obj_st.st_size, PROT_READ, MAP_SHARED, fd_share_memory, 0);
        if (MAP_FAILED == pointer || MAP_FAILED == aux_pointer)
        {
                printf(ERROR_TEXT);
                perror("Map failed in read process");
                exit(-1);
        }
        int files = (int)atoi(aux_pointer);
        aux_pointer += sizeof(files);
        printf("\n");
        printf("VIEW Process: %d to print\n", files);
        printf("\n ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
        ans = 0;

        while (files > 0)
        {
                char buffer[SIZEOF_RESPONSE] = {'\0'};
                ans = sem_wait(sem_r_share_memory);
                if (ans < 0)
                {
                        printf(ERROR_TEXT);
                        perror("sem_wait");
                        exit(-1);
                }

                files--;
                memcpy(buffer, aux_pointer, SIZEOF_RESPONSE);
                aux_pointer += SIZEOF_RESPONSE;

                write(STDOUT_FILENO, buffer, SIZEOF_RESPONSE);
                clean_buffer(buffer);
        }
        printf("\n ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
        printf("VIEW Process: all solutions printed\n\n");

        if(sem_close(sem_r_share_memory) < 0){
                printf(ERROR_TEXT);
                perror("SEM close");
                exit(-1);
        }

        if (munmap(pointer, share_memory_obj_st.st_size) < 0)
        {
                printf(ERROR_TEXT);
                perror("munmap share memory");
                exit(-1);
        }
        close(fd_share_memory);
        return 0;
}