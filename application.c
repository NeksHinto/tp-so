// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.h"

int main(int argc, char const *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, 0, _IONBF, 0);

    int files_count_to_send = argc - 1;
    validate_format(files_count_to_send, (argv + 1), ".cnf");

    create_pipes();
    create_named_pipe();
    create_workers();

    sem_t *sem_w_shm = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 0);

    int i = 1;

    int fd_shared_memory = shm_create(SIZEOF_RESPONSE * files_count_to_send);
    pointer_sh_mem = mmap(NULL, SIZEOF_RESPONSE * files_count_to_send, PROT_WRITE, MAP_SHARED, fd_shared_memory, 0);
    aux_pointer_sh_mem = mmap(NULL, SIZEOF_RESPONSE * files_count_to_send, PROT_WRITE, MAP_SHARED, fd_shared_memory, 0);
    if (pointer_sh_mem == MAP_FAILED || aux_pointer_sh_mem == MAP_FAILED)
    {
        print_error(FILE_NAME, "main: mmap", errno);
    }

    sleep(2); // Espera al proceso vista (consigna)

    char args_count[10];
    sprintf(args_count, "%d", files_count_to_send);
    strcpy(aux_pointer_sh_mem, args_count);
    aux_pointer_sh_mem += sizeof(args_count);

    int return_sem;
    return_sem = sem_post(sem_w_shm);
    if (return_sem < 0)
    {
        print_error(FILE_NAME, "main: sem_post", errno);
    }

    int solved_fd;
    if ((solved_fd = open(FILE_OUTPUT, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR)) < 0)
    {
        print_error(FILE_NAME, "main: open", errno);
    }

    char files_concat[BUFFER_SIZE] = {'\0'};

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

    while ((argc - 1) > solved_files_count)
    {
        char buf[BUFFER_SIZE] = {'\0'};

        initialize_fd_set(&nfds, &fd_workers);

        int sret = select(nfds, &fd_workers, NULL, NULL, NULL);

        if (sret < 0)
        {
            print_error(FILE_NAME, "main: select", errno);
        }
        else if (sret)
        {
            for (i = 0; i < PROCESSES_COUNT; i++)
            {
                if (FD_ISSET(fd_results[i][0], &fd_workers) != 0)
                {
                    solved_per_process[i]++;
                    if (read(fd_results[i][0], buf, sizeof(buf)) < 0)
                    {
                        print_error(FILE_NAME, "main: read", errno);
                    }

                    solved_files_count++;

                    if (write(solved_fd, buf, sizeof(buf)) < 0)
                    {
                        print_error(FILE_NAME, "main: write", errno);
                    }

                    memcpy(aux_pointer_sh_mem, buf, sizeof(buf));
                    int ret_val;
                    ret_val = sem_post(sem_w_shm);
                    if (ret_val < 0)
                    {
                        print_error(FILE_NAME, "main: sem_post", errno);
                    }
                    aux_pointer_sh_mem += SIZEOF_RESPONSE;
                    clean_buffer(buf);

                    if (solved_per_process[i] >= INITIAL_FILES_COUNT && files_count_to_send > 0)
                    {
                        char aux_buffer[BUFFER_SIZE] = {'\0'};

                        strcpy(aux_buffer, argv[offset_args++]);
                        strcat(aux_buffer, "\n");

                        write(fd_works[i][1], aux_buffer, strlen(aux_buffer));
                        files_count_to_send--;
                        clean_buffer(aux_buffer);
                    }
                    else if (files_count_to_send == 0)
                    {
                        close(fd_works[i][1]);
                        close(fd_results[i][0]);
                        flags_fd_work_open[i] = 0;
                        waitpid(processes[i], NULL, 0);
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

    sprintf(buffer, "Number of solved files: %d\n", solved_files_count);
    memcpy(aux_pointer_sh_mem, buffer, sizeof(buffer));

    if (sem_close(sem_w_shm) < 0 || sem_unlink(SEMAPHORE_NAME) < 0)
    {
        print_error(FILE_NAME, "main: sem_close", errno);
    }

    if (munmap(pointer_sh_mem, (SIZEOF_RESPONSE * solved_files_count)) < 0)
    {

        print_error(FILE_NAME, "main: munmap", errno);
    }

    if (shm_unlink(SHARED_MEMORY_OBJ_NAME) < 0)
    {
        print_error(FILE_NAME, "main: shm_unlink", errno);
    }

    close(fd_shared_memory);

    write(solved_fd, SEPARATOR, sizeof(SEPARATOR));
    close(solved_fd);
    return 0;
}

void validate_format(int files_count, const char *files[], char *format)
{
    int i = 0;
    for (; i < files_count; i++)
    {
        if (strstr(files[i], format) == NULL)
        {
            print_error(FILE_NAME, "validate_format", errno);
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
    }
    if (-1 == ftruncate(fd, size))
    {
        print_error(FILE_NAME, "shm_create: shm_open", errno);
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
        }
        flags_fd_work_open[i] = 1;
        if (pipe(fd_results[i]) != 0)
        {
            print_error(FILE_NAME, "create_pipes: results", errno);
        }
    }
}

void create_named_pipe()
{
    int error = mkfifo(NAMED_PIPE, 0666);
    if (error != 0)
    {
        print_error(FILE_NAME, "create_named_pipe: mkfifo", errno);
    }
    
}

void create_workers()
{
    int i;
    for (i = 0; i < PROCESSES_COUNT; i++)
    {
        if (0 == (processes[i] = fork()))
        {
            int j;
            for (j = 0; j < PROCESSES_COUNT; j++)
            {
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

            if (dup2(fd_works[i][0], STDIN_FILENO) < 0)
            {
                print_error(FILE_NAME, "create_workers: workers' dup", errno);
            }

            if (dup2(fd_results[i][1], STDOUT_FILENO) < 0)
            {
                print_error(FILE_NAME, "create_workers: results's dup", errno);
            }

            char *const argv[] = {"worker", NULL};
            int res_execv = execv(argv[0], argv);
            if (res_execv < 0)
            {
                print_error(FILE_NAME, "create_workers: execv", errno);
            }
        }
        else if (processes[i] < 0)
        {
            print_error(FILE_NAME, "create_workers: fork", errno);
        }
    }
}

void concat_files(int files_count, const char *files[], char str[])
{
    strcpy(str, files[0]);
    strcat(str, "\n");
    int i = 1;
    for (; i < files_count; i++)
    {
        strcat(str, files[i]);
        strcat(str, "\n");
    }
}

void initialize_fd_set(int *nfds, fd_set *fd_workers)
{

    int aux_nfds = 0;

    fd_set aux_workers_fds;
    FD_ZERO(&aux_workers_fds);

    int i = 0;
    for (; i < PROCESSES_COUNT; i++)
    {
        if (flags_fd_work_open[i] != 0)
        {
            FD_SET(fd_results[i][0], &aux_workers_fds);
            if (fd_results[i][0] > aux_nfds)
            {
                aux_nfds = fd_results[i][0] + 1;
            }
        }
    }

    *nfds = aux_nfds;
    *fd_workers = aux_workers_fds;
}