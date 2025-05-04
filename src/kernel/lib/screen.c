#include "../drivers/vga.h"
#define WIDTH 80
#define HEIGHT 25
char arr[HEIGHT][WIDTH];
int x = 0;
int y = 0;

void shift_up() {
  for (int i = 0; i < HEIGHT - 1; i++) {
    for (int j = 0; j < WIDTH; j++) {
      arr[i][j] = arr[i + 1][j];
    }
  }
  for (int i = 0; i < WIDTH; i++) {
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
    x++;
    break;
  }
  }
  for (int y = 0; y < 25; y++) {
    for (int x = 0; x < 80; x++) {
      text_putfast(x, y, arr[y][x]);
    }
  }
  return c;
}
