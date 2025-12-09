/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef csky
	#define JACL_ARCH csky
	#define JACL_ARCH_CSKY 1
	#define __jacl_arch_syscall __csky_syscall
	#define __jacl_arch_tls_set __csky_set_tp_register
	#define __jacl_arch_tls_get __csky_get_tp_register
	#define __jacl_arch_clone_thread __csky_clone_thread
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __csky_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r7 __asm__("r7") = num;
		register long r0 __asm__("r0") = a1;
		register long r1 __asm__("r1") = a2;
		register long r2 __asm__("r2") = a3;
		register long r3 __asm__("r3") = a4;
		register long r4 __asm__("r4") = a5;
		register long r5 __asm__("r5") = a6;

		__asm__ volatile ("trap 0"
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
		"movi r8, 0\n"
		"mov r0, sp\n"
		"andi sp, sp, -16\n"
		"jsr _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"movi r7, 139\n"
			"trap 0\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __csky_set_tp_register(void* addr) {
		__asm__ volatile("mov r31, %0" : : "r"(addr) : "memory");
	}

	static inline void* __csky_get_tp_register(void) {
		void* result;

		__asm__("mov %0, r31" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __csky_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~7UL) - 8;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mov a0, %2\n\t"
			"mov a1, %3\n\t"
			"movi a2, 0\n\t"
			"movi a3, 0\n\t"
			"movi t0, 0\n\t"
			"movi r7, 220\n\t"
			"trap 0\n\t"
			"cmpnei a0, 0\n\t"
			"bt 1f\n\t"

			"movi r8, 0\n\t"
			"mov a0, %5\n\t"
			"jsr %4\n\t"
			"movi r7, 93\n\t"
			"trap 0\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)220), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "a0", "a1", "a2", "a3", "r7", "r8", "t0", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
