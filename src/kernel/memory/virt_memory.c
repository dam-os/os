#include "../lib/common.h"
#include "paging.h"

#define sv39 8

#define PAGE_VALID (1 << 0)
#define PAGE_READ (1 << 1)
#define PAGE_WRITE (1 << 2)
#define PAGE_EXECUTE (1 << 3)
#define PAGE_USER (1 << 4)
#define PAGE_GLOBAL (1 << 5)
#define PAGE_ACCESSED (1 << 6)
#define PAGE_DIRTY (1 << 7)

void map_virt_mem(u64 *table2, u64 vaddr, u64 paddr) {
  u32 vpn2 = (vaddr >> 30) & 0x1ff; // hex(0b111111111) '0x1ff'
  u32 vpn1 = (vaddr >> 21) & 0x1ff;
  u32 vpn0 = (vaddr >> 12) & 0x1ff;
  // u32 page_offset = vaddr & 0x1ff;

  if ((table2[vpn2] & PAGE_VALID) == 0) {
    u64 pt_paddr = (u64)alloc_pages(
        1); // 1 page table is 4kb, We map a vpn 2 to a full page table
    table2[vpn2] = ((pt_paddr / PAGE_SIZE) << 10) |
                   PAGE_VALID; // Page table entry has 10 reserved bits
  }

  u64 *table1 = (u64 *)((table2[vpn2] >> 10) * PAGE_SIZE);

  if ((table1[vpn1] & PAGE_VALID) == 0) {
    u64 pt_paddr = (u64)alloc_pages(1);
    table1[vpn1] = ((pt_paddr / PAGE_SIZE) << 10) | PAGE_VALID;
  }

  u64 *table0 = (u64 *)((table1[vpn1] >> 10) * PAGE_SIZE);

  table0[vpn0] =
      ((paddr / PAGE_SIZE) << 10) |
      (PAGE_VALID | PAGE_READ | PAGE_WRITE | PAGE_EXECUTE | PAGE_USER);
}

u64 translate_va_to_pa(u64 vaddr, u64 satp_val) {
  u64 *pt = (u64 *)((satp_val & 0xFFFFFFFFFFF) *
                    PAGE_SIZE); // Convert PPN to physical address

  u64 vpn2 = (vaddr >> 30) & 0x1ff;
  u64 vpn1 = (vaddr >> 21) & 0x1ff;
  u64 vpn0 = (vaddr >> 12) & 0x1ff;
  u64 page_offset = vaddr & 0xfff;

  // Walk level 2 page table
  u64 pte = (u64)pt[vpn2];
  if (!(pte & PAGE_VALID))
    return 0;                            // Page not mapped
  pt = (u64 *)((pte >> 10) * PAGE_SIZE); // Next level

  // Walk level 1 page table
  pte = pt[vpn1];
  if (!(pte & PAGE_VALID))
    return 0;
  pt = (u64 *)((pte >> 10) * PAGE_SIZE);

  // Walk level 0 page table
  pte = pt[vpn0];
  if (!(pte & PAGE_VALID))
    return 0;

  // Get physical address
  u64 pa = (pte >> 10) * PAGE_SIZE + page_offset;
  return pa;
}
