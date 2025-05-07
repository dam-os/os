#include "device_tree.h"
#include "../lib/assert.h"
#include "../lib/common.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include "../memory/kheap.h"
#include "../memory/memory.h"
#include "system.h"

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

  u32 magic = swap_endian_32(hdr->magic);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return NULL;
  }
  u32 struct_offset = swap_endian_32(hdr->off_dt_struct);
  u32 strings_offset = swap_endian_32(hdr->off_dt_strings);
  u32 version = swap_endian_32(hdr->version);

  kassert(version == 17);

  const u8 *struct_block = (u8 *)fdt_addr + struct_offset;
  const u8 *strings_block = (u8 *)fdt_addr + strings_offset;

  const u8 *ptr = struct_block;

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
  u32 result = 0;
  s8 curr_int;
  while (*curr != '\0') {
    curr_int = hex_char_to_int(*curr);
    if (curr_int == -1)
      return -1;
    result = (result << 4) | curr_int;
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

  u32 magic = swap_endian_32(hdr->magic);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return;
  }
  u32 struct_offset = swap_endian_32(hdr->off_dt_struct);
  u32 strings_offset = swap_endian_32(hdr->off_dt_strings);
  u32 version = swap_endian_32(hdr->version);

  kassert(version == 17);

  const u8 *struct_block = (u8 *)fdt_addr + struct_offset;
  const u8 *strings_block = (u8 *)fdt_addr + strings_offset;

  const u8 *ptr = struct_block;

  u32 tok = go_to_next_token(&ptr);
  kassert(tok == FDT_BEGIN_NODE);

  fdt_node_t *node = kmalloc(sizeof(fdt_node_t));
  get_node(&ptr, strings_block, node);
  print_node(node, 0);
  free_node(node);
}

/*
 * Iterates through all the props of the node at ptr, until it finds one whose
 * name matches prop_name. If no such prop is found, returns NULL. If prop_value
 * is not NULL, the value is also checked. If the value does not match, NULL is
 * returned. If everything matches, the value is returned.
 * value_type has the following values determines the data type of the
 * prop_value parameter. It is typed in the function header as a char * for
 * compatibility.
 */
void *check_node_props(const u8 *ptr, const u8 *strings_block, char *prop_name,
                       char *prop_value, u8 value_type, u8 *state) {
  u32 tok = token(ptr), prop_name_off, found_len;
  char *found_name;
  void *found_value;
  kassert(tok == FDT_BEGIN_NODE);
  ptr += 4;

  while (1) {
    tok = go_to_next_token(&ptr);

    switch (tok) {
    case FDT_PROP:
      found_len = swap_endian_32(*(u32 *)(ptr + 4));
      prop_name_off = swap_endian_32(*(u32 *)(ptr + 8));
      found_name = (char *)(strings_block + prop_name_off);
      if (cstrcmp(prop_name, found_name) != 0) {
        // Not the prop we are looking for, skip it
        skip_property(&ptr);
        break;
      }
      found_value = (void *)(ptr + 12);

      u8 negate = (value_type & PROP_CMP_NEG);
      u8 match_result;
      if (value_type & PROP_CMP_NIL) {
        // No value provided to check, so we just return this one
        *state = PROP_STATE_GOOD_MATCH;
        return found_value;
      } else if (value_type & PROP_CMP_STR) {
        // Compare string value
        match_result = cstrcmp(prop_value, found_value) == 0;
        if (negate ? !match_result : match_result) {
          *state = PROP_STATE_GOOD_MATCH;
          return found_value;
        }
      } else if (value_type & PROP_CMP_U64) {
        // Compare u64 value
        match_result = swap_endian_64(*(u64 *)found_value) == (u64)prop_value;
        if (negate ? !match_result : match_result) {
          *state = PROP_STATE_GOOD_MATCH;
          return found_value;
        }
      } else if (value_type & PROP_CMP_U32) {
        // Compare u32 value
        // TODO: value_type might be incorrectly set to u32 here if it's a u64
        // that only uses the lower 32 bits. This means we only compare the
        // lower 32 bits of the found value, the higher bits may be important!
        match_result =
            swap_endian_32(*(u32 *)found_value) == (u32)(u64)prop_value;
        if (negate ? !match_result : match_result) {
          *state = PROP_STATE_GOOD_MATCH;
          return found_value;
        }
      } else {
        PANIC("Unexpected prop value comparison type");
      }
      // No matched conditions returned, skip the prop and try the next one
      *state = PROP_STATE_BAD_MATCH;
      skip_property(&ptr);
      break;
    case FDT_NOP:
      break;
    case FDT_BEGIN_NODE:
    case FDT_END_NODE:
    case FDT_END:
      *state = PROP_STATE_NOT_FOUND;
      return NULL;
    default:
      PANIC("Unexpected token");
    }
  }
}

void *scan_node_path(char *path, const u8 **ptr, const u8 *strings_block) {
  // Path is in the form node_name.prop_name. prop_name is optional.
  char *node_name = path;
  char *search_prop_name = NULL;
  char *search_prop_value = NULL;
  u8 search_prop_type = PROP_CMP_NIL;
  char *prop_name = path;
  const char *current_node_name;
  u8 loop = TRUE;

  // Split the path into two separate variables, node_name and prop_name
  prop_name--; // don't skip first char
  while (loop) {
    prop_name++;
    switch (*prop_name) {
    case '[':
      // We have reached the start of a prop name lookup, set bracket to null
      // so node_name terminates, and set search_prop_name to after the
      // bracket.
      *prop_name = '\0';
      prop_name++;
      search_prop_name = prop_name;
      search_prop_type = search_prop_type & ~PROP_CMP_NIL;
      break;

    case '?':
      // We have reached start of a prop value lookup, but optional
      *prop_name = '\0';
      search_prop_type |= PROP_CMP_OPT;
      break;

    case '!':
      // We have reached the start of a prop value lookup, but negative
      *prop_name = '\0';
      search_prop_type |= PROP_CMP_NEG;
      break;

    case '=':
      // We have reached the start of a prop value lookup, set equals to null
      // so search_prop_name terminates, and set search_prop_value to after
      // the equals.
      *prop_name = '\0';
      search_prop_value = prop_name + 1;
      break;

    case '\'':
      // The quote means that the prop value lookup is a string. We need to
      // determine whether this is the start of the string, or the end of it
      if (*search_prop_value == '\'') {
        // The first character of the search value is ', so it is the start of
        // the string. Increment search_prop_value as it should not include the
        // quotes.
        search_prop_value++;
        search_prop_type |= PROP_CMP_STR;
      } else {
        // It is not the start of the string, set it to null so that
        // search_prop_value does not include the quote
        *prop_name = '\0';
        prop_name++; // Next char is expected to be ]
        kassert(*prop_name == ']');
      }
      break;
    case '<':
      // The open angle bracket means that the prop value lookup is a hex
      // string. To indicate this, we set prop_value_type to the smallest
      // integer value, u32.
      search_prop_value = 0;
      search_prop_type |= PROP_CMP_U32;
      break;
    case '>':
      // The open angle bracket means the hex string is now finished. The next
      // char is expected to be ]
      kassert(*(prop_name + 1) == ']');
      break;
    case '0' ... '9':
    case 'a' ... 'f':
    case 'A' ... 'F':
      // Only match hexadecimal digits if prop_value_type is of an integer type.
      // Else, we skip.
      if (!(search_prop_type & PROP_CMP_MASK_INT)) {
        // We're not supposed to match these chars
        break;
      }
      // Note that this part will never run if we were currently located in
      // node_name, search_prop_name or property_name, as prop_value_type does
      // not get set until after node_name/search_prop_name, and the loop ends
      // as soon as property_name is reached.

      // Change prop_value_type if it would become too big, convert to integer,
      // add to search_prop_value.
      // Need to do some casting shenanigans because search_prop_value is typed
      // as a char *, but we're using it as an u64/u32. In cases where it's used
      // as a u32, it first needs to be cast to a u64.
      if ((search_prop_type & PROP_CMP_U32) &&
          ((u32)(u64)search_prop_value) > U32_MAX / 16) {
        // Multiplying by 16 would put us above the u32 max, so update type to
        // u64. Need to remove the U32 bit and add the U64 bit
        search_prop_type = (search_prop_type & ~PROP_CMP_U32) | PROP_CMP_U64;
      }

      s8 val = hex_char_to_int(*prop_name);
      if (search_prop_type & PROP_CMP_U32) {
        u32 new_val = (((u32)(u64)search_prop_value) << 4) | val;
        search_prop_value = (void *)(u64)new_val;
      } else if (search_prop_type & PROP_CMP_U64) {
        u64 new_val = ((u64)search_prop_value) << 4 | val;
        search_prop_value = (void *)new_val;
      } else {
        PANIC("Invalid prop_value_type %b", search_prop_type);
      }
      break;

    case '*':
      // We have reached the separator, set separator to null so node_name
      // terminates, and set prop_name to after the separator. No need to
      // continue at this point, as prop_name is the final element in the
      // search string
      *prop_name = '\0';
      prop_name++;
      loop = FALSE;
      break;

    case '\0':
      // Null terminator encountered before seeing '*', so no prop_name was
      // specified
      prop_name = NULL;
      loop = FALSE;
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

      // Return node name if (1) no prop was requested and (2) no search prop
      // was specified
      if (prop_name == NULL && (search_prop_type == PROP_CMP_NIL)) {
        return (void *)current_node_name;
      }

      void *value;
      u8 state;
      // Node name matched, check if any props match
      // If a search prop is specified, need to verify that this node contains
      // the required prop first
      if (!(search_prop_type & PROP_CMP_NIL)) {
        // Note that we're passing a copy of ptr, as we don't want
        // check_node_props to modify it. Once it finds a match, we need to go
        // back to the start of the node to look through all its props again to
        // find the actual target prop, which we would have to do manually if
        // ptr was moved here.
        value = check_node_props(*ptr, strings_block, search_prop_name,
                                 search_prop_value, search_prop_type, &state);
        if ((state == PROP_STATE_NOT_FOUND &&
             !(search_prop_type & PROP_CMP_OPT)) ||
            (state == PROP_STATE_BAD_MATCH)) {
          // Either (1) no matching prop name was found, and optional flag was
          // not set or (2) a matching prop was found, but the value did not
          // match
          skip_node_not_nested(ptr);
          break;
        }
      }
      // Valid match was found. If no prop_name is specified, return node name.
      if (prop_name == NULL) {
        return (void *)current_node_name;
      }
      // Else, we look through the props for the actual target prop
      value = check_node_props(*ptr, strings_block, prop_name, NULL,
                               search_prop_type, &state);
      if (state == PROP_STATE_NOT_FOUND) {
        // No prop was found that matches the name, so continue to next node
        skip_node_not_nested(ptr);
        break;
      }
      return value;
    case FDT_PROP:
      // We should in theory never end up here, as all prop parsing should
      // happen in the FDT_BEGIN_NODE section
      skip_property(ptr);
      break;
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
 * Given a node path identifier, returns the address of the path target. A
 * path has the following format:
 * `node_identifier`*`prop_identifier`
 * A `node_identifier` is the start of a node name such as "cpus" or "clint@".
 * The `node_identifier` is the start of a prop name such as
 * "timebase-frequency". The star (*) and `prop_identifier` are optional. If
 * they are not specified, the full node name is returned. If they are
 * specified, the prop value is returned.
 * Examples:
 * "clint@" -> return the address of the name of the clint node, which is
 * "clint@<address>"
 * "cpus*timebase-frequency" -> return the address of the prop
 * value of the timebase-frequency prop in the cpus node.
 */
void *match_node(const char *path) {
  int len = cstrlen((char *)path);
  kassert(len < 256);

  char str[256]; // Max path length
  memcpy(path, str, len + 1);

  struct fdt_header *hdr = (struct fdt_header *)fdt_addr;

  u32 magic = swap_endian_32(hdr->magic);

  if (magic != FDT_MAGIC) {
    print("Invalid FDT magic number!\n");
    return NULL;
  }
  u32 struct_offset = swap_endian_32(hdr->off_dt_struct);
  u32 strings_offset = swap_endian_32(hdr->off_dt_strings);
  u32 version = swap_endian_32(hdr->version);

  kassert(version == 17);

  const u8 *struct_block = (u8 *)fdt_addr + struct_offset;
  const u8 *strings_block = (u8 *)fdt_addr + strings_offset;

  const u8 *ptr = struct_block;

  return scan_node_path(str, &ptr, strings_block);
}
