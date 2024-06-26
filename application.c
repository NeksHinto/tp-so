// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.h"

int main(int argc, char const *argv[])
{

    // No buffering (tp0)
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, 0, _IONBF, 0);

    int files_count_to_send = argc - 1;
    check_format(files_count_to_send, (argv + 1), ".cnf");

    create_pipes();
    create_slaves();

    sem_t *sem_w_shm = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 0);

    int i = 1;

    int fd_shared_memory = shm_create(SIZEOF_RESPONSE * files_count_to_send);
    pointer_sh_mem = mmap(NULL, SIZEOF_RESPONSE * files_count_to_send, PROT_WRITE, MAP_SHARED, fd_shared_memory, 0);
    aux_pointer_sh_mem = mmap(NULL, SIZEOF_RESPONSE * files_count_to_send, PROT_WRITE, MAP_SHARED, fd_shared_memory, 0);
    if (pointer_sh_mem == MAP_FAILED || aux_pointer_sh_mem == MAP_FAILED)
    {
        print_error(FILE_NAME, "create_slaves: mmap", errno);
        exit(EXIT_FAILURE);
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
        print_error(FILE_NAME, "main: sem_post", errno);
        exit(EXIT_FAILURE);
    }

    int resolved_fd;
    // Resource: https://linux.die.net/man/3/open
    if ((resolved_fd = open(FILE_OUTPUT, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR)) < 0)
    {
        print_error(FILE_NAME, "main: open", errno);
        exit(EXIT_FAILURE);
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
                    }

                    files_count_resolved++;

                    if (write(resolved_fd, buf, sizeof(buf)) < 0)
                    {
                        printf(ERROR_TEXT);
                        perror("write");
                        exit(EXIT_FAILURE);
                    }

                    memcpy(aux_pointer_sh_mem, buf, sizeof(buf));
                    int ret_val;
                    ret_val = sem_post(sem_w_shm);
                    if (ret_val < 0)
                    {
                        printf(ERROR_TEXT);
                        perror("sem_post");
                        exit(EXIT_FAILURE);
                    }

                    aux_pointer_sh_mem += SIZEOF_RESPONSE;
                    clean_buffer(buf);

                    if (res_processes[i] >= INITIAL_FILES_COUNT && files_count_to_send > 0)
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
            print_error(FILE_NAME, "check_format", errno);
            exit(EXIT_FAILURE);
        }
    }
}

int shm_create(size_t size)
{
    int fd;
    fd = shm_open(SHARED_MEMORY_OBJ_NAME, O_CREAT | O_RDWR, 00700);
    if (-1 == fd)
    {
        print_error(FILE_NAME, "shm_create: shm_open", errno);
        exit(EXIT_FAILURE);
    }
    if (-1 == ftruncate(fd, size))
    {
        print_error(FILE_NAME, "shm_create: shm_open", errno);
        exit(EXIT_FAILURE);
    }
    return fd;
}

void create_pipes()
{
    int i;
    for (i = 0; i < PROCESSES_COUNT; i++)
    {
        if (pipe(fd_works[i]) != 0)
        {
            print_error(FILE_NAME, "create_pipes: workers", errno);
            exit(EXIT_FAILURE);
        }
        flags_fd_work_open[i] = 1;
        if (pipe(fd_results[i]) != 0)
        {
            print_error(FILE_NAME, "create_pipes: results", errno);
            exit(EXIT_FAILURE);
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
                print_error(FILE_NAME, "create_slaves: workers's dup", errno);
                exit(EXIT_FAILURE);
            }

            if (dup2(fd_results[i][1], STDOUT_FILENO) < 0)
            {
                print_error(FILE_NAME, "create_slaves: results's dup", errno);
                exit(EXIT_FAILURE);
            }

            char *const params[] = {"worker", NULL};
            int res_execv = execv(params[0], params);
            if (res_execv < 0)
            {
                print_error(FILE_NAME, "create_slaves: execv", errno);
                exit(EXIT_FAILURE);
            }
        }
        else if (processes[i] < 0)
        {
          print_error(FILE_NAME, "create_slaves: fork", errno);

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