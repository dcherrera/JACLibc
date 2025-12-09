/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef or1k
	#define JACL_ARCH or1k
	#define JACL_ARCH_OR1K 1
	#define __jacl_arch_syscall __or1k_syscall
	#define __jacl_arch_tls_set __or1k_set_tp_register
	#define __jacl_arch_tls_get __or1k_get_tp_register
	#define __jacl_arch_clone_thread __or1k_clone_thread
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __or1k_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r11 __asm__("r11") = num;
		register long r3 __asm__("r3") = a1;
		register long r4 __asm__("r4") = a2;
		register long r5 __asm__("r5") = a3;
		register long r6 __asm__("r6") = a4;
		register long r7 __asm__("r7") = a5;
		register long r8 __asm__("r8") = a6;

		__asm__ volatile ("l.sys 1"
			: "=r"(r11)
			: "r"(r11), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8)
			: "memory");

		result = r11;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"l.or r2, r0, r0\n"
		"l.or r3, r1, r0\n"
		"l.andi r1, r1, -16\n"
		"l.jal _start_main\n"
		"l.nop\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"l.addi r11, r0, 173\n"
			"l.sys 1\n"
			"l.nop\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __or1k_set_tp_register(void* addr) {
		__asm__ volatile("l.or r10, %0, r0" : : "r"(addr) : "memory");
	}

	static inline void* __or1k_get_tp_register(void) {
		void* result;

		__asm__("l.or %0, r10, r0" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __or1k_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~3UL) - 4;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"l.or r3, %2, r0\n\t"
			"l.or r4, %3, r0\n\t"
			"l.or r5, r0, r0\n\t"
			"l.or r6, r0, r0\n\t"
			"l.or r7, r0, r0\n\t"
			"l.ori r11, r0, 220\n\t"
			"l.sys 1\n\t"
			"l.sfnei r11, 0\n\t"
			"l.bf 1f\n\t"

			"l.or r2, r0, r0\n\t"
			"l.or r3, %5, r0\n\t"
			"l.jalr %4\n\t"
			"l.ori r11, r0, 93\n\t"
			"l.sys 1\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)220), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "r2", "r3", "r4", "r5", "r6", "r7", "r11", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
