#include "uart.h"
#include "device_tree.h"
#include <stdint.h>

// Magic header
#define FDT_MAGIC       0xD00DFEED
// Nodes of tree
#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

void print_num(int num) {
    char buffer[12]; // Buffer to hold the number as a string, enough for a 32-bit integer
    int i = 0;
    
    // Handle the case where num is 0
    if (num == 0) {
        putchar('0');
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        putchar('-');
        num = -num; // Make num positive for further processing
    }

    // Convert the number to a string in reverse order
    while (num > 0) {
        buffer[i++] = (num % 10) + '0'; // Store the digit as a character
        num /= 10;
    }

    // Print the number in the correct order
    for (int j = i - 1; j >= 0; j--) {
        putchar(buffer[j]);
    }
}

void print_hex(uintptr_t ptr) {
    const char hex_digits[] = "0123456789ABCDEF";
    char buffer[16];  // Buffer for the hexadecimal representation

    // Print the "0x" prefix
    putchar('0');
    putchar('x');

    // Convert the pointer value to hexadecimal
    for (int j = 15; j >= 0; j--) {
        buffer[j] = hex_digits[ptr & 0xF];  // Get the least significant nibble
        ptr >>= 4;  // Shift the pointer to the next nibble
    }

    // Print the hexadecimal address, skipping leading zeros
    int leading_zero = 1;
    for (int j = 0; j < 16; j++) {
        if (buffer[j] != '0' || !leading_zero) {
            putchar(buffer[j]);
            leading_zero = 0;
        }
    }
}

uint32_t swap_endianess(uint32_t val) {
    return ((val >> 24) & 0xff) |
           ((val >> 8) & 0xff00) |
           ((val << 8) & 0xff0000) |
           ((val << 24) & 0xff000000);
}


void read_fdt(const uintptr_t fdt_addr) {

  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  uint32_t magic = swap_endianess(hdr->magic);
  uint32_t totalsize = swap_endianess(hdr->totalsize);
  uint32_t struct_offset = swap_endianess(hdr->off_dt_struct);
  uint32_t strings_offset = swap_endianess(hdr->off_dt_strings);

  if (magic != FDT_MAGIC) {
      print("Invalid FDT magic number!\n");
      return;
  }
    print("Valid Device tree found!\n");
    print("Total Size:");
    print_num(totalsize);
    print("\nStructure Block Offset:");
    print_num(struct_offset);
    print("\nStrings Block Offset: ");
    print_num(strings_offset);

    // Point to the Structure Block and Strings Block
    const uint8_t *struct_block = (uint8_t *)fdt_addr + struct_offset;
    const uint8_t *strings_block = (uint8_t *)fdt_addr + strings_offset;
    
    // Example parsing loop (simplified)
    const uint8_t *ptr = struct_block;
    const uint8_t *end = (const uint8_t *)(fdt_addr) + totalsize;
    while (ptr < end) {
        uint32_t token = swap_endianess(*(uint32_t *)ptr);
        ptr += 4;

        switch (token) {
            case FDT_BEGIN_NODE: {
                // Node Name (from Structure Block)
                const char *name = (const char *)ptr;
                print("\n -------------- Node: ");
                print(name);
                print(" -------------- \n");
                //ptr += strlen(name) + 1;
                for (int i = 0; name[i] != '\0'; ++i) {
                    ++ptr;
                }
                ++ptr;
                // Align to 4-byte boundary
                while ((uintptr_t)ptr % 4 != 0) ptr++;
                break;
            }
            case FDT_PROP: {
                uint32_t prop_len = swap_endianess(*(uint32_t *)ptr);
                uint32_t prop_name_off = swap_endianess(*(uint32_t *)(ptr + 4));
                ptr += 8;

                // Get Property Name from Strings Block using offset
                const char *prop_name = (const char *)(strings_block + prop_name_off);

                // Print Property Value
                print("\n - Property:");
                print(prop_name);
                print("\nLen:");
                print_num(prop_len);
                print(" = ");

                if (ptr[prop_len-1] == '\0') {
                    // Print property as string
                    print("str: ");
                    print((const char *)(ptr));
                } else {
                    // Print property as hex (binary data)
                    for (uint32_t i = 0; i < prop_len; i++) {
                        print_num((int)ptr[i]);
                    }
                }
                print("\n");


                ptr += prop_len;
                while ((uintptr_t)ptr % 4 != 0) ptr++;
                break;

            }
            case FDT_END_NODE:
                break;
            case FDT_END:
                return;
            default:
                break;
        }
    }
}