/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef POLL_H
#define POLL_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>   // sigset_t, SIG_SETMASK
#include <time.h>     // struct timespec

#if JACL_OS_WINDOWS
	#define POLL_WIN32 1
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <io.h>
#elif JACL_ARCH_WASM
	#define POLL_DUMMY 1
#else
	#define POLL_POSIX 1
	#include <unistd.h>
	#include <pthread.h>
	#include <sys/stat.h>
	#include <sys/syscall.h>
#endif

#define APPEND_EVENT(flag, name) \
	if ((events & (flag)) && pos < bufsize - 1) { \
		if (pos > 0 && pos < bufsize - 1) { buf[pos++] = '|'; } \
		const char *n = (name); \
		while (*n && pos < bufsize - 1) { buf[pos++] = *n++; } \
		buf[pos] = '\0'; \
	}

/* ================================================================ */
/* Types                                                            */
/* ================================================================ */

typedef unsigned long nfds_t;

struct pollfd {
	int fd;         /* File descriptor */
	short events;   /* Requested events */
	short revents;  /* Returned events */
};

/* ================================================================ */
/* Event flag constants                                             */
/* ================================================================ */

#define POLLIN      0x0001  /* Data other than high-priority data may be read */
#define POLLPRI     0x0002  /* High priority data may be read without blocking */
#define POLLOUT     0x0004  /* Normal data may be written without blocking */
#define POLLERR     0x0008  /* An error has occurred */

#define POLLHUP     0x0010  /* Device has been disconnected */
#define POLLNVAL    0x0020  /* Invalid fd member */
#define POLLRDNORM  0x0040  /* Normal data may be read without blocking */
#define POLLRDBAND  0x0080  /* Priority data may be read without blocking */

#define POLLWRNORM  0x0100  /* Equivalent to POLLOUT */
#define POLLWRBAND  0x0200  /* Priority data may be written */

/* Platform extensions */
#define POLLRDHUP   0x2000  /* Linux: stream socket peer closed */
#define POLLMSG     0x0800  /* Linux: message available */
#define POLLREMOVE  0x1000  /* Linux: remove from monitoring */

#define INFTIM      (-1)    /* Infinite timeout */

/* ================================================================ */
/* Windows file descriptor type detection                          */
/* ================================================================ */

#if POLL_WIN32

typedef enum {
	WIN_FD_INVALID,
	WIN_FD_SOCKET,
	WIN_FD_PIPE,
	WIN_FD_FILE,
	WIN_FD_CONSOLE
} win_fd_type_t;

static inline win_fd_type_t win32_get_fd_type(int fd) {
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	if (h == INVALID_HANDLE_VALUE) return WIN_FD_INVALID;

	DWORD type = GetFileType(h);
	switch (type) {
		case FILE_TYPE_PIPE:
			/* Could be named pipe or anonymous pipe */
			return WIN_FD_PIPE;
		case FILE_TYPE_DISK:
			return WIN_FD_FILE;
		case FILE_TYPE_CHAR:
			return WIN_FD_CONSOLE;
		default:
			/* Test if it's a socket */
			int sock_type;
			int sock_type_len = sizeof(sock_type);
			if (getsockopt((SOCKET)h, SOL_SOCKET, SO_TYPE, (char*)&sock_type, &sock_type_len) == 0) {
				return WIN_FD_SOCKET;
			}
			return WIN_FD_INVALID;
	}
}

static inline int win32_poll_pipe(HANDLE h, short events, int timeout_ms) {
	short revents = 0;

	if (events & (POLLIN | POLLRDNORM)) {
		DWORD bytes_avail = 0;
		DWORD total_bytes = 0;

		/* Use PeekNamedPipe to check if data is available */
		if (PeekNamedPipe(h, NULL, 0, NULL, &bytes_avail, &total_bytes)) {
			if (bytes_avail > 0) {
				revents |= POLLIN;
			} else if (timeout_ms > 0) {
				/* Wait for data to become available */
				DWORD wait_result = WaitForSingleObject(h, (DWORD)timeout_ms);
				if (wait_result == WAIT_OBJECT_0) {
					/* Check again after wait */
					if (PeekNamedPipe(h, NULL, 0, NULL, &bytes_avail, &total_bytes) && bytes_avail > 0) {
						revents |= POLLIN;
					}
				} else if (wait_result == WAIT_TIMEOUT) {
					return 0; /* Timeout, no events */
				}
			}
		} else {
			/* PeekNamedPipe failed - pipe might be broken */
			DWORD error = GetLastError();
			if (error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA) {
				revents |= POLLHUP;
			} else {
				revents |= POLLERR;
			}
		}
	}

	if (events & (POLLOUT | POLLWRNORM)) {
		/* For pipes, we assume write is always ready (simplification) */
		/* A more sophisticated implementation would use overlapped I/O */
		revents |= POLLOUT;
	}

	return revents;
}

static inline int win32_poll_file(HANDLE h, short events) {
	(void)h;
	short revents = 0;

	/* Regular files are always ready for reading and writing */
	if (events & (POLLIN | POLLRDNORM)) revents |= POLLIN;
	if (events & (POLLOUT | POLLWRNORM)) revents |= POLLOUT;

	return revents;
}

#endif

/* ================================================================ */
/* Function implementations                                         */
/* ================================================================ */

#if POLL_WIN32

/* ================================================================ */
/* Enhanced Windows implementation                                  */
/* ================================================================ */

static inline int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	if (!fds && nfds > 0) {
		errno = EINVAL;
		return -1;
	}

	if (nfds == 0) {
		if (timeout > 0) {
			Sleep((DWORD)timeout);
		} else if (timeout == 0) {
			return 0;
		} else {
			Sleep(INFINITE);
		}
		return 0;
	}

	/* Separate different file descriptor types */
	nfds_t socket_count = 0;
	nfds_t other_count = 0;
	int ready_count = 0;

	/* First pass: classify file descriptors and handle immediate cases */
	for (nfds_t i = 0; i < nfds; i++) {
		fds[i].revents = 0;

		if (fds[i].fd < 0) {
			fds[i].revents = POLLNVAL;
			ready_count++;
			continue;
		}

		win_fd_type_t fd_type = win32_get_fd_type(fds[i].fd);

		switch (fd_type) {
			case WIN_FD_SOCKET:
				socket_count++;
				break;

			case WIN_FD_FILE:
				/* Regular files are always ready */
				fds[i].revents = win32_poll_file((HANDLE)_get_osfhandle(fds[i].fd), fds[i].events);
				if (fds[i].revents) ready_count++;
				break;

			case WIN_FD_PIPE:
				other_count++;
				break;

			case WIN_FD_CONSOLE:
				/* Console input/output - simplified handling */
				if (fds[i].events & (POLLIN | POLLRDNORM)) {
					/* Check if console input is available */
					HANDLE h = (HANDLE)_get_osfhandle(fds[i].fd);
					DWORD events_count;
					if (GetNumberOfConsoleInputEvents(h, &events_count) && events_count > 1) {
						fds[i].revents |= POLLIN;
					}
				}
				if (fds[i].events & (POLLOUT | POLLWRNORM)) {
					fds[i].revents |= POLLOUT; /* Console output usually ready */
				}
				if (fds[i].revents) ready_count++;
				break;

			default:
				fds[i].revents = POLLNVAL;
				ready_count++;
				break;
		}
	}

	/* Handle sockets using WSAPoll if available */
	if (socket_count > 0) {
		/* Try WSAPoll first */
		typedef int (WSAAPI *WSAPollFunc)(LPWSAPOLLFD fdArray, ULONG fds, INT timeout);

		HMODULE ws2_32 = GetModuleHandleA("ws2_32.dll");
		WSAPollFunc WSAPollPtr = NULL;

		if (ws2_32) {
			WSAPollPtr = (WSAPollFunc)GetProcAddress(ws2_32, "WSAPoll");
		}

		if (WSAPollPtr) {
			/* Use WSAPoll for sockets */
			WSAPOLLFD *wsafds = (WSAPOLLFD *)_alloca(socket_count * sizeof(WSAPOLLFD));
			nfds_t *socket_indices = (nfds_t *)_alloca(socket_count * sizeof(nfds_t));
			nfds_t sock_idx = 0;

			/* Build WSAPoll array */
			for (nfds_t i = 0; i < nfds; i++) {
				if (win32_get_fd_type(fds[i].fd) == WIN_FD_SOCKET) {
					socket_indices[sock_idx] = i;
					wsafds[sock_idx].fd = (SOCKET)_get_osfhandle(fds[i].fd);
					wsafds[sock_idx].events = 0;
					wsafds[sock_idx].revents = 0;

					if (fds[i].events & (POLLIN | POLLRDNORM | POLLRDBAND)) {
						wsafds[sock_idx].events |= POLLRDNORM | POLLRDBAND;
					}
					if (fds[i].events & POLLOUT) wsafds[sock_idx].events |= POLLWRNORM;
					if (fds[i].events & POLLPRI) wsafds[sock_idx].events |= POLLPRI;

					sock_idx++;
				}
			}

			int sock_timeout = (ready_count > 0 || other_count > 0) ? 0 : timeout;
			int sock_result = WSAPollPtr(wsafds, (ULONG)socket_count, sock_timeout);

			if (sock_result > 0) {
				/* Convert results back */
				for (nfds_t i = 0; i < socket_count; i++) {
					nfds_t orig_idx = socket_indices[i];
					if (wsafds[i].revents & (POLLRDNORM | POLLRDBAND)) {
						fds[orig_idx].revents |= POLLIN;
					}
					if (wsafds[i].revents & POLLWRNORM) fds[orig_idx].revents |= POLLOUT;
					if (wsafds[i].revents & POLLPRI) fds[orig_idx].revents |= POLLPRI;
					if (wsafds[i].revents & POLLERR) fds[orig_idx].revents |= POLLERR;
					if (wsafds[i].revents & POLLHUP) fds[orig_idx].revents |= POLLHUP;
					if (wsafds[i].revents & POLLNVAL) fds[orig_idx].revents |= POLLNVAL;

					if (fds[orig_idx].revents != 0) ready_count++;
				}
			} else if (sock_result < 0) {
				errno = EINVAL;
				return -1;
			}
		}
	}

	/* Handle pipes and other special file types */
	if (other_count > 0) {
		int pipe_timeout = (ready_count > 0) ? 0 : timeout;

		for (nfds_t i = 0; i < nfds; i++) {
			if (fds[i].revents != 0) continue; /* Already handled */

			if (win32_get_fd_type(fds[i].fd) == WIN_FD_PIPE) {
				HANDLE h = (HANDLE)_get_osfhandle(fds[i].fd);
				short pipe_revents = win32_poll_pipe(h, fds[i].events, pipe_timeout);
				fds[i].revents = pipe_revents;
				if (pipe_revents != 0) ready_count++;
			}
		}
	}

	return ready_count;
}

static inline int ppoll(struct pollfd fds[], nfds_t nfds, const struct timespec *timeout_ts, const sigset_t *sigmask) {
	(void)sigmask; /* Signal masking not supported on Windows */

	int timeout_ms = -1;
	if (timeout_ts) {
		timeout_ms = (int)(timeout_ts->tv_sec * 1000 + timeout_ts->tv_nsec / 1000000);
	}

	return poll(fds, nfds, timeout_ms);
}

#elif POLL_DUMMY

/* ================================================================ */
/* WebAssembly - Polling not supported                              */
/* ================================================================ */

static inline int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	(void)fds; (void)nfds; (void)timeout;
	errno = ENOSYS;
	return -1;
}

static inline int ppoll(struct pollfd fds[], nfds_t nfds, const struct timespec *timeout_ts, const sigset_t *sigmask) {
	(void)fds; (void)nfds; (void)timeout_ts; (void)sigmask;
	errno = ENOSYS;
	return -1;
}

#else

/* ================================================================ */
/* Enhanced POSIX implementation                                    */
/* ================================================================ */

static inline int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	if (nfds == 0) return 0;
	if (!fds && nfds > 0) { errno = EINVAL; return -1; }

	/* All POSIX platforms have poll syscall */
	return (int)syscall(SYS_poll, fds, nfds, timeout);
}

static inline int ppoll(struct pollfd fds[], nfds_t nfds, const struct timespec *timeout_ts, const sigset_t *sigmask) {
	if (nfds == 0) return 0;

	if (!fds && nfds > 0) {
		errno = EINVAL;
		return -1;
	}

	#if JACL_OS_LINUX
		/* Linux requires sigsetsize as 5th argument */
		size_t sigsetsize = sigmask ? 8 : 0;
		return (int)syscall(SYS_ppoll, fds, nfds, timeout_ts, sigmask, sigsetsize);
	#elif JACL_OS_FREEBSD || JACL_OS_NETBSD || JACL_OS_DRAGONFLY
		/* BSD systems have ppoll (4 args) */
		return (int)syscall(SYS_ppoll, fds, nfds, timeout_ts, sigmask);
	#else
		/* Darwin and OPENBSD: use poll + pthread_sigmask fallback */
		sigset_t old_sigmask;
		int timeout_ms = -1;

		if (timeout_ts) timeout_ms = (int)(timeout_ts->tv_sec * 1000 + timeout_ts->tv_nsec / 1000000);
		if (sigmask && pthread_sigmask(SIG_SETMASK, sigmask, &old_sigmask) != 0) return -1;

		int result = poll(fds, nfds, timeout_ms);

		if (sigmask) pthread_sigmask(SIG_SETMASK, &old_sigmask, NULL);

		return result;
	#endif
}


#endif

/* ================================================================ */
/* Enhanced utility functions                                       */
/* ================================================================ */

static inline void pollfd_init(struct pollfd *pfd, int fd, short events) {
	if (pfd) {
		pfd->fd = fd;
		pfd->events = events;
		pfd->revents = 0;
	}
}

static inline int pollfd_has_input(const struct pollfd *pfd) {
	return pfd && (pfd->revents & (POLLIN | POLLRDNORM | POLLRDBAND));
}

static inline int pollfd_has_output(const struct pollfd *pfd) {
	return pfd && (pfd->revents & (POLLOUT | POLLWRNORM));
}

static inline int pollfd_has_error(const struct pollfd *pfd) {
	return pfd && (pfd->revents & (POLLERR | POLLHUP | POLLNVAL));
}

static inline int pollfd_has_priority(const struct pollfd *pfd) {
	return pfd && (pfd->revents & POLLPRI);
}

static inline int pollfd_has_hangup(const struct pollfd *pfd) {
	return pfd && (pfd->revents & POLLHUP);
}

// Enhanced validation with better error messages
static inline int poll_validate(const struct pollfd fds[], nfds_t nfds) {
	if (!fds && nfds > 0) {
		errno = EINVAL;
		return -1;
	}

	/* Check for obviously invalid file descriptors */
	for (nfds_t i = 0; i < nfds; i++) {
		if (fds[i].fd < -1) {
			errno = EINVAL;
			return -1;
		}

		/* Check for invalid event flags */
		short valid_events = POLLIN | POLLOUT | POLLPRI | POLLERR | POLLHUP | POLLNVAL |
		                     POLLRDNORM | POLLRDBAND | POLLWRNORM | POLLWRBAND;

		#if JACL_OS_LINUX
			valid_events |= POLLRDHUP | POLLMSG | POLLREMOVE;
		#endif

		if (fds[i].events & ~valid_events) { errno = EINVAL; return -1; }
	}

	return 0;
}

// Thread-safe event string conversion
static inline int poll_events_to_string_r(short events, char *buf, size_t bufsize) {
	if (!buf || bufsize == 0) return -1;

	if (events == 0) {
		snprintf(buf, bufsize, "(none)");
		return 0;
	}

	buf[0] = '\0';
	size_t pos = 0;

	APPEND_EVENT(POLLIN, "POLLIN");
	APPEND_EVENT(POLLOUT, "POLLOUT");
	APPEND_EVENT(POLLPRI, "POLLPRI");
	APPEND_EVENT(POLLERR, "POLLERR");
	APPEND_EVENT(POLLHUP, "POLLHUP");
	APPEND_EVENT(POLLNVAL, "POLLNVAL");
	APPEND_EVENT(POLLRDNORM, "POLLRDNORM");
	APPEND_EVENT(POLLRDBAND, "POLLRDBAND");
	APPEND_EVENT(POLLWRNORM, "POLLWRNORM");
	APPEND_EVENT(POLLWRBAND, "POLLWRBAND");

	#if JACL_OS_LINUX
		APPEND_EVENT(POLLRDHUP, "POLLRDHUP");
	#endif

	return 0;
}

// Backwards compatibility (non-thread-safe)
static inline const char *poll_events_to_string(short events) {
	static char buf[256];
	poll_events_to_string_r(events, buf, sizeof(buf));
	return buf;
}

#undef APPEND_EVENT

#ifdef __cplusplus
}
#endif

#endif // POLL_H
