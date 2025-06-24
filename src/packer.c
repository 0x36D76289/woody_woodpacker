#include "woody.h"

static const char woody_template[] =
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include <unistd.h>\n"
    "#include <sys/stat.h>\n"
    "#include <fcntl.h>\n"
    "#include <string.h>\n"
    "\n"
    "unsigned char decryption_key[] = {%s};\n"
    "size_t text_offset = %zu;\n"
    "size_t text_size = %zu;\n"
    "\n"
    "void decrypt_and_execute() {\n"
    "    printf(\"....WOODY....\\n\");\n"
    "    fflush(stdout);\n"
    "    \n"
    "    // Read encrypted binary from embedded data\n"
    "    FILE *self = fopen(\"/proc/self/exe\", \"rb\");\n"
    "    if (!self) { perror(\"fopen self\"); exit(1); }\n"
    "    \n"
    "    // Seek to the embedded binary data (after the C program)\n"
    "    fseek(self, -%zu, SEEK_END);\n"
    "    \n"
    "    unsigned char *binary_data = malloc(%zu);\n"
    "    if (!binary_data) { perror(\"malloc\"); exit(1); }\n"
    "    \n"
    "    if (fread(binary_data, 1, %zu, self) != %zu) {\n"
    "        perror(\"fread\"); exit(1);\n"
    "    }\n"
    "    fclose(self);\n"
    "    \n"
    "    // Decrypt the text section\n"
    "    for (size_t i = 0; i < text_size; i++) {\n"
    "        binary_data[text_offset + i] ^= decryption_key[i %% 16];\n"
    "    }\n"
    "    \n"
    "    // Write decrypted binary to temporary file\n"
    "    char temp_path[] = \"/tmp/woody_temp_XXXXXX\";\n"
    "    int fd = mkstemp(temp_path);\n"
    "    if (fd == -1) { perror(\"mkstemp\"); exit(1); }\n"
    "    \n"
    "    if (write(fd, binary_data, %zu) != (ssize_t)%zu) {\n"
    "        perror(\"write\"); close(fd); unlink(temp_path); exit(1);\n"
    "    }\n"
    "    \n"
    "    close(fd);\n"
    "    chmod(temp_path, 0755);\n"
    "    \n"
    "    // Execute the decrypted binary\n"
    "    execl(temp_path, temp_path, (char *)NULL);\n"
    "    perror(\"exec\"); unlink(temp_path); exit(1);\n"
    "}\n"
    "\n"
    "int main() {\n"
    "    decrypt_and_execute();\n"
    "    return 0;\n"
    "}\n";

char *key_to_c_array(unsigned char *key)
{
    char *result = malloc(KEY_LENGTH * 6 + 20);
    if (!result)
        return NULL;

    char *ptr = result;
    ptr += sprintf(ptr, "    ");

    for (int i = 0; i < KEY_LENGTH; i++)
    {
        if (i > 0)
            ptr += sprintf(ptr, ", ");
        ptr += sprintf(ptr, "0x%02x", key[i]);
    }

    return result;
}

int create_woody(t_elf *original, const char *output_name, t_encryption_info *enc_info)
{
    void *encrypted_binary;
    char *key_array;
    char *woody_source;
    FILE *source_file, *encrypted_file, *woody_file;
    int ret = 0;

    encrypted_binary = malloc(original->size);
    if (!encrypted_binary)
    {
        perror("malloc");
        return -1;
    }

    memcpy(encrypted_binary, original->data, original->size);

    void *text_data = (char *)encrypted_binary + enc_info->text_offset;
    encrypt_data(text_data, enc_info->text_size, enc_info->key);

    key_array = key_to_c_array(enc_info->key);
    if (!key_array)
    {
        free(encrypted_binary);
        return -1;
    }

    size_t source_size = strlen(woody_template) + strlen(key_array) + 500;
    woody_source = malloc(source_size);
    if (!woody_source)
    {
        free(encrypted_binary);
        free(key_array);
        return -1;
    }

    snprintf(woody_source, source_size, woody_template,
             key_array, enc_info->text_offset, enc_info->text_size,
             original->size, original->size, original->size, original->size,
             original->size, original->size);

    source_file = fopen("woody_temp.c", "w");
    if (!source_file)
    {
        perror("fopen");
        ret = -1;
        goto cleanup;
    }

    if (fputs(woody_source, source_file) == EOF)
    {
        perror("fputs");
        ret = -1;
        fclose(source_file);
        goto cleanup;
    }

    fclose(source_file);

    encrypted_file = fopen("woody_temp.bin", "wb");
    if (!encrypted_file)
    {
        perror("fopen encrypted");
        ret = -1;
        goto cleanup;
    }

    if (fwrite(encrypted_binary, 1, original->size, encrypted_file) != original->size)
    {
        perror("fwrite encrypted");
        ret = -1;
        fclose(encrypted_file);
        goto cleanup;
    }

    fclose(encrypted_file);

    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd), "gcc -o woody_temp woody_temp.c");

    if (system(compile_cmd) != 0)
    {
        printf("Error: Failed to compile woody binary\n");
        ret = -1;
        goto cleanup;
    }

    woody_file = fopen(output_name, "wb");
    if (!woody_file)
    {
        perror("fopen woody output");
        ret = -1;
        goto cleanup;
    }

    FILE *temp_woody = fopen("woody_temp", "rb");
    if (!temp_woody)
    {
        perror("fopen temp woody");
        ret = -1;
        fclose(woody_file);
        goto cleanup;
    }

    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), temp_woody)) > 0)
    {
        if (fwrite(buffer, 1, bytes_read, woody_file) != bytes_read)
        {
            perror("fwrite woody");
            ret = -1;
            break;
        }
    }

    fclose(temp_woody);

    if (ret == 0)
    {
        if (fwrite(encrypted_binary, 1, original->size, woody_file) != original->size)
        {
            perror("fwrite encrypted data");
            ret = -1;
        }
    }

    fclose(woody_file);

    if (ret == 0)
        chmod(output_name, 0755);

    unlink("woody_temp.c");
    unlink("woody_temp.bin");
    unlink("woody_temp");

cleanup:
    free(encrypted_binary);
    free(key_array);
    free(woody_source);

    return ret;
}
