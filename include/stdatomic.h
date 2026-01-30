/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef STDATOMIC_H
#define STDATOMIC_H
#pragma once

#include <config.h>
#include <stdbool.h>
#include <stdint.h>

#if !JACL_HAS_C11
#error "stdatomic.h requires C11 or later"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* C11/C23 feature detection */
#if JACL_HAS_C23
#  define __STDC_VERSION_STDATOMIC_H__ 202311L
#elif JACL_HAS_C11
#  define __STDC_VERSION_STDATOMIC_H__ 201112L
#endif

/* Memory ordering enumeration */
typedef enum {
	memory_order_relaxed = 0,
	memory_order_consume = 1,
	memory_order_acquire = 2,
	memory_order_release = 3,
	memory_order_acq_rel = 4,
	memory_order_seq_cst = 5
} memory_order;

/* Platform detection for WebAssembly */
#if JACL_ARCH_WASM && !defined(__WASM_ATOMICS__)
#define FAKE_ATOMICS
#endif

typedef _Atomic bool atomic_bool;
typedef _Atomic char atomic_char;
typedef _Atomic signed char atomic_schar;
typedef _Atomic unsigned char atomic_uchar;
typedef _Atomic short atomic_short;
typedef _Atomic unsigned short atomic_ushort;
typedef _Atomic int atomic_int;
typedef _Atomic unsigned int atomic_uint;
typedef _Atomic long atomic_long;
typedef _Atomic unsigned long atomic_ulong;
typedef _Atomic long long atomic_llong;
typedef _Atomic unsigned long long atomic_ullong;
typedef _Atomic uintptr_t atomic_voidptr;
typedef struct { _Atomic  bool _Value; } atomic_flag;

// Memory Barrier
#ifdef FAKE_ATOMICS

static inline void __jacl_spin_lock(void *ptr) { (void)ptr; }
static inline void __jacl_spin_unlock(void *ptr) { (void)ptr; }

static inline void __jacl_barrier_acquire(void) {}
static inline void __jacl_barrier_release(void) {}
static inline void __jacl_barrier_seq_cst(void) {}

#else

static volatile unsigned char __jacl_spinlock_table[64] = {0};
static inline void __jacl_spin_lock(void *ptr) {
	union { void *p; uintptr_t u; } addr_conv = { .p = ptr };
	uintptr_t addr = addr_conv.u;
	unsigned idx = ((addr >> 3) ^ (addr >> 9)) & 63;
	volatile unsigned char *lock = &__jacl_spinlock_table[idx];

	#if JACL_ARCH_X64 || JACL_ARCH_X86
		while (__sync_lock_test_and_set(lock, 1)) { while (*lock) { /* spin */ } }
	#else
		unsigned char expected = 0; while (1) { if (*lock == 0) { expected = 0; if (__atomic_compare_exchange_n(lock, &expected, 1, false, 5, 5)) break; } for (volatile int i = 0; i < 10; i++); }
	#endif
}
static inline void __jacl_spin_unlock(void *ptr) {
	union { void *p; uintptr_t u; } addr_conv = { .p = ptr };
	uintptr_t addr = addr_conv.u;
	unsigned idx = ((addr >> 3) ^ (addr >> 9)) & 63;
	volatile unsigned char *lock = &__jacl_spinlock_table[idx];

	#if JACL_ARCH_X64 || JACL_ARCH_X86
		__sync_lock_release(lock);
	#else
		__atomic_store_n(lock, 0, 5);
	#endif
}
static inline void __jacl_barrier_acquire(void) {
	#if JACL_ARCH_X64 || JACL_ARCH_X86
		__asm__ volatile ("" ::: "memory");
	#elif JACL_ARCH_ARM64
		__asm__ volatile ("dmb ishld" ::: "memory");
	#elif JACL_ARCH_ARM32
		__asm__ volatile ("dmb" ::: "memory");
	#else
		__asm__ volatile ("" ::: "memory");
	#endif
}
static inline void __jacl_barrier_release(void) {
	#if JACL_ARCH_X64 || JACL_ARCH_X86
		__asm__ volatile ("" ::: "memory");
	#elif JACL_ARCH_ARM64
		__asm__ volatile ("dmb ishst" ::: "memory");
	#elif JACL_ARCH_ARM32
		__asm__ volatile ("dmb" ::: "memory");
	#else
		__asm__ volatile ("" ::: "memory");
	#endif
}
static inline void __jacl_barrier_seq_cst(void) {
	#if JACL_ARCH_X64 || JACL_ARCH_X86
		__asm__ volatile ("mfence" ::: "memory");
	#elif JACL_ARCH_ARM64 || JACL_ARCH_ARM32
		__asm__ volatile ("dmb ish" ::: "memory");
	#else
		__asm__ volatile ("" ::: "memory");
	#endif
}

#endif /* FAKE_ATOMICS */

#ifdef FAKE_ATOMICS
static inline void atomic_thread_fence(memory_order mo) { (void)mo; }
static inline void atomic_signal_fence(memory_order mo) { (void)mo; }
#else
static inline void atomic_thread_fence(memory_order mo) {
	switch (mo) {
		case memory_order_acquire: __jacl_barrier_acquire(); break;
		case memory_order_release: __jacl_barrier_release(); break;
		case memory_order_acq_rel: __jacl_barrier_acquire(); __jacl_barrier_release(); break;
		case memory_order_seq_cst: __jacl_barrier_acquire(); __jacl_barrier_release(); __jacl_barrier_seq_cst(); break;
		default: __asm__ volatile ("" ::: "memory"); break;
	}
}
static inline void atomic_signal_fence(memory_order mo) {
	switch (mo) {
		case memory_order_relaxed: break;
		default: __asm__ volatile ("" ::: "memory"); break;
	}
}
#endif

#ifdef FAKE_ATOMICS

// Single-threaded fake
#define JACL_ATOMIC_SIZES \
	JACL_ATOMIC_BUNDLE(8,  uint8_t ) \
	JACL_ATOMIC_BUNDLE(16, uint16_t) \
	JACL_ATOMIC_BUNDLE(32, uint32_t) \
	JACL_ATOMIC_BUNDLE(64, uint64_t)
#define JACL_ATOMIC_BUNDLE(bits, type) \
	static inline type __jacl_atomic_load_##bits(volatile type *ptr, memory_order mo) {(void)mo;return *ptr;} \
	static inline void __jacl_atomic_store_##bits(volatile type *ptr, type val, memory_order mo) {(void)mo;*ptr=val;} \
	static inline type __jacl_atomic_exchange_##bits(volatile type *ptr, type val, memory_order mo) {(void)mo;type old=*ptr;*ptr=val;return old;} \
	static inline type __jacl_atomic_fetch_add_##bits(volatile type *ptr, type val, memory_order mo) {(void)mo;type old=*ptr;*ptr+=val;return old;} \
	static inline bool __jacl_atomic_cmpxchg_##bits(volatile type *ptr, type *expected, type desired, memory_order mo) {(void)mo;if(*ptr==*expected){*ptr=desired;return true;}*expected=*ptr;return false;}

#elif JACL_ARCH_X64 || JACL_ARCH_X86

// x86/x64 with inline assembly
#define JACL_ATOMIC_SIZES \
	JACL_ATOMIC_BUNDLE(8,  uint8_t,  "b") \
	JACL_ATOMIC_BUNDLE(16, uint16_t, "w") \
	JACL_ATOMIC_BUNDLE(32, uint32_t, "l") \
	JACL_ATOMIC_BUNDLE(64, uint64_t, "q")
#define JACL_ATOMIC_BUNDLE(bits, type, suffix) \
	static inline type __jacl_atomic_load_##bits(volatile type *ptr, memory_order mo) {atomic_thread_fence(mo);return *ptr;} \
	static inline void __jacl_atomic_store_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo);*ptr=val;} \
	static inline type __jacl_atomic_exchange_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo);__asm__ volatile("xchg" suffix " %0,%1":"=r"(val),"+m"(*ptr):"0"(val):"memory");return val;} \
	static inline type __jacl_atomic_fetch_add_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo);__asm__ volatile("lock xadd" suffix " %0,%1":"=r"(val),"+m"(*ptr):"0"(val):"memory");return val;} \
	static inline bool __jacl_atomic_cmpxchg_##bits(volatile type *ptr, type *expected, type desired, memory_order mo) {atomic_thread_fence(mo);type old=*expected;__asm__ volatile("lock cmpxchg" suffix " %2,%1":"=a"(old),"+m"(*ptr):"r"(desired),"0"(old):"memory");if(old==*expected)return true;*expected=old;return false;}

#elif JACL_ARCH_ARM64

// ARM64 with LDXR/STXR - use compiler builtins for simplicity
#define JACL_ATOMIC_SIZES \
	JACL_ATOMIC_BUNDLE(8,  uint8_t ) \
	JACL_ATOMIC_BUNDLE(16, uint16_t) \
	JACL_ATOMIC_BUNDLE(32, uint32_t) \
	JACL_ATOMIC_BUNDLE(64, uint64_t)
#define JACL_ATOMIC_BUNDLE(bits, type) \
	static inline type __jacl_atomic_load_##bits(volatile type *ptr, memory_order mo) {(void)mo;type val;__asm__ volatile("ldar %0, [%1]":"=r"(val):"r"(ptr):"memory");return val;} \
	static inline void __jacl_atomic_store_##bits(volatile type *ptr, type val, memory_order mo) {(void)mo;__asm__ volatile("stlr %1, [%0]"::"r"(ptr),"r"(val):"memory");} \
	static inline type __jacl_atomic_exchange_##bits(volatile type *ptr, type val, memory_order mo) {(void)mo;type old;unsigned s;do{__asm__ volatile("ldxr %0, [%2]\nstxr %w1, %3, [%2]":"=&r"(old),"=&r"(s):"r"(ptr),"r"(val):"memory");}while(s);return old;} \
	static inline type __jacl_atomic_fetch_add_##bits(volatile type *ptr, type val, memory_order mo) {(void)mo;type old,tmp;unsigned s;do{__asm__ volatile("ldxr %0, [%3]\nadd %1, %0, %4\nstxr %w2, %1, [%3]":"=&r"(old),"=&r"(tmp),"=&r"(s):"r"(ptr),"r"(val):"memory");}while(s);return old;} \
	static inline bool __jacl_atomic_cmpxchg_##bits(volatile type *ptr, type *expected, type desired, memory_order mo) {(void)mo;type old;unsigned s=1;__asm__ volatile("ldxr %0, [%2]":"=&r"(old):"m"(*ptr),"r"(ptr):"memory");if(old==*expected){__asm__ volatile("stxr %w0, %2, [%1]":"=&r"(s):"r"(ptr),"r"(desired):"memory");}if(s==0)return true;*expected=old;return false;}

#elif JACL_ARCH_ARM32

// ARM32 with LDREX/STREX
#define JACL_ATOMIC_SIZES \
	JACL_ATOMIC_BUNDLE(8,  uint8_t,  "ldrexb", "strexb") \
	JACL_ATOMIC_BUNDLE(16, uint16_t, "ldrexh", "strexh") \
	JACL_ATOMIC_BUNDLE(32, uint32_t, "ldrex",  "strex") \
	JACL_ATOMIC_BUNDLE(64, uint64_t, "ldrex",  "strex")
#define JACL_ATOMIC_BUNDLE(bits, type, ldr, str) \
	static inline type __jacl_atomic_load_##bits(volatile type *ptr, memory_order mo) {atomic_thread_fence(mo);return *ptr;} \
	static inline void __jacl_atomic_store_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo);*ptr=val;} \
	static inline type __jacl_atomic_exchange_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo);type old;unsigned s;do{__asm__ volatile(ldr" %0,[%2]\n"str" %1,%3,[%2]\n":"=&r"(old),"=&r"(s):"r"(ptr),"r"(val):"memory");}while(s);return old;} \
	static inline type __jacl_atomic_fetch_add_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo);type old;unsigned s;do{__asm__ volatile(ldr" %0,[%2]\nadd %0,%0,%3\n"str" %1,%0,[%2]\n":"=&r"(old),"=&r"(s):"r"(ptr),"r"(val):"memory");}while(s);return old;} \
	static inline bool __jacl_atomic_cmpxchg_##bits(volatile type *ptr, type *expected, type desired, memory_order mo) {atomic_thread_fence(mo);type old;unsigned s;bool ok=false;do{__asm__ volatile(ldr" %0,[%2]\n":"=&r"(old):"m"(*ptr),"r"(ptr):"memory");if(old==*expected){__asm__ volatile(str" %1,%3,[%2]\n":"=&r"(s):"r"(ptr),"r"(desired):"memory");ok=(s==0);break;}}while(0);if(ok)return true;*expected=old;return false;}

#else

// Spinlock fallback for unknown compilers
#define JACL_ATOMIC_SIZES \
	JACL_ATOMIC_BUNDLE(8,  uint8_t ) \
	JACL_ATOMIC_BUNDLE(16, uint16_t) \
	JACL_ATOMIC_BUNDLE(32, uint32_t) \
	JACL_ATOMIC_BUNDLE(64, uint64_t)
#define JACL_ATOMIC_BUNDLE(bits, type) \
	static inline type __jacl_atomic_load_##bits(volatile type *ptr, memory_order mo) {atomic_thread_fence(mo); __jacl_spin_lock(ptr); type val=*ptr; __jacl_spin_unlock(ptr); return val;} \
	static inline void __jacl_atomic_store_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo); __jacl_spin_lock(ptr); *ptr=val; __jacl_spin_unlock(ptr);} \
	static inline type __jacl_atomic_exchange_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo); __jacl_spin_lock(ptr); type old=*ptr; *ptr=val; __jacl_spin_unlock(ptr); return old;} \
	static inline type __jacl_atomic_fetch_add_##bits(volatile type *ptr, type val, memory_order mo) {atomic_thread_fence(mo); __jacl_spin_lock(ptr); type old=*ptr; *ptr+=val; __jacl_spin_unlock(ptr); return old;} \
	static inline bool __jacl_atomic_cmpxchg_##bits(volatile type *ptr, type *expected, type desired, memory_order mo) {atomic_thread_fence(mo); __jacl_spin_lock(ptr); if(*ptr==*expected){*ptr=desired; __jacl_spin_unlock(ptr); return true;} *expected=*ptr; __jacl_spin_unlock(ptr); return false;}

#endif /* FAKE_ATOMICS */

// Special Long  handling
#if ULONG_MAX == UINT64_MAX
#define __JACL_ATOMIC_DISPATCH_LONG(fn) __jacl_atomic_##fn##_64
#else
#define __JACL_ATOMIC_DISPATCH_LONG(fn) __jacl_atomic_##fn##_32
#endif

// Generic dispatch main macro
#define __JACL_ATOMIC_DISPATCH(fn, obj) \
	_Generic((obj), \
		atomic_uchar*: __jacl_atomic_##fn##_8, \
		atomic_schar*: __jacl_atomic_##fn##_8, \
		atomic_ushort*: __jacl_atomic_##fn##_16, \
		atomic_short*: __jacl_atomic_##fn##_16, \
		atomic_uint*: __jacl_atomic_##fn##_32, \
		atomic_int*: __jacl_atomic_##fn##_32, \
		atomic_ulong*: __JACL_ATOMIC_DISPATCH_LONG(fn), \
		atomic_long*: __JACL_ATOMIC_DISPATCH_LONG(fn), \
		atomic_ullong*: __jacl_atomic_##fn##_64, \
		atomic_llong*: __jacl_atomic_##fn##_64, \
		default: __jacl_atomic_##fn##_32 \
	)

// Generic dispatch macros
#define __JACL_ATOMIC_DISPATCH_LOAD(obj, mo) __JACL_ATOMIC_DISPATCH(load, obj)((volatile void*)(obj), (mo))
#define __JACL_ATOMIC_DISPATCH_STORE(obj, val, mo) __JACL_ATOMIC_DISPATCH(store, obj)((volatile void*)(obj), (val), (mo))
#define __JACL_ATOMIC_DISPATCH_EXCHANGE(obj, val, mo) __JACL_ATOMIC_DISPATCH(exchange, obj)((volatile void*)(obj), (val), (mo))
#define __JACL_ATOMIC_DISPATCH_FETCH_ADD(obj, val, mo) __JACL_ATOMIC_DISPATCH(fetch_add, obj)((volatile void*)(obj), (val), (mo))
#define __JACL_ATOMIC_DISPATCH_CMPXCHG(obj, expected, desired, mo) __JACL_ATOMIC_DISPATCH(cmpxchg, obj)((volatile void*)(obj), (void*)(expected), (desired), (mo))

/// Generate all atomic functions
JACL_ATOMIC_SIZES

#undef JACL_ATOMIC_BUNDLE
#undef JACL_ATOMIC_SIZES

// Standard C11/C23 atomic operations
#define atomic_init(obj, val) (*(obj) = (val))
#define atomic_load_explicit(obj, mo) __JACL_ATOMIC_DISPATCH_LOAD(obj, mo)
#define atomic_store_explicit(obj, val, mo) __JACL_ATOMIC_DISPATCH_STORE(obj, val, mo)
#define atomic_exchange_explicit(obj, val, mo) __JACL_ATOMIC_DISPATCH_EXCHANGE(obj, val, mo)
#define atomic_fetch_add_explicit(obj, val, mo) __JACL_ATOMIC_DISPATCH_FETCH_ADD(obj, val, mo)
#define atomic_fetch_sub_explicit(obj, val, mo) __JACL_ATOMIC_DISPATCH_FETCH_ADD(obj, -(val), mo)
#define atomic_compare_exchange_strong_explicit(obj, expected, desired, succ, fail) __JACL_ATOMIC_DISPATCH_CMPXCHG(obj, expected, desired, succ)
#define atomic_load(obj) atomic_load_explicit(obj, memory_order_seq_cst)
#define atomic_store(obj, val) atomic_store_explicit(obj, val, memory_order_seq_cst)
#define atomic_exchange(obj, val) atomic_exchange_explicit(obj, val, memory_order_seq_cst)
#define atomic_fetch_add(obj, val) atomic_fetch_add_explicit(obj, val, memory_order_seq_cst)
#define atomic_fetch_sub(obj, val) atomic_fetch_sub_explicit(obj, val, memory_order_seq_cst)
#define atomic_compare_exchange_strong(obj, expected, desired) atomic_compare_exchange_strong_explicit(obj, expected, desired, memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_weak(obj, expected, desired) atomic_compare_exchange_strong(obj, expected, desired)
#define atomic_compare_exchange_weak_explicit(obj, expected, desired, succ, fail) atomic_compare_exchange_strong_explicit(obj, expected, desired, succ, fail)

// Bitwise operations
static inline uint32_t __jacl_atomic_fetch_and_32(volatile uint32_t *ptr, uint32_t val, memory_order mo) { atomic_thread_fence(mo); uint32_t expected = *ptr; while (!__jacl_atomic_cmpxchg_32(ptr, &expected, expected & val, mo)) {}; return expected; }
static inline uint32_t __jacl_atomic_fetch_or_32(volatile uint32_t *ptr, uint32_t val, memory_order mo) { atomic_thread_fence(mo); uint32_t expected = *ptr; while (!__jacl_atomic_cmpxchg_32(ptr, &expected, expected | val, mo)) {}; return expected; }
static inline uint32_t __jacl_atomic_fetch_xor_32(volatile uint32_t *ptr, uint32_t val, memory_order mo) { atomic_thread_fence(mo); uint32_t expected = *ptr; while (!__jacl_atomic_cmpxchg_32(ptr, &expected, expected ^ val, mo)) {}; return expected; }

#define atomic_fetch_and_explicit(obj, val, mo) \
	_Generic((obj), \
		atomic_uint*: __jacl_atomic_fetch_and_32, \
		default: __jacl_atomic_fetch_and_32 \
	)((volatile void*)(obj), (val), (mo))
#define atomic_fetch_or_explicit(obj, val, mo) \
	_Generic((obj), \
		atomic_uint*: __jacl_atomic_fetch_or_32, \
		default: __jacl_atomic_fetch_or_32 \
	)((volatile void*)(obj), (val), (mo))
#define atomic_fetch_xor_explicit(obj, val, mo) \
	_Generic((obj), \
		atomic_uint*: __jacl_atomic_fetch_xor_32, \
		default: __jacl_atomic_fetch_xor_32 \
	)((volatile void*)(obj), (val), (mo))
#define atomic_fetch_and(obj, val) atomic_fetch_and_explicit(obj, val, memory_order_seq_cst)
#define atomic_fetch_or(obj, val) atomic_fetch_or_explicit(obj, val, memory_order_seq_cst)
#define atomic_fetch_xor(obj, val) atomic_fetch_xor_explicit(obj, val, memory_order_seq_cst)

/* Atomic flag operations */
static inline bool atomic_flag_test_and_set(atomic_flag *f) {
		return (bool)__jacl_atomic_exchange_8((volatile uint8_t*)&f->_Value, 1, memory_order_seq_cst);
}
static inline bool atomic_flag_test_and_set_explicit(atomic_flag *f, memory_order mo) {
		return (bool)__jacl_atomic_exchange_8((volatile uint8_t*)&f->_Value, 1, mo);
}
static inline void atomic_flag_clear(atomic_flag *f) {
		__jacl_atomic_store_8((volatile uint8_t*)&f->_Value, 0, memory_order_seq_cst);
}
static inline void atomic_flag_clear_explicit(atomic_flag *f, memory_order mo) {
		__jacl_atomic_store_8((volatile uint8_t*)&f->_Value, 0, mo);
}


/* Lock-free macros */
#define ATOMIC_BOOL_LOCK_FREE 2
#define ATOMIC_CHAR_LOCK_FREE 2
#define ATOMIC_CHAR16_T_LOCK_FREE 2
#define ATOMIC_CHAR32_T_LOCK_FREE 2
#define ATOMIC_WCHAR_T_LOCK_FREE 2
#define ATOMIC_SHORT_LOCK_FREE 2
#define ATOMIC_INT_LOCK_FREE 2
#define ATOMIC_LONG_LOCK_FREE 2
#define ATOMIC_LLONG_LOCK_FREE 2
#define ATOMIC_POINTER_LOCK_FREE 2

#define ATOMIC_FLAG_INIT {false}

#if JACL_HAS_C11 && !JACL_HAS_C23
	#define ATOMIC_VAR_INIT(x) (x)  /* Removed in C23 */
#endif

#ifdef __cplusplus
}
#endif

#endif /* STDATOMIC_H */
