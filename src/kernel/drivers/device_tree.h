#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

#include "../lib/common.h"

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

#define FDT_MAGIC 0xD00DFEED

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
fdt_node_t *find_fdt(char *target);
void free_node(fdt_node_t *node_ptr);

void print_node(fdt_node_t *node, u8 indent);
void print_fdt();
uptr get_node_addr(const char *name);

#endif
