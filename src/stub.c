#include "woody.h"

char stub_start[] = {
    0x90, 0x90, 0x90, 0x90 // NOP instructions
};

char stub_end[] = {0x00};

size_t stub_size = sizeof(stub_start);
