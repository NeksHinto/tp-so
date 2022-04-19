// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.h"

int main(int argc, char const *argv[])
{

    // No buffering (tp0)
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, 0, _IONBF, 0);

    int files_count_to_send = argc - 1;
    validate_format(files_count_to_send, (argv + 1), ".cnf");

    create_pipes();
    create_workers();

    // From sem_open documentation: "Both read and write permission
    // should be granted to each class of user that will access the semaphore."
    // 0644 = 6 (owner permissions: R&W), 4 (other group users: READONLY), 4 (other: READONLY)
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

    // "Pushea" en el "stack" compartido el nro de args
    char args_count[10];
    sprintf(args_count, "%d", files_count_to_send);
    strcpy(aux_pointer_sh_mem, args_count);
    aux_pointer_sh_mem += sizeof(args_count);

    // Unlock semaphore
    int return_sem;
    return_sem = sem_post(sem_w_shm);
    if (return_sem < 0)
    {
        print_error(FILE_NAME, "main: sem_post", errno);
    }

    int solved_fd;
    // Creamos archivo para printear los resultados
    if ((solved_fd = open(FILE_OUTPUT, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR)) < 0)
    {
        print_error(FILE_NAME, "main: open", errno);
    }

    char files_concat[BUFFER_SIZE] = {'\0'};

    // Le mandamos 2 archivos a cada proceso hijo
    for (i = 0; i < PROCESSES_COUNT; i++)
    {
        concat_files(INITIAL_FILES_COUNT, (argv + offset_args), files_concat);
        offset_args += INITIAL_FILES_COUNT;
        write(fd_works[i][1], files_concat, strlen(files_concat) + 1);
        clean_buffer(files_concat);
        files_count_to_send -= INITIAL_FILES_COUNT;
    }

    int nfds = 0; // number of fds to monitor
    fd_set fd_workers;

    while ((argc - 1) > solved_files_count)
    {
        char buf[BUFFER_SIZE] = {'\0'};

        initialize_fd_set(&nfds, &fd_workers);

        int sret = select(nfds, &fd_workers, NULL, NULL, NULL); // returns number of fds ready to be read

        if (sret < 0)
        {
            print_error(FILE_NAME, "main: select", errno);
        }
        else if (sret)
        {
            for (i = 0; i < PROCESSES_COUNT; i++)
            {
                if (FD_ISSET(fd_results[i][0], &fd_workers)) // checks if worker result ready to be read
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

                    // If worker finishes with its 2 files & there are more files to solve, send more files to worker
                    if (solved_per_process[i] >= INITIAL_FILES_COUNT && files_count_to_send > 0)
                    {
                        char aux_buffer[BUFFER_SIZE] = {'\0'};

                        strcpy(aux_buffer, argv[offset_args++]); // chequear esto con concat_files
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
                // Cerramos los pipes ajenos a este hijo
                if (j != i)
                {
                    close(fd_works[j][0]);
                    close(fd_works[j][1]);
                    close(fd_results[j][0]);
                    close(fd_results[j][1]);
                }
            }
            // Cerramos canal de escritura en el works y el de lectura en el results
            // del worker que estamos creando
            close(fd_works[i][1]);
            close(fd_results[i][0]);

            // Redireccionamos la entrada del hijo al fd_read
            if (dup2(fd_works[i][0], STDIN_FILENO) < 0)
            {
                print_error(FILE_NAME, "create_workers: workers' dup", errno);
            }

            // Redireccionamos la salida del hijo al fd_write
            if (dup2(fd_results[i][1], STDOUT_FILENO) < 0)
            {
                print_error(FILE_NAME, "create_workers: results's dup", errno);
            }

            // Ejecutar worker desde application (reemplaza pid_application = pid_worker)
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
    FD_ZERO(&aux_workers_fds); // sets all fds to 0

    int i = 0;
    for (; i < PROCESSES_COUNT; i++)
    {
        if (flags_fd_work_open[i] != 0)
        {
            FD_SET(fd_results[i][0], &aux_workers_fds); // sets fd_read from result pipe to workers' set of fds
            if (fd_results[i][0] > aux_nfds)
            {
                aux_nfds = fd_results[i][0] + 1;
            }
        }
    }

    *nfds = aux_nfds;
    *fd_workers = aux_workers_fds;
}