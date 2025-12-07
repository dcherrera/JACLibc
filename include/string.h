/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef STRING_H
#define STRING_H
#pragma once

#include <config.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#if JACL_HAS_C23
#define __STDC_VERSION_STRING_H__ 202311L
#endif

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t);

/* Memory Operations */
static inline void* memcpy(void* restrict dest, const void*  restrict src, size_t n) {
	if (JACL_UNLIKELY(!dest || !src)) return NULL;

	unsigned char* d = (unsigned char* )dest;
	const unsigned char* s = (const unsigned char* )src;

	for (size_t i = 0; i < n; i++) d[i] = s[i];

	return dest;
}

static inline void* memset(void* s, int c, size_t n) {
	if (JACL_UNLIKELY(!s)) return NULL;

	unsigned char* p = (unsigned char* )s;

	for (size_t i = 0; i < n; i++) p[i] = (unsigned char)c;

	return s;
}

static inline void* memmove(void* dest, const void* src, size_t n) {
	if (JACL_UNLIKELY(!dest || !src)) return NULL;

	unsigned char* d = (unsigned char* )dest;
	const unsigned char* s = (const unsigned char* )src;

	if (d < s) {
		for (size_t i = 0; i < n; i++) d[i] = s[i];
	} else {
		for (size_t i = n; i-- > 0;) d[i] = s[i];
	}

	return dest;
}

static inline int memcmp(const void* a, const void* b, size_t n) {
	const unsigned char* x = (const unsigned char* )a;
	const unsigned char* y = (const unsigned char* )b;

	for (size_t i = 0; i < n; i++) {
		if (x[i] != y[i]) return x[i] < y[i] ? -1 : 1;
	}

	return 0;
}

static inline void* memchr(const void* s, int c, size_t n) {
	const unsigned char* p = (const unsigned char* )s;

	for (size_t i = 0; i < n; i++) {
		if (p[i] == (unsigned char)c) return (void* )&p[i];
	}

	return NULL;
}

#ifdef JACL_HAS_POSIX
static inline void* memmem(const void* hay, size_t hl, const void* ndl, size_t nl) {
	if (nl > hl) return NULL;

	const unsigned char* h = (const unsigned char* )hay;

	for (size_t i = 0; i <= hl - nl; i++) {
		if (!memcmp(h + i, ndl, nl)) return (void* )(h + i);
	}

	return NULL;
}
#endif

/* UTF-8–aware String Length & Copy */
static inline size_t strlen(const char* s) {
	size_t len = 0;

	while (s[len]) len++;

	return len;
}

static inline size_t strnlen(const char* s, size_t n) {
	size_t i = 0;

	while (i < n && s[i]) i++;

	return i;
}

static inline char* strcpy(char*  restrict dest, const char* src) {
	char* p = dest;

	while ((*p++ = *src++));

	return dest;
}

static inline char* strncpy(char*  restrict dest, const char* src, size_t n) {
	size_t i = 0;

	for (; i < n && src[i]; i++) dest[i] = src[i];
	for (; i < n; i++) dest[i] = '\0';

	return dest;
}

static inline char* strcat(char*  restrict dest, const char* src) {
	char* p = dest + strlen(dest);

	while ((*p++ = *src++));

	return dest;
}

static inline char* strncat(char*  restrict dest, const char* src, size_t n) {
	char* p = dest + strlen(dest);
	size_t i = 0;

	for (; i < n && src[i]; i++) p[i] = src[i];

	p[i] = '\0';

	return dest;
}

/* Comparison & Collation (byte-safe, Unicode case-fold hooks) */
static inline int strcmp(const char* a, const char* b) {
	while (*a && *a == *b) {
		a++;
		b++;
	}

	return (*a < *b)
		? -1 :
		(*a > *b);
}

static inline int strncmp(const char* a, const char* b, size_t n) {
	size_t i = 0;

	for (; i < n && a[i] && a[i] == b[i]; i++);

	if (i == n) return 0;

	return (a[i] < b[i]) ? -1 : 1;
}

static inline int strcoll(const char* a, const char* b) {
	return strcmp(a, b);
}

static inline size_t strxfrm(char*  restrict dest, const char* src, size_t n) {
	size_t l = strlen(src);

	if (n > l) strcpy(dest, src);

	return l;
}

/* Thread-safe strerror & strerror_r */
#if JACL_HAS_C11
static _Thread_local char _jacl_errbuf[64];
#else
static __thread char _jacl_errbuf[64];
#endif

static inline char* strerror(int err) {
	const char* msg;

	switch (err) {
		case 0: msg = "Success"; break;
		case EPERM: msg = "Operation not permitted"; break;
		case ENOENT: msg = "No such file or directory"; break;
		case ESRCH: msg = "No such process"; break;
		case EINTR: msg = "Interrupted system call"; break;
		case EIO: msg = "I/O error"; break;
		case ENXIO: msg = "No such device or address"; break;
		case E2BIG: msg = "Argument list too long"; break;
		case ENOEXEC: msg = "Exec format error"; break;
		case EBADF: msg = "Bad file descriptor"; break;
		case ECHILD: msg = "No child processes"; break;
		case EAGAIN: msg = "Try again"; break;
		case ENOMEM: msg = "Out of memory"; break;
		case EACCES: msg = "Permission denied"; break;
		case EFAULT: msg = "Bad address"; break;
		case ENOSYS: msg = "Function not implemented"; break;
		case EDOM: msg = "Math argument out of domain"; break;
		case ERANGE: msg = "Math result out of range"; break;
		default: msg = "Unknown error"; break;
	}

	size_t len = strlen(msg);

	if (len >= sizeof(_jacl_errbuf)) len = sizeof(_jacl_errbuf) - 1;

	memcpy(_jacl_errbuf, msg, len);
	_jacl_errbuf[len] = '\0';

	return _jacl_errbuf;
}

static inline int strerror_r(int err, char* buf, size_t buflen) {
	if (!buf || buflen == 0) return EINVAL;

	// Reuse existing strerror() - it handles all the switch logic
	const char* msg = strerror(err);
	size_t len = strlen(msg);

	if (len >= buflen) {
		// Buffer too small - copy what fits and null terminate
		if (buflen > 0) {
			memcpy(buf, msg, buflen - 1);
			buf[buflen - 1] = '\0';
		}

		return ERANGE;
	}

	// Buffer is large enough - copy entire message including null terminator
	memcpy(buf, msg, len + 1);

	return 0;
}

/* Search & Tokenization */
static inline char* strchr(const char* s, int c) {
	for (;; s++) {
		if (*s == (char)c) return (char* )s;
		if (!*s) break;
	}

	return NULL;
}

static inline char* strrchr(const char* s, int c) {
	const char* r = NULL;

	for (;; s++) {
		if (*s == (char)c) r = s;
		if (!*s) break;
	}

	return (char* )r;
}

static inline char* strstr(const char* h, const char* n) {
	size_t nl = strlen(n);

	if (!nl) return (char* )h;

	for (; *h; h++) {
		if (!strncmp(h, n, nl)) return (char* )h;
	}

	return NULL;
}

static inline size_t strspn(const char* s, const char* accept) {
	size_t i = 0;

	while (s[i] && strchr(accept, s[i])) i++;

	return i;
}

static inline size_t strcspn(const char* s, const char* reject) {
	size_t i = 0;

	while (s[i] && !strchr(reject, s[i])) i++;

	return i;
}

static inline char* strpbrk(const char* s, const char* accept) {
	for (; *s; s++) if (strchr(accept, *s)) return (char* )s;

	return NULL;
}

static inline char* strtok_r(char* s, const char* delim, char* *save) {
	char* p = s ? s : *save;

	if (!p) return NULL;

	p += strspn(p, delim);

	if (!*p) { *save = NULL; return NULL; }

	char* q = p + strcspn(p, delim);

	if (*q) *q++ = '\0';

	*save = q;

	return p;
}

static inline char* strtok(char* s, const char* delim) {
	static __thread char* tls_save;

	return strtok_r(s, delim, &tls_save);
}

/* BSD-Style Safe Copy/Cat */
#if JACL_OS_BSD
static inline size_t strlcpy(char*  restrict dest, const char* src, size_t n) {
	size_t len = strlen(src);
	size_t end = (len >= n) ? n - 1 : len;

	memcpy(dest, src, end);
	dest[end] = '\0';

	return len;
}

static inline size_t strlcat(char*  restrict dest, const char* src, size_t n) {
	size_t dlen = strlen(dest);
	size_t slen = strlen(src);

	if (dlen >= n) return n + slen;

	size_t end = ((dlen + slen) >= n) ? (n - dlen - 1) : slen;

	memcpy(dest + dlen, src, end);
	dest[dlen + end] = '\0';

	return dlen + slen;
}
#endif

/* POSIX basename simple no-alloc */
#if JACL_HAS_POSIX
static inline const char* basename(const char* path) {
		const char* p = strrchr(path, '/');
		return p ? p + 1 : path;
}
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <type_traits>
template<typename T> T *memcpy(T*, const T*, size_t) = delete;
template<typename T> T *memmove(T*, const T*, size_t) = delete;
template<typename T> T *strcpy(T*, const char*) = delete;
template<typename T> T *strcat(T*, const char*) = delete;
static_assert(std::is_same<decltype(::memcpy((int*)0, (int*)0, 0)), void*>::value, "Use C memcpy, not template");
#endif

#endif /* STRING_H */
