#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

int main(int argc, char **argv) {
    const char *read_fifo = "client_fifo";
    const char *write_fifo = argv[1];

    // Создаем именованный канал (fifo)
    if (mkfifo(write_fifo, 0666) == -1) {
        perror("Error creating FIFO");
        exit(EXIT_FAILURE);
    }

    printf("Creating FIFO...\n");
    sleep(3);

    char *input = NULL;
    size_t input_size = 0;

    while (1) {
        printf("Введите строку (или 'stop' для завершения): ");
        ssize_t read_size = getline(&input, &input_size, stdin);

        if (read_size == -1) {
            perror("Error reading input");
            free(input);
            exit(EXIT_FAILURE);
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Копируем строку и отправляем её
        char *message = strdup(input);
        if (message == NULL) {
            perror("Error duplicating input");
            free(input);
            exit(EXIT_FAILURE);
        }

        send_message(write_fifo, message);

        // Проверяем условие завершения
        if (strcmp(message, "stop") == 0) {
            free(input);
            break;
        }


        // Читаем вывод сервера из client_pipe
        char *server_output = receive_message(read_fifo, 100);
        printf("Результат выполнения команды на сервере:\n%s\n", server_output);

        // Освобождаем память
        free(message);
        free(server_output);
    }

    // Удаляем именованный канал
    if (unlink(write_fifo) == -1) {
        perror("Error unlinking FIFO");
        exit(EXIT_FAILURE);
    }

    return 0;
}
