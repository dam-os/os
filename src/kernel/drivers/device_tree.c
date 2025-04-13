#include "device_tree.h"
#include "../lib/assert.h"
#include "../lib/common.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include "../memory/kheap.h"
#include "../memory/memory.h"

uptr fdt_addr = NULL;

void init_fdt(const uptr addr) { fdt_addr = addr; }

/** Align to 4-byte boundary */
void align_pointer(const u8 **ptr) {
  // Add distance to next 4-byte boundary if not already aligned
  u8 dist = (uptr)*ptr % 4;
  *ptr += (dist != 0) * (4 - dist);
}

u32 token(const u8 *ptr) { return swap_endian_32(*(u32 *)ptr); }

/**
 * Advances ptr until the next byte that matches an FDT token.
 */
u32 go_to_next_token(const u8 **ptr) {
  while (1) {
    u32 tok = token(*ptr);
    switch (tok) {
    case FDT_BEGIN_NODE:
    case FDT_END_NODE:
    case FDT_PROP:
    case FDT_NOP:
    case FDT_END:
      return tok;
    }

    *ptr += 4;
  };
}

void get_property(const u8 **ptr, const u8 *strings_block,
                  fdt_property_t *out) {
  u32 tok = token(*ptr);
  kassert(tok == FDT_PROP);
  *ptr += 4;

  out->len = swap_endian_32(*(u32 *)*ptr);
  *ptr += 4;

  u32 prop_name_off = swap_endian_32(*(u32 *)*ptr);
  out->name = (const char *)(strings_block + prop_name_off);
  *ptr += 4;

  out->value = (void *)*ptr;
  *ptr += out->len;
  align_pointer(ptr);
}

/**
 * Given an array and its current size, either sets its size to 1 if the size is
 * 0, or doubles the size. Returns new size.
 */
u8 double_or_init(void **array_ptr_ptr, u8 current_size, size_t struct_size) {
  // No elements, init with size 1
  if (current_size == 0) {
    *array_ptr_ptr = kmalloc(struct_size);
    return 1;
  }

  // Create new array twice the size
  void *new_array_ptr = kmalloc(struct_size * current_size * 2);
  // Copy data from old arr to new arr
  memcpy(*array_ptr_ptr, new_array_ptr, struct_size * current_size);

  // Overwrite old array with new one
  kfree(*array_ptr_ptr);
  *array_ptr_ptr = (void *)new_array_ptr;

  // Return new size
  return current_size * 2;
}

/**
 * Given a pointer to a node, and the device tree strings block, returns an
 * allocated struct for that node, containing properties and sub-nodes. NOTE!:
 * Remember to free the node with free_node when done!
 */
void get_node(const u8 **ptr, const u8 *strings_block, fdt_node_t *out) {
  kassert(token(*ptr) == FDT_BEGIN_NODE);
  *ptr += 4;

  out->property_count = 0;
  u8 properties_size = 0;
  out->child_count = 0;
  u8 children_size = 0;
  out->name = (const char *)*ptr;
  *ptr += cstrlen((char *)out->name);

  align_pointer(ptr);

  // ptr may now point to any token except FDT_END

  u32 tok;
  while (1) {
    tok = go_to_next_token(ptr);

    switch (tok) {
    case FDT_BEGIN_NODE:
      // Make space if we are out
      if (out->child_count == children_size)
        children_size = double_or_init((void **)&out->children, children_size,
                                       sizeof(fdt_node_t));

      // Insert new node into furthest position in child arr
      get_node(ptr, strings_block, &out->children[out->child_count]);
      out->child_count++;
      break;
    case FDT_END_NODE:
      *ptr += 4;
      return;
    case FDT_PROP:
      // Make space if we are out
      if (out->property_count == properties_size)
        properties_size = double_or_init(
            (void **)&out->properties, properties_size, sizeof(fdt_property_t));

      // Insert new prop into furthest position in prop arr
      get_property(ptr, strings_block, &out->properties[out->property_count]);
      out->property_count++;
      break;
    case FDT_NOP:
      *ptr += 4;
      break;
    }
  }
};

fdt_node_t *find_fdt(char *target) {

  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  uint32_t magic = swap_endian_32(hdr->magic);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return NULL;
  }
  uint32_t totalsize = swap_endian_32(hdr->totalsize);
  uint32_t struct_offset = swap_endian_32(hdr->off_dt_struct);
  uint32_t strings_offset = swap_endian_32(hdr->off_dt_strings);
  uint32_t version = swap_endian_32(hdr->version);

  const uint8_t *struct_block = (uint8_t *)fdt_addr + struct_offset;
  const uint8_t *strings_block = (uint8_t *)fdt_addr + strings_offset;

  const uint8_t *ptr = struct_block;
  const uint8_t *end = (const uint8_t *)(fdt_addr) + totalsize;

  // cprintf("Looking for target node: %s\n", target);

  u32 tok;
  while (1) {
    tok = go_to_next_token(&ptr);

    switch (tok) {
    case FDT_END:
      return NULL;
    case FDT_BEGIN_NODE: {
      // Get node name and check if it matches
      const char *name = (const char *)(ptr + 4);
      if (cstrcmp(target, (char *)name) != 0) {
        // Skip node
        break;
      }

      fdt_node_t *node = kmalloc(sizeof(fdt_node_t));
      get_node(&ptr, strings_block, node);
      return node;
    }
    case FDT_END_NODE:
    case FDT_PROP:
    default:
    case FDT_NOP:
      break;
    }

    ptr += 4;
  }
  return NULL;
}

void print_indent(u8 indent) {
  for (u8 i = 0; i < indent; i++) {
    print(" ");
  }
}

void print_property(fdt_property_t *property, u8 indent) {
  void *val = property->value;
  u32 len = property->len;

  print_indent(indent);
  print(property->name);

  if (len == 0) {
    print("\n");
    return;
  }
  print(": ");

  if (((char *)val)[0] != '\0' && ((char *)val)[len - 1] == '\0') {
    // Print as string
    print((const char *)property->value);
  } else {
    // Print as hex
    for (u32 i = 0; i < len; i++) {
      print_char_hex(((char *)val)[i]);
    }
  }
  print("\n");
}

void print_node(fdt_node_t *node, u8 indent) {
  print_indent(indent);
  cprintf("Node: %s\n", node->name);

  int i;
  if (node->property_count > 0) {

    print_indent(indent);
    cprintf("Properties:\n");

    for (i = 0; i < node->property_count; i++) {
      print_property(&node->properties[i], indent + 2);
    }
  }
  if (node->child_count > 0) {
    print_indent(indent);
    cprintf("Children:\n");
    for (i = 0; i < node->child_count; i++) {
      print_node(&node->children[i], indent + 2);
    }
  }
  print("\n");
}

/**
 * Frees a nodes properties and children from memory. Does not free the node
 * itself.
 */
void free_node_metadata(fdt_node_t *node_ptr) {
  // Free properties list
  if (node_ptr->property_count > 0) {
    kfree(node_ptr->properties);
  }

  // Free each child
  if (node_ptr->child_count > 0) {
    for (size_t i = 0; i < node_ptr->child_count; i++) {
      free_node_metadata(&node_ptr->children[i]);
    }
    // Free child list
    kfree(node_ptr->children);
  }
}

/**
 * Frees a node, its properties and its children from memory.
 */
void free_node(fdt_node_t *node_ptr) {
  // Free props and children
  free_node_metadata(node_ptr);

  // Free top node
  kfree(node_ptr);
}

// LEGACY - kinda used for discovery of new nodes? all the new code relies on
// you already knowing the target node name
void print_fdt() {
  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  uint32_t magic = swap_endian_32(hdr->magic);
  uint32_t totalsize = swap_endian_32(hdr->totalsize);
  uint32_t struct_offset = swap_endian_32(hdr->off_dt_struct);
  uint32_t strings_offset = swap_endian_32(hdr->off_dt_strings);

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
    uint32_t token = swap_endian_32(*(uint32_t *)ptr);
    ptr += 4;

    switch (token) {
    case FDT_BEGIN_NODE: {
      // Node Name (from Structure Block)
      const char *name = (const char *)ptr;
      print("\n -------------- Node: ");
      print(name);
      print(" -------------- \n");
      ptr += cstrlen((char *)name) + 1;
      ++ptr;
      // Align to 4-byte boundary
      while ((uintptr_t)ptr % 4 != 0)
        ptr++;
      break;
    }
    case FDT_PROP: {
      uint32_t prop_len = swap_endian_32(*(uint32_t *)ptr);
      uint32_t prop_name_off = swap_endian_32(*(uint32_t *)(ptr + 4));
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
