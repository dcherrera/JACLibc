/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef sh4
	#define JACL_ARCH sh4
	#define JACL_ARCH_SH4 1
	#define __jacl_arch_syscall __sh4_syscall
	#define __jacl_arch_tls_set __sh4_set_tp_register
	#define __jacl_arch_tls_get __sh4_get_tp_register
	#define __jacl_arch_clone_thread __sh4_clone_thread
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __sh4_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r3 __asm__("r3") = num;
		register long r4 __asm__("r4") = a1;
		register long r5 __asm__("r5") = a2;
		register long r6 __asm__("r6") = a3;
		register long r7 __asm__("r7") = a4;
		register long r0 __asm__("r0") = a5;
		register long r1 __asm__("r1") = a6;

		__asm__ volatile ("trapa #31"
			: "=r"(r0)
			: "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r0), "r"(r1)
			: "memory");

		result = r0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"mov #0, r14\n"
		"mov r15, r4\n"
		"mov #-16, r0\n"
		"and r0, r15\n"
		"bsr _start_main\n"
		"nop\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"mov #173, r3\n"
			"trapa #31\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __sh4_set_tp_register(void* addr) {
		__asm__ volatile("ldc %0, gbr" : : "r"(addr) : "memory");
	}

	static inline void* __sh4_get_tp_register(void) {
		void* result;

		__asm__("stc gbr, %0" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __sh4_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~7UL) - 8;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mov %2, r4\n\t"
			"mov %3, r5\n\t"
			"mov #0, r6\n\t"
			"mov #0, r7\n\t"
			"mov #0, r0\n\t"
			"mov #120, r3\n\t"
			"trapa #23\n\t"
			"tst r0, r0\n\t"
			"bf 1f\n\t"

			"mov #0, r14\n\t"
			"mov %5, r4\n\t"
			"jsr @%4\n\t"
			"mov #1, r3\n\t"
			"trapa #23\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)120), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "r0", "r3", "r4", "r5", "r6", "r7", "r14", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
