/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef arm32
	#define JACL_ARCH arm32
	#define JACL_ARCH_ARM32 1
	#define __jacl_arch_syscall __arm32_syscall
	#define __jacl_arch_tls_set __arm32_set_tp_register
	#define __jacl_arch_tls_get __arm32_get_tp_register
	#define __jacl_arch_clone_thread __arm32_clone_thread
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __arm32_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r7 __asm__("r7") = num;
		register long r0 __asm__("r0") = a1;
		register long r1 __asm__("r1") = a2;
		register long r2 __asm__("r2") = a3;
		register long r3 __asm__("r3") = a4;
		register long r4 __asm__("r4") = a5;
		register long r5 __asm__("r5") = a6;

		__asm__ volatile ("svc #0"
			: "=r"(r0)
			: "r"(r7), "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
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
		"mov fp, #0\n"
		"mov lr, #0\n"
		"mov a1, sp\n"
		"and sp, sp, #-16\n"
		"bl _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"mov r7, #173\n"
			"swi #0\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __arm32_set_tp_register(void* addr) {
		__asm__ volatile("mcr p15, 0, %0, c13, c0, 3" : : "r"(addr) : "memory");
	}

	static inline void* __arm32_get_tp_register(void) {
		void* result;
		__asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(result));
		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __arm32_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~7UL) - 8;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mov r0, %2\n\t"
			"mov r1, %3\n\t"
			"mov r2, #0\n\t"
			"mov r3, #0\n\t"
			"mov r4, #0\n\t"
			"mov r7, #120\n\t"
			"svc #0\n\t"
			"cmp r0, #0\n\t"
			"bne 1f\n\t"

			"mov fp, #0\n\t"
			"mov r0, %5\n\t"
			"blx %4\n\t"
			"mov r7, #1\n\t"
			"svc #0\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)120), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "r0", "r1", "r2", "r3", "r4", "r7", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
