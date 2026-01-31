/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef wasm
	#define JACL_ARCH wasm
	#define JACL_ARCH_WASM 1
	#define __jacl_arch_syscall __wasm_syscall
	#define JACL_BITS 32
	/* WASM uses 128-bit long double (IEEE-754 quad precision) */
	#define JACL_LDBL_BITS 128
	#include JACL_HEADER(arch,wasm_helpers);
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	#include <jsio.h>

	static inline long __wasm_syscall(long num, long a1, long a2, long a3,
	                                  long a4, long a5, long a6) {
		(void)a4;
		(void)a5;
		(void)a6;

		switch ((int)num) {
			case SYS_read:
				return js_read((int)a1, (void*)a2, (size_t)a3);
			case SYS_write:
				return js_write((int)a1, (const void*)a2, (size_t)a3);
			/* TODO: map more syscalls as needed */
			default:
				/* ENOSYS: function not supported */
				return -ENOSYS;
		}
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	#include <jsio.h>

	JS_EXPORT(_start) void js_start();
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	// WebAssembly doesn't have traditional TLS registers
	// TLS is typically handled via __tls_base or linear memory
	// For now, provide stubs that the OS layer can override
	static inline void __wasm_set_tls_base(void* addr) {
		// WASM TLS typically uses __tls_base global or similar
		// This may need JS glue code or WASI implementation
		(void)addr;
	}

	static inline void* __wasm_get_tls_base(void) {
		// Return NULL for now - WASM TLS needs special handling
		return (void*)0;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
