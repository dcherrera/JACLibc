/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef loongarch64
	#define JACL_ARCH loongarch64
	#define JACL_ARCH_LOONGARCH64 1
	#define __jacl_arch_syscall __loongarch64_syscall
	#define __jacl_arch_tls_set __loongarch64_set_tp_register
	#define __jacl_arch_tls_get __loongarch64_get_tp_register
	#define __jacl_arch_clone_thread __loongarch64_clone_thread
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __loongarch64_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long a7 __asm__("$a7") = num;
		register long a0 __asm__("$a0") = a1;
		register long a1_reg __asm__("$a1") = a2;
		register long a2_reg __asm__("$a2") = a3;
		register long a3_reg __asm__("$a3") = a4;
		register long a4_reg __asm__("$a4") = a5;
		register long a5_reg __asm__("$a5") = a6;

		__asm__ volatile ("syscall 0"
			: "=r"(a0)
			: "r"(a7), "r"(a0), "r"(a1_reg), "r"(a2_reg), "r"(a3_reg), "r"(a4_reg), "r"(a5_reg)
			: "memory", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8");

		result = a0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"move $fp, $zero\n"
		"move $a0, $sp\n"
		"bstrins.d $sp, $zero, 3, 0\n"
		"bl _start_main\n"
	);
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __loongarch64_set_tp_register(void* addr) {
		__asm__ volatile("move $tp, %0" : : "r"(addr) : "memory");
	}

	static inline void* __loongarch64_get_tp_register(void) {
		void* result;

		__asm__("move %0, $tp" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __loongarch64_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~15UL) - 16;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"move $a0, %2\n\t"
			"move $a1, %3\n\t"
			"move $a2, $zero\n\t"
			"move $a3, $zero\n\t"
			"move $a4, $zero\n\t"
			"li.w $a7, 220\n\t"
			"syscall 0\n\t"
			"bnez $a0, 1f\n\t"

			"move $fp, $zero\n\t"
			"move $a0, %5\n\t"
			"jirl $ra, %4, 0\n\t"
			"li.w $a7, 93\n\t"
			"syscall 0\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)220), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "$a0", "$a1", "$a2", "$a3", "$a4", "$a7", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
