#define _XOPEN_SOURCE 700

#include <ftw.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CYAN "\x1b[36m"
#define YELLOW "\x1b[33m"
#define RED_UNDERLINE "\x1b[31;4m"
#define RESET "\x1b[0m"

static const char *pattern;
static size_t pattern_len;

static bool valid_utf8(const unsigned char *s, size_t len) {
    size_t i = 0;

    while (i < len) {
        unsigned char c = s[i];

        if (c <= 0x7F) {
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= len) return false;
            if ((s[i + 1] & 0xC0) != 0x80) return false;
            if (c < 0xC2) return false;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= len) return false;
            if ((s[i + 1] & 0xC0) != 0x80) return false;
            if ((s[i + 2] & 0xC0) != 0x80) return false;
            if (c == 0xE0 && s[i + 1] < 0xA0) return false;
            if (c == 0xED && s[i + 1] >= 0xA0) return false;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= len) return false;
            if ((s[i + 1] & 0xC0) != 0x80) return false;
            if ((s[i + 2] & 0xC0) != 0x80) return false;
            if ((s[i + 3] & 0xC0) != 0x80) return false;
            if (c == 0xF0 && s[i + 1] < 0x90) return false;
            if (c == 0xF4 && s[i + 1] >= 0x90) return false;
            if (c > 0xF4) return false;
            i += 4;
        } else {
            return false;
        }
    }

    return true;
}

static const void *find_mem(const void *hay,
                            size_t hay_len,
                            const void *needle,
                            size_t needle_len) {
    if (needle_len == 0 || hay_len < needle_len) return NULL;

    const unsigned char *h = hay;
    const unsigned char *n = needle;

    for (size_t i = 0; i <= hay_len - needle_len; i++) {
        if (h[i] == n[0] && memcmp(h + i, n, needle_len) == 0) {
            return h + i;
        }
    }

    return NULL;
}

static void print_match(const char *path,
                        size_t line_no,
                        const char *line_start,
                        size_t line_len,
                        const char *match_start) {
    size_t before_len = (size_t)(match_start - line_start);
    size_t after_start = before_len + pattern_len;
    size_t after_len = line_len - after_start;

    printf(CYAN "%s" RESET " " YELLOW "%zu" RESET ": ",
           path,
           line_no);

    fwrite(line_start, 1, before_len, stdout);

    fputs(RED_UNDERLINE, stdout);
    fwrite(match_start, 1, pattern_len, stdout);
    fputs(RESET, stdout);

    fwrite(line_start + after_start, 1, after_len, stdout);
    putchar('\n');
}

static void search_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    struct stat st;
    if (fstat(fd, &st) != 0 || st.st_size <= 0) {
        close(fd);
        return;
    }

    size_t size = (size_t)st.st_size;

    char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (data == MAP_FAILED) return;

    if (!valid_utf8((const unsigned char *)data, size)) {
        munmap(data, size);
        return;
    }

    size_t line_no = 1;
    char *line_start = data;
    char *end = data + size;

    for (char *p = data; p <= end; p++) {
        if (p == end || *p == '\n') {
            size_t line_len = (size_t)(p - line_start);

            const char *match = find_mem(
                line_start,
                line_len,
                pattern,
                pattern_len
            );

            if (match) {
                print_match(path, line_no, line_start, line_len, match);
            }

            line_no++;
            line_start = p + 1;
        }
    }

    munmap(data, size);
}

static int walk_cb(const char *fpath,
                   const struct stat *sb,
                   int typeflag,
                   struct FTW *ftwbuf) {
    (void)sb;
    (void)ftwbuf;

    if (typeflag == FTW_F) {
        search_file(fpath);
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [PATTERN] [DIRECTORY]\n", argv[0]);
        return 1;
    }

    pattern = argv[1];
    pattern_len = strlen(pattern);

    if (pattern_len == 0) {
        fprintf(stderr, "Pattern darf nicht leer sein\n");
        return 1;
    }

    const char *directory = argv[2];

    if (nftw(directory, walk_cb, 64, FTW_PHYS) != 0) {
        perror("nftw");
        return 1;
    }

    return 0;
}
