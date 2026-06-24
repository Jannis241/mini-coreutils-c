#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <src> <dest>\n", argv[0]);
        return 1;
    }

    // wenn der input eine c datei wäre
    // char clang_cmd[256];
    // snprintf(clang_cmd, sizeof clang_cmd,
    //          "clang -Oz -fno-unwind-tables -fno-asynchronous-unwind-tables "
    //          "-ffunction-sections -fdata-sections -Wl,--gc-sections "
    //          "-Wl,--strip-all %s -o app",
    //          argv[1]);
    //
    // clang_cmd[255] = '\0';
    // system(clang_cmd);

    char strip[64];
    snprintf(strip, sizeof strip, "strip --strip-all %s", argv[1]);
    system(strip);

    char xz[128];
    snprintf(xz, sizeof xz,
             "xz -9e --check=crc32 --lzma2=preset=9e,dict=64MiB -c %s > app.xz",
             argv[1]);
    system(xz);

    FILE *input_file = fopen("app.xz", "rb");
    if (!input_file) {
        perror("fopen");
        return 1;
    }

    FILE *output_file = fopen(argv[2], "wb");
    if (!output_file) {
        perror("fopen");
        return 1;
    }

    uint8_t bytes_buffer[16];
    size_t n;

    fprintf(output_file, "#include <stddef.h>\n");
    fprintf(output_file, "static const unsigned char bytes[] = \"");

    size_t col = 0;
    while ((n = fread(bytes_buffer, 1, sizeof bytes_buffer, input_file)) > 0) {
        for (size_t i = 0; i < n; i++) {
            fprintf(output_file, "\\x%02X", bytes_buffer[i]);
        }
    }
    fprintf(output_file, "\";\n");
    fprintf(output_file, "static const size_t bytes_len = sizeof bytes - 1;\n");
    fprintf(
        output_file,
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "#include <unistd.h>\n"
        "#include <sys/stat.h>\n"
        "int main(){"
        "FILE*f=fopen(\"/tmp/payload.xz\",\"wb\");"
        "if(!f)return 1;"
        "fwrite(bytes,1,bytes_len,f);"
        "fclose(f);"
        "if(system(\"xz -d -c /tmp/payload.xz >/tmp/payload_app\"))return 1;"
        "chmod(\"/tmp/payload_app\",0700);"
        "char*a[]={\"/tmp/payload_app\",0};"
        "execv(a[0],a);"
        "return 1;"
        "}\n");

    system("rm app.xz");

    fclose(input_file);
    fclose(output_file);

    char runner_cmd[512];
    snprintf(runner_cmd, sizeof runner_cmd,
             "clang -Oz "
             "-fno-unwind-tables "
             "-fno-asynchronous-unwind-tables "
             "-ffunction-sections "
             "-fdata-sections "
             "-Wl,--gc-sections "
             "-Wl,--strip-all "
             "%s -o compiled_output",
             argv[2]);

    system(runner_cmd);
    system("strip --strip-all compiled_output");
}
