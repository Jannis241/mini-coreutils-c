# mini-coreutils-c

Small versions of Unix command line tools, written in C.

I wanted to know how tools like `grep` or `cat` work, so I tried to build simple versions of them myself. They don't have all the options of the real tools, but they work for the basic use cases.

## Tools

| Tool | What it does | Usage |
|---|---|---|
| `cat` | prints the content of a file | `cat <file>` |
| `dump` | shows a file as hex bytes with offsets (like `xxd`) | `dump <file>` |
| `grep` | searches a pattern in all files of a directory (recursive) and highlights the matches in color | `grep <pattern> <directory>` |
| `dirsize` | calculates the total size of a directory | `dirsize <directory>` |
| `elf` | reads and prints the header of an ELF file (Linux executables) | `elf <file>` |
| `xor-cipher` | decrypts a file, opens it in Neovim and encrypts it again when you close Neovim | `xor-cipher <file> <key>` |
| `compress_exe_into_c` | compresses an executable with `xz` and puts it into a C file that unpacks and runs it | `compress_exe_into_c <src> <dest>` |
| `vector.h` | A very small macro to easily create vectors in C. Useful for keeping things simple but not very efficient. |

## Some details

- **grep** maps each file into memory with `mmap` and walks through the directory with `nftw`. Files that are not valid UTF-8 (for example binaries) are skipped.
- **elf** reads the ELF header byte by byte, without using `<elf.h>`. `elf/hello_world.c` is a small test program you can compile and inspect.
- **dirsize** also uses `nftw` and prints the size in B, KiB, MiB or GiB, plus how long it took.
- **xor-cipher**: XOR with a key is **not** real encryption
- **compress_exe_into_c** needs `strip`, `xz` and `clang` to be installed.

## Build

Every tool is a single C file. For example:

```sh
cc -Wall -Wextra -o grep_tool grep/main.c
./grep_tool main .
```
