#ifndef COMMON_H
#define COMMON_H

// === Types ===

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef long double uint128_t;
typedef uint64_t size_t;
typedef uint64_t uintptr_t;
typedef uint64_t size_t;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uintptr_t uintptr;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;
typedef uintptr uptr;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uintptr_t uintptr;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;
typedef uintptr uptr;

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
