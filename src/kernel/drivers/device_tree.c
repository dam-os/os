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

void skip_property(const u8 **ptr) {
  u32 tok = token(*ptr);
  kassert(tok == FDT_PROP);
  *ptr += 4; // Skip token
  u32 len = swap_endian_32(*(u32 *)*ptr);
  *ptr += 4;   // Skip len
  *ptr += 4;   // Skip name offset
  *ptr += len; // Skip value length
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
  *array_ptr_ptr = krealloc(*array_ptr_ptr, struct_size * current_size * 2);
  print_heap_contents();
  // void *new_array_ptr = kmalloc(struct_size * current_size * 2);
  // // Copy data from old arr to new arr
  // memcpy(*array_ptr_ptr, new_array_ptr, struct_size * current_size);
  //
  // // Overwrite old array with new one
  // kfree(*array_ptr_ptr);
  // *array_ptr_ptr = (void *)new_array_ptr;
  //
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

/**
 * Skips a node and all its properties, but does not skip nested nodes.
 */
void skip_node_not_nested(const u8 **ptr) {
  u32 tok = token(*ptr);
  kassert(tok == FDT_BEGIN_NODE);
  *ptr += 4;
  *ptr += cstrlen((char *)*ptr);
  align_pointer(ptr);
  while (1) {
    tok = go_to_next_token(ptr);
    switch (tok) {
    case FDT_END_NODE:
      return;
    case FDT_PROP:
      skip_property(ptr);
      break;
    case FDT_BEGIN_NODE:
      // Might be our target, let's return
      *ptr -= 4;
      return;
    }
  }
}

u32 get_phandle(fdt_node_t *node) {
  for (size_t i = 0; i < node->property_count; i++) {
    if (cstrcmp((char *)node->properties[i].name, "phandle") == 0) {
      return *(u32 *)node->properties[i].value;
    }
  }
  return -1;
}

fdt_node_t *find_node_by_phandle(u32 phandle) {
  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  uint32_t magic = swap_endian_32(hdr->magic);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return NULL;
  }
  uint32_t struct_offset = swap_endian_32(hdr->off_dt_struct);
  uint32_t strings_offset = swap_endian_32(hdr->off_dt_strings);
  uint32_t version = swap_endian_32(hdr->version);

  kassert(version == 17);

  const uint8_t *struct_block = (uint8_t *)fdt_addr + struct_offset;
  const uint8_t *strings_block = (uint8_t *)fdt_addr + strings_offset;

  const uint8_t *ptr = struct_block;

  // cprintf("Looking for target node: %s\n", target);

  u32 tok;
  while (1) {
    tok = go_to_next_token(&ptr);

    switch (tok) {
    case FDT_END:
      return NULL;
    case FDT_BEGIN_NODE: {
      fdt_node_t *node = kmalloc(sizeof(fdt_node_t));
      const u8 *ptr_copy = ptr;
      get_node(&ptr_copy, strings_block, node);
      if (get_phandle(node) == phandle)
        return node;
      else {
        skip_node_not_nested(&ptr);
        break;
      }
    }
    case FDT_PROP:
      skip_property(&ptr);
      break;
    case FDT_END_NODE:
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
    print(";\n");
    return;
  }
  print(" = ");

  if (((char *)val)[0] != '\0' && ((char *)val)[len - 1] == '\0') {
    // Print as string
    cprintf("\"%s\";", (const char *)property->value);
  } else {
    // Print as hex
    print("<0x");
    for (u32 i = 0; i < len; i++) {
      print_char_hex(((char *)val)[i]);
    }
    print(">;");
  }
  print("\n");
}

void print_node(fdt_node_t *node, u8 indent) {
  print_indent(indent);
  cprintf("%s {\n", node->name);

  unsigned int i;
  if (node->property_count > 0) {
    for (i = 0; i < node->property_count; i++) {
      print_property(&node->properties[i], indent + 4);
    }
  }
  if (node->child_count > 0) {
    print_indent(indent);
    for (i = 0; i < node->child_count; i++) {
      print_node(&node->children[i], indent + 4);
    }
  }

  print_indent(indent);
  cprintf("};\n\n");
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
 * Convert a node name to a memory location, assuming it has one
 */
uptr get_node_addr(const char *name) {
  const char *curr = name;

  // Move to @
  while (1) {
    if (*curr == '\0') {
      return -1;
    }
    if (*curr == '@') {
      curr++;
      break;
    }
    curr++;
  }

  // Read hex digits
  int result = 0;
  while (*curr != '\0') {
    if (*curr >= '0' && *curr <= '9') {
      result = (result << 4) | (*curr - '0');
    } else if (*curr >= 'a' && *curr <= 'f') {
      result = (result << 4) | (*curr - 'a' + 10);
    } else if (*curr >= 'A' && *curr <= 'F') {
      result = (result << 4) | (*curr - 'A' + 10);
    } else {
      return -1;
    }
    curr++;
  }
  return result;
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

void print_fdt(void) {
  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  uint32_t magic = swap_endian_32(hdr->magic);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return;
  }
  uint32_t struct_offset = swap_endian_32(hdr->off_dt_struct);
  uint32_t strings_offset = swap_endian_32(hdr->off_dt_strings);
  uint32_t version = swap_endian_32(hdr->version);

  kassert(version == 17);

  const uint8_t *struct_block = (uint8_t *)fdt_addr + struct_offset;
  const uint8_t *strings_block = (uint8_t *)fdt_addr + strings_offset;

  const uint8_t *ptr = struct_block;

  u32 tok = go_to_next_token(&ptr);
  kassert(tok == FDT_BEGIN_NODE);

  fdt_node_t *node = kmalloc(sizeof(fdt_node_t));
  get_node(&ptr, strings_block, node);
  print_node(node, 0);
  free_node(node);
}

void *scan_node_path(char *path, const u8 **ptr, const u8 *strings_block) {
  // Path is in the form node_name.prop_name. prop_name is optional.
  char *node_name = path;
  char *prop_name = path;
  const char *current_node_name;

  // Split the path into two separate variables, node_name and prop_name
  while (1) {
    prop_name++;
    if (*prop_name == '>') {
      // We have reached the separator, set separator to null so node_name
      // terminates, and set prop_name to after the separator.
      *prop_name = '\0';
      prop_name++;
      break;
    }
    if (*prop_name == '\0') {
      // Node prop name has been provided.
      prop_name = NULL;
      break;
    }
  }

  u32 tok;
  while (1) {
    tok = go_to_next_token(ptr);

    switch (tok) {
    case FDT_END:
      return NULL;
    case FDT_BEGIN_NODE:
      current_node_name = (const char *)(*ptr + 4);
      if (startswith(node_name, (char *)current_node_name) != 0) {
        // Skip node
        skip_node_not_nested(ptr);
        break;
      }

      // Return node name if no prop was requested
      if (prop_name == NULL) {
        return (void *)current_node_name;
      }

      // Node found, now we continue to search for its props
      break;
    case FDT_PROP:
      if (prop_name == NULL ||
          startswith(node_name, (char *)current_node_name) != 0) {
        // Skip prop
        skip_property(ptr);
        break;
      }
      *ptr += 4;
      u32 len = swap_endian_32(*(u32 *)*ptr);
      *ptr += 4;
      u32 prop_name_offset = swap_endian_32(*(u32 *)*ptr);
      *ptr += 4;

      // Skip prop if not the one we're looking for
      if (startswith(prop_name, (char *)(strings_block + prop_name_offset)) !=
          0) {
        *ptr += len;
        align_pointer(ptr);
        *ptr -= 4; // cause we add 4 at the end of the loop
        break;
      }

      // Grab prop
      return (void *)*ptr;

    case FDT_END_NODE:
    case FDT_NOP:
    default:
      break;
    }

    *ptr += 4;
  }

  return NULL;
}

/**
 * Given a node path identifier, returns the address of the path target. A path
 * has the following format:
 * `node_identifier`>`prop_identifier`
 * A `node_identifier` is the start of a node name such as "cpus" or "clint@".
 * The `node_identifier` is the start of a prop name such as
 * "timebase-frequency". The arrow (>) and `prop_identifier` are optional. If
 * they are not specified, the full node name is returned. If they are
 * specified, the prop value is returned.
 * Examples:
 * "clint@" -> return the address of the name of the clint node, which is
 * "clint@<address>"
 * "cpus>timebase-frequency" -> return the address of the prop
 * value of the timebase-frequency prop in the cpus node.
 */
void *match_node(const char *path) {
  int len = cstrlen((char *)path);
  kassert(len < 256);

  char str[256]; // Max path length
  memcpy(path, str, len + 1);

  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  uint32_t magic = swap_endian_32(hdr->magic);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return NULL;
  }
  uint32_t struct_offset = swap_endian_32(hdr->off_dt_struct);
  uint32_t strings_offset = swap_endian_32(hdr->off_dt_strings);
  uint32_t version = swap_endian_32(hdr->version);

  kassert(version == 17);

  const uint8_t *struct_block = (uint8_t *)fdt_addr + struct_offset;
  const uint8_t *strings_block = (uint8_t *)fdt_addr + strings_offset;

  const uint8_t *ptr = struct_block;

  return scan_node_path(str, &ptr, strings_block);
}
