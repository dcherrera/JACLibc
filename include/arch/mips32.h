/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef mips32
	#define JACL_ARCH mips32
	#define JACL_ARCH_MIPS32 1
	#define __jacl_arch_syscall __mips32_syscall
	#define __jacl_arch_tls_set __mips32_set_tp_register
	#define __jacl_arch_tls_get __mips32_get_tp_register
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __mips32_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long v0 __asm__("$2") = num;
		register long a0 __asm__("$4") = a1;
		register long a1_reg __asm__("$5") = a2;
		register long a2_reg __asm__("$6") = a3;
		register long a3_reg __asm__("$7") = a4;

		(void)a5;
		(void)a6;

		__asm__ volatile ("syscall"
			: "=r"(v0)
			: "r"(v0), "r"(a0), "r"(a1_reg), "r"(a2_reg), "r"(a3_reg)
			: "memory", "$1", "$3", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25");

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
			"li $v0, 4193\n"
			"syscall\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __mips32_set_tp_register(void* addr) {
		__asm__ volatile("move $3, %0" : : "r"(addr) : "memory");
	}

	static inline void* __mips32_get_tp_register(void) {
		void* result;

		__asm__("move %0, $3" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
