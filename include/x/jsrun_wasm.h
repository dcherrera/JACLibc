/**
 * JSRUN/WASM SYSCALLS
 * Syscall definitions for WebAssembly running in JavaScript environment.
 * Uses Linux-compatible syscall numbers for library compatibility.
 * Most operations are handled via JS imports rather than actual syscalls.
 */

/* Core I/O */
X(SYS_read, 63, read)
X(SYS_write, 64, write)
X(SYS_openat, 56, openat)
X(SYS_close, 57, close)
X(SYS_lseek, 62, lseek)
X(SYS_llseek, 62, llseek)
X(SYS_readv, 65, readv)
X(SYS_writev, 66, writev)
X(SYS_pread64, 67, pread64)
X(SYS_pwrite64, 68, pwrite64)

/* Legacy open for compatibility */
X(SYS_open, 1024, open)

/* File info */
X(SYS_fstat, 80, fstat)
X(SYS_fstat64, 80, fstat64)
X(SYS_fstatat64, 79, fstatat64)
X(SYS_newfstatat, 79, newfstatat)
X(SYS_stat, 1038, stat)
X(SYS_lstat, 1039, lstat)
X(SYS_statx, 291, statx)

/* File operations */
X(SYS_ioctl, 29, ioctl)
X(SYS_fcntl, 25, fcntl)
X(SYS_fcntl64, 25, fcntl64)
X(SYS_dup, 23, dup)
X(SYS_dup2, 1041, dup2)
X(SYS_dup3, 24, dup3)
X(SYS_pipe2, 59, pipe2)
X(SYS_ftruncate, 46, ftruncate)
X(SYS_ftruncate64, 46, ftruncate64)
X(SYS_truncate, 45, truncate)
X(SYS_truncate64, 45, truncate64)
X(SYS_fsync, 82, fsync)
X(SYS_fdatasync, 83, fdatasync)
X(SYS_sync, 81, sync)
X(SYS_fadvise64, 223, fadvise64)
X(SYS_fallocate, 47, fallocate)

/* Directory operations */
X(SYS_getdents64, 61, getdents64)
X(SYS_chdir, 49, chdir)
X(SYS_fchdir, 50, fchdir)
X(SYS_getcwd, 17, getcwd)
X(SYS_mkdirat, 34, mkdirat)
X(SYS_unlinkat, 35, unlinkat)
X(SYS_renameat, 38, renameat)
X(SYS_faccessat, 48, faccessat)
X(SYS_fchmodat, 53, fchmodat)
X(SYS_fchownat, 54, fchownat)
X(SYS_linkat, 37, linkat)
X(SYS_symlinkat, 36, symlinkat)
X(SYS_readlinkat, 78, readlinkat)

/* Legacy path operations */
X(SYS_mkdir, 1030, mkdir)
X(SYS_rmdir, 1031, rmdir)
X(SYS_unlink, 1026, unlink)
X(SYS_rename, 1034, rename)
X(SYS_link, 1025, link)
X(SYS_symlink, 1036, symlink)
X(SYS_readlink, 1035, readlink)
X(SYS_chmod, 1028, chmod)
X(SYS_chown, 1029, chown)
X(SYS_lchown, 1032, lchown)
X(SYS_fchmod, 52, fchmod)
X(SYS_fchown, 55, fchown)
X(SYS_access, 1033, access)

/* Memory */
X(SYS_brk, 214, brk)
X(SYS_mmap, 222, mmap)
X(SYS_mmap2, 222, mmap2)
X(SYS_munmap, 215, munmap)
X(SYS_mprotect, 226, mprotect)
X(SYS_mremap, 216, mremap)
X(SYS_madvise, 233, madvise)

/* Process */
X(SYS_exit, 93, exit)
X(SYS_exit_group, 94, exit_group)
X(SYS_getpid, 172, getpid)
X(SYS_getppid, 173, getppid)
X(SYS_getuid, 174, getuid)
X(SYS_geteuid, 175, geteuid)
X(SYS_getgid, 176, getgid)
X(SYS_getegid, 177, getegid)
X(SYS_gettid, 178, gettid)
X(SYS_getpgid, 155, getpgid)
X(SYS_setpgid, 154, setpgid)
X(SYS_getpgrp, 1060, getpgrp)
X(SYS_setsid, 157, setsid)
X(SYS_getsid, 156, getsid)
X(SYS_getgroups, 158, getgroups)
X(SYS_setgroups, 159, setgroups)
X(SYS_setuid, 146, setuid)
X(SYS_setgid, 144, setgid)
X(SYS_setreuid, 145, setreuid)
X(SYS_setregid, 143, setregid)

/* Time */
X(SYS_clock_gettime, 113, clock_gettime)
X(SYS_clock_gettime64, 403, clock_gettime64)
X(SYS_clock_settime, 112, clock_settime)
X(SYS_clock_getres, 114, clock_getres)
X(SYS_gettimeofday, 169, gettimeofday)
X(SYS_settimeofday, 170, settimeofday)
X(SYS_nanosleep, 101, nanosleep)
X(SYS_times, 153, times)

/* Signals */
X(SYS_rt_sigaction, 134, rt_sigaction)
X(SYS_sigaction, 134, sigaction)
X(SYS_rt_sigprocmask, 135, rt_sigprocmask)
X(SYS_sigprocmask, 135, sigprocmask)
X(SYS_rt_sigpending, 136, rt_sigpending)
X(SYS_sigpending, 136, sigpending)
X(SYS_rt_sigsuspend, 133, rt_sigsuspend)
X(SYS_sigsuspend, 133, sigsuspend)
X(SYS_sigaltstack, 132, sigaltstack)
X(SYS_kill, 129, kill)
X(SYS_tkill, 130, tkill)
X(SYS_tgkill, 131, tgkill)

/* Misc */
X(SYS_uname, 160, uname)
X(SYS_sysinfo, 179, sysinfo)
X(SYS_getrandom, 278, getrandom)
X(SYS_umask, 166, umask)
X(SYS_prctl, 167, prctl)
X(SYS_getcpu, 168, getcpu)
X(SYS_personality, 92, personality)

/* Futex for threading */
X(SYS_futex, 98, futex)
X(SYS_futex_time64, 422, futex_time64)
X(SYS_set_tid_address, 96, set_tid_address)

/* Scheduling */
X(SYS_sched_yield, 124, sched_yield)
X(SYS_sched_getaffinity, 123, sched_getaffinity)
X(SYS_sched_setaffinity, 122, sched_setaffinity)
X(SYS_sched_getparam, 121, sched_getparam)
X(SYS_sched_setparam, 118, sched_setparam)
X(SYS_sched_getscheduler, 120, sched_getscheduler)
X(SYS_sched_setscheduler, 119, sched_setscheduler)

/* Poll/Select */
X(SYS_ppoll, 73, ppoll)
X(SYS_pselect6, 72, pselect6)
X(SYS_epoll_create1, 20, epoll_create1)
X(SYS_epoll_ctl, 21, epoll_ctl)
X(SYS_epoll_pwait, 22, epoll_pwait)

/* Resource limits */
X(SYS_getrlimit, 163, getrlimit)
X(SYS_setrlimit, 164, setrlimit)
X(SYS_prlimit64, 261, prlimit64)
X(SYS_getrusage, 165, getrusage)

/* Wait */
X(SYS_wait4, 260, wait4)
X(SYS_waitid, 95, waitid)

/* Clone/Fork/Exec */
X(SYS_clone, 220, clone)
X(SYS_clone3, 435, clone3)
X(SYS_execve, 221, execve)
X(SYS_execveat, 281, execveat)

/* Sockets */
X(SYS_socket, 198, socket)
X(SYS_socketpair, 199, socketpair)
X(SYS_bind, 200, bind)
X(SYS_listen, 201, listen)
X(SYS_accept, 202, accept)
X(SYS_accept4, 242, accept4)
X(SYS_connect, 203, connect)
X(SYS_getsockname, 204, getsockname)
X(SYS_getpeername, 205, getpeername)
X(SYS_sendto, 206, sendto)
X(SYS_recvfrom, 207, recvfrom)
X(SYS_setsockopt, 208, setsockopt)
X(SYS_getsockopt, 209, getsockopt)
X(SYS_shutdown, 210, shutdown)
X(SYS_sendmsg, 211, sendmsg)
X(SYS_recvmsg, 212, recvmsg)
