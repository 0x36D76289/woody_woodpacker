#include "woody.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    t_elf elf;
    t_encryption_info enc_info;

    // Parse the ELF file
    if (parse_elf(argv[1], &elf) != 0)
    {
        printf("File architecture not supported. x86_64 only\n");
        return 1;
    }

    // Validate it's a 64-bit ELF
    if (validate_elf64(&elf) != 0)
    {
        printf("File architecture not supported. x86_64 only\n");
        free_elf(&elf);
        return 1;
    }

    // Find the .text section
    if (find_text_section(&elf, &enc_info.text_offset, &enc_info.text_size) != 0)
    {
        printf("Error: Could not find .text section\n");
        free_elf(&elf);
        return 1;
    }

    // Generate encryption key
    generate_key(enc_info.key);

    enc_info.original_entry = elf.ehdr->e_entry;

    // Create the packed binary
    if (create_woody(&elf, "woody", &enc_info) != 0)
    {
        printf("Error: Could not create woody binary\n");
        free_elf(&elf);
        return 1;
    }

    // Print the encryption key
    printf("key_value: ");
    print_key(enc_info.key);
    printf("\n");

    free_elf(&elf);
    return 0;
}
