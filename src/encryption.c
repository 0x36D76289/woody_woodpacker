#include "woody.h"

void generate_key(unsigned char *key)
{
    srand(time(NULL));
    for (int i = 0; i < KEY_LENGTH; i++)
    {
        key[i] = rand() % 256;
    }
}

void encrypt_data(void *data, size_t size, const unsigned char *key)
{
    unsigned char *bytes = (unsigned char *)data;
    for (size_t i = 0; i < size; i++)
    {
        bytes[i] ^= key[i % KEY_LENGTH];
    }
}

void decrypt_data(void *data, size_t size, const unsigned char *key)
{
    // XOR encryption is symmetric
    encrypt_data(data, size, key);
}

void print_key(const unsigned char *key)
{
    for (int i = 0; i < KEY_LENGTH; i++)
    {
        printf("%02X", key[i]);
    }
}
