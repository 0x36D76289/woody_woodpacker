#include "woody.h"

int parse_elf(const char *filename, t_elf *elf)
{
    int fd;
    struct stat st;

    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        return -1;
    }

    elf->size = st.st_size;
    elf->data = mmap(NULL, elf->size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (elf->data == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    close(fd);

    // Basic ELF header validation before casting
    if (elf->size < sizeof(Elf64_Ehdr))
    {
        munmap(elf->data, elf->size);
        return -1;
    }

    // Check ELF magic first
    if (memcmp(elf->data, ELFMAG, SELFMAG) != 0)
    {
        munmap(elf->data, elf->size);
        return -1;
    }

    // Check if it's 64-bit before casting to Elf64_Ehdr
    unsigned char *ident = (unsigned char *)elf->data;
    if (ident[EI_CLASS] != ELFCLASS64)
    {
        munmap(elf->data, elf->size);
        return -1;
    }

    elf->ehdr = (Elf64_Ehdr *)elf->data;
    elf->phdr = (Elf64_Phdr *)((char *)elf->data + elf->ehdr->e_phoff);
    elf->shdr = (Elf64_Shdr *)((char *)elf->data + elf->ehdr->e_shoff);

    if (elf->ehdr->e_shstrndx != SHN_UNDEF)
    {
        elf->shstrtab = (char *)elf->data + elf->shdr[elf->ehdr->e_shstrndx].sh_offset;
    }
    else
    {
        elf->shstrtab = NULL;
    }

    return 0;
}

void free_elf(t_elf *elf)
{
    if (elf->data && elf->data != MAP_FAILED)
    {
        munmap(elf->data, elf->size);
    }
}

int validate_elf64(t_elf *elf)
{
    if (elf->size < sizeof(Elf64_Ehdr))
    {
        return -1;
    }

    // Check ELF magic
    if (memcmp(elf->ehdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        return -1;
    }

    // Check if it's 64-bit
    if (elf->ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        return -1;
    }

    // Check if it's x86_64
    if (elf->ehdr->e_machine != EM_X86_64)
    {
        return -1;
    }

    // Additional validation for ELF structure
    if (elf->ehdr->e_phoff == 0 || elf->ehdr->e_shoff == 0)
    {
        return -1;
    }

    if (elf->ehdr->e_phoff + (elf->ehdr->e_phnum * sizeof(Elf64_Phdr)) > elf->size)
    {
        return -1;
    }

    if (elf->ehdr->e_shoff + (elf->ehdr->e_shnum * sizeof(Elf64_Shdr)) > elf->size)
    {
        return -1;
    }

    return 0;
}

int find_text_section(t_elf *elf, size_t *offset, size_t *size)
{
    if (!elf->shstrtab)
    {
        return -1;
    }

    for (int i = 0; i < elf->ehdr->e_shnum; i++)
    {
        Elf64_Shdr *shdr = &elf->shdr[i];
        char *name = elf->shstrtab + shdr->sh_name;

        if (strcmp(name, ".text") == 0)
        {
            *offset = shdr->sh_offset;
            *size = shdr->sh_size;
            return 0;
        }
    }

    return -1;
}
