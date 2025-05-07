#ifndef COMMON_H
#define COMMON_H

// === Types ===

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef u64 uptr;
typedef u64 size_t;

// === Constants ===

#define NULL 0

#define TRUE 1
#define FALSE 0

// === Aliases ===
/**
 * Variable Argument (VA) aliases are taken from the LLVM Project source code,
 * licensed under the Apache License v2.0 with LLVM Exceptions.
 * Re-implemented here to remove dependency on the C Standard Library.
 */
#ifndef _VA_LIST
#define _VA_LIST
typedef __builtin_va_list va_list;
#endif
#ifndef va_arg

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
/* C23 does not require the second parameter for va_start. */
#define va_start(ap, ...) __builtin_va_start(ap, 0)
#else
/* Versions before C23 do require the second parameter. */
#define va_start(ap, param) __builtin_va_start(ap, param)
#endif
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)

#endif

// === Functions ===

u32 swap_endian_32(u32 val);

#endif // !COMMON_H
