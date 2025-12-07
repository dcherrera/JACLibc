/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef x86
	#define JACL_ARCH x86
	#define JACL_ARCH_X86 1
	#define __jacl_arch_syscall __x86_syscall
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __x86_syscall(long number, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		__asm__ volatile ("int $0x80"
			: "=a"(result)
			: "a"(number), "b"(a1), "c"(a2), "d"(a3), "S"(a4), "D"(a5)
			: "memory");

		(void)a6; /* i386 only supports 5 args */

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".text\n"
		".global _start\n"
		"_start:\n"
		"xor %ebp,%ebp\n"
		"mov %esp,%eax\n"
		"and $-16,%esp\n"
		"push %eax\n"
		"push %eax\n"
		"call _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"mov $173, %eax\n"
			"int $0x80\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __x86_set_gs_register(void* addr) {
		__asm__ volatile("mov %0, %%gs:0" : : "r"(addr) : "memory");
	}

	static inline void* __x86_get_gs_register(void) {
		void* result;
		__asm__("mov %%gs:0, %0" : "=r"(result));
		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
