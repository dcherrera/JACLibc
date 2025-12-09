/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef x86
	#define JACL_ARCH x86
	#define JACL_ARCH_X86 1
	#define __jacl_arch_syscall __x86_syscall
	#define __jacl_arch_tls_set __x86_set_gs_register
	#define __jacl_arch_tls_get __x86_get_gs_register
	#define __jacl_arch_clone_thread __x86_clone_thread
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

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __x86_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~15UL) - 4;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"pushl %5\n\t"
			"pushl %4\n\t"
			"movl $120, %%eax\n\t"
			"movl %2, %%ebx\n\t"
			"movl %3, %%ecx\n\t"
			"xor %%edx, %%edx\n\t"
			"xor %%esi, %%esi\n\t"
			"xor %%edi, %%edi\n\t"
			"int $0x80\n\t"
			"test %%eax, %%eax\n\t"
			"jnz 1f\n\t"

			"popl %%ecx\n\t"
			"popl %%eax\n\t"
			"xor %%ebp, %%ebp\n\t"
			"push %%eax\n\t"
			"call *%%ecx\n\t"
			"movl $1, %%eax\n\t"
			"int $0x80\n\t"

			"1:\n\t"
			"addl $8, %%esp\n\t"
			: "=a"(ret)
			: "a"((long)120), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "ebx", "ecx", "edx", "esi", "edi", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
