/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef sparc64
	#define JACL_ARCH sparc64
	#define JACL_ARCH_SPARC64 1
	#define __jacl_arch_syscall __sparc64_syscall
	#define __jacl_arch_tls_set __sparc64_set_tp_register
	#define __jacl_arch_tls_get __sparc64_get_tp_register
	#define __jacl_arch_clone_thread __sparc64_clone_thread
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __sparc64_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long g1 __asm__("g1") = num;
		register long o0 __asm__("o0") = a1;
		register long o1 __asm__("o1") = a2;
		register long o2 __asm__("o2") = a3;
		register long o3 __asm__("o3") = a4;
		register long o4 __asm__("o4") = a5;
		register long o5 __asm__("o5") = a6;

		__asm__ volatile ("t 0x6d"
			: "=r"(o0)
			: "r"(g1), "r"(o0), "r"(o1), "r"(o2), "r"(o3), "r"(o4), "r"(o5)
			: "memory", "cc");

		result = o0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"mov %g0, %fp\n"
		"mov %sp, %o0\n"
		"and %sp, -16, %sp\n"
		"call _start_main\n"
		"nop\n"
	);

#if JACL_OS_LINUX
	__asm__(
		".text\n"
		".globl __restore_rt\n"
		".type __restore_rt,@function\n"
		"__restore_rt:\n"
		"mov 101, %g1\n"
		"ta 0x6d\n"
	);
#endif

#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __sparc64_set_tp_register(void* addr) {
		__asm__ volatile("mov %0, %%g7" : : "r"(addr) : "memory");
	}

	static inline void* __sparc64_get_tp_register(void) {
		void* result;

		__asm__("mov %%g7, %0" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __sparc64_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~15UL) - 176;  // SPARC stack frame

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mov %2, %%o0\n\t"
			"mov %3, %%o1\n\t"
			"clr %%o2\n\t"
			"clr %%o3\n\t"
			"clr %%o4\n\t"
			"mov 217, %%g1\n\t"
			"ta 0x6d\n\t"
			"brnz,pn %%o0, 1f\n\t"

			"clr %%fp\n\t"
			"mov %5, %%o0\n\t"
			"call %4\n\t"
			" nop\n\t"
			"mov 1, %%g1\n\t"
			"ta 0x6d\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)217), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "g1", "o0", "o1", "o2", "o3", "o4", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
