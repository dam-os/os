#include "device_tree.h"
#include "common.h"
#include "print.h"
#include "string.h"
#include "uart.h"

// Magic header
#define FDT_MAGIC 0xD00DFEED
// Nodes of tree
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

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

void print_char_hex(char c) {
  char hex_chars[] = "0123456789ABCDEF";

  putchar(hex_chars[(c >> 4) & 0xF]); // Print upper 4 bits
  putchar(hex_chars[c & 0xF]);        // Print lower 4 bits
}

uint32_t swap_endianess(uint32_t val) {
  return ((val >> 24) & 0xff) | ((val >> 8) & 0xff00) |
         ((val << 8) & 0xff0000) | ((val << 24) & 0xff000000);
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
  cprintf("Total Size: %d", totalsize);
  cprintf("\nStructure Block Offset: %d", struct_offset);
  cprintf("\nStrings Block Offset: %d", strings_offset);

  const uint8_t *struct_block = (uint8_t *)fdt_addr + struct_offset;
  const uint8_t *strings_block = (uint8_t *)fdt_addr + strings_offset;

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
      ptr += strlen((char *)name) + 1;
      ++ptr;
      // Align to 4-byte boundary
      while ((uintptr_t)ptr % 4 != 0)
        ptr++;
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
      cprintf("\nLen: %d", prop_len);
      print(" = ");

      if (prop_len > 0 && ptr[0] != '\0' && ptr[prop_len - 1] == '\0') {
        // Print property as string
        print("str: ");
        print((const char *)(ptr));
      } else {
        // Print property as hex (binary data)
        print("hex: ");
        for (uint32_t i = 0; i < prop_len; i++) {
          print_char_hex((char)ptr[i]);
        }
      }
      print("\n");

      ptr += prop_len;
      while ((uintptr_t)ptr % 4 != 0)
        ptr++;
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
