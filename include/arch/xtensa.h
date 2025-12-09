/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef xtensa
	#define JACL_ARCH xtensa
	#define JACL_ARCH_XTENSA 1
	#define __jacl_arch_syscall __xtensa_syscall
	#define __jacl_arch_tls_set __xtensa_set_tp_register
	#define __jacl_arch_tls_get __xtensa_get_tp_register
	#define __jacl_arch_clone_thread __xtensa_clone_thread
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __xtensa_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long a2_reg __asm__("a2") = num;
		register long a3_reg __asm__("a3") = a1;
		register long a4_reg __asm__("a4") = a2;
		register long a5_reg __asm__("a5") = a3;
		register long a6_reg __asm__("a6") = a4;
		register long a7_reg __asm__("a7") = a5;
		register long a8_reg __asm__("a8") = a6;

		__asm__ volatile ("syscall"
			: "=r"(a2_reg)
			: "r"(a2_reg), "r"(a3_reg), "r"(a4_reg), "r"(a5_reg), "r"(a6_reg), "r"(a7_reg), "r"(a8_reg)
			: "memory");

		result = a2_reg;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"movi a0, 0\n"
		"mov a2, a1\n"
		"addi a1, a1, -16\n"
		"call0 _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"movi a2, 139\n"
			"syscall\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __xtensa_set_tp_register(void* addr) {
		__asm__ volatile("mov a15, %0" : : "r"(addr) : "memory");
	}

	static inline void* __xtensa_get_tp_register(void) {
		void* result;

		__asm__("mov %0, a15" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __xtensa_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~15UL) - 16;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mov a6, %2\n\t"
			"mov a3, %3\n\t"
			"movi a4, 0\n\t"
			"movi a5, 0\n\t"
			"movi a7, 0\n\t"
			"movi a2, 220\n\t"
			"syscall\n\t"
			"bnez a2, 1f\n\t"

			"movi a15, 0\n\t"
			"mov a6, %5\n\t"
			"callx0 %4\n\t"
			"movi a2, 93\n\t"
			"syscall\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)220), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "a2", "a3", "a4", "a5", "a6", "a7", "a15", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
