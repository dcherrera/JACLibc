/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#define JACL_VERSION 1.83
#define JACL_RELEASE 202511L

// avoid compile screams
#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
  #pragma GCC diagnostic ignored "-Wincompatible-library-redeclaration"
#endif /* __GNUC__ || __clang__ */

// useful macros
#define JACL_QUOTE(x) #x
#define JACL_EXPAND(x) x
#define JACL_CONCAT(a, b, space) a##space##b
#define JACL_CONCAT_EXPAND(a, b, space) JACL_CONCAT(a,b,space)
#define JACL_CONCAT_HAS(a, b) __##a##_has_##b
#define JACL_CONCAT_HAS_EXPAND(a, b) JACL_CONCAT_HAS(a, b)
#define JACL_HEADER(dir, file) <dir/file.h>
#define JACL_OS_ARCH JACL_CONCAT_EXPAND(JACL_OS, JACL_ARCH, _)
#define JACL_HASSYS(name) JACL_CONCAT_HAS_EXPAND(JACL_OS_ARCH, name)

#define JACL_X_SYSCALL JACL_HEADER(x, JACL_OS_ARCH)
#define JACL_X_SIGNALS JACL_HEADER(x, JACL_CONCAT_EXPAND(signals,JACL_OS,_))

/* ============================================================= */
/* C Standards Detection                                         */
/* ============================================================= */

// standard version
#if defined(__STDC_VERSION__)
	#if __STDC_VERSION__ >= 202311L
		#define JACL_STDC_VERSION 2023
	#elif __STDC_VERSION__ >= 201710L
		#define JACL_STDC_VERSION 2017
	#elif __STDC_VERSION__ >= 201112L
		#define JACL_STDC_VERSION 2011
	#elif __STDC_VERSION__ >= 199901L
		#define JACL_STDC_VERSION 1999
	#else /* too old to care */
		#define JACL_STDC_VERSION 1989
	#endif /* __STDC_VERSION__ */
#else
	#define JACL_STDC_VERSION 1989
#endif /* __STDC_VERSION__ */

// has standard
#define JACL_HAS_C99 (JACL_STDC_VERSION >= 1999)
#define JACL_HAS_C11 (JACL_STDC_VERSION >= 2011)
#define JACL_HAS_C17 (JACL_STDC_VERSION >= 2017)
#define JACL_HAS_C23 (JACL_STDC_VERSION >= 2023)

// Deprecation polyfill
#if JACL_HAS_C23
	#define __deprecated            [[deprecated]]
	#define __deprecated_msg(msg)   [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__)
	#define __deprecated            __attribute__((deprecated))
	#define __deprecated_msg(msg)   __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
	#define __deprecated            __declspec(deprecated)
	#define __deprecated_msg(msg)   __declspec(deprecated(msg))
#else
	#define __deprecated
	#define __deprecated_msg(msg)
#endif

// Noreturn polyfill
#if JACL_HAS_C23
	#define noreturn [[noreturn]]
#elif JACL_HAS_C11
	/* C11/C17: standard keyword */
	#define noreturn _Noreturn
#else
	/* Pre-C11: compiler extensions */
	#if defined(__GNUC__) || defined(__clang__)
		#define noreturn __attribute__((noreturn))
	#elif defined(_MSC_VER)
		#define noreturn __declspec(noreturn)
	#else
		#define noreturn
	#endif
#endif

// C99 features polyfill
#if !JACL_HAS_C99
  #define inline

	#if defined(__GNUC__) || defined(__clang__)
    #define restrict __restrict__
	#elif defined(_MSC_VER)
    #define restrict __restrict
	#else /* no restrict */
    #define restrict
	#endif /* __GNUC__ || __clang__ */
#endif /* JACL_HAS_C99 */

// C11 features polyfill
#if !JACL_HAS_C11
	#define _Static_assert(cond, msg) typedef char JACL_CONCAT(static_assertion_,__LINE__)[(cond) ? 1 : -1]
	#define _Atomic volatile

  #if defined(__GNUC__) || defined(__clang__) || defined(__SUNPRO_C) || defined(__TINYC__)
    #define _Thread_local __thread
  #elif defined(_MSC_VER)
    #define _Thread_local __declspec(thread)
  #else /* no thread local */
    #define _Thread_local
  #endif /* __GNUC__ || __clang__ || __SUNPRO_C || __TINY_CC__ */

	#if defined(_WIN32) || defined(_WIN64)
		#define _Alignas(x) __declspec(align(x))
		#define _Alignof(t) __alignof(t)
	#elif defined(__GNUC__) || defined(__clang__)
		#define _Alignas(x) __attribute__((aligned(x)))
		#define _Alignof(t) __alignof__(t)
	#else /* default alignment */
		#define aoffset(type, member) ((unsigned long)&(((type*)0)->member))
		#define _Alignas(x)
		#define _Alignof(t) aoffset(struct { char c; t m; }, m)
	#endif /* allignment */
#endif /* !JACL_HAS_C11 */

// C23 features
#if !JACL_HAS_C23
	#ifndef __cplusplus
		#define static_assert _Static_assert
	#endif

	#if !defined(__cplusplus)
  	#define thread_local _Thread_local
	#endif /* !__cplusplus */
#endif /* !JACL_HAS_C23 */

#ifndef _Complex_I
	#define _Complex_I (0.0f+1.0fi)
#endif

#ifndef I
	#define I _Complex_I
#endif

#ifndef complex
	#define complex _Complex
#endif


/* ============================================================= */
/* Architecture and Operating System Detection                   */
/* ============================================================= */

#define __ARCH_CONFIG
#include JACL_HEADER(arch,detect)

#define __OS_CONFIG
#include JACL_HEADER(os,detect)

/* ============================================================= */
/* Bit Depth Definitions                                         */
/* ============================================================= */

// Define bit depth
#define JACL_8BIT  (JACL_BITS == 8)
#define JACL_16BIT (JACL_BITS == 16)
#define JACL_32BIT (JACL_BITS == 32)
#define JACL_64BIT (JACL_BITS == 64)

// Long double precision bits
#if defined(JACL_ARCH_X86) || defined(JACL_ARCH_X64)
	#if defined(_MSC_VER)
		#define JACL_LDBL_BITS 64
	#elif defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
		#define JACL_LDBL_BITS 128  // binary128
	#elif defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 64
		#define JACL_LDBL_BITS 80   // x87
	#elif defined(__SIZEOF_LONG_DOUBLE__) && __SIZEOF_LONG_DOUBLE__ == 16
		#define JACL_LDBL_BITS 80   // probably x87 padded
	#else
		#define JACL_LDBL_BITS 64   // fallback to double
	#endif
#elif defined(JACL_ARCH_ARM64)
	#if defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
		#define JACL_LDBL_BITS 128
	#else
		#define JACL_LDBL_BITS 64
	#endif
#endif /* long double bits check */

// Integer max precision bits
#ifdef __INTMAX_WIDTH__
  #define JACL_INTMAX_BITS __INTMAX_WIDTH__
#else
  #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #define JACL_INTMAX_BITS 64
  #elif defined(__LONG_LONG_MAX__)
    #define JACL_INTMAX_BITS 64
  #elif defined(LONG_MAX) && LONG_MAX > 2147483647L
    #define JACL_INTMAX_BITS 64
  #else
    #define JACL_INTMAX_BITS 32
  #endif
#endif

// wchar_t width detection
#ifdef __WCHAR_WIDTH__
  #define JACL_WCHAR_BITS __WCHAR_WIDTH__
#elif defined(__SIZEOF_WCHAR_T__)
  #define JACL_WCHAR_BITS (__SIZEOF_WCHAR_T__ * 8)
#elif defined(_WIN32) || defined(_WIN64)
  #define JACL_WCHAR_BITS 16    // Windows uses UTF-16
#elif defined(__ARM_SIZEOF_WCHAR_T)
  #define JACL_WCHAR_BITS (__ARM_SIZEOF_WCHAR_T * 8)
#else
  #define JACL_WCHAR_BITS 32    // Default: UTF-32/UCS-4
#endif

/* ============================================================= */
/* Constructor Initialization                                    */
/* ============================================================= */

typedef void (*__jacl_init_fn)(void);

#if JACL_OS_DARWIN
  #define JACL_INIT_SECTION "__DATA,__mod_init_func"
#elif JACL_OS_WINDOWS
  #define JACL_INIT_SECTION ".CRT$XCU"
#else
  #define JACL_INIT_SECTION ".init_array"
#endif

#if defined(_MSC_VER) && JACL_OS_WINDOWS
	/* MSVC uses different syntax */
	#define JACL_INIT(name) \
		static void __jacl_init_##name(void); \
		__pragma(section(JACL_INIT_SECTION, read)) \
		__declspec(allocate(JACL_INIT_SECTION)) \
		static const __jacl_init_fn __jacl_init_ptr_##name = __jacl_init_##name; \
		static void __jacl_init_##name(void)
#else
	/* everybody else is standard */
	#define JACL_INIT(name) \
		static void __jacl_init_##name(void); \
		static const __jacl_init_fn __jacl_init_ptr_##name \
		__attribute__((section(JACL_INIT_SECTION), used)) = __jacl_init_##name; \
		static void __jacl_init_##name(void)
#endif

/* ============================================================= */
/* Builtin Polyfills                                             */
/* ============================================================= */

// Feature attribute detection
#ifndef __has_attribute
  #define __has_attribute(x) 0
#endif /* !__has_attribute */

// Guarantee builtin detection
#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif /* !__has_builtin */

// Builtin Prefetch Polyfill
#if !__has_builtin(__builtin_prefetch)
	#define __builtin_prefetch(addr, rw, locality) ((void)(addr))
#endif /* __builtin_prefetch */

// Builtin Trap Polyfill
#if !__has_builtin(__builtin_trap)
	#if JACL_ARCH_WASM
		#define __builtin_trap() __asm__ volatile("unreachable")
	#else
		#define __builtin_trap() do { *(volatile int*)0 = 0; } while(0)
	#endif
#endif /* __builtin_trap */

// Builtin Popcount Polyfill
#if !__has_builtin(__builtin_popcount)
static inline int __builtin_popcount(unsigned int x) {
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);

	return (((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}
#endif /* __builtin_popcount */

// Builtin Frame Polyfill
#if !__has_builtin(__builtin_frame_address)
static inline void* __jacl_frame_address(int level) {
	if (level == 0) {
		volatile char frame_marker;
		return (void*)&frame_marker;
	}
	return NULL;
}
#endif /* __builtin_frame_address */

/* ============================================================= */
/* Speed Optimizations                                           */
/* ============================================================= */

// Fast math detection
#ifndef JACL_FAST_MATH
  #if (__FAST_MATH__ + 0 == 1) || (__FINITE_MATH_ONLY__ + 0 == 1)
    #define JACL_FAST_MATH 1
  #else /* no fast math */
    #define JACL_FAST_MATH 0
  #endif /* fast math check */
#endif /* !JACL_FAST_MATH */

// Safety check macro - compiles away in fast-math mode
#if JACL_FAST_MATH
  #define JACL_SAFETY(chk, rtn) do {} while(0)
#else /* safe but slower */
  #define JACL_SAFETY(chk, rtn) if (chk) return (rtn)
#endif /* JACL_FAST_MATH */

//Compiler Optimization
#if __has_builtin(__builtin_expect)
		#define JACL_LIKELY(x)   __builtin_expect(!!(x), 1)
		#define JACL_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else /* notexpectation builtin */
		#define JACL_LIKELY(x)   (x)
		#define JACL_UNLIKELY(x) (x)
#endif /* __builtin_expect */

/* ============================================================= */
/* Feature Flags                                                 */
/* ============================================================= */

// Has POSIX
#if defined(JACL_OS_WINDOWS) || defined(JACL_OS_NONE) || (defined(__STDC_HOSTED__) && __STDC_HOSTED__ == 0)
	#define JACL_HAS_POSIX 0
#else /* everything else */
	#define JACL_HAS_POSIX 1
#endif /* posix check */

// Check for atomic builtin support
#if __has_builtin(__atomic_load_n) && __has_builtin(__atomic_compare_exchange_n) && __has_builtin(__atomic_store_n)
	#define JACL_HAS_ATOMIC_BUILTINS 1
#else /* no atomic builtins */
	#define JACL_HAS_ATOMIC_BUILTINS 0
#endif /*atomic builtins */

// Thread support detection
#if JACL_HAS_POSIX
  #define JACL_HAS_PTHREADS 1
#else /* no pthreads */
  #define JACL_HAS_PTHREADS 0
#endif /* pthreads */

// SIMD intrinsics
#ifndef JACL_HAS_IMMINTRIN
  #if JACL_HAS_C99 && (defined(__SSE2__) || defined(__ARM_NEON) || defined(__wasm_simd128__))
    #define JACL_HAS_IMMINTRIN 1
  #else /* no immintrin */
    #define JACL_HAS_IMMINTRIN 0
  #endif /* immintrin check */
#endif /* !JACL_HAS_IMMINTRIN */

// Floating point math
#ifndef JACL_HAS_FLOAT
  #if defined(__STDC_NO_FLOAT__)
    #define JACL_HAS_FLOAT 0
  #else
    #define JACL_HAS_FLOAT 1
  #endif
#endif /* !JACL_HAS_FLOAT */

// Wide character support
#ifndef JACL_HAS_WCHAR
  #if defined(__STDC_NO_WCHAR_T__) || !JACL_HAS_POSIX
    #define JACL_HAS_WCHAR 0
  #else
    #define JACL_HAS_WCHAR 1
  #endif
#endif /* !JACL_HAS_WCHAR */

// Large file support
#if JACL_64BIT
  #define JACL_HAS_LFS 1
#elif defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
  #define JACL_HAS_LFS 1
#else /* no large file support */
  #define JACL_HAS_LFS 0
#endif /* large file support check */

// Syscall checks
#include JACL_HEADER(x, config_has)

#endif /* CONFIG_H */
