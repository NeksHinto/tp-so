// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.h"

int main(int argc, char const *argv[])
{

    // MODULARIZAR LOS MENSAJES DE ERROR
    //  Resource: Arielito
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
        write(fd_works[i][1], files_concat, strlen(files_concat) + 1);
        clean_buffer(files_concat);
        files_count_to_send -= INITIAL_FILES_COUNT;
    }

    int nfds = 0;
    fd_set fd_workers;

    while ((argc - 1) > files_count_resolved)
    {
        char buf[BUFFER_SIZE] = {'\0'};

        reinitialize_fd_set(&nfds, &fd_workers);

        int select_ans = select(nfds, &fd_workers, NULL, NULL, NULL);

        if (select_ans < 0)
        {
            printf(ERROR_TEXT);
            perror("select");
            exit(EXIT_FAILURE);
        }
        else if (select_ans)
        {
            for (i = 0; i < PROCESSES_COUNT; i++)
            {
                if (FD_ISSET(fd_results[i][0], &fd_workers))
                {
                    res_processes[i]++;
                    if (read(fd_results[i][0], buf, sizeof(buf)) < 0)
                    {
                        printf(ERROR_TEXT);
                        perror("read");
                        exit(EXIT_FAILURE);
                    };

                    if (write(resolved_fd, buf, sizeof(buf) < 0))
                    {
                        printf(ERROR_TEXT);
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    files_count_resolved++;

                    memcpy(aux_pointer_sh_mem, buf, sizeof(buf));
                    int ret_val = sem_post(sem_w_shm);
                    if (ret_val < 0)
                    {
                        printf(ERROR_TEXT);
                        perror("sem_post");
                        exit(EXIT_FAILURE);
                    }

                    aux_pointer_sh_mem += SIZEOF_RESPONSE;
                    clean_buffer(buf);

                    if (res_processes[i] >= INITIAL_FILES_COUNT && files_count_to_send)
                    {
                        char aux_buffer[BUFFER_SIZE] = {'\0'};

                        strcpy(aux_buffer, argv[offset_args++]);
                        strcat(aux_buffer, "\n");

                        // Mandamos otra tarea a un worker
                        write(fd_works[i][1], aux_buffer, strlen(aux_buffer));
                        files_count_to_send--;
                        clean_buffer(aux_buffer);
                    }
                    else if (0 == files_count_to_send)
                    {
                        close(fd_works[i][1]);
                        close(fd_results[i][0]);
                        flags_fd_work_open[i] = 0;
                        waitpid(processes[i], NULL, 0); // esperando que terminen los hijos
                    }
                }
            }
        }
        else
        {
            printf("Select timeout expired\n");
        }
    }
    char buffer[BUFFER_SIZE] = {'\0'};

    sprintf(buffer, "Number of resolved files: %d\n", files_count_resolved);
    memcpy(aux_pointer_sh_mem, buffer, sizeof(buffer));

    if (sem_close(sem_w_shm) < 0 || sem_unlink(SEMAPHORE_NAME) < 0)
    {
        printf(ERROR_TEXT);
        perror("sem_close | sem_unlink");
        exit(EXIT_FAILURE);
    }

    if (munmap(pointer_sh_mem, (SIZEOF_RESPONSE * files_count_resolved)) < 0)
    {

        printf(ERROR_TEXT);
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(SHARED_MEMORY_OBJ_NAME) < 0)
    {
        printf(ERROR_TEXT);
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    close(fd_shared_memory);

    write(resolved_fd, SEPARATOR, sizeof(SEPARATOR));
    close(resolved_fd);
    return 0;
}

void check_format(int files_count, const char *files[], char *format)
{
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

int shm_create(size_t size)
{
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

void clean_buffer(char *buffer)
{
    int j = 0;
    while (buffer[j] != '\0')
    {
        buffer[j++] = '\0';
    }
}

void create_pipes()
{
    int i;
    for (i = 0; i < PROCESSES_COUNT; i++)
    {
        if (pipe(fd_works[i]) != 0)
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

void create_slaves()
{
    int i;
    for (i = 0; i < PROCESSES_COUNT; i++)
    {
        if (0 == (processes[i] = fork()))
        {
            int j;
            for (j = 0; j < PROCESSES_COUNT; j++)
            {
                // Cerramos los pipes ajenos a este hijo
                if (j != i)
                {
                    close(fd_works[j][0]);
                    close(fd_works[j][1]);
                    close(fd_results[j][0]);
                    close(fd_results[j][1]);
                }
            }
            close(fd_works[i][1]);
            close(fd_results[i][0]);

            // Redireccionamos la entrada del hijo al nuevo pipe
            if (dup2(fd_works[i][0], STDIN_FILENO) < 0)
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
        else if (processes[i] < 0)
        {
            printf(ERROR_TEXT);
            perror("Fork");
        }
    }
}

void concat_files(int files_count, const char *files[], char concat[])
{
    strcpy(concat, files[0]);
    strcat(concat, "\n");
    int i = 1;
    for (; i < files_count; i++)
    {
        strcat(concat, files[i]);
        strcat(concat, "\n");
    }
}

void reinitialize_fd_set(int *nfds, fd_set *fd_workers)
{

    int aux_nfds = 0;

    fd_set aux_fd_workers;
    FD_ZERO(&aux_fd_workers); // clears (removes all file descriptors from) set

    int i = 0;
    for (; i < PROCESSES_COUNT; i++)
    {
        if (flags_fd_work_open[i] != 0)
        {
            FD_SET(fd_results[i][0], &aux_fd_workers); // adds the file descriptor fd to set
            if (fd_results[i][0] > aux_nfds)
            {
                aux_nfds = fd_results[i][0] + 1;
            }
        }
    }

    *nfds = aux_nfds;
    *fd_workers = aux_fd_workers;
}