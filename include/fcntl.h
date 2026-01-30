/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef FCNTL_H
#define FCNTL_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <config.h>
#include <sys/types.h>
#include <stdarg.h>

#if JACL_OS_WINDOWS
	#define FCNTL_WIN32 1
	#include <windows.h>
	#include <io.h>
#else
	#define FCNTL_POSIX 1
	#include <sys/syscall.h>
#endif

/* ================================================================ */
/* File access modes (mutually exclusive)                          */
/* ================================================================ */

#define O_RDONLY    0x0000  /* Open for reading only */
#define O_WRONLY    0x0001  /* Open for writing only */
#define O_RDWR      0x0002  /* Open for reading and writing */
#define O_SEARCH    0x0003  /* Open directory for search only (POSIX.1-2008) */
#define O_EXEC      0x0003  /* Open for execute only (POSIX.1-2008) */
#define O_ACCMODE   0x0003  /* Mask for file access modes */

/* ================================================================ */
/* File creation flags                                              */
/* ================================================================ */

#define O_CREAT     0x0040  /* Create file if it doesn't exist */
#define O_EXCL      0x0080  /* Fail if file exists (with O_CREAT) */
#define O_NOCTTY    0x0100  /* Don't assign controlling terminal */
#define O_TRUNC     0x0200  /* Truncate file to zero length */

/* POSIX.1-2008 additions */
#define O_CLOEXEC   0x0400  /* Set FD_CLOEXEC on new FD */
#define O_DIRECTORY 0x0800  /* Must be a directory */
#define O_NOFOLLOW  0x1000  /* Don't follow symbolic links */
#define O_TTY_INIT  0x2000  /* Initialize terminal parameters */

/* ================================================================ */
/* File status flags                                                */
/* ================================================================ */

#define O_APPEND    0x4000  /* Append on each write */
#define O_NONBLOCK  0x8000  /* Non-blocking mode */
#define O_NDELAY    O_NONBLOCK  /* BSD compatibility */
#define O_SYNC      0x10000 /* Synchronous writes */
#define O_DSYNC     0x20000 /* Synchronous data writes */
#define O_RSYNC     0x40000 /* Synchronous reads */

/* Linux-specific flags */
#if JACL_OS_LINUX
#define O_DIRECT    0x80000  /* Direct I/O */
#define O_NOATIME   0x100000 /* Don't update access time */
#define O_PATH      0x200000 /* Path only (no file access) */
#define O_TMPFILE   0x400000 /* Create temporary file */
#define O_LARGEFILE 0x800000 /* Large file support */
#endif

/* ================================================================ */
/* fcntl() commands                                                 */
/* ================================================================ */

#define F_DUPFD     0   /* Duplicate file descriptor */
#define F_GETFD     1   /* Get file descriptor flags */
#define F_SETFD     2   /* Set file descriptor flags */
#define F_GETFL     3   /* Get file status flags */
#define F_SETFL     4   /* Set file status flags */

/* POSIX record locks */
#define F_GETLK     5   /* Get record locking information */
#define F_SETLK     6   /* Set record locking information */
#define F_SETLKW    7   /* Set record locking information (wait) */

/* POSIX.1-2008 additions */
#define F_DUPFD_CLOEXEC 1030  /* Duplicate FD with O_CLOEXEC */

/* Open file description locks (Linux) */
#if JACL_OS_LINUX
#define F_OFD_GETLK  36  /* Get OFD lock */
#define F_OFD_SETLK  37  /* Set OFD lock */
#define F_OFD_SETLKW 38  /* Set OFD lock (wait) */
#endif

/* Owner management */
#define F_GETOWN    9   /* Get owner for SIGIO */
#define F_SETOWN    8   /* Set owner for SIGIO */
#define F_GETOWN_EX 16  /* Get owner (extended) */
#define F_SETOWN_EX 15  /* Set owner (extended) */

/* Signal management */
#define F_GETSIG    11  /* Get signal sent when I/O possible */
#define F_SETSIG    10  /* Set signal sent when I/O possible */

/* Linux-specific extensions */
#if JACL_OS_LINUX
#define F_SETLEASE  1024  /* Set lease */
#define F_GETLEASE  1025  /* Get lease */
#define F_NOTIFY    1026  /* Notify on directory changes */
#define F_SETPIPE_SZ 1031 /* Set pipe buffer size */
#define F_GETPIPE_SZ 1032 /* Get pipe buffer size */

/* File sealing (memfd) */
#define F_ADD_SEALS 1033  /* Add seals */
#define F_GET_SEALS 1034  /* Get seals */
#endif

/* ================================================================ */
/* File descriptor flags                                            */
/* ================================================================ */

#define FD_CLOEXEC  1   /* Close on exec */
#define FD_CLOFORK  2   /* Close on fork (POSIX.1-2008) */

/* ================================================================ */
/* File locking types                                               */
/* ================================================================ */

#define F_RDLCK     0   /* Shared (read) lock */
#define F_WRLCK     1   /* Exclusive (write) lock */
#define F_UNLCK     2   /* Unlock */

/* ================================================================ */
/* Owner types for F_*OWN_EX                                        */
/* ================================================================ */

#define F_OWNER_TID 0   /* Thread group */
#define F_OWNER_PID 1   /* Process */
#define F_OWNER_PGRP 2  /* Process group */

/* ================================================================ */
/* File sealing constants                                           */
/* ================================================================ */

#if JACL_OS_LINUX
#define F_SEAL_SEAL    0x0001  /* Prevent further sealing */
#define F_SEAL_SHRINK  0x0002  /* Prevent shrinking */
#define F_SEAL_GROW    0x0004  /* Prevent growing */
#define F_SEAL_WRITE   0x0008  /* Prevent writing */
#define F_SEAL_FUTURE_WRITE 0x0010  /* Prevent future writes */
#endif

/* ================================================================ */
/* AT_* constants for *at() functions                               */
/* ================================================================ */

#define AT_FDCWD            -100  /* Use current working directory */
#define AT_SYMLINK_NOFOLLOW 0x100 /* Don't follow symbolic links */
#define AT_SYMLINK_FOLLOW   0x400 /* Follow symbolic links */
#define AT_REMOVEDIR        0x200 /* Remove directory instead of file */
#define AT_EACCESS          0x200 /* Use effective IDs for access check */

/* ================================================================ */
/* POSIX advisory constants                                         */
/* ================================================================ */

#define POSIX_FADV_NORMAL     0  /* No special advice */
#define POSIX_FADV_RANDOM     1  /* Random access pattern */
#define POSIX_FADV_SEQUENTIAL 2  /* Sequential access pattern */
#define POSIX_FADV_WILLNEED   3  /* Will need this data soon */
#define POSIX_FADV_DONTNEED   4  /* Won't need this data soon */
#define POSIX_FADV_NOREUSE    5  /* Data will be accessed once */

/* ================================================================ */
/* Seek constants                                                   */
/* ================================================================ */

#ifndef SEEK_SET
#define SEEK_SET    0   /* Seek from beginning */
#define SEEK_CUR    1   /* Seek from current position */
#define SEEK_END    2   /* Seek from end */
#endif

#if JACL_OS_LINUX
#define SEEK_DATA   3   /* Seek to next data region */
#define SEEK_HOLE   4   /* Seek to next hole */
#endif

/* ================================================================ */
/* Structures                                                       */
/* ================================================================ */

/* Standard file locking structure */
struct flock {
	short l_type;    /* Type of lock: F_RDLCK, F_WRLCK, F_UNLCK */
	short l_whence;  /* Where l_start is relative to: SEEK_SET, SEEK_CUR, SEEK_END */
	off_t l_start;   /* Starting offset for lock */
	off_t l_len;     /* Number of bytes to lock (0 means to EOF) */
	pid_t l_pid;     /* PID of process blocking our lock (F_GETLK only) */
};

/* Large file support */
#if FCNTL_HAS_LFS
struct flock64 {
	short l_type;
	short l_whence;
	off64_t l_start;
	off64_t l_len;
	pid_t l_pid;
};
#endif

/* Extended owner structure for F_*OWN_EX */
struct f_owner_ex {
	int type;    /* F_OWNER_TID, F_OWNER_PID, or F_OWNER_PGRP */
	pid_t pid;   /* Process/thread/group ID */
};

/* ================================================================ */
/* Function implementations                                         */
/* ================================================================ */

#if FCNTL_WIN32
/* ================================================================ */
/* Windows implementation using Win32 APIs                         */
/* ================================================================ */

/* Convert POSIX open flags to Windows access/creation flags */
static inline void win32_convert_flags(int flags, DWORD *access, DWORD *creation, DWORD *attrs) {
	*access = 0;
	*creation = 0;
	*attrs = FILE_ATTRIBUTE_NORMAL;

	/* Access modes */
	switch (flags & O_ACCMODE) {
		case O_RDONLY:
		case O_SEARCH:
			*access = GENERIC_READ;
			break;
		case O_WRONLY:
			*access = GENERIC_WRITE;
			break;
		case O_RDWR:
			*access = GENERIC_READ | GENERIC_WRITE;
			break;
		case O_EXEC:
			*access = GENERIC_EXECUTE;
			break;
	}

	/* Creation flags */
	if (flags & O_CREAT) {
		if (flags & O_EXCL) {
			*creation = CREATE_NEW;      /* Fail if exists */
		} else if (flags & O_TRUNC) {
			*creation = CREATE_ALWAYS;   /* Create or truncate */
		} else {
			*creation = OPEN_ALWAYS;     /* Create or open */
		}
	} else {
		if (flags & O_TRUNC) {
			*creation = TRUNCATE_EXISTING; /* Truncate existing */
		} else {
			*creation = OPEN_EXISTING;     /* Open existing only */
		}
	}

	/* Directory flag */
	if (flags & O_DIRECTORY) {
		*attrs |= FILE_FLAG_BACKUP_SEMANTICS;
	}
}

static inline int open(const char *pathname, int flags, ...) {
	if (!pathname) return -1;

	DWORD access, creation, attrs;
	win32_convert_flags(flags, &access, &creation, &attrs);

	HANDLE hFile = CreateFileA(pathname, access,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		creation, attrs, NULL);

	if (hFile == INVALID_HANDLE_VALUE) return -1;

	/* Convert Windows HANDLE to integer file descriptor */
	int fd_flags = 0;
	if (flags & O_APPEND) fd_flags |= _O_APPEND;
	if (flags & O_RDONLY) fd_flags |= _O_RDONLY;

	int fd = _open_osfhandle((intptr_t)hFile, fd_flags);
	return fd;
}

static inline int openat(int dirfd, const char *pathname, int flags, ...) {
	/* Windows doesn't have openat, simulate if possible */
	(void)dirfd;
	if (!pathname) return -1;

	/* If pathname is absolute, ignore dirfd */
	if (pathname[0] == '/' || (pathname[0] && pathname[1] == ':')) {
		return open(pathname, flags);
	}

	/* Otherwise, not easily supported on Windows */
	return -1;
}

static inline int creat(const char *pathname, mode_t mode) {
	(void)mode; /* Windows doesn't use POSIX permissions */
	return open(pathname, O_CREAT | O_WRONLY | O_TRUNC, 0);
}

static inline int fcntl(int fd, int cmd, ...) {
	va_list args;
	va_start(args, cmd);

	switch (cmd) {
		case F_DUPFD: {
			int min_fd = va_arg(args, int);
			int new_fd = _dup(fd);

			va_end(args);

			return (new_fd >= min_fd) ? new_fd : -1;
		}

		case F_DUPFD_CLOEXEC: {
			int min_fd = va_arg(args, int);
			int new_fd = _dup(fd);

			va_end(args);

			/* Windows: Set inheritable to FALSE for CLOEXEC equivalent */
			if (new_fd >= min_fd) {
				HANDLE h = (HANDLE)_get_osfhandle(new_fd);
				SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0);

				return new_fd;
			}

			return -1;
		}

		case F_GETFD:
			va_end(args);

			/* Check if handle is inheritable */
			{
				HANDLE h = (HANDLE)_get_osfhandle(fd);
				DWORD flags;

				if (GetHandleInformation(h, &flags)) return (flags & HANDLE_FLAG_INHERIT) ? 0 : FD_CLOEXEC;
			}

			return 0;

		case F_SETFD: {
			int flags = va_arg(args, int);

			va_end(args);

			HANDLE h = (HANDLE)_get_osfhandle(fd);
			DWORD win_flags = (flags & FD_CLOEXEC) ? 0 : HANDLE_FLAG_INHERIT;

			return SetHandleInformation(h, HANDLE_FLAG_INHERIT, win_flags) ? 0 : -1;
		}

		case F_GETFL:
			va_end(args);

			/* Cannot easily determine original open flags on Windows */
			return 0;

		case F_SETFL: {
			int flags = va_arg(args, int);

			va_end(args);

			/* Limited flag setting support on Windows */
			(void)flags;

			return 0;
		}

		default:
			va_end(args);

			return -1;
	}
}

/* POSIX advisory functions - limited Windows support */
static inline int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
	(void)fd; (void)offset; (void)len; (void)advice;

	return 0; /* No-op on Windows */
}

static inline int posix_fallocate(int fd, off_t offset, off_t len) {
	(void)fd; (void)offset; (void)len;

	return -1; /* Not easily supported on Windows */
}

#else
/* ================================================================ */
/* POSIX/Linux implementation using system calls                   */
/* ================================================================ */

static inline int open(const char *pathname, int flags, ...) {
	if (!pathname) return -1;

	mode_t mode = 0;
	if (flags & O_CREAT) {
		va_list args;
		va_start(args, flags);
		mode = va_arg(args, mode_t);
		va_end(args);
	}

	return (int)syscall(SYS_open, pathname, flags, mode);
}

static inline int openat(int dirfd, const char *pathname, int flags, ...) {
	if (!pathname) return -1;

	mode_t mode = 0;
	if (flags & O_CREAT) {
		va_list args;
		va_start(args, flags);
		mode = va_arg(args, mode_t);
		va_end(args);
	}

	return (int)syscall(SYS_openat, dirfd, pathname, flags, mode);
}

static inline int creat(const char *pathname, mode_t mode) {
	return open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

static inline int fcntl(int fd, int cmd, ...) {
	va_list args;
	long arg = 0;
	struct flock *lock = NULL;
	struct f_owner_ex *owner = NULL;

	va_start(args, cmd);

	/* Get argument based on command type */
	switch (cmd) {
		case F_GETLK:
		case F_SETLK:
		case F_SETLKW:
#if JACL_OS_LINUX
		case F_OFD_GETLK:
		case F_OFD_SETLK:
		case F_OFD_SETLKW:
#endif
			lock = va_arg(args, struct flock *);
			arg = (long)lock;
			break;
		case F_GETOWN_EX:
		case F_SETOWN_EX:
			owner = va_arg(args, struct f_owner_ex *);
			arg = (long)owner;
			break;
		default:
			arg = va_arg(args, long);
			break;
	}

	va_end(args);

	return (int)syscall(SYS_fcntl, fd, cmd, arg);
}

/* POSIX advisory functions */
#if JACL_OS_DARWIN
/* Darwin has no fadvise/fallocate syscalls - these are no-ops */
static inline int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
	(void)fd; (void)offset; (void)len; (void)advice;
	return 0; /* Success - advice is optional */
}

static inline int posix_fallocate(int fd, off_t offset, off_t len) {
	/* Darwin: use ftruncate to extend file, best effort fallback */
	(void)offset;
	return (int)syscall(SYS_ftruncate, fd, offset + len);
}
#else
static inline int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
	return (int)syscall(SYS_fadvise64, fd, offset, len, advice);
}

static inline int posix_fallocate(int fd, off_t offset, off_t len) {
	return (int)syscall(SYS_fallocate, fd, 0, offset, len);
}
#endif

/* Large file support */
#if JACL_HAS_LFS
static inline int fcntl64(int fd, int cmd, ...) {
	va_list args;
	long arg = 0;
	struct flock64 *lock = NULL;

	va_start(args, cmd);

	switch (cmd) {
		case F_GETLK:
		case F_SETLK:
		case F_SETLKW:
			lock = va_arg(args, struct flock64 *);
			arg = (long)lock;
			break;
		default:
			arg = va_arg(args, long);
			break;
	}

	va_end(args);

	#if defined(SYS_fcntl64)
		return (int)syscall(SYS_fcntl64, fd, cmd, arg);
	#endif

	/* Fallback to regular fcntl */
	return fcntl(fd, cmd, arg);
}

static inline int posix_fadvise64(int fd, off64_t offset, off64_t len, int advice) {
	#if defined(SYS_fadvise64_64)
		return (int)syscall(SYS_fadvise64_64, fd, offset, len, advice);
	#else
		errno = ENOSYS;

		return -1;
	#endif
}


static inline int posix_fallocate64(int fd, off64_t offset, off64_t len) {
	#if defined(SYS_fallocate)
		return (int)syscall(SYS_fallocate, fd, 0, offset, len);
	#endif

	return posix_fallocate(fd, (off_t)offset, (off_t)len);
}
#endif /* JACL_HAS_LFS */

#endif

/* ================================================================ */
/* Helper functions                                                 */
/* ================================================================ */

/* Check if file descriptor is valid */
static inline int fcntl_valid_fd(int fd) {
	return fcntl(fd, F_GETFD) != -1;
}

/* Set close-on-exec flag */
static inline int fcntl_set_cloexec(int fd) {
	int flags = fcntl(fd, F_GETFD);
	if (flags == -1) return -1;
	return fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

/* Clear close-on-exec flag */
static inline int fcntl_clear_cloexec(int fd) {
	int flags = fcntl(fd, F_GETFD);
	if (flags == -1) return -1;
	return fcntl(fd, F_SETFD, flags & ~FD_CLOEXEC);
}

/* Set non-blocking mode */
static inline int fcntl_set_nonblock(int fd) {
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/* Clear non-blocking mode */
static inline int fcntl_clear_nonblock(int fd) {
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) return -1;
	return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

/* Duplicate file descriptor with CLOEXEC */
static inline int fcntl_dupfd_cloexec(int fd, int min_fd) {
	int new_fd = fcntl(fd, F_DUPFD_CLOEXEC, min_fd);
	if (new_fd != -1) return new_fd;

	/* Fallback for systems without F_DUPFD_CLOEXEC */
	new_fd = fcntl(fd, F_DUPFD, min_fd);
	if (new_fd == -1) return -1;

	if (fcntl_set_cloexec(new_fd) == -1) {
		syscall(SYS_close, new_fd);

		return -1;
	}

	return new_fd;
}

#ifdef __cplusplus
}
#endif
#endif /* FCNTL_H */

