#include "screen.h"
#include "../drivers/vga.h"
#include "file.h"

#define WIDTH 80
#define HEIGHT 25
char arr[HEIGHT][WIDTH];
u32 x = 0;
u32 y = 0;

file stdout_screen = {
    .write = &screen_write,
};

void shift_up() {
  for (u32 i = 0; i < HEIGHT - 1; i++) {
    for (u32 j = 0; j < WIDTH; j++) {
      arr[i][j] = arr[i + 1][j];
    }
  }
  for (u32 i = 0; i < WIDTH; i++) {
    arr[HEIGHT - 1][i] = ' ';
  }
}

char sputchar(char c) {
  switch (c) {
  case '\n':
  case '\r': {
    if (y == HEIGHT - 1) {
      shift_up();
    } else {
      y++;
    }
    x = 0;
    set_cursor(x, (y+1)%25);
    break;
  }
  case 127: {
    if (x == 0) {
      x = WIDTH - 1;
      if (y != 0) {
        y--;
      }
    } else {

      x--;
    }
    arr[y][x] = ' ';
    set_cursor(x%80, (y+1)%25);
    break;
  }
  default: {
    if (x == WIDTH - 1) {
      x = 0;
      if (y == HEIGHT - 1) {
        shift_up();
        y--;
      }
      y++;
    }
    arr[y][x] = c;
    //set_cursor(x%31, 2);
    set_cursor((x+1)%80, (y+1)%25);

    x++;
    break;
  }
  }
  for (u32 y = 0; y < 25; y++) {
    for (u32 x = 0; x < 80; x++) {
      text_putfast(x, y, arr[y][x]);
    }
  }
  return c;
}

s8 screen_write(__attribute__((unused)) file *context, const char c) {
  sputchar(c);
  return 0;
}
