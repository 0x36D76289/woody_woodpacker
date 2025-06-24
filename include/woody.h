#ifndef WOODY_H
#define WOODY_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>
#include <time.h>

#define WOODY_SIGNATURE "....WOODY...."
#define KEY_LENGTH 16

typedef struct
{
    void *data;
    size_t size;
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Shdr *shdr;
    char *shstrtab;
} t_elf;

typedef struct
{
    unsigned char key[KEY_LENGTH];
    size_t text_offset;
    size_t text_size;
    size_t original_entry;
} t_encryption_info;

int parse_elf(const char *filename, t_elf *elf);
void free_elf(t_elf *elf);
int validate_elf64(t_elf *elf);
int find_text_section(t_elf *elf, size_t *offset, size_t *size);
void generate_key(unsigned char *key);
void encrypt_data(void *data, size_t size, const unsigned char *key);
void decrypt_data(void *data, size_t size, const unsigned char *key);
int create_woody(t_elf *original, const char *output_name, t_encryption_info *enc_info);
int create_woody_wrapper(const char *binary_name, t_encryption_info *enc_info);
void print_key(const unsigned char *key);
extern char stub_start[];
extern char stub_end[];
extern size_t stub_size;

#endif
