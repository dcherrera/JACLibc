/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef x64
	#define JACL_ARCH x64
	#define JACL_ARCH_X64 1
	#define __jacl_arch_syscall __x64_syscall
	#define __jacl_arch_tls_set __x64_set_fs_register
	#define __jacl_arch_tls_get __x64_get_fs_register
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __x64_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long rax __asm__("rax") = num;
		register long rdi __asm__("rdi") = a1;
		register long rsi __asm__("rsi") = a2;
		register long rdx __asm__("rdx") = a3;
		register long r10 __asm__("r10") = a4;
		register long r8 __asm__("r8")   = a5;
		register long r9 __asm__("r9")   = a6;

		__asm__ volatile ("syscall"
			: "=a"(result)
			: "a"(rax), "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10), "r"(r8), "r"(r9)
			: "memory", "rcx", "r11");

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".text\n"
		".global _start\n"
		"_start:\n"
		"xor %rbp,%rbp\n"
		"mov %rsp,%rdi\n"
		"andq $-16,%rsp\n"
		"call _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"mov $15, %rax\n"
			"syscall\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	// Only CPU register manipulation
	static inline void __x64_set_fs_register(void* addr) {
		__asm__ volatile("mov %0, %%fs:0" : : "r"(addr) : "memory");
	}

	static inline void* __x64_get_fs_register(void) {
		void* result;

		__asm__("mov %%fs:0, %0" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
