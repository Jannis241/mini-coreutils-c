#define _XOPEN_SOURCE 700
#define CONVERSION 1024.0
#define DESCRIPTORS 64

#include <time.h>
#include <stdint.h>
#include <stddef.h>
#include <ftw.h>
#include <stdio.h>

static double diff_seconds(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec)
         + (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

static const char* sizes[] = {"BYTES", "KiB", "MiB", "GiB", "TiB"};

static uintmax_t counter = 0;

static void print_pretty() {
    size_t len = sizeof(sizes) / sizeof(sizes[0]);

    double bytes = (double) counter;

    for (size_t i = 0; i < len; i++) {
        if (bytes >= 1024.0) {
            bytes /= CONVERSION;
        }
        else {
            printf("%0.2f %s \n", bytes, sizes[i]);
            break;
        }
    }
}

static int op(const char *fpath, const struct stat *sb, int typeflag,
       struct FTW *ftwbuf) {
    if (typeflag == FTW_F) {
        counter += sb->st_size;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <dir> \n", argv[0]);
        return 1;
    }
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    nftw(argv[1], op, DESCRIPTORS, FTW_PHYS);
    print_pretty();

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Time: %.6fs\n", diff_seconds(start, end));
    return 0;
}




ab ab ab
