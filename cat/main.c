#include <stddef.h>
#include <stdio.h>

#define BUFFER_SIZE 32



int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <name> \n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");

    if (!file) {
        perror("fopen");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t read = 0;

    while((read = fread(buffer, 1, BUFFER_SIZE - 1, file)) > 0) {
        buffer[read] = '\0';
        printf("%s", buffer);
    }

    return 0;
}
