#include "../lib/common.h"
void init_virtio_vga();
void debug_print_virtio();
void text_putchar(uint8_t x, uint8_t y, char ch, uint8_t fg_color);
void clear_screen();
void text_putfast(uint8_t x, uint8_t y, char ch);
void print_screen_buf(char **buf);
