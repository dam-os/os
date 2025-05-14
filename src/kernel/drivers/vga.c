#include "vga.h"
#include "../lib/io.h"
#include "../lib/timer.h"
#include "../memory/memory.h"
#include "pci.h"
#include "vga_const.h"

#define fb_base 0x50000000
#define DEFAULT_VALUES ((0 << 4) | (1 & 0x0F)) << 8

u8 *PORT_0300 = NULL;

// Mode 13 is inspired by
// https://github.com/neri/riscv-vga-sample

// VGA Registers
// http://www.osdever.net/FreeVGA/vga/crtcreg.htm#0F

void write_to_ports(const PortWrite *mode) {
  for (u8 i = 0; i < 28; i++) {
    u8 vport = mode[i].vport;
    u8 index = mode[i].index;
    u8 data = mode[i].data;
    u8 *port = PORT_0300 + vport;

    __attribute__((unused)) u32 _;

    switch (vport) {
    case 0xC0:
      _ = *(PORT_0300 + 0xDA); // Set C0 port back to index

      *port = index;
      *port = data;
      break;

    case 0xC2:
      *port = data;
      break;

    default:
      *port = index;
      *(port + 1) = data;
      break;
    }
  }
}

void set_colors(const u32 palette[]) {
  u8 *p = PORT_0300 + 0xc9;
  for (u32 i = 0; i < 256; i++) {
    u8 b = palette[i] & 0xFF;
    u8 g = (palette[i] >> 8) & 0xFF;
    u8 r = (palette[i] >> 16) & 0xFF;

    *p = r;
    *p = g;
    *p = b;
  }
}

// Mode 13h functions

void draw_pixel(u32 x, u32 y, u8 color) {
  volatile u8 *fb = (volatile u8 *)fb_base;
  u32 offset = (y * 320) + x;
  fb[offset] = color;
}

void clear_screen() {
  for (u32 i = 0; i < (320 * 200); i++) {
    ((volatile u8 *)fb_base)[i] = 0;
  }
}

void rainbow_animation() {
  u32 frame = 0;
  u32 backwards = 0;
  while (1) {
    for (u32 y = 0; y < 200; y++) {
      for (u32 x = 0; x < 320; x++) {
        u32 color_index = (x + frame) % 256;
        ((volatile u8 *)fb_base)[y * 320 + x] = color_index;
      }
    }
    if (backwards == -50) {
      backwards = 50;
    } else if (backwards > 0) {
      frame++;
    } else if (backwards <= 0) {
      frame--;
    }
    backwards--;
    sleep(10);
  }
}

void draw_compressed_image() {
  u8 *p = (u8 *)fb_base;
  memset((const char *)fb_base, 0, 320 * 200);
  u32 c = 0;
  u32 index = 0;
  u32 count = 0;
  for (u32 i = 0; i < 64000; i++) {
    if (count == 0) {
      count = compressed_image[index++];
      c = !c;
    }
    p[i] = c;
    count--;
  }
}

void draw_loading(u32 percentage) {
  u32 val = 50 + (((float)percentage) / 100) * 220;
  for (int x = 50; x < val; x++) {
    for (int y = 180; y < 200; y++) {
      draw_pixel(x, y, 0);
    }
  }
}

void init_mode13() {
  write_to_ports(MODE_13_REGS);
  // 0x3C9 since 0x3C0 is at 0x400
  set_colors(PALETTE_MODE13);
}

// Mode 3 / Text mode functions

void text_putfast(u8 x, u8 y, char ch) {
  volatile u16 *p = (volatile u16 *)fb_base;
  if (ch == NULL)
    ch = ' ';
  p[(80 * y + x) * 2] = (u16)ch | DEFAULT_VALUES;
}

void text_putchar(u8 x, u8 y, char ch, u8 fg_color) {
  volatile u16 *p = (volatile u16 *)fb_base;
  u8 bg_color = 0;
  u16 attrib = (bg_color << 4) | (fg_color & 0x0F);
  u32 offset = 80 * y + x;
  p[offset * 2] = (u16)ch | (attrib << 8);
}

void load_font() {
  volatile u16 *p = (volatile u16 *)fb_base;

  for (u32 x = 0; x < 256; x++) {  // For each char in font
    u32 offset = x * 16 * 4;       // 4 planes of 16 bytes each
    for (u32 i = 0; i < 16; i++) { // For each char 16 bytes of font data
      p[offset + (i * 2) + 1] = (real_font_data[(offset / 4) + i]);
    }
  }
}

// sets index 0x30f and 0x30e of port 0xD4 which has the lower, and upper bytes
// of the cursor pos
void set_cursor(u8 x, u8 y) {
  u32 offset = 80 * y + x;
  u8 lower = offset & 0xff;
  u8 upper = offset >> 8;
  __attribute__((unused)) u8 *port = PORT_0300 + 0xD4;

  PortWrite writes[2] = {{0xD4, 0x0f, lower}, {0xD4, 0x0e, upper}};
  write_to_ports(writes);
}

PortWrite TEXT_PALETTE_SELECT[28] = {
    {0xC0, 0x00, 0x0},
    {0xC0, 0x01, 0x1},
};

int verify_pci_device(u32 *devbase) {
  u32 pci_class = (*(devbase + 2)) >> 8;

  if (pci_class != 0x030000) {
    cprintf("[vga] VGA not found\r\n");
    return 0;
  } else {
    cprintf("[vga] VGA device found\r\n");
    return 1;
  }
}

u8 *setup_pci_bars(u32 *devbase) {
  // Write 0xFFFFFFFF and read size of bar0
  *(devbase + 4) = 0xFFFFFFFF;
  u32 fb_size = (~(*(devbase + 4)) | 0xF) + 1;
  u8 *io_base = (u8 *)fb_base + fb_size;

  // Write 0xFFFFFFFF and read size of bar2
  *(devbase + 6) = 0xFFFFFFFF;
  u32 io_size = (~(*(devbase + 6)) | 0xF) + 1;

  // Write where framebuffer is to bar 0 and where io is to bar 2
  *(devbase + 4) = (u32)(uptr)(fb_base) | 8;
  *(devbase + 6) = (u32)(uptr)(io_base) | 8;

  // Enable Memory Space in command register
  u32 cmd = *(devbase + 1);
  *(devbase + 1) = cmd | 0x0002;

  cprintf("[vga] VGA fb_base %x size %dM io_base %x size %d\n",
          (u32)(uptr)fb_base, (fb_size + 0x80000) >> 20, (u32)(uptr)io_base,
          io_size);

  return io_base;
}

void init_text_mode() {
  write_to_ports(TEXT_MODE_REGS);

  write_to_ports(TEXT_PALETTE_SELECT);

  __attribute__((unused)) u32 _ =
      *(PORT_0300 + 0xDA); // Set C0 port back to index
  *(PORT_0300 + 0xc0) = 0x30;

  set_colors(PALETTE_TEXT);
  load_font();
}

void init_virtio_vga() {
  if (PCI_CONFIG_BASE == NULL) {
    print("[vga] PCI not available, can't initialise VGA.\r\n");
    return;
  }

  u32 *devbase = pci_get_addr(0, 16, 0, 0x0);

  if (!verify_pci_device(devbase))
    return;

  u8 *io_base = setup_pci_bars(devbase);

  // 0xC0 is mapped directly to +0x400, so we subtract so the ports are visible,
  // aka we can add 0xC0 to write there.
  PORT_0300 = (io_base + (0x400 - 0xC0));

  init_mode13();
  draw_compressed_image();
  for (int x = 0; x <= 100; x++) {
    draw_loading(x);
    sleep(25);
  }

  init_text_mode();
}

void debug_print_virtio() {
  u8 bus = 0, device = 16, function = 0;

  u32 reg0 = pci_read_word(bus, device, function, 0x00);
  u32 reg1 = pci_read_word(bus, device, function, 0x04);
  u32 reg2 = pci_read_word(bus, device, function, 0x08);
  u32 reg3 = pci_read_word(bus, device, function, 0x0C); // New register

  u16 vendor_id = reg0 & 0xFFFF;
  u16 device_id = (reg0 >> 16) & 0xFFFF;
  u16 command = reg1 & 0xFFFF;
  u16 status = (reg1 >> 16) & 0xFFFF;
  u8 revision_id = reg2 & 0xFF;
  u8 prog_if = (reg2 >> 8) & 0xFF;
  u8 subclass = (reg2 >> 16) & 0xFF;
  u8 class_code = (reg2 >> 24) & 0xFF;

  u8 cache_line_size = reg3 & 0xFF;
  u8 latency_timer = (reg3 >> 8) & 0xFF;
  u8 header_type = (reg3 >> 16) & 0xFF;
  u8 bist = (reg3 >> 24) & 0xFF;

  cprintf("[vga] Device ID:        0x%x\r\n", device_id);
  cprintf("[vga] Vendor ID:        0x%x\r\n", vendor_id);
  cprintf("[vga] Command:          0x%x\r\n", command);
  cprintf("[vga] Status:           0x%b\r\n", status);
  cprintf("[vga] Class Code:       0x%x\r\n", class_code);
  cprintf("[vga] Subclass:         0x%x\r\n", subclass);
  cprintf("[vga] Programming IF:   0x%x\r\n", prog_if);
  cprintf("[vga] Revision ID:      0x%x\r\n", revision_id);
  cprintf("[vga] Cache Line Size:  0x%x (%d bytes)\r\n", cache_line_size,
          cache_line_size * 4);
  cprintf("[vga] Latency Timer:    0x%x\r\n", latency_timer);
  cprintf("[vga] Header Type:      0x%x\r\n", header_type);
  cprintf("[vga] BIST:             0x%x\r\n", bist);
}
