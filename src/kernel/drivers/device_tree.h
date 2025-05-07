#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

#include "../lib/common.h"

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

#define FDT_MAGIC 0xD00DFEED

#define PROP_CMP_NIL (0b00000001) /* No value provided */
#define PROP_CMP_STR (0b00000010) /* Value is a string */
#define PROP_CMP_U64 (0b00000100) /* Value is a u64 */
#define PROP_CMP_U32 (0b00001000) /* Value is a u32 */

/* Key is optional, match if it doesn't exist */
#define PROP_CMP_OPT (0b01000000)
#define PROP_CMP_NEG (0b10000000) /* Negate the match on the value */

#define PROP_CMP_MASK_INT (0b00001100) /* Mask for integer types */

#define PROP_STATE_NOT_FOUND (0x01) /* No prop name was found */
/* Prop name was found, but value did not match */
#define PROP_STATE_BAD_MATCH (0x02)
#define PROP_STATE_GOOD_MATCH (0x03) /* Prop name and value matched */

struct fdt_header {
  u32 magic;
  u32 totalsize;
  u32 off_dt_struct;
  u32 off_dt_strings;
  u32 off_mem_rsvmap;
  u32 version;
  u32 last_comp_version;
  u32 boot_cpuid_phys;
  u32 size_dt_strings;
  u32 size_dt_struct;
};

struct fdt_property {
  u32 len;
  const char *name;
  void *value;
};
typedef struct fdt_property fdt_property_t;

struct fdt_node {
  const char *name;
  size_t property_count;
  fdt_property_t *properties;
  size_t child_count;
  struct fdt_node *children;
};
typedef struct fdt_node fdt_node_t;

void init_fdt(const uptr addr);
fdt_node_t *find_node_by_phandle(u32 phandle);
void free_node(fdt_node_t *node_ptr);

void print_node(fdt_node_t *node, u8 indent);
void print_fdt(void);
uptr get_node_addr(const char *name);
void *match_node(const char *path);

#endif
