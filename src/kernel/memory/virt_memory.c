#import "../lib/common.h"
#import "../lib/print.h"
#import "paging.h"
#define sv39 8

#define PAGE_VALID (1 << 0)
#define PAGE_READ (1 << 1)
#define PAGE_WRITE (1 << 2)
#define PAGE_EXECUTE (1 << 3)
#define PAGE_USER (1 << 4)
#define PAGE_GLOBAL (1 << 5)
#define PAGE_ACCESSED (1 << 6)
#define PAGE_DIRTY (1 << 7)

int is_leaf(uint64) { return 1; }

void map_virt_mem(uint64 *table2, uint64 vaddr, uint64 paddr) {
  uint32 vpn2 = (vaddr >> 30) & 0x1ff; // hex(0b111111111) '0x1ff'
  uint32 vpn1 = (vaddr >> 21) & 0x1ff;
  uint32 vpn0 = (vaddr >> 12) & 0x1ff;
  uint32 page_offset = vaddr & 0x1ff;
  if ((table2[vpn2] & PAGE_VALID) == 0) {
    uint64 pt_paddr = (uint64)alloc_pages(
        1); // 1 page table is 4kb, We map a vpn 2 to a full page table
    cprintf("Allocing page at %p", pt_paddr);
    table2[vpn2] = ((pt_paddr / PAGE_SIZE) << 10) |
                   PAGE_VALID; // Page table entry has 10 reserved bits
  }
  uint64 *table1 = (uint64 *)((table2[vpn2] >> 10) * PAGE_SIZE);

  if ((table1[vpn1] & PAGE_VALID) == 0) {
    uint64 pt_paddr = (uint64)alloc_pages(1);
    table1[vpn1] = ((pt_paddr / PAGE_SIZE) << 10) | PAGE_VALID;
  }

  uint64 *table0 = (uint64 *)((table1[vpn1] >> 10) * PAGE_SIZE);

  table0[vpn0] =
      ((paddr / PAGE_SIZE) << 10) |
      (PAGE_VALID | PAGE_READ | PAGE_WRITE | PAGE_EXECUTE | PAGE_USER);
}
