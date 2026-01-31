/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef STDIO_H
#define STDIO_H
#pragma once

#include <config.h>

#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>

#if JACL_HAS_POSIX
#include <sys/wait.h>
#include <signal.h>
#endif

#if JACL_HAS_C23
#define __STDC_VERSION_STDIO_H__ 202311L
#endif

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

#ifndef OVRSIZ
#define OVRSIZ 32
#endif

#define OVRBUF (BUFSIZ + OVRSIZ)

#ifndef FILENAME_MAX
#define FILENAME_MAX 4096
#endif

#ifndef FOPEN_MAX
#define FOPEN_MAX 16
#endif

#ifndef TMP_MAX
#define TMP_MAX 32767
#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define _IOFBF 0  /* Full buffering */
#define _IOLBF 1  /* Line buffering */
#define _IONBF 2  /* No buffering */

#define __SRD    0x0001  /* read */
#define __SWR    0x0002  /* write */
#define __SEOF   0x0020  /* EOF */
#define __SERR   0x0040  /* error */

#ifndef EOF
#define EOF (-1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TEMP_DIR
#ifdef JACL_OS_WINDOWS
#define TEMP_DIR "C:\\Temp\\"
#else
#define TEMP_DIR "/tmp/"
#endif
#endif

#ifndef L_tmpnam
#define L_tmpnam 64
#endif

#ifdef __cplusplus
#define restrict __restrict__
#elif !JACL_HAS_C99
#define restrict	 /* nothing */
#endif

// main FILE type - cache optimized
typedef struct __jacl_file {
	// IO fields
	int _cnt;          // Characters remaining
	int _fd;           // File descriptor
	char *_ptr;        // Current position in buffer
	char *_end;        // End of buffer
	char *_base;       // Buffer base

	// Control fields
	unsigned char _flags;        // Status flags (__SRD, __SWR, __SEOF, __SERR)
	unsigned char _bufmode;      // Buffering mode (_IOFBF, _IOLBF, _IONBF)
	unsigned char _orientation;  // Stream orientation (0=unset, 1=byte, 2=wide)
	unsigned char _last_op;      // Last operation (0=none, 1=read, 2=write)
	unsigned int _buf_owned;     // 0=user buffer, 1=internal buffer

	// Position fields
	off_t _read_pos;   // File position of read buffer start
	off_t _write_pos;  // File position of write buffer start

	// File fields
	char *_tmpfname;   // Temp filename or pid (tmpfile/popen)

	// Buffer size fields
	size_t _bufsiz;    // Buffer size (excluding overflow)
	size_t _ovrsiz;    // Overflow/pushback size
} FILE;

// Stream Registry Helpers
void __jacl_stream_register(FILE *stream);
void __jacl_stream_unregister(FILE *stream);
int __jacl_stream_flush(FILE *stream);

// Stub for fpos_t (implementation-defined)
typedef long long fpos_t;

/* Standard Streams */
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

/* ============================================================= */
/* IO Initialization                                             */
/* ============================================================= */
extern int __jacl_stdio_init;
static inline void __jacl_init_stdio(void) {
	if (!__jacl_stdio_init) {
		__jacl_stream_register(stdin);
		__jacl_stream_register(stdout);
		__jacl_stream_register(stderr);
		stdout->_bufmode = isatty(stdout->_fd) ? _IOLBF : _IOFBF;
		__jacl_stdio_init = 1;
	}
}

/* ============================================================= */
/* File Macros                                                   */
/* ============================================================= */
#define FILE_INIT(fd, flags, mode, buf) { \
	0, fd, buf + OVRSIZ, buf + OVRBUF, buf, \
	flags, mode, 0, 0, 1, \
	0, 0, NULL, BUFSIZ, OVRSIZ \
}
#define STATIC_FILE(name, fd, flags, mode) \
static char __jacl_##name##_buf[OVRBUF]; \
static FILE __jacl_##name##_file = FILE_INIT(fd, flags, mode, __jacl_##name##_buf); \
FILE* name = &__jacl_##name##_file
#define SETUP_FILE(name, fd, flags, mode) FILE* name = __jacl_file_setup(fd, flags, mode);
static inline FILE* __jacl_file_setup(int fd, int flags, int mode) {
	FILE *f = (FILE *)malloc(sizeof(FILE));

	if (!f) { errno = ENOMEM; return NULL; }

	char *buf = malloc(OVRBUF);

	if (!buf) { free(f); errno = ENOMEM; return NULL; }

	*f = (FILE)FILE_INIT(fd, flags, mode, buf);

	__jacl_stream_register(f);

	return f;
}

/* ============================================================= */
/* Buffer Helpers                                                */
/* ============================================================= */
static inline int __jacl_buffer_flags(const char *mode) {
	int flags = 0;

	if (!mode || !*mode) return -1;
	switch (*mode) {
		case 'r': flags = O_RDONLY; break;
		case 'w': flags = O_WRONLY | O_CREAT | O_TRUNC; break;
		case 'a': flags = O_WRONLY | O_CREAT | O_APPEND; break;
		default: return -1;
	}
	if (strchr(mode, '+')) flags = (flags & ~O_WRONLY) | O_RDWR;

	#if JACL_HAS_C11 || (defined(O_EXCL) && JACL_HAS_POSIX)
		if (strchr(mode, 'x')) flags |= O_EXCL;
	#endif

	return flags;
}
static inline int __jacl_buffer_reload(FILE *f) {
	if (JACL_UNLIKELY(!f || !(f->_flags & __SRD))) return EOF;

	off_t cur_pos = lseek(f->_fd, 0, SEEK_CUR);

	if (JACL_UNLIKELY(cur_pos < 0)) { f->_flags |= __SERR; return EOF; }
	if (f->_cnt > 0) cur_pos -= f->_cnt;

	f->_ptr = f->_base + OVRSIZ;
	f->_cnt = 0;

	ssize_t n = read(f->_fd, f->_ptr, BUFSIZ);

	if (JACL_UNLIKELY(n < 0)) { f->_flags |= __SERR; return EOF; }
	if (JACL_UNLIKELY(n == 0)) { f->_flags |= __SEOF; return EOF; }

	f->_read_pos = cur_pos;
	f->_cnt = (int)n - 1;
	f->_last_op = 1;

	return (unsigned char)*f->_ptr++;
}
static inline int __jacl_buffer_flush(FILE *f) {
	if (JACL_UNLIKELY(!f || !(f->_flags & __SWR))) return 0;

	size_t to_write = f->_ptr - (f->_base + OVRSIZ);

	if (to_write == 0) return 0;

	ssize_t written = write(f->_fd, f->_base + OVRSIZ, to_write);

	if (JACL_UNLIKELY(written < 0)) { f->_flags |= __SERR; return EOF; }
	if ((size_t)written < to_write) {
		memmove(f->_base + OVRSIZ, f->_base + OVRSIZ + written, to_write - written);
		f->_ptr = f->_base + OVRSIZ + (to_write - written);
	} else f->_ptr = f->_base + OVRSIZ;

	f->_write_pos = lseek(f->_fd, 0, SEEK_CUR);
	f->_last_op = 2;

	return 0;
}
static inline int __jacl_buffer_input(FILE *f) {
	__jacl_init_stdio();

	if (JACL_UNLIKELY(!f || !(f->_flags & __SRD))) return EOF;
	if (f->_last_op == 2) { __jacl_buffer_flush(f); f->_cnt = 0; }
	if (f->_cnt > 0) { int c = (unsigned char)*f->_ptr++; f->_cnt--; return c; }

	return __jacl_buffer_reload(f);
}
static inline int __jacl_buffer_output(FILE *f) {
	__jacl_init_stdio();

	if (JACL_UNLIKELY(!f || !(f->_flags & __SWR))) return 0;

	int flush = 0;

	if (f->_bufmode == _IONBF) flush = 1;
	else if (f->_bufmode == _IOLBF) {
		if (f->_ptr > f->_base + OVRSIZ && *(f->_ptr - 1) == '\n') flush = 1;
		if (f->_ptr >= f->_end) flush = 1;
	} else if (f->_bufmode == _IOFBF) {
		if (f->_ptr >= f->_end) flush = 1;
	}

	if (flush) return __jacl_buffer_flush(f);

	return 0;
}

/* ============================================================= */
/* File IO                                                       */
/* ============================================================= */
static inline FILE* fopen(const char* restrict path, const char* restrict mode) {
	if (JACL_UNLIKELY(!path || !mode)) { errno = EINVAL; return NULL; }

	int open_flags = __jacl_buffer_flags(mode);

	if (JACL_UNLIKELY(open_flags < 0)) { errno = EINVAL; return NULL; }

	int fd = open(path, open_flags, 0666);

	if (JACL_UNLIKELY(fd < 0)) return NULL;
	if (open_flags & O_APPEND) lseek(fd, 0, SEEK_END);

	int stdio_flags = (open_flags & O_RDWR)
	                ? (__SRD|__SWR)
	                : (open_flags & O_WRONLY)
	                ? __SWR
	                : __SRD;

	SETUP_FILE(f, fd, stdio_flags, _IOFBF);

	if (!f) close(fd);

	return f;
}
static inline int fclose(FILE* stream) {
	if (!stream) { errno = EBADF; return EOF; }

	int ret = 0;

	if (stream->_flags & __SWR) {
		if (__jacl_buffer_flush(stream) == EOF) ret = EOF;
		if (fsync(stream->_fd) == -1) ret = EOF;
	}

	if (close(stream->_fd) == -1) ret = EOF;
	if (stream->_base && stream->_buf_owned) free(stream->_base);
	if (stream->_tmpfname) free(stream->_tmpfname);

	if (stream != stdin && stream != stdout && stream != stderr) {
		__jacl_stream_unregister(stream);
		free(stream);
	}

	return ret;
}
static inline int fflush(FILE* stream) { return __jacl_stream_flush(stream); }
static inline size_t fread(void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream) {
	if (!stream || !ptr || size == 0 || nmemb == 0) return 0;

	size_t total = size * nmemb;
	unsigned char *buf = (unsigned char *)ptr;
	size_t copied = 0;

	while (copied < total) {
		if (JACL_LIKELY(stream->_cnt > 0)) {
			size_t avail = (size_t)stream->_cnt < total - copied ? (size_t)stream->_cnt : total - copied;

			memcpy(buf + copied, stream->_ptr, avail);

			stream->_ptr += avail;
			stream->_cnt -= avail;

			copied += avail;
		} else {
			int c = __jacl_buffer_input(stream);

			if (JACL_UNLIKELY(c == EOF)) break;

			buf[copied++] = (unsigned char)c;
		}
	}

	return copied / size;
}
static inline size_t fwrite(const void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream) {
	if (JACL_UNLIKELY(!stream || !ptr || size == 0 || nmemb == 0)) return 0;

	size_t total = size * nmemb;
	const unsigned char *buf = (const unsigned char *)ptr;

	if (stream->_bufmode == _IONBF) {
		ssize_t written = write(stream->_fd, buf, total);

		if (written < 0) {
			stream->_flags |= __SERR;

			return 0;
		}

		return written / size;
	}

	if (stream->_ptr == stream->_base && total >= stream->_bufsiz) {
		ssize_t written = write(stream->_fd, buf, total);

		if (written < 0) { stream->_flags |= __SERR; return 0; }

		return written / size;
	}

	size_t written = 0;
	int has_newline = 0;

	while (written < total) {
		size_t space = stream->_end - stream->_ptr;

		if (JACL_UNLIKELY(space == 0)) {
			if (JACL_UNLIKELY(__jacl_buffer_output(stream) == EOF)) break;

			space = stream->_bufsiz;
		}

		size_t to_copy = (total - written < space) ? total - written : space;

		memcpy(stream->_ptr, buf + written, to_copy);

		if (stream->_bufmode == _IOLBF && !has_newline) {
			for (size_t i = 0; i < to_copy; i++) {
				if (buf[written + i] == '\n') { has_newline = 1; break; }
			}
		}

		stream->_ptr += to_copy;
		written += to_copy;
	}

	if (stream->_bufmode == _IOLBF && has_newline) __jacl_buffer_output(stream);

	return written / size;
}
static inline FILE *freopen(const char* restrict path, const char* restrict mode, FILE* restrict stream) {
	if (!stream) {
		errno = EBADF;

		return NULL;
	}

	if (stream->_flags & __SWR) __jacl_buffer_output(stream);

	if (!path) {
		#if defined(__linux__)
			char procpath[64];
			char realpath_buf[PATH_MAX];

			snprintf(procpath, sizeof(procpath), "/proc/self/fd/%d", stream->_fd);

			ssize_t len = readlink(procpath, realpath_buf, sizeof(realpath_buf) - 1);

			if (len > 0) {
				realpath_buf[len] = '\0';
				path = realpath_buf;
			} else {
				return NULL;
			}
		#else
			errno = EINVAL;

			return NULL;
		#endif
	}

	close(stream->_fd);

	int flags = __jacl_buffer_flags(mode);

	if (flags < 0) {
		errno = EINVAL;

		return NULL;
	}

	int fd = open(path, flags, 0666);

	if (fd < 0) return NULL;

	// Reinitialize stream
	stream->_fd = fd;
	stream->_ptr = stream->_base + OVRSIZ;
	stream->_cnt = 0;
	stream->_flags = (flags & O_RDWR)
		? (__SRD|__SWR)
		: (flags & O_WRONLY ? __SWR : __SRD);
	stream->_flags &= ~(__SEOF | __SERR);

	return stream;
}
static inline int remove(const char *filename) {
	if (!filename) {
		errno = EINVAL;

		return -1;
	}

	return unlink(filename);
}
static inline int rename(const char *old, const char *newpath) {
	if (!old || !newpath) {
		errno = EINVAL;

		return -1;
	}

	return renameat(AT_FDCWD, old, AT_FDCWD, newpath);
}

// Character I/O
static inline int fputc(int c, FILE* stream) {
	if (JACL_UNLIKELY(!stream || !(stream->_flags & __SWR))) { errno = EBADF; return EOF; }
	if (stream->_last_op == 1) { stream->_cnt = 0; stream->_ptr = stream->_base + (stream->_ovrsiz ? stream->_ovrsiz : OVRSIZ); }
	if (stream->_bufmode == _IONBF) {
		unsigned char ch = (unsigned char)c;
		ssize_t written = write(stream->_fd, &ch, 1);

		if (written < 0) { stream->_flags |= __SERR; return EOF; }

		stream->_last_op = 2;

		return (unsigned char)c;
	}

	if (stream->_ptr >= stream->_end && __jacl_buffer_output(stream) == EOF) return EOF;

	*stream->_ptr++ = (unsigned char)c;
	stream->_last_op = 2;

	if (stream->_bufmode == _IOLBF && c == '\n' && __jacl_buffer_output(stream) == EOF) return EOF;

	return (unsigned char)c;
}
static inline int fgetc(FILE* stream) {
	if (JACL_UNLIKELY(!stream || !(stream->_flags & __SRD))) {
		errno = EBADF;
		return EOF;
	}

	return __jacl_buffer_input(stream);
}
static inline int putc(int c, FILE* stream) { return fputc(c, stream); }
static inline int getc(FILE* stream)	   { return fgetc(stream); }
static inline int ungetc(int c, FILE* stream) {
	if (!stream || stream->_ptr == stream->_base) return EOF;

	*(--stream->_ptr) = (unsigned char)c;

	stream->_cnt++;

	return (unsigned char)c;
}
static inline int getchar(void)	  { return getc(stdin); }
static inline int putchar(int c)	 { return putc(c, stdout); }
static inline int fputs(const char* restrict s, FILE* restrict stream) {
	if (!s || !stream) {
		errno = EINVAL;

		return EOF;
	}

	while (*s) {
		if (putc(*s++, stream) == EOF) return EOF;
	}

	return 0;
}
static inline char* fgets(char* restrict s, int n, FILE* restrict stream) {
	if (!s || !stream || n <= 0) {
		errno = EINVAL;

		return NULL;
	}

	int i = 0, c;

	while (i + 1 < n && (c = getc(stream)) != EOF) {
		s[i++] = (char)c;

		if (c == '\n') break;
	}

	if (i == 0 && c == EOF) return NULL;

	s[i] = '\0';

	return s;
}
static inline int puts(const char* s) {
	if (!s) {
		errno = EINVAL;

		return EOF;
	}

	if (fputs(s, stdout) == EOF) return EOF;

	return putchar('\n') == EOF ? EOF : 0;
}
#if !JACL_HAS_C11
static inline char* gets(char *s) {
	// Unsafe—do NOT use this. Use fgets or getline instead.
	// This would crash or overflow.
	return fgets(s, INT_MAX, stdin);  // Still unsafe!
}
#endif

/* Printf Implementations */
int vprintf(const char * restrict fmt, va_list ap);
int printf(const char* restrict fmt, ...);
int vfprintf(FILE * restrict stream, const char * restrict fmt, va_list ap);
int fprintf(FILE* restrict stream, const char* restrict fmt, ...);
int vsprintf(char * restrict s, const char * restrict fmt, va_list ap);
int sprintf(char * restrict s, const char * restrict fmt, ...);

#if JACL_HAS_C99
int vsnprintf(char * restrict s, size_t n, const char * restrict fmt, va_list ap);
int snprintf(char * restrict s, size_t n, const char * restrict fmt, ...);
int vdprintf(int fd, const char* restrict fmt, va_list ap);
int dprintf(int fd, const char* restrict fmt, ...);
int vasprintf(char **strp, const char *fmt, va_list ap);
int asprintf(char **strp, const char *fmt, ...);
#endif

/* Scanf Implementations */
int vscanf(const char * restrict fmt, va_list ap);
int scanf(const char * restrict fmt, ...);
int vfscanf(FILE * restrict stream, const char * restrict fmt, va_list ap);
int fscanf(FILE* restrict stream, const char * restrict fmt, ...);
int vsscanf(const char * restrict s, const char* restrict fmt, va_list ap);
int sscanf(const char * restrict s, const char * restrict fmt, ...);

/* Error Handling */
static inline int feof(FILE* stream) { return stream ? (stream->_flags & __SEOF) : 0; }
static inline int ferror(FILE* stream) { return stream ? (stream->_flags & __SERR) : 0; }
static inline void clearerr(FILE* stream) { if (stream) stream->_flags &= ~(__SEOF | __SERR); errno = 0; }
static inline void perror(const char* s) { fprintf(stderr, "%s%s%s\n", s ? s : "", s ? ": " : "", strerror(errno)); }

/* File Positioning */
static inline int fseek(FILE* stream, long offset, int whence) {
	if (!stream) { errno = EBADF; return -1; }
	if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) { errno = EINVAL; return -1; }
	if (stream->_flags & __SWR && __jacl_buffer_flush(stream) == EOF) return -1;
	if (whence == SEEK_CUR && (stream->_flags & __SRD) && stream->_cnt > 0) offset -= stream->_cnt;

	off_t result = lseek(stream->_fd, offset, whence);

	if (result < 0) return -1;

	stream->_cnt = 0;
	stream->_ptr = stream->_base + OVRSIZ;
	stream->_end = stream->_base + OVRBUF;
	stream->_read_pos = result;
	stream->_write_pos = result;
	stream->_flags &= ~__SEOF;
	stream->_last_op = 0;

	return 0;
}
static inline long ftell(FILE* stream) {
	if (!stream) { errno = EBADF; return -1L; }

	off_t pos = lseek(stream->_fd, 0, SEEK_CUR);

	if (pos < 0) return -1L;
	if (stream->_flags & __SWR) pos += (stream->_ptr - (stream->_base + OVRSIZ));
	else if (stream->_flags & __SRD) pos -= stream->_cnt;

	return (long)pos;
}
static inline void rewind(FILE* stream) {
	if (!stream) return;
	if (stream->_flags & __SWR) __jacl_buffer_flush(stream);
	if (stream->_flags & __SRD) { stream->_cnt = 0; stream->_ptr = stream->_base + OVRSIZ; }

	fseek(stream, 0L, SEEK_SET);
	clearerr(stream);
}
static inline int fgetpos(FILE* restrict stream, fpos_t* restrict pos) {
	if (!stream || !pos) { errno = EINVAL; return -1; }

	long offset = ftell(stream);

	if (offset < 0) return -1;

	*pos = (fpos_t)offset;

	return 0;
}
static inline int fsetpos(FILE* stream, const fpos_t* pos) {
	if (!stream || !pos) { errno = EINVAL; return -1; }

	return fseek(stream, (long)*pos, SEEK_SET);
}

// Buffering
static inline int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size) {
	__jacl_init_stdio();

	if (!stream) { errno = EBADF; return EOF; }
	if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF) { errno = EINVAL; return EOF; }
	if (stream->_flags & __SWR) __jacl_buffer_output(stream);
	if (stream->_buf_owned && stream->_base) free(stream->_base);

	stream->_bufmode = mode;

	if (mode == _IONBF) { // No buffering
		stream->_base = NULL;
		stream->_ptr = NULL;
		stream->_end = NULL;
		stream->_bufsiz = 0;
		stream->_buf_owned = 0;
	} else if (buf) { // User buffer
		stream->_base = buf;
		stream->_ovrsiz = size / 10;
		stream->_bufsiz = size - stream->_ovrsiz;
		stream->_ptr = buf + stream->_ovrsiz;
		stream->_end = buf + size;
		stream->_buf_owned = 0;
	} else { // Internal buffer
		size_t bufsize = size > 0 ? size : BUFSIZ;
		stream->_base = malloc(bufsize + OVRSIZ);

		if (!stream->_base) { errno = ENOMEM; return EOF; }

		stream->_ptr = stream->_base + OVRSIZ;
		stream->_end = stream->_base + bufsize + OVRSIZ;
		stream->_bufsiz = bufsize;
		stream->_buf_owned = 1;
	}

	stream->_cnt = 0;
	stream->_flags &= ~(__SERR | __SEOF);

	return 0;
}

static inline void setbuf(FILE* restrict stream, char* restrict buf) { setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ); }

// Unlocked I/O
static inline int getchar_unlocked(void) { return getchar(); }
static inline int putchar_unlocked(int c) { return putchar(c); }
static inline int fgetc_unlocked(FILE *stream) { return fgetc(stream); }
static inline int fputc_unlocked(int c, FILE *stream) { return fputc(c, stream); }
static inline char *fgets_unlocked(char *s, int n, FILE *stream) { return fgets(s, n, stream); }
static inline int fputs_unlocked(const char *s, FILE *stream) { return fputs(s, stream); }
static inline size_t fread_unlocked(void *ptr, size_t size, size_t n, FILE *stream) { return fread(ptr, size, n, stream); }
static inline size_t fwrite_unlocked(const void *ptr, size_t size, size_t n, FILE *stream) { return fwrite(ptr, size, n, stream); }
static inline void clearerr_unlocked(FILE *stream) { clearerr(stream); }
static inline int feof_unlocked(FILE *stream) { return feof(stream); }
static inline int ferror_unlocked(FILE *stream) { return ferror(stream); }

#if JACL_OS_BSD || JACL_OS_DARWIN
static inline int setlinebuf(FILE *stream) { return setvbuf(stream, NULL, _IOLBF, BUFSIZ); }
#endif

// Posix File I/O
#if JACL_HAS_POSIX
static inline FILE *fdopen(int fd, const char *mode) {
	if (fd < 0 || !mode) { errno = EINVAL; return NULL; }

	int stdio_flags = (mode[0] == 'r') ? __SRD : __SWR;

	if (strchr(mode, '+')) stdio_flags = __SRD | __SWR;

	SETUP_FILE(f, fd, stdio_flags, _IOFBF);

	if (!f) return NULL;

	return f;
}
static inline FILE *popen(const char *command, const char *mode) {
	if (!command || !mode) { errno = EINVAL; return NULL; }

	int pipefd[2];
	int is_read = (mode[0] == 'r');

	if (pipe(pipefd) < 0) return NULL;

	pid_t pid = fork();

	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);

		return NULL;
	}

	if (pid == 0) {
		// Child process
		if (is_read) {
			close(pipefd[0]);  // Close read end
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);
		} else {
			close(pipefd[1]);  // Close write end
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
		}

		execl("/bin/sh", "sh", "-c", command, (char *)NULL);
		_exit(127);  // exec failed
	}

	// Parent process
	int fd = is_read ? pipefd[0] : pipefd[1];

	close(is_read ? pipefd[1] : pipefd[0]);

	FILE *f = fdopen(fd, mode);

	if (!f) {
		close(fd);
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);

		return NULL;
	}

	f->_tmpfname = (unsigned char *)(uintptr_t)pid;

	return f;
}
static inline int pclose(FILE *stream) {
	if (!stream) { errno = EBADF; return -1; }

	pid_t pid = (pid_t)(uintptr_t)stream->_tmpfname;
	stream->_tmpfname = NULL;

	if (stream->_flags & __SWR) __jacl_buffer_output(stream);

	close(stream->_fd);

	if (stream->_base && stream->_buf_owned) free(stream->_base);

	__jacl_stream_unregister(stream);
	free(stream);

	int status;

	while (waitpid(pid, &status, 0) < 0) {
		if (errno != EINTR) return -1;
	}

	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
typedef int (*readfn_t)(void *cookie, char *buf, int size);
typedef int (*writefn_t)(void *cookie, const char *buf, int size);
typedef fpos_t (*seekfn_t)(void *cookie, fpos_t offset, int whence);
typedef int (*closefn_t)(void *cookie);
static inline FILE *funopen(const void *cookie, readfn_t readfn, writefn_t writefn, seekfn_t seekfn, closefn_t closefn){
  (void)cookie; (void)readfn; (void)writefn; (void)seekfn; (void)closefn;

  errno = ENOSYS;

  return NULL;
}
static inline int fileno(FILE *stream) { if (!stream) { errno = EBADF; return -1; } return stream->_fd; }
static inline ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream) {
	if (!lineptr || !n || !stream) { errno = EINVAL; return -1; }

	size_t pos = 0;
	int c;

	while ((c = fgetc(stream)) != EOF) {
		if (pos + 1 >= *n) {
			size_t new_size = *n ? *n * 2 : 128;
			char *new_ptr = (char *)realloc(*lineptr, new_size);

			if (!new_ptr) return -1;

			*lineptr = new_ptr;
			*n = new_size;
		}

		(*lineptr)[pos++] = (char)c;

		if (c == delim) break;
	}
	if (pos == 0 && c == EOF) return -1;
	if (pos >= *n) {
		char *new_ptr = (char *)realloc(*lineptr, pos + 1);

		if (!new_ptr) return -1;

		*lineptr = new_ptr;
		*n = pos + 1;
	}

	(*lineptr)[pos] = '\0';

	return (ssize_t)pos;
}
static inline ssize_t getline(char **lineptr, size_t *n, FILE *stream) { return getdelim(lineptr, n, '\n', stream); }
#endif /* JACL_HAS_POSIX */

// Treads stubs
#if JACL_HAS_THREADS
static inline void flockfile(FILE *stream) { (void)stream; /* No-op stub */ }
static inline int ftrylockfile(FILE *stream) { (void)stream; return 0;  /* Pretend lock always succeeds */ }
static inline void funlockfile(FILE *stream) { (void)stream; /* No-op stub */ }
#endif /* JACL_HAS_THREADS */

// Tempfile
static inline FILE *tmpfile(void) {
	char tmpl[] = TEMP_DIR "/tmpXXXXXX";
	int fd = mkstemp(tmpl);

	if (fd < 0) return NULL;

	unlink(tmpl);

	FILE *f = fdopen(fd, "w+");

	if (!f) close(fd);

	return f;
}
static inline char *tmpnam(char *s) {
	static char buf[L_tmpnam];
	static unsigned long counter = 0;
	char *target = s ? s : buf;

	do { snprintf(target, L_tmpnam, "%stmp_%lu_%ld", TEMP_DIR, counter++, (long)time(NULL)); } while (access(target, F_OK) == 0 && counter < TMP_MAX);

	return (counter < TMP_MAX) ? target : NULL;
}

#ifdef __cplusplus
}
#endif

#include <wchar.h>

#endif /* STDIO_H */
