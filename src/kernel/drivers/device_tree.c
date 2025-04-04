#include "device_tree.h"
#include "../lib/common.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include "../memory/kheap.h"
#include "uart.h"

uptr fdt_addr = NULL;

void init_fdt(const uptr addr) { fdt_addr = addr; }

uint32_t swap_endianess(uint32_t val) {
  return ((val >> 24) & 0xff) | ((val >> 8) & 0xff00) |
         ((val << 8) & 0xff0000) | ((val << 24) & 0xff000000);
}

void align_pointer(const u8 **ptr) {
  // Align to 4-byte boundary
  while ((uintptr_t)*ptr % 4 != 0)
    (*ptr)++;
}

int go_to(const u8 **ptr, u32 target) {
  align_pointer(ptr);

  u32 token = swap_endianess(*(u32 *)*ptr);

  while (token != target) {
    if (token == FDT_END)
      return -1;
    if (token == FDT_END_NODE && target == FDT_PROP)
      return -2;
    *ptr += 4;
    token = swap_endianess(*(u32 *)*ptr);
  }

  return 0;
}

const char *get_name(const u8 **ptr) {
  align_pointer(ptr);

  // Continue to start of node
  if (go_to(ptr, FDT_BEGIN_NODE) < 0) {
    return NULL;
  }
  *ptr += 4;

  const char *name = (const char *)*ptr;
  *ptr += 4;
  align_pointer(ptr);
  return name;
}

fdt_property_t **read_node_properties(const u8 **ptr,
                                      const uint8_t *strings_block) {
  print("Reading node properties\n");
  align_pointer(ptr);

  // Information to use for resizing
  int prop_size = 1;
  int prop_filled = 0;
  fdt_property_t **props = kmalloc(prop_size);

  int idx = 0;

  while (1) {
    if (go_to(ptr, FDT_PROP) < 0)
      return props;

    if (prop_filled == prop_size) {
      // Double size and copy over
      fdt_property_t **new_props = kmalloc(prop_size * 2);
      for (int i = 0; i < prop_size; i++) {
        new_props[i] = props[i];
      }
      kfree(props);
      props = new_props;
      prop_size *= 2;
    }

    prop_filled++;

    *ptr += 4;

    props[idx] = (fdt_property_t *)kmalloc(sizeof(fdt_property_t));
    print_heap_contents();

    props[idx]->len = swap_endianess(*(u32 *)*ptr);
    *ptr += 4;

    uint32_t prop_name_off = swap_endianess(*(uint32_t *)(*ptr));
    *ptr += 4;

    // Get Property Name from Strings Block using offset
    props[idx]->name = (const char *)(strings_block + prop_name_off);
    props[idx]->value = (void *)*ptr;

    cprintf("Found property: %s(%d) = ", props[idx]->name, props[idx]->len);
    for (int i = 0; i < props[idx]->len; i++) {
      cprintf("%x ", ((uint8_t *)props[idx]->value)[i]);
    }
    print("\n");

    *ptr += props[idx]->len;
    align_pointer(ptr);

    idx++;
  }
}

fdt_node_t *find_fdt(char *target) {

  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  uint32_t magic = swap_endianess(hdr->magic);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return NULL;
  }
  uint32_t totalsize = swap_endianess(hdr->totalsize);
  uint32_t struct_offset = swap_endianess(hdr->off_dt_struct);
  uint32_t strings_offset = swap_endianess(hdr->off_dt_strings);
  uint32_t version = swap_endianess(hdr->version);

  const uint8_t *struct_block = (uint8_t *)fdt_addr + struct_offset;
  const uint8_t *strings_block = (uint8_t *)fdt_addr + strings_offset;

  const uint8_t *ptr = struct_block;
  const uint8_t *end = (const uint8_t *)(fdt_addr) + totalsize;

  cprintf("Looking for target node: %s\n", target);
  while (ptr < end) {
    const char *name = get_name(&ptr);
    if (name == NULL) {
      return NULL;
    }

    if (cstrcmp(target, (char *)name) != 0) {
      // Skip node
      go_to(&ptr, FDT_END_NODE);
      continue;
    }

    cprintf("Found node: %s\n", name);

    fdt_node_t *node = kmalloc(sizeof(fdt_node_t));
    node->name = name;

    node->properties = read_node_properties(&ptr, strings_block);

    return node;
  }

  return NULL;
}
