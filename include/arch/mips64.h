/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef mips64
	#define JACL_ARCH mips64
	#define JACL_ARCH_MIPS64 1
	#define __jacl_arch_syscall __mips64_syscall
	#define __jacl_arch_tls_set __mips64_set_tp_register
	#define __jacl_arch_tls_get __mips64_get_tp_register
	#define __jacl_arch_clone_thread __mips64_clone_thread
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __mips64_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long v0 __asm__("$2") = num;
		register long a0 __asm__("$4") = a1;
		register long a1_reg __asm__("$5") = a2;
		register long a2_reg __asm__("$6") = a3;
		register long a3_reg __asm__("$7") = a4;
		register long a4_reg __asm__("$8") = a5;
		register long a5_reg __asm__("$9") = a6;

		__asm__ volatile ("syscall"
			: "=r"(v0)
			: "r"(v0), "r"(a0), "r"(a1_reg), "r"(a2_reg), "r"(a3_reg), "r"(a4_reg), "r"(a5_reg)
			: "memory", "$1", "$3", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25");

		result = v0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"move $fp, $0\n"
		"move $a0, $sp\n"
		"and $sp, $sp, -16\n"
		"jal _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			".set noreorder\n"
			"li $v0, 5211\n"
			"syscall\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __mips64_set_tp_register(void* addr) {
		__asm__ volatile("move $3, %0" : : "r"(addr) : "memory");
	}

	static inline void* __mips64_get_tp_register(void) {
		void* result;

		__asm__("move %0, $3" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __mips64_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
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
			"li $v0, 5055\n\t"
			"syscall\n\t"
			"bnez $v0, 1f\n\t"

			"move $fp, $zero\n\t"
			"move $a0, %5\n\t"
			"jalr %4\n\t"
			"li $v0, 5058\n\t"
			"syscall\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)5055), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "$v0", "$a0", "$a1", "$a2", "$a3", "$a4", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
