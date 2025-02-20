#ifndef SYSTEM_H
#define SYSTEM_H

#define PANIC(fmt, ...)                                                    \
  do {                                                                     \
    printfmt("PANIC: %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    while (1) { }                                                          \
  } while (0)                                                              \

void poweroff(void);
void poweroffmsg(char *msg);

#endif /* !SYSTEM_H */
