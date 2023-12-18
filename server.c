#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

void send_message(const char *fifo_name, const char *message) {
    // Открываем канал для записи
    int fd = open(fifo_name, O_WRONLY);
    if (fd == -1) {
        perror("Error opening FIFO for writing");
        exit(EXIT_FAILURE);
    }

    // Пишем сообщение в канал
    size_t message_len = strlen(message);
    ssize_t bytes_written = write(fd, message, message_len);
    if (bytes_written == -1) {
        perror("Error writing to FIFO");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

char *receive_message(const char *fifo_name, int buffer_size) {
    int fd = open(fifo_name, O_RDONLY);
    if (fd == -1) {
        perror("Error opening FIFO for reading");
        exit(EXIT_FAILURE);
    }

    // Считываем сообщение из канала
    char *buffer = malloc(buffer_size);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read = read(fd, buffer, buffer_size);
    if (bytes_read == -1) {
        perror("Error reading from FIFO");
        close(fd);
        free(buffer); // Освобождаем память в случае ошибки
        exit(EXIT_FAILURE);
    }

    close(fd);

    return buffer;
}

void echo(const char * write_fifo, char * argument) {
                // Выполняем команду echo
                if (argument != NULL) {
                    // Отправляем аргумент обратно клиенту
                    send_message(write_fifo, argument);
                } else {
                    // Отправляем сообщение об ошибке клиенту
                    send_message(write_fifo, "Invalid usage of echo command");
                }
}

void get_time(const char * write_fifo) {
	
}

int main() {
    const char *write_fifo = "client_fifo";
    const char *read_fifo = "server_fifo";
    const char *message = "Hello, client!";

    if (mkfifo(write_fifo, 0666) == -1) {
        perror("Failed to create server fifo\n");
        exit(EXIT_FAILURE);
    }

    printf("Creating FIFO...\n");

    sleep(3);

    while (1) {
        char *command_line = receive_message(read_fifo, 100);
        printf("Command: %s\n", command_line);
        if (strncmp(command_line, "stop", 4) == 0) {
        	printf("STOP\n");	
            free(command_line);
            break;
        }

        // Парсим команду и её аргументы
        char *command = strtok(command_line, " ");
        char *argument = strtok(NULL, "\n");
        printf("Receieved argument: %s\n", argument);
        if (command != NULL) {
            if (strcmp(command, "echo") == 0) {
				echo(write_fifo, argument);
            } else {
                // Отправляем сообщение об ошибке клиенту (неизвестная команда)
                send_message(write_fifo, "Unknown command");
            }
        }

        printf("Received command from client: %s\n", command_line);
        free(command_line);
    }

    // Удаляем именованный канал
    if (unlink(write_fifo) == -1) {
        perror("Error unlinking FIFO");
        exit(EXIT_FAILURE);
    }

    return 0;
}
