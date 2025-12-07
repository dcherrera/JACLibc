/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __OS_CONFIG
	#undef elf
	#undef linux
	#define JACL_OS linux
	#define JACL_OS_LINUX 1
	#define JACL_FMT elf
	#define JACL_FMT_ELF 1
	#define __jacl_os_syscall __linux_syscall
	#if defined(JACL_ARCH_X64) || defined(JACL_ARCH_X86)
		#undef __jacl_arch_tls_set
		#undef __jacl_arch_tls_get
		#define __jacl_arch_tls_set __os_set_fs_register
		#define __jacl_arch_tls_get __os_get_fs_register
	#endif
#undef __OS_CONFIG
#endif

#ifdef __OS_SYSCALL
	#define __ARCH_SYSCALL
	#include JACL_HEADER(arch, JACL_ARCH)
	#define __linux_syscall __jacl_arch_syscall
#undef __OS_SYSCALL
#endif

#ifdef __OS_INIT
	/* Pull in format helpers for TLS metadata */
	#define __FMT_INIT
	#include JACL_HEADER(fmt, JACL_FMT)

	#if defined(JACL_ARCH_X64) || defined(JACL_ARCH_X86)
		#define ARCH_SET_FS 0x1002
		#define ARCH_GET_FS 0x1003

		static inline void __os_set_fs_register(void* addr) {
			__linux_syscall(SYS_arch_prctl, ARCH_SET_FS, (long)addr, 0, 0, 0, 0);
		}
		static inline void* __os_get_fs_register(void) {
			return __linux_syscall(SYS_arch_prctl, ARCH_GET_FS, 0, 0, 0, 0, 0);
		}
	#else
		/* Pull in arch TLS register ops */
		#define __ARCH_TLS
		#include JACL_HEADER(arch, JACL_ARCH)
	#endif

	static inline void __jacl_init_os(void) {
		size_t tls_size, tls_align, tls_init_size;
		void* tls_image;

		/* 1. Get TLS metadata from format (PT_TLS) */
		if (!__jacl_tls_get_info(&tls_size, &tls_align, &tls_image, &tls_init_size)) return;
		if (tls_size == 0) return;

		/* 2. Allocate TLS+TCB (Linux-specific) */
		size_t total = tls_size + tls_align + JACL_TCB_SIZE;
		void* mem = mmap(NULL, total, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		if (mem == MAP_FAILED) return;

		/* 3. Compute aligned addresses */
		uintptr_t tls_base = JACL_ALIGN_UP((uintptr_t)mem, tls_align);
		uintptr_t tcb_addr = tls_base + tls_size;

		/* 4. Initialize TLS data */
		if (tls_image && tls_init_size > 0)
			memcpy((void*)tls_base, tls_image, tls_init_size);
		if (tls_size > tls_init_size) memset((void*)(tls_base + tls_init_size), 0, tls_size - tls_init_size);

		/* 5. Set TCB self-pointer */
		*(void**)tcb_addr = (void*)tcb_addr;

		/* 6. Install TLS pointer */
		__jacl_arch_tls_set((void*)tcb_addr);
	}
#undef __OS_INIT
#endif
