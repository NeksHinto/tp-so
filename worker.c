// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "worker.h"

int main(int argc, char const *argv[])
{
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stdin, 0, _IONBF, 0);
        char buffer[BUFFER_SIZE] = {'\0'};

        while (read(STDIN_FILENO, buffer, sizeof(buffer)))
        {
                int i = 0;
                int j = 0;
                int buffer_eof = 0;
                char buffer_aux[BUFFER_SIZE] = {'\0'};
                while (!buffer_eof)
                {
                        if (buffer[i] != '\n' && buffer[i] != '\0')
                        {
                                buffer_aux[j++] = buffer[i];
                        }
                        else if (buffer[i] == '\n')
                        {
                                buffer_aux[j + 1] = '\0';
                                j = 0;

                                char cmd[BUFFER_SIZE] = {'\0'};
                                char param[BUFFER_SIZE] = {"\0"};
                                strcpy(param, MINISAT);

                                strcat(param, buffer_aux);
                                strcat(param, GREP_AND_FLAGS);

                                int len = sprintf(cmd, "PID: %d | Solucion de: %s | ", getpid(), buffer_aux);
                                if (len < 0)
                                {
                                        printf("PID: %d | ", getpid());
                                        printf(ERROR_TEXT);
                                        perror("sprintf\n");
                                        abort();
                                }

                                FILE *fp = popen(param, "r");
                                if (fp == NULL)
                                {
                                        printf("PID: %d | ", getpid());
                                        printf(ERROR_TEXT);
                                        perror("popen\n");
                                        abort();
                                }
                                fgets(&cmd[len], BUFFER_SIZE, fp); // Validar que no retorne NULL ?
                                int status = pclose(fp);
                                if (status < 0)
                                {
                                        printf("PID: %d | ", getpid());
                                        printf(ERROR_TEXT);
                                        perror("pclose\n");
                                        abort();
                                }
                                printf("%s \n", cmd);
                                clean_buffer(cmd);
                                clean_buffer(param);
                                clean_buffer(buffer_aux);
                        }
                        else if (buffer[i] == '\0')
                        {
                                buffer_eof = 1;
                        }
                        i++;
                }
                clean_buffer(buffer);
        }

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