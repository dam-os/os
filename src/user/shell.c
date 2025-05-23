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

s32 startswith(char *search, char *target) {
  u32 l = cstrlen(search);
  // can't have a search longer than the target now
  if (l > cstrlen(target))
    return -2;

  for (u32 i = 0; i < l; i++) {
    if (search[i] ^ target[i]) {
      return -1;
    }
  }

  return 0;
}


void cmemset(const char *dst, char c, size_t n) {
  char *d = (char *)dst;
  while (n > 0) {
    *d++ = c;
    n--;
  }
}

void strip_carriage_return(char *str) {
  int len = cstrlen(str);
  if (len > 0 && str[len-1] == '\r') {
      str[len-1] = '\0';
  }
}

int login() {
  char username[32];
  char password[32];
  
  print("Please start by logging in:\r\n");
  
  while (1) {
      cmemset(username, '\0', 32);
      cmemset(password, '\0', 32);

      print("Username: ");
      read(username, 32);
      print("\r\nlogging in as user: ");
      print(username);
      print("\r\nPassword: ");
      read(password, 32);
      print("\r\n");
      print("\r\nChecking credentials, please wait...\r\n");
      sleep(1000);
      strip_carriage_return(username);
      strip_carriage_return(password);

      if (cstrcmp(username, "user") == 0 && cstrcmp(password, "password") == 0) {
          print("Login successful!\r\n");
          return 1;
      } else {
          print("Invalid credentials. Please try again.\r\n");
          sleep(1000);  // Short delay between attempts
          print("\r\n");
      }
  }
}

void cmd_echo(char *input) {
  char *message = input + 5;
  print(message);
  print("\r\n");
}

void cmd_help() {
  print("Available commands:\r\n");
  print("  help     - Show this help message\r\n");
  print("  echo     - Echo text back to terminal\r\n");
  print("  clear    - Clear the screen\r\n");
  print("  shutdown - Shutdown the system\r\n");
  print("  whoami   - Shows the username\r\n");
}

void print_welcome() {
  sleep(10);
  print("\r\n");
  print(" _______    ______   __       __           ______    ______  \r\n");
  sleep(10);
  print("/       \\  /      \\ /  \\     /  |         /      \\  /      \\ \r\n");
  sleep(10);
  print("$$$$$$$  |/$$$$$$  |$$  \\   /$$ |        /$$$$$$  |/$$$$$$  |\r\n");
  sleep(10);
  print("$$ |  $$ |$$ |__$$ |$$$  \\ /$$$ | ______ $$ |  $$ |$$ \\__$$/ \r\n");
  sleep(10);
  print("$$ |  $$ |$$    $$ |$$$$  /$$$$ |/      |$$ |  $$ |$$      \\ \r\n");
  sleep(10);
  print("$$ |  $$ |$$$$$$$$ |$$ $$ $$/$$ |$$$$$$/ $$ |  $$ | $$$$$$  |\r\n");
  sleep(10);
  print("$$ |__$$ |$$ |  $$ |$$ |$$$/ $$ |        $$ \\__$$ |/  \\__$$ |\r\n");
  sleep(10);
  print("$$    $$/ $$ |  $$ |$$ | $/  $$ |        $$    $$/ $$    $$/ \r\n");
  sleep(10);
  print("$$$$$$$/  $$/   $$/ $$/      $$/          $$$$$$/   $$$$$$/  \r\n");
  sleep(10);
  print("\r\n");                                                  
}

void cmd_whoami() {
  print("\r\nuser\r\n");
}

void cmd_clear() {
  // ANSI escape sequence to clear screen
  print("\033[2J\033[H");
  print_welcome();
}

void shell() {
  char input[128];
  u64 time_start = 0;
  
  print("\r\nDAM-OS - Type 'help' for commands\r\n");
  
  while (1) {
      print("dam-os > ");
      cmemset(input, '\0', 128);
      read(input, 128);
      print("\r\n");
      strip_carriage_return(input);
      
      if (cstrcmp(input, "help") == 0) {
          cmd_help();
      }
      else if (cstrcmp(input, "clear") == 0) {
          cmd_clear();
      }
      else if (cstrcmp(input, "whoami") == 0) {
          cmd_whoami();
      }
      else if (cstrcmp(input, "shutdown") == 0 || cstrcmp(input, "exit") == 0) {
          print("Shutting down DAM-OS...\r\n");
          poweroff();  
      }
      else if (startswith("echo ", input) == 0) {
          cmd_echo(input);
      }
      else if (cstrlen(input) == 0) {
      }
      else {
          print("Command not found: ");
          print(input);
          print("\r\nType 'help' for available commands.\r\n");
      }
      
      print("\r\n");
  }
}

int main(void) {
  print_welcome();
  login();
  shell();
  poweroff();
}
