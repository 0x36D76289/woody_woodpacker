# Woody Woodpacker

A simple ELF binary packer that encrypts 64-bit executable programs and creates a new executable that decrypts and runs the original program at runtime.

```bash
make
./woody_woodpacker <binary_file>
```

### Example

```bash
cc -m64 -o sample64 resources/sample.c

./sample64
./woody_woodpacker sample64

./woody
# Output: 
# ....WOODY....
# Hello, World!
```

## How It Works

1. **Parse**: The program parses the input ELF file and validates it's a 64-bit x86_64 executable
2. **Locate**: It finds the `.text` section containing the executable code
3. **Encrypt**: The `.text` section is encrypted using XOR with a randomly generated 16-byte key
4. **Generate**: A new C program is generated that contains:
   - The encrypted binary data embedded at the end
   - The decryption key and metadata
   - Code to decrypt and execute the original program
5. **Compile**: The generated C program is compiled to create the final "woody" executable

## Algorithm

The packer uses XOR encryption with a 16-byte randomly generated key. While simple, XOR provides sufficient obfuscation for the project requirements. The key is generated using the system's random number generator seeded with the current time.

## Limitations

- Only supports 64-bit ELF executables
- Only supports x86_64 architecture
- Uses XOR encryption (not cryptographically secure)
- Creates temporary files during packing process
- Packed binaries are larger than originals due to the unpacking stub

## Files Created

When packing a binary, the following files are created:
- `woody`: The final packed executable
- Temporary files (automatically cleaned up):
  - `woody_temp.c`: Generated C source
  - `woody_temp.bin`: Encrypted binary data
  - `woody_temp`: Intermediate compiled binary

## Testing

```bash
# Test with 64-bit binary (should work)
./woody_woodpacker sample64

# Test with 32-bit binary (should be rejected...)
./woody_woodpacker resources/sample

# Compare original and packed execution
./sample64        # Original
./woody           # Packed version
```

## Cleanup

```bash
make clean
make fclean
```