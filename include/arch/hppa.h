/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef hppa
	#define JACL_ARCH hppa
	#define JACL_ARCH_HPPA 1
	#define __jacl_arch_syscall __hppa_syscall
	#define __jacl_arch_tls_set __hppa_set_tp_register
	#define __jacl_arch_tls_get __hppa_get_tp_register
	#define __jacl_arch_clone_thread __hppa_clone_thread
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __hppa_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r20 __asm__("r20") = num;
		register long r26 __asm__("r26") = a1;
		register long r25 __asm__("r25") = a2;
		register long r24 __asm__("r24") = a3;
		register long r23 __asm__("r23") = a4;
		register long r22 __asm__("r22") = a5;
		register long r21 __asm__("r21") = a6;

		__asm__ volatile ("ble 0x100(%%sr2, %%r0)\n\tcopy %%r31, %%r2"
			: "=r"(r28)
			: "r"(r20), "r"(r26), "r"(r25), "r"(r24), "r"(r23), "r"(r22), "r"(r21)
			: "memory");

		result = r28;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"ldi 0, %r3\n"
		"copy %sp, %r26\n"
		"ldo -64(%sp), %sp\n"
		"bl _start_main, %r2\n"
		"nop\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"ldi 173, %r20\n"
			"ble 0x100(%%sr2, %%r0)\n"
			"nop\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __hppa_set_tp_register(void* addr) {
		__asm__ volatile("copy %0, %%r27" : : "r"(addr) : "memory");
	}

	static inline void* __hppa_get_tp_register(void) {
		void* result;

		__asm__("copy %%r27, %0" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __hppa_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~63UL) - 64;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"copy %2, %%r26\n\t"
			"copy %3, %%r25\n\t"
			"ldi 0, %%r24\n\t"
			"ldi 0, %%r23\n\t"
			"ldi 0, %%r22\n\t"
			"ldi 120, %%r20\n\t"
			"ble 0x100(%%sr2, %%r0)\n\t"
			"ldi -1, %%r1\n\t"
			"cmpib,= 0, %%r28, 1f\n\t"

			"ldi 0, %%r3\n\t"
			"copy %5, %%r26\n\t"
			"ble 0(%%sr0, %4)\n\t"
			"ldi 1, %%r20\n\t"
			"ble 0x100(%%sr2, %%r0)\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)120), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "r1", "r20", "r22", "r23", "r24", "r25", "r26", "r28", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
