// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.h"


int main(int argc, char const *argv[]){

    // Resource: Arielito
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, 0, _IONBF, 0);

    check_format(files_count_to_send, (argv + 1), ".cnf");
    create_pipes();
    create_slaves();
    
    sem_t *sem_w_shm = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 0);

    int files_count_to_send = argc - 1;
    int i = 1;

    int fd_shared_memory = shm_create(SIZEOF_RESPONSE * files_count_to_send);
    pointer_sh_mem = mmap(NULL, SIZEOF_RESPONSE * files_count_to_send, PROT_WRITE, MAP_SHARED, fd_shared_memory, 0);
    aux_pointer_sh_mem = mmap(NULL, SIZEOF_RESPONSE * files_count_to_send, PROT_WRITE, MAP_SHARED, fd_shared_memory, 0);
    if (pointer_sh_mem == MAP_FAILED || aux_pointer_sh_mem == MAP_FAILED)
    {
        printf(ERROR_TEXT);
        perror("Map failed in write process");
        exit(1);
    }

    sleep(2); // Espera al proceso vista (consigna)

    char num[10];
    sprintf(num, "%d", files_count_to_send);
    strcpy(aux_pointer_sh_mem, num);
    aux_pointer_sh_mem += sizeof(num);

    int return_sem;
    return_sem = sem_post(sem_w_shm);
    if (return_sem < 0)
    {
        printf(ERROR_TEXT);
        perror("SEM_POST");
        exit(1);
    }

    int resolved_fd;
    // Resource: https://linux.die.net/man/3/open
    if ((resolved_fd = open(FILE_OUTPUT, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR)) < 0)
    {
        printf(ERROR_TEXT);
        perror("Open");
        exit(1);
    }
    
    char files_concat[BUFFER_SIZE] = {'\0'};

    // Mandamos archivos a los hijos
    for (i = 0; i < PROCESSES_COUNT; i++)
    {
        concat_files(INITIAL_FILES_COUNT, (argv + offset_args), files_concat);
        offset_args += INITIAL_FILES_COUNT;
        write(fd_work[i][1], files_concat, strlen(files_concat) + 1);
        clean_buffer(files_concat);
        files_count_to_send -= INITIAL_FILES_COUNT;
    }
    
    return 0;
}