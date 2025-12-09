/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef riscv64
	#define JACL_ARCH riscv64
	#define JACL_ARCH_RISCV64 1
	#define __jacl_arch_syscall __riscv64_syscall
	#define __jacl_arch_tls_set __riscv64_set_tp_register
	#define __jacl_arch_tls_get __riscv64_get_tp_register
	#define __jacl_arch_clone_thread __riscv64_clone_thread
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __riscv64_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long a7 __asm__("a7") = num;
		register long a0 __asm__("a0") = a1;
		register long a1_reg __asm__("a1") = a2;
		register long a2_reg __asm__("a2") = a3;
		register long a3_reg __asm__("a3") = a4;
		register long a4_reg __asm__("a4") = a5;
		register long a5_reg __asm__("a5") = a6;

		__asm__ volatile ("ecall"
			: "=r"(a0)
			: "r"(a7), "r"(a0), "r"(a1_reg), "r"(a2_reg), "r"(a3_reg), "r"(a4_reg), "r"(a5_reg)
			: "memory");

		result = a0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"li fp, 0\n"
		"li ra, 0\n"
		"mv a0, sp\n"
		"andi sp, sp, -16\n"
		"call _start_main\n"
	);
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __riscv64_set_tp_register(void* addr) {
		__asm__ volatile("mv tp, %0" : : "r"(addr) : "memory");
	}

	static inline void* __riscv64_get_tp_register(void) {
		void* result;

		__asm__("mv %0, tp" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __riscv64_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~15UL) - 16;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mv a0, %2\n\t"
			"mv a1, %3\n\t"
			"li a2, 0\n\t"
			"li a3, 0\n\t"
			"li a4, 0\n\t"
			"li a7, 220\n\t"
			"ecall\n\t"
			"bnez a0, 1f\n\t"

			"li s0, 0\n\t"
			"mv a0, %5\n\t"
			"jalr %4\n\t"
			"li a7, 93\n\t"
			"ecall\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)220), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "a0", "a1", "a2", "a3", "a4", "a7", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
