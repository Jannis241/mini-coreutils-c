#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 64

void xor_file(FILE *file, const char *key) {
    size_t key_len = strlen(key);

    if (key_len == 0) {
        fprintf(stderr, "Key darf nicht leer sein.\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t offset = 0;

    rewind(file);

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {

        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] ^= key[(offset + i) % key_len];
        }

        fseek(file, -(long)bytes_read, SEEK_CUR);
        fwrite(buffer, 1, bytes_read, file);

        offset += bytes_read;
    }

    fflush(file);
    rewind(file);
}

void decrypt_file(FILE *file, const char *key) {
    xor_file(file, key);
}

void encrypt_file(FILE *file, const char *key) {
    xor_file(file, key);
}

int open_nvim(const char *file_name) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execlp("nvim", "nvim", file_name, NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("nvim beendet (Exit-Code %d)\n", WEXITSTATUS(status));
    }

    return 0;
}

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <datei> <key>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    const char *key = argv[2];

    FILE *file = fopen(filename, "r+b");
    if (!file) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    printf("Entschlüssele Datei...\n");
    decrypt_file(file, key);

    fclose(file);

    printf("Öffne Neovim...\n");
    if (open_nvim(filename) != 0)
        return EXIT_FAILURE;

    file = fopen(filename, "r+b");
    if (!file) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    printf("Verschlüssele Datei...\n");
    encrypt_file(file, key);

    fclose(file);

    printf("Fertig.\n");

    return EXIT_SUCCESS;
}
