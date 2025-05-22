#include "user.h"
#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_GETCHAR 3
#define SYS_PUTCHAR 4
#define SYS_SLEEP 5
#define SYS_POWEROFF 6
#define SYS_YIELD 8
#define SYS_EXIT 9

extern char _binary_shell_bin_start[];

void print(char * buf) {
  syscall(SYS_WRITE, buf, 0, 0);
}

void read(char *out, size_t size) {
  syscall(SYS_READ, out, (void *)size, 0);
}

void sleep(size_t duration) {
  syscall(SYS_SLEEP, (void *)duration, 0, 0);
}

void poweroff() {
  syscall(SYS_POWEROFF, 0, 0, 0);
}

int cstrlen(char *src) {
  u32 len = 0;
  while (*src != '\0') {
    src += 1;
    len++;
  }
  return len;
}

u32 cstrcmp(char *src, char *dest) {
  u32 l = cstrlen(src);
  if (l != cstrlen(dest))
    return -1;
  for (u32 i = 0; i < l; i++) {
    if (src[i] ^ dest[i]) {
      return -1;
    }
  }

  return 0;
}


int main(void) {
  
  char username[32];
  char password[32];
  print("Welcome to DAM-OS' User Mode!\r\n");
  print("Please start by entering your LearnIt credentials so we can set our grade:\r\n");
  print("Username: ");
  read(username, 32);
  print("\r\nVerifying username ");
  print(username);
  sleep(1000);
  print("\r\nPassword: ");
  read(password, 32);
  print("\r\nChecking credentials, please wait...\r\n");
  sleep(3000);
  if(cstrcmp(username,"user\r") == 0 && cstrcmp(password, "password\r") == 0) {
    print("Updating LearnIT grades! Goodbye!\r\n");
  } else {
    print("Invalid credentials, shutting down :(\r\n");
  }
  sleep(5000);
  poweroff();
}
