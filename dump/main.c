#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE 8

int main(int argc, char** argv) {
    // hexdumb <datei>
    if (argc != 2 ){
        printf("Usage: %s <datei> \n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");

    if (file == NULL) {
        perror("fopen");
        return 1;
    }
    char buffer[BUFFER_SIZE];
    size_t read_bytes = 0;

    bool new_line = false;
    uint8_t offset = 0;
    printf("00000000   ");
    while ((read_bytes = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (i < read_bytes) {
                uint8_t byte = buffer[i];
                printf("%02x ",byte);
            }
        }

        offset += read_bytes;
        if (new_line) {
            printf("\n");
            printf("%08x   ", offset);
            new_line = false;
        }

        else if (!new_line) {
            printf("  ");
            new_line = true;
        }
    }
    printf("\n");
}
