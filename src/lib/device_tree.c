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
  uint32 magic;
  uint32 totalsize;
  uint32 off_dt_struct;
  uint32 off_dt_strings;
  uint32 off_mem_rsvmap;
  uint32 version;
  uint32 last_comp_version;
  uint32 boot_cpuid_phys;
  uint32 size_dt_strings;
  uint32 size_dt_struct;
};

void print_char_hex(char c) {
  char hex_chars[] = "0123456789ABCDEF";

  putchar(hex_chars[(c >> 4) & 0xF]); // Print upper 4 bits
  putchar(hex_chars[c & 0xF]);        // Print lower 4 bits
}

uint32 swap_endianess(uint32 val) {
  return ((val >> 24) & 0xff) | ((val >> 8) & 0xff00) |
         ((val << 8) & 0xff0000) | ((val << 24) & 0xff000000);
}

void read_fdt(const uintptr fdt_addr) {

  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  uint32 magic = swap_endianess(hdr->magic);
  uint32 totalsize = swap_endianess(hdr->totalsize);
  uint32 struct_offset = swap_endianess(hdr->off_dt_struct);
  uint32 strings_offset = swap_endianess(hdr->off_dt_strings);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return;
  }
  print("Valid Device tree found!\n");
  cprintf("Total Size: %d", totalsize);
  cprintf("\nStructure Block Offset: %d", struct_offset);
  cprintf("\nStrings Block Offset: %d", strings_offset);

  const uint8 *struct_block = (uint8 *)fdt_addr + struct_offset;
  const uint8 *strings_block = (uint8 *)fdt_addr + strings_offset;

  const uint8 *ptr = struct_block;
  const uint8 *end = (const uint8 *)(fdt_addr) + totalsize;
  while (ptr < end) {
    uint32 token = swap_endianess(*(uint32 *)ptr);
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
      while ((uintptr)ptr % 4 != 0)
        ptr++;
      break;
    }
    case FDT_PROP: {
      uint32 prop_len = swap_endianess(*(uint32 *)ptr);
      uint32 prop_name_off = swap_endianess(*(uint32 *)(ptr + 4));
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
        for (uint32 i = 0; i < prop_len; i++) {
          print_char_hex((char)ptr[i]);
        }
      }
      print("\n");

      ptr += prop_len;
      while ((uintptr)ptr % 4 != 0)
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
