#include "../lib/print.h"
#include "../lib/timer.h"
#include "../memory/memory.h"
#include "pci.h"

// Mostly port of
// https://github.com/neri/riscv-vga-sample

void draw_pixel(uintptr_t fb_base, int x, int y, uint8_t color) {
  volatile uint8_t *fb = (volatile uint8_t *)fb_base;
  int offset = (y * 320) + x;
  fb[offset] = color; // Write color value
}

void clear_screen(uintptr_t fb_base) {
  for (int i = 0; i < (320 * 200); i++) {
    ((volatile uint8_t *)fb_base)[i] = 0;
  }
}
void set_screen_a(uintptr_t fb_base) {
  uint8_t backcolour = 0;  // Black background (0 = black)
  uint8_t forecolour = 15; // White text (15 = white)
  uint16_t attrib = (backcolour << 4) | (forecolour & 0x0F);
  for (int i = 0; i < (80 * 25); i++) {
    ((volatile uint8_t *)fb_base)[i] = 'A' | (attrib << 8);
  }
}

void rainbow_animation(uintptr_t fb_base) {
  int frame = 0;
  int backwards = 0;
  while (1) {
    for (int y = 0; y < 200; y++) {
      for (int x = 0; x < 320; x++) {
        int color_index = (x + frame) % 256;
        ((volatile uint8_t *)fb_base)[y * 320 + x] = color_index;
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

// time.h Structure to hold the (vport, index, data) values
typedef struct {
  uint8_t vport;
  uint8_t index;
  uint8_t data;
} RegisterMode;

// Array of register values for setting mode 13h
RegisterMode MODE_13_REGS[28] = {
    {0xC2, 0x00, 0x63}, {0xD4, 0x11, 0x0E}, {0xD4, 0x00, 0x5F},
    {0xD4, 0x01, 0x4F}, {0xD4, 0x02, 0x50}, {0xD4, 0x03, 0x82},
    {0xD4, 0x04, 0x54}, {0xD4, 0x05, 0x80}, {0xD4, 0x06, 0xBF},
    {0xD4, 0x07, 0x1F}, {0xD4, 0x08, 0x00}, {0xD4, 0x09, 0x41},
    {0xD4, 0x10, 0x9C}, {0xD4, 0x11, 0x8E}, {0xD4, 0x12, 0x8F},
    {0xD4, 0x13, 0x28}, {0xD4, 0x14, 0x40}, {0xD4, 0x15, 0x96},
    {0xD4, 0x16, 0xB9}, {0xD4, 0x17, 0xA3}, {0xC4, 0x01, 0x01},
    {0xC4, 0x02, 0x0F}, {0xC4, 0x04, 0x0E}, {0xCE, 0x00, 0x00},
    {0xCE, 0x05, 0x40}, {0xCE, 0x06, 0x05}, {0xC0, 0x30, 0x41},
    {0xC0, 0x33, 0x00}};

RegisterMode TEXT_MODE_REGS[28] = {
    {0xC2, 0x00, 0x67},      {0xD4, 0x11, 0x0E}, {0xD4, 0x00, 0x5F},
    {0xD4, 0x01, 0x4F},      {0xD4, 0x02, 0x50}, {0xD4, 0x03, 0x82},
    {0xD4, 0x04, 0x55},      {0xD4, 0x05, 0x81}, {0xD4, 0x06, 0xBF},
    {0xD4, 0x07, 0x1F},      {0xD4, 0x08, 0x00}, {0xD4, 0x09, 0x4F},
    {0xD4, 0x10, 0x9C},      {0xD4, 0x11, 0x8E}, {0xD4, 0x12, 0x8F},
    {0xD4, 0x13, 0x28},      {0xD4, 0x14, 0x1F}, {0xD4, 0x15, 0x96},
    {0xD4, 0x16, 0xB9},      {0xD4, 0x17, 0xA3}, {0xC4, 0x01, 0x00},
    /**/ {0xC4, 0x02, 0x0F}, {0xC4, 0x04, 0x07}, /**/ {0xCE, 0x00, 0x00},
    {0xCE, 0x05, 0x10},      {0xCE, 0x06, 0x0E}, /**/ {0xC0, 0x30, 0x41},
    /**/ {0xC0, 0x33, 0x00}};

const uint32_t PALETTE[256] = {
    0xffffff, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080,
    0xc0c0c0, 0xc0dcc0, 0xa6caf0, 0x2a3faa, 0x2a3fff, 0x2a5f00, 0x2a5f55,
    0x2a5faa, 0x2a5fff, 0x2a7f00, 0x2a7f55, 0x2a7faa, 0x2a7fff, 0x2a9f00,
    0x2a9f55, 0x2a9faa, 0x2a9fff, 0x2abf00, 0x2abf55, 0x2abfaa, 0x2abfff,
    0x2adf00, 0x2adf55, 0x2adfaa, 0x2adfff, 0x2aff00, 0x2aff55, 0x2affaa,
    0x2affff, 0x550000, 0x550055, 0x5500aa, 0x5500ff, 0x551f00, 0x551f55,
    0x551faa, 0x551fff, 0x553f00, 0x553f55, 0x553faa, 0x553fff, 0x555f00,
    0x555f55, 0x555faa, 0x555fff, 0x557f00, 0x557f55, 0x557faa, 0x557fff,
    0x559f00, 0x559f55, 0x559faa, 0x559fff, 0x55bf00, 0x55bf55, 0x55bfaa,
    0x55bfff, 0x55df00, 0x55df55, 0x55dfaa, 0x55dfff, 0x55ff00, 0x55ff55,
    0x55ffaa, 0x55ffff, 0x7f0000, 0x7f0055, 0x7f00aa, 0x7f00ff, 0x7f1f00,
    0x7f1f55, 0x7f1faa, 0x7f1fff, 0x7f3f00, 0x7f3f55, 0x7f3faa, 0x7f3fff,
    0x7f5f00, 0x7f5f55, 0x7f5faa, 0x7f5fff, 0x7f7f00, 0x7f7f55, 0x7f7faa,
    0x7f7fff, 0x7f9f00, 0x7f9f55, 0x7f9faa, 0x7f9fff, 0x7fbf00, 0x7fbf55,
    0x7fbfaa, 0x7fbfff, 0x7fdf00, 0x7fdf55, 0x7fdfaa, 0x7fdfff, 0x7fff00,
    0x7fff55, 0x7fffaa, 0x7fffff, 0xaa0000, 0xaa0055, 0xaa00aa, 0xaa00ff,
    0xaa1f00, 0xaa1f55, 0xaa1faa, 0xaa1fff, 0xaa3f00, 0xaa3f55, 0xaa3faa,
    0xaa3fff, 0xaa5f00, 0xaa5f55, 0xaa5faa, 0xaa5fff, 0xaa7f00, 0xaa7f55,
    0xaa7faa, 0xaa7fff, 0xaa9f00, 0xaa9f55, 0xaa9faa, 0xaa9fff, 0xaabf00,
    0xaabf55, 0xaabfaa, 0xaabfff, 0xaadf00, 0xaadf55, 0xaadfaa, 0xaadfff,
    0xaaff00, 0xaaff55, 0xaaffaa, 0xaaffff, 0xd40000, 0xd40055, 0xd400aa,
    0xd400ff, 0xd41f00, 0xd41f55, 0xd41faa, 0xd41fff, 0xd43f00, 0xd43f55,
    0xd43faa, 0xd43fff, 0xd45f00, 0xd45f55, 0xd45faa, 0xd45fff, 0xd47f00,
    0xd47f55, 0xd47faa, 0xd47fff, 0xd49f00, 0xd49f55, 0xd49faa, 0xd49fff,
    0xd4bf00, 0xd4bf55, 0xd4bfaa, 0xd4bfff, 0xd4df00, 0xd4df55, 0xd4dfaa,
    0xd4dfff, 0xd4ff00, 0xd4ff55, 0xd4ffaa, 0xd4ffff, 0xff0055, 0xff00aa,
    0xff1f00, 0xff1f55, 0xff1faa, 0xff1fff, 0xff3f00, 0xff3f55, 0xff3faa,
    0xff3fff, 0xff5f00, 0xff5f55, 0xff5faa, 0xff5fff, 0xff7f00, 0xff7f55,
    0xff7faa, 0xff7fff, 0xff9f00, 0xff9f55, 0xff9faa, 0xff9fff, 0xffbf00,
    0xffbf55, 0xffbfaa, 0xffbfff, 0xffdf00, 0xffdf55, 0xffdfaa, 0xffdfff,
    0xffff55, 0xffffaa, 0xccccff, 0xffccff, 0x33ffff, 0x66ffff, 0x99ffff,
    0xccffff, 0x007f00, 0x007f55, 0x007faa, 0x007fff, 0x009f00, 0x009f55,
    0x009faa, 0x009fff, 0x00bf00, 0x00bf55, 0x00bfaa, 0x00bfff, 0x00df00,
    0x00df55, 0x00dfaa, 0x00dfff, 0x00ff55, 0x00ffaa, 0x2a0000, 0x2a0055,
    0x2a00aa, 0x2a00ff, 0x2a1f00, 0x2a1f55, 0x2a1faa, 0x2a1fff, 0x2a3f00,
    0x2a3f55, 0xfffbf0, 0xa0a0a4, 0x808080, 0xff0000, 0x00ff00, 0xffff00,
    0x0000ff, 0xff00ff, 0x00ffff, 0xffffff};
static const int compressed_image[] = {
    24047, 24, 24,  16, 16,  8,  24,  8,  32,  16, 24,  24, 104, 24, 24,  16,
    16,    8,  24,  8,  32,  16, 24,  24, 104, 24, 24,  16, 16,  8,  24,  8,
    32,    16, 24,  24, 104, 24, 24,  16, 16,  8,  24,  8,  32,  16, 24,  24,
    104,   24, 24,  16, 16,  8,  24,  8,  32,  16, 24,  24, 104, 24, 24,  16,
    16,    8,  24,  8,  32,  16, 24,  24, 104, 24, 24,  16, 16,  8,  24,  8,
    32,    16, 24,  24, 104, 24, 24,  16, 16,  8,  24,  8,  32,  16, 24,  24,
    104,   8,  16,  8,  8,   8,  16,  8,  8,   16, 8,   16, 24,  8,  16,  8,
    8,     8,  128, 8,  16,  8,  8,   8,  16,  8,  8,   16, 8,   16, 24,  8,
    16,    8,  8,   8,  128, 8,  16,  8,  8,   8,  16,  8,  8,   16, 8,   16,
    24,    8,  16,  8,  8,   8,  128, 8,  16,  8,  8,   8,  16,  8,  8,   16,
    8,     16, 24,  8,  16,  8,  8,   8,  128, 8,  16,  8,  8,   8,  16,  8,
    8,     16, 8,   16, 24,  8,  16,  8,  8,   8,  128, 8,  16,  8,  8,   8,
    16,    8,  8,   16, 8,   16, 24,  8,  16,  8,  8,   8,  128, 8,  16,  8,
    8,     8,  16,  8,  8,   16, 8,   16, 24,  8,  16,  8,  8,   8,  128, 8,
    16,    8,  8,   8,  16,  8,  8,   16, 8,   16, 24,  8,  16,  8,  8,   8,
    128,   8,  16,  8,  8,   32, 8,   8,  8,   8,  8,   8,  24,  8,  16,  8,
    16,    16, 112, 8,  16,  8,  8,   32, 8,   8,  8,   8,  8,   8,  24,  8,
    16,    8,  16,  16, 112, 8,  16,  8,  8,   32, 8,   8,  8,   8,  8,   8,
    24,    8,  16,  8,  16,  16, 112, 8,  16,  8,  8,   32, 8,   8,  8,   8,
    8,     8,  24,  8,  16,  8,  16,  16, 112, 8,  16,  8,  8,   32, 8,   8,
    8,     8,  8,   8,  24,  8,  16,  8,  16,  16, 112, 8,  16,  8,  8,   32,
    8,     8,  8,   8,  8,   8,  24,  8,  16,  8,  16,  16, 112, 8,  16,  8,
    8,     32, 8,   8,  8,   8,  8,   8,  24,  8,  16,  8,  16,  16, 112, 8,
    16,    8,  8,   32, 8,   8,  8,   8,  8,   8,  24,  8,  16,  8,  16,  16,
    112,   8,  16,  8,  8,   8,  16,  8,  8,   8,  24,  8,  24,  8,  16,  8,
    32,    8,  104, 8,  16,  8,  8,   8,  16,  8,  8,   8,  24,  8,  24,  8,
    16,    8,  32,  8,  104, 8,  16,  8,  8,   8,  16,  8,  8,   8,  24,  8,
    24,    8,  16,  8,  32,  8,  104, 8,  16,  8,  8,   8,  16,  8,  8,   8,
    24,    8,  24,  8,  16,  8,  32,  8,  104, 8,  16,  8,  8,   8,  16,  8,
    8,     8,  24,  8,  24,  8,  16,  8,  32,  8,  104, 8,  16,  8,  8,   8,
    16,    8,  8,   8,  24,  8,  24,  8,  16,  8,  32,  8,  104, 8,  16,  8,
    8,     8,  16,  8,  8,   8,  24,  8,  24,  8,  16,  8,  32,  8,  104, 8,
    16,    8,  8,   8,  16,  8,  8,   8,  24,  8,  24,  8,  16,  8,  32,  8,
    104,   24, 16,  8,  16,  8,  8,   8,  24,  8,  32,  16, 16,  24, 112, 24,
    16,    8,  16,  8,  8,   8,  24,  8,  32,  16, 16,  24, 112, 24, 16,  8,
    16,    8,  8,   8,  24,  8,  32,  16, 16,  24, 112, 24, 16,  8,  16,  8,
    8,     8,  24,  8,  32,  16, 16,  24, 112, 24, 16,  8,  16,  8,  8,   8,
    24,    8,  32,  16, 16,  24, 112, 24, 16,  8,  16,  8,  8,   8,  24,  8,
    32,    16, 16,  24, 112, 24, 16,  8,  16,  8,  8,   8,  24,  8,  32,  16,
    16,    24, 112, 24, 16,  8,  16,  8,  8,   8,  24,  8,  32,  16, 16,  24,
    12893, 6,  4,   2,  6,   2,  6,   6,  6,   4,  4,   2,  6,   2,  2,   6,
    2,     6,  18,  4,  4,   2,  8,   8,  2,   2,  4,   2,  4,   4,  4,   2,
    4,     2,  2,   6,  4,   8,  2,   6,  10,  4,  4,   2,  4,   2,  2,   6,
    8,     2,  6,   2,  4,   4,  4,   6,  4,   6,  2,   2,  4,   2,  4,   6,
    46,    6,  4,   2,  6,   2,  6,   6,  6,   4,  4,   2,  6,   2,  2,   6,
    2,     6,  18,  4,  4,   2,  8,   8,  2,   2,  4,   2,  4,   4,  4,   2,
    4,     2,  2,   6,  4,   8,  2,   6,  10,  4,  4,   2,  4,   2,  2,   6,
    8,     2,  6,   2,  4,   4,  4,   6,  4,   6,  2,   2,  4,   2,  4,   6,
    46,    2,  4,   2,  2,   2,  6,   2,  6,   2,  4,   2,  2,   2,  4,   2,
    2,     2,  6,   2,  4,   2,  4,   2,  4,   2,  14,  2,  4,   2,  2,   2,
    8,     2,  8,   2,  4,   2,  2,   2,  4,   2,  2,   4,  2,   2,  2,   2,
    4,     2,  2,   2,  8,   2,  4,   2,  6,   2,  4,   2,  2,   4,  2,   2,
    2,     2,  4,   2,  6,   4,  2,   4,  2,   2,  4,   2,  2,   2,  4,   2,
    4,     2,  4,   2,  4,   2,  2,   2,  52,  2,  4,   2,  2,   2,  6,   2,
    6,     2,  4,   2,  2,   2,  4,   2,  2,   2,  6,   2,  4,   2,  4,   2,
    4,     2,  14,  2,  4,   2,  2,   2,  8,   2,  8,   2,  4,   2,  2,   2,
    4,     2,  2,   4,  2,   2,  2,   2,  4,   2,  2,   2,  8,   2,  4,   2,
    6,     2,  4,   2,  2,   4,  2,   2,  2,   2,  4,   2,  6,   4,  2,   4,
    2,     2,  4,   2,  2,   2,  4,   2,  4,   2,  4,   2,  4,   2,  2,   2,
    52,    6,  6,   6,  8,   2,  4,   2,  2,   8,  4,   2,  2,   2,  6,   2,
    4,     2,  4,   2,  14,  8,  2,   2,  8,   6,  6,   4,  4,   8,  2,   2,
    2,     4,  2,   2,  4,   2,  2,   6,  4,   6,  8,   8,  2,   2,  2,   4,
    2,     2,  4,   2,  6,   2,  2,   2,  2,   2,  2,   8,  2,   6,  6,   2,
    4,     2,  4,   2,  4,   4,  48,  6,  6,   6,  8,   2,  4,   2,  2,   8,
    4,     2,  2,   2,  6,   2,  4,   2,  4,   2,  14,  8,  2,   2,  8,   6,
    6,     4,  4,   8,  2,   2,  2,   4,  2,   2,  4,   2,  2,   6,  4,   6,
    8,     8,  2,   2,  2,   4,  2,   2,  4,   2,  6,   2,  2,   2,  2,   2,
    2,     8,  2,   6,  6,   2,  4,   2,  4,   2,  4,   4,  48,  2,  4,   2,
    6,     2,  10,  2,  4,   2,  2,   2,  4,   2,  4,   2,  2,   2,  6,   2,
    4,     2,  4,   2,  4,   2,  8,   2,  4,   2,  2,   2,  8,   2,  8,   2,
    4,     2,  2,   2,  4,   2,  2,   2,  4,   2,  2,   2,  4,   2,  2,   2,
    8,     2,  2,   2,  8,   2,  4,   2,  2,   2,  4,   2,  2,   2,  4,   2,
    6,     2,  6,   2,  2,   2,  4,   2,  2,   2,  2,   2,  6,   2,  4,   2,
    4,     2,  8,   2,  46,  2,  4,   2,  6,   2,  10,  2,  4,   2,  2,   2,
    4,     2,  4,   2,  2,   2,  6,   2,  4,   2,  4,   2,  4,   2,  8,   2,
    4,     2,  2,   2,  8,   2,  8,   2,  4,   2,  2,   2,  4,   2,  2,   2,
    4,     2,  2,   2,  4,   2,  2,   2,  8,   2,  2,   2,  8,   2,  4,   2,
    2,     2,  4,   2,  2,   2,  4,   2,  6,   2,  6,   2,  2,   2,  4,   2,
    2,     2,  2,   2,  6,   2,  4,   2,  4,   2,  8,   2,  46,  6,  8,   2,
    10,    6,  4,   2,  4,   2,  6,   2,  6,   6,  2,   6,  6,   2,  8,   2,
    4,     2,  2,   8,  2,   8,  2,   2,  4,   2,  2,   2,  4,   2,  2,   2,
    4,     2,  2,   6,  4,   8,  2,   2,  4,   2,  6,   2,  4,   2,  2,   2,
    4,     2,  2,   6,  8,   2,  6,   2,  2,   2,  4,   2,  2,   2,  4,   2,
    2,     6,  4,   4,  4,   6,  48,  6,  8,   2,  10,  6,  4,   2,  4,   2,
    6,     2,  6,   6,  2,   6,  6,   2,  8,   2,  4,   2,  2,   8,  2,   8,
    2,     2,  4,   2,  2,   2,  4,   2,  2,   2,  4,   2,  2,   6,  4,   8,
    2,     2,  4,   2,  6,   2,  4,   2,  2,   2,  4,   2,  2,   6,  8,   2,
    6,     2,  2,   2,  4,   2,  2,   2,  4,   2,  2,   6,  4,   4,  4,   6};

void set_mode(uint8_t *port0300, RegisterMode *mode) {
  for (int i = 0; i < 28; i++) {
    uint8_t vport = mode[i].vport;
    uint8_t index = mode[i].index;
    uint8_t data = mode[i].data;
    uint8_t *port = port0300 + vport;

    switch (vport) {
    case 0xC0:
      *(port0300 + 0xDA) = 0;

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

void set_colors(uint8_t *p) {
  for (int i = 0; i < 256; i++) {
    uint8_t b = PALETTE[i] & 0xFF;
    uint8_t g = (PALETTE[i] >> 8) & 0xFF;
    uint8_t r = (PALETTE[i] >> 16) & 0xFF;

    *p = r;
    *p = g;
    *p = b;
  }
}

void draw_compressed_image(uint8_t *fb_base) {
  memset((const char *)fb_base, 0, 320 * 200);
  int c = 0;
  int index = 0;
  int count = 0;
  for (int i = 0; i < 64000; i++) {
    if (count == 0) {
      count = compressed_image[index++];
      c = !c;
    }
    fb_base[i] = c;
    count--;
  }
}

#define fb_base 0x50000000

int verify_pci_device(uint32_t *devbase) {
  uint32_t pci_class = (*(devbase + 2)) >> 8;

  if (pci_class != 0x030000) {
    cprintf("VGA not found\n");
    return 0;
  } else {
    cprintf("VGA device found\n");
    return 1;
  }
}

uint8_t *setup_pci_bars(uint32_t *devbase) {
  // Write 0xFFFFFFFF and read size of bar0
  *(devbase + 4) = 0xFFFFFFFF;
  uint32_t fb_size = (~(*(devbase + 4)) | 0xF) + 1;
  uint8_t *io_base = (uint8_t *)fb_base + fb_size;

  // Write 0xFFFFFFFF and read size of bar2
  *(devbase + 6) = 0xFFFFFFFF;
  uint32_t io_size = (~(*(devbase + 6)) | 0xF) + 1;

  // Write where framebuffer is to bar 0 and where io is to bar 2
  *(devbase + 4) = (uint32_t)(uintptr_t)(fb_base) | 8;
  *(devbase + 6) = (uint32_t)(uintptr_t)(io_base) | 8;

  // Enable Memory Space in command register
  uint32_t cmd = *(devbase + 1);
  *(devbase + 1) = cmd | 0x0002;

  cprintf("VGA fb_base %x size %dM io_base %x size %d\n",
          (uint32_t)(uintptr_t)fb_base, (fb_size + 0x80000) >> 20,
          (uint32_t)(uintptr_t)io_base, io_size);

  return io_base;
}

void init_virtio_vga() {
  uint32_t *devbase = pci_get_addr(0, 16, 0, 0x0);

  if (!verify_pci_device(devbase))
    return;

  uint8_t *io_base = setup_pci_bars(devbase);

  uint8_t *port0300 = (io_base + (0x400 - 0xC0));
  /*set_mode(port0300, TEXT_MODE_REGS);*/

  set_mode(port0300, MODE_13_REGS);
  draw_compressed_image((unsigned char *)fb_base);

  *(io_base + 0x406) = 0xFF;
  *(io_base + 0x408) = 0;

  uint8_t *p = io_base + 0x409;
  /*set_colors(p);*/
  /*set_screen_a(fb_base);*/
}

void debug_print_virtio() {
  uint8_t bus = 0, device = 16, function = 0;

  uint32_t reg0 = pci_read_word(bus, device, function, 0x00);
  uint32_t reg1 = pci_read_word(bus, device, function, 0x04);
  uint32_t reg2 = pci_read_word(bus, device, function, 0x08);
  uint32_t reg3 = pci_read_word(bus, device, function, 0x0C); // New register

  uint16_t vendor_id = reg0 & 0xFFFF;
  uint16_t device_id = (reg0 >> 16) & 0xFFFF;
  uint16_t command = reg1 & 0xFFFF;
  uint16_t status = (reg1 >> 16) & 0xFFFF;
  uint8_t revision_id = reg2 & 0xFF;
  uint8_t prog_if = (reg2 >> 8) & 0xFF;
  uint8_t subclass = (reg2 >> 16) & 0xFF;
  uint8_t class_code = (reg2 >> 24) & 0xFF;

  uint8_t cache_line_size = reg3 & 0xFF;
  uint8_t latency_timer = (reg3 >> 8) & 0xFF;
  uint8_t header_type = (reg3 >> 16) & 0xFF;
  uint8_t bist = (reg3 >> 24) & 0xFF;

  cprintf("Device ID:        0x%x\n", device_id);
  cprintf("Vendor ID:        0x%x\n", vendor_id);
  cprintf("Command:          0x%x\n", command);
  cprintf("Status:           0x%b\n", status);
  cprintf("Class Code:       0x%x\n", class_code);
  cprintf("Subclass:         0x%x\n", subclass);
  cprintf("Programming IF:   0x%x\n", prog_if);
  cprintf("Revision ID:      0x%x\n", revision_id);
  cprintf("Cache Line Size:  0x%x (%d bytes)\n", cache_line_size,
          cache_line_size * 4);
  cprintf("Latency Timer:    0x%x\n", latency_timer);
  cprintf("Header Type:      0x%x\n", header_type);
  cprintf("BIST:             0x%x\n", bist);
}
