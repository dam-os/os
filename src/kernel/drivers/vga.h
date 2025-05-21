#include "../lib/common.h"
s8 init_virtio_vga();
void debug_print_virtio();
void text_putchar(u8 x, u8 y, char ch, u8 fg_color);
void clear_screen();
void text_putfast(u8 x, u8 y, char ch);
void set_cursor(u8 x, u8 y);
