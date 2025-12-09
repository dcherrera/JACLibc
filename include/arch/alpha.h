/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef alpha
	#define JACL_ARCH alpha
	#define JACL_ARCH_ALPHA 1
	#define __jacl_arch_syscall __alpha_syscall
	#define __jacl_arch_tls_set __alpha_set_tp_register
	#define __jacl_arch_tls_get __alpha_get_tp_register
	#define __jacl_arch_clone_thread __alpha_clone_thread
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __alpha_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long v0 __asm__("$0") = num;
		register long a0 __asm__("$16") = a1;
		register long a1r __asm__("$17") = a2;
		register long a2r __asm__("$18") = a3;
		register long a3r __asm__("$19") = a4;
		register long a4r __asm__("$20") = a5;
		register long a5r __asm__("$21") = a6;

		__asm__ volatile ("callsys"
			: "=r"(v0)
			: "r"(v0), "r"(a0), "r"(a1r), "r"(a2r), "r"(a3r), "r"(a4r), "r"(a5r)
			: "memory", "$1", "$22", "$23", "$24", "$25", "$27", "$28");

		result = v0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"mov $31, $15\n"
		"mov $30, $16\n"
		"lda $30, -16($30)\n"
		"bsr $26, _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"lda $0, 298\n"
			"callsys\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __alpha_set_tp_register(void* addr) {
		__asm__ volatile("mov %0, $0; call_pal 158" : : "r"(addr) : "memory", "$0");
	}

	static inline void* __alpha_get_tp_register(void) {
		void* result;

		__asm__("call_pal 9; mov $0, %0" : "=r"(result) : : "$0");

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __alpha_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~15UL) - 16;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mov %2, $16\n\t"
			"mov %3, $17\n\t"
			"clr $18\n\t"
			"clr $19\n\t"
			"clr $20\n\t"
			"lda $0, 312\n\t"
			"call_pal 0x83\n\t"
			"bne $0, 1f\n\t"

			"clr $15\n\t"
			"mov %5, $16\n\t"
			"jsr $26, (%4)\n\t"
			"lda $0, 1\n\t"
			"call_pal 0x83\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)312), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "$0", "$16", "$17", "$18", "$19", "$20", "$26", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
