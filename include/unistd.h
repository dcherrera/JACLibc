/* (c) 2025 FRINKnet & Friends - MIT licence */
#ifndef UNISTD_H
#define UNISTD_H
#pragma once

#include <config.h>
#include <errno.h>        // errno, ENOSYS
#include <stddef.h>       // size_t
#include <stdarg.h>   // va_list, va_start(), va_end(), va_arg()
#include <stdnoreturn.h>  // noreturn
#include <sys/types.h>    // pid_t, uid_t, gid_t, ssize_t, off_t
#include <sys/syscall.h>  // syscall()
#include <sys/ioctl.h>    // winsize, TIOCGWINSZ
#include <time.h>         // timespec

#ifdef __cplusplus
extern "C" {
#endif

extern char **environ;
typedef unsigned int useconds_t;

#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1

#define HOST_NAME_MAX 256

#if JACL_HAS_POSIX

/* Process control */
noreturn static inline void _exit(int status) { syscall(SYS_exit, status); while(1); }
static inline pid_t getpid(void) { return (pid_t)syscall(SYS_getpid); }
static inline pid_t getppid(void) { return (pid_t)syscall(SYS_getppid); }
static inline pid_t fork(void) { return (pid_t)syscall(SYS_fork); }
static inline pid_t vfork(void) { return fork(); }

/* File I/O */
static inline ssize_t read(int fd, void *buf, size_t count) { return syscall(SYS_read, fd, buf, count); }
static inline ssize_t write(int fd, const void *buf, size_t count) { return syscall(SYS_write, fd, buf, count); }
static inline int close(int fd) { return (int)syscall(SYS_close, fd); }
static inline off_t lseek(int fd, off_t offset, int whence) { return (off_t)syscall(SYS_lseek, fd, offset, whence); }

/* File descriptors */
static inline int dup(int fd) { return (int)syscall(SYS_dup, fd); }
static inline int dup2(int oldfd, int newfd) { return (int)syscall(SYS_dup2, oldfd, newfd); }
static inline int pipe(int pipefd[2]) {
	#if JACL_HASSYS(pipe2)
		return (int)syscall(SYS_pipe2, pipefd, 0);
	#elif JACL_HASSYS(pipe)
		return (int)syscall(SYS_pipe, pipefd);
	#else
		errno = ENOSYS; return -1;
	#endif
}

/* File operations */
static inline int access(const char *pathname, int mode) { return (int)syscall(SYS_access, pathname, mode); }
static inline int unlink(const char *pathname) { return (int)syscall(SYS_unlink, pathname); }
static inline int rmdir(const char *pathname) { return (int)syscall(SYS_rmdir, pathname); }
static inline int link(const char *oldpath, const char *newpath) { return (int)syscall(SYS_link, oldpath, newpath); }
static inline int symlink(const char *target, const char *linkpath) { return (int)syscall(SYS_symlink, target, linkpath); }
static inline ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) { return syscall(SYS_readlink, pathname, buf, bufsiz); }
static inline int ftruncate(int fd, off_t length) { return (int)syscall(SYS_ftruncate, fd, length); }
static inline int truncate(const char *path, off_t length) { return (int)syscall(SYS_truncate, path, length); }
static inline int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath) { return (int)syscall(SYS_renameat, olddirfd, oldpath, newdirfd, newpath); }

/* Directory operations */
static inline int chdir(const char *path) { return (int)syscall(SYS_chdir, path); }
#if JACL_OS_DARWIN
/* Darwin has no SYS_getcwd - use fcntl F_GETPATH on current directory fd */
static inline char *getcwd(char *buf, size_t size) {
	int fd = (int)syscall(SYS_open, ".", 0, 0);
	if (JACL_UNLIKELY(fd < 0)) return NULL;
	int result = (int)syscall(SYS_fcntl, fd, 50 /* F_GETPATH */, buf); /* F_GETPATH = 50 on Darwin */
	syscall(SYS_close, fd);
	if (JACL_UNLIKELY(result < 0)) return NULL;
	return buf;
}
#else
static inline char *getcwd(char *buf, size_t size) { return (char*)syscall(SYS_getcwd, buf, size); }
#endif

/* Process execution */
static inline int execve(const char *pathname, char *const argv[], char *const envp[]) { return (int)syscall(SYS_execve, pathname, argv, envp); }
static inline int execv(const char *pathname, char *const argv[]) { return (int)syscall(SYS_execve, pathname, argv, environ); }
static inline int execvp(const char *file, char *const argv[]) { return (int)syscall(SYS_execve, file, argv, environ); }
static inline int execl(const char *pathname, const char *arg, ...) {
	const char *argv[256];
	va_list ap;
	int i = 0;

	argv[i++] = arg;

	va_start(ap, arg);

	while (i < 255 && (argv[i] = va_arg(ap, const char*)) != NULL) i++;

	va_end(ap);

	argv[i] = NULL;

	return execve(pathname, (char *const*)argv, environ);
}
static inline int execlp(const char *file, const char *arg, ...) {
	const char *argv[256];
	va_list ap;
	int i = 0;

	argv[i++] = arg;

	va_start(ap, arg);

	while (i < 255 && (argv[i] = va_arg(ap, const char*)) != NULL) i++;

	va_end(ap);

	argv[i] = NULL;

	return execvp(file, (char *const*)argv);
}
static inline int execle(const char *pathname, const char *arg, ...) {
	const char *argv[256];
	char *const *envp;
	va_list ap;
	int i = 0;

	argv[i++] = arg;
	va_start(ap, arg);
	while (i < 255 && (argv[i] = va_arg(ap, const char*)) != NULL) i++;
	envp = va_arg(ap, char *const*);
	va_end(ap);
	argv[i] = NULL;

	return execve(pathname, (char *const*)argv, envp);
}

/* User/Group IDs */
static inline uid_t getuid(void) { return (uid_t)syscall(SYS_getuid); }
static inline gid_t getgid(void) { return (gid_t)syscall(SYS_getgid); }
static inline uid_t geteuid(void) { return (uid_t)syscall(SYS_geteuid); }
static inline gid_t getegid(void) { return (gid_t)syscall(SYS_getegid); }
static inline int setuid(uid_t uid) { return (int)syscall(SYS_setuid, uid); }
static inline int setgid(gid_t gid) { return (int)syscall(SYS_setgid, gid); }
static inline int getgroups(int size, gid_t list[]) { return (int)syscall(SYS_getgroups, size, list); }

/* Process groups & sessions */
static inline pid_t getpgrp(void) { return (pid_t)syscall(SYS_getpgrp); }
static inline int setpgid(pid_t pid, pid_t pgid) { return (int)syscall(SYS_setpgid, pid, pgid); }
static inline pid_t setsid(void) { return (pid_t)syscall(SYS_setsid); }

/* Hostname */
static inline int gethostname(char *name, size_t len) {
	#if JACL_HASSYS(gethostname)
		return (int)syscall(SYS_gethostname, name, len);
	#else
		errno = ENOSYS;

		return -1;
	#endif
}
static inline int sethostname(const char *name, size_t len) {
	#if JACL_HASSYS(sethostname)
		return (int)syscall(SYS_sethostname, name, len);
	#else
		errno = ENOSYS;

		return -1;
	#endif
}

/* File properties */
static inline int isatty(int fd) {
	#if JACL_HASSYS(ioctl) && defined(TIOCGWINSZ)
		struct winsize ws;
		return syscall(SYS_ioctl, fd, TIOCGWINSZ, &ws) + 1;
	#else
		errno = ENOSYS;
		return 0;
	#endif
}

/* File ownership */
static inline int chown(const char *pathname, uid_t owner, gid_t group) { return (int)syscall(SYS_chown, pathname, owner, group); }
static inline int fchown(int fd, uid_t owner, gid_t group) { return (int)syscall(SYS_fchown, fd, owner, group); }
static inline int lchown(const char *pathname, uid_t owner, gid_t group) { return (int)syscall(SYS_lchown, pathname, owner, group); }
static inline int fchdir(int fd) { return (int)syscall(SYS_fchdir, fd); }

/* File synchronization */
static inline void sync(void) { syscall(SYS_sync); }
static inline int fsync(int fd) { return (int)syscall(SYS_fsync, fd); }
static inline int fdatasync(int fd) {
	#if JACL_HASSYS(fdatasync)
		return (int)syscall(SYS_fdatasync, fd);
	#else
		return fsync(fd);
	#endif
}

/* Sleep/alarm */
static inline unsigned int sleep(unsigned int seconds) {
	#if JACL_HASSYS(nanosleep)
		struct timespec req = {seconds, 0}, rem = {0, 0};

		if (syscall(SYS_nanosleep, &req, &rem) == -1) {
			return (unsigned int)rem.tv_sec + (rem.tv_nsec > 0 ? 1 : 0);
		}

		return 0;
	#else
		errno = ENOSYS;
		return seconds;
	#endif
}
static inline int usleep(useconds_t usec) {
	#if JACL_HASSYS(nanosleep)
		struct timespec req = {usec / 1000000, (usec % 1000000) * 1000};

		return (int)syscall(SYS_nanosleep, &req, NULL);
	#else
		errno = ENOSYS;
		return -1;
	#endif
}
static inline unsigned int alarm(unsigned int seconds) {
	#if JACL_HASSYS(alarm)
		return (unsigned int)syscall(SYS_alarm, seconds);
	#else
		errno = ENOSYS; return 0;
	#endif
}
static inline int pause(void) {
	#if JACL_HASSYS(pause)
		return (int)syscall(SYS_pause);
	#else
		errno = ENOSYS; return -1;
	#endif
}

#else /* !JACL_HAS_POSIX */

/* Process control */
static inline noreturn void _exit(int status) { (void)status; while(1); }
static inline pid_t getpid(void) { return 1; }
static inline pid_t getppid(void) { return 1; }
static inline pid_t fork(void) { errno = ENOSYS; return -1; }
static inline pid_t vfork(void) { return fork(); }

/* File I/O */
static inline ssize_t read(int fd, void *buf, size_t count) { (void)fd; (void)buf; (void)count; errno = ENOSYS; return -1; }
static inline ssize_t write(int fd, const void *buf, size_t count) { (void)fd; (void)buf; (void)count; errno = ENOSYS; return -1; }
static inline int close(int fd) { (void)fd; errno = ENOSYS; return -1; }
static inline off_t lseek(int fd, off_t offset, int whence) { (void)fd; (void)offset; (void)whence; errno = ENOSYS; return (off_t)-1; }

/* File descriptors */
static inline int dup(int fd) { (void)fd; errno = ENOSYS; return -1; }
static inline int dup2(int oldfd, int newfd) { (void)oldfd; (void)newfd; errno = ENOSYS; return -1; }
static inline int pipe(int pipefd[2]) { (void)pipefd; errno = ENOSYS; return -1; }

/* File operations */
static inline int access(const char *pathname, int mode) { (void)pathname; (void)mode; errno = ENOSYS; return -1; }
static inline int unlink(const char *pathname) { (void)pathname; errno = ENOSYS; return -1; }
static inline int rmdir(const char *pathname) { (void)pathname; errno = ENOSYS; return -1; }
static inline int link(const char *oldpath, const char *newpath) { (void)oldpath; (void)newpath; errno = ENOSYS; return -1; }
static inline int symlink(const char *target, const char *linkpath) { (void)target; (void)linkpath; errno = ENOSYS; return -1; }
static inline ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) { (void)pathname; (void)buf; (void)bufsiz; errno = ENOSYS; return -1; }
static inline int ftruncate(int fd, off_t length) { (void)fd; (void)length; errno = ENOSYS; return -1; }
static inline int truncate(const char *path, off_t length) { (void)path; (void)length; errno = ENOSYS; return -1; }

/* Directory operations */
static inline int chdir(const char *path) { (void)path; errno = ENOSYS; return -1; }
static inline char *getcwd(char *buf, size_t size) { (void)buf; (void)size; errno = ENOSYS; return NULL; }

/* Process execution */
static inline int execve(const char *pathname, char *const argv[], char *const envp[]) { (void)pathname; (void)argv; (void)envp; errno = ENOSYS; return -1; }
static inline int execv(const char *pathname, char *const argv[]) { (void)pathname; (void)argv; errno = ENOSYS; return -1; }
static inline int execvp(const char *file, char *const argv[]) { (void)file; (void)argv; errno = ENOSYS; return -1; }
static inline int execl(const char *pathname, const char *arg, ...) { (void)pathname; (void)arg; errno = ENOSYS; return -1; }
static inline int execlp(const char *file, const char *arg, ...) { (void)file; (void)arg; errno = ENOSYS; return -1; }
static inline int execle(const char *pathname, const char *arg, ...) { (void)pathname; (void)arg; errno = ENOSYS; return -1; }

/* User/Group IDs */
static inline uid_t getuid(void) { return 0; }
static inline gid_t getgid(void) { return 0; }
static inline uid_t geteuid(void) { return 0; }
static inline gid_t getegid(void) { return 0; }
static inline int setuid(uid_t uid) { (void)uid; errno = ENOSYS; return -1; }
static inline int setgid(gid_t gid) { (void)gid; errno = ENOSYS; return -1; }
static inline int getgroups(int size, gid_t list[]) { (void)size; (void)list; errno = ENOSYS; return -1; }

/* Process groups & sessions */
static inline pid_t getpgrp(void) { return 1; }
static inline int setpgid(pid_t pid, pid_t pgid) { (void)pid; (void)pgid; errno = ENOSYS; return -1; }
static inline pid_t setsid(void) { errno = ENOSYS; return -1; }

static inline int gethostname(char *name, size_t len) { (void)name; (void)len; errno = ENOSYS; return -1; }
static inline int sethostname(const char *name, size_t len) { (void)name; (void)len; errno = ENOSYS; return -1; }

/* File properties */
static inline int isatty(int fd) { (void)fd; errno = ENOSYS; return 0; }

/* File ownership */
static inline int chown(const char *pathname, uid_t owner, gid_t group) { (void)pathname; (void)owner; (void)group; errno = ENOSYS; return -1; }
static inline int fchown(int fd, uid_t owner, gid_t group) { (void)fd; (void)owner; (void)group; errno = ENOSYS; return -1; }
static inline int lchown(const char *pathname, uid_t owner, gid_t group) { (void)pathname; (void)owner; (void)group; errno = ENOSYS; return -1; }
static inline int fchdir(int fd) { (void)fd; errno = ENOSYS; return -1; }

/* File synchronization */
static inline void sync(void) { /* noop */ }
static inline int fsync(int fd) { (void)fd; errno = ENOSYS; return -1; }
static inline int fdatasync(int fd) { (void)fd; errno = ENOSYS; return -1; }

/* Sleep/alarm */
static inline unsigned int sleep(unsigned int seconds) { return seconds; }
static inline int usleep(useconds_t usec) { (void)usec; errno = ENOSYS; return -1; }
static inline unsigned int alarm(unsigned int seconds) { (void)seconds; errno = ENOSYS; return 0; }
static inline int pause(void) { errno = ENOSYS; return -1; }

/* System configuration */
static inline long sysconf(int name) { (void)name; errno = ENOSYS; return -1; }
static inline long pathconf(const char *path, int name) { (void)path; (void)name; errno = ENOSYS; return -1; }
static inline long fpathconf(int fd, int name) { (void)fd; (void)name; errno = ENOSYS; return -1; }
static inline size_t confstr(int name, char *buf, size_t len) { (void)name; (void)buf; (void)len; errno = ENOSYS; return 0; }

#endif /* JACL_HAS_POSIX */

#ifdef __cplusplus
}
#endif

#endif /* UNISTD_H */
