/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef STDLIB_H
#define STDLIB_H
#pragma once

#include <config.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdbit.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/wait.h>

#if JACL_HAS_C23
#define __STDC_VERSION_STDLIB_H__ 202311L
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Standard exit codes */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* Multibyte character constants */
#define MB_CUR_MAX 1

#ifndef MB_LEN_MAX
#define MB_LEN_MAX 1
#endif

#ifndef TMP_MAX
#define TMP_MAX 238328
#endif

/* ============================================================= */
/* Type Definitions                                              */
/* ============================================================= */

typedef struct { int8_t quot, rem; } div8_t;
typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;

#if JACL_HAS_C99
	typedef struct { long long quot, rem; } lldiv_t;
#endif

/* ============================================================= */
/* Memory Allocation                                             */
/* ============================================================= */
void* malloc(size_t n);
void free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);

#if JACL_HAS_C11
void *aligned_alloc(size_t a, size_t s);
#endif

/* ============================================================= */
/* Integer Conversion & Parsing                                  */
/* ============================================================= */

int atoi(const char* str);
double atof(const char* str);

long atol(const char* str);
long strtol(const char* str, char ** endptr, int base);
double strtod(const char *nptr, char **endptr);

#if JACL_HAS_C99
long long atoll(const char*);

unsigned long strtoul(const char* str, char** endptr, int base);
long long strtoll(const char*, char ** str, int base);
unsigned long long strtoull(const char* str, char** endptr, int base);

float strtof(const char *nptr, char **endptr);
long double strtold(const char *nptr, char **endptr);
#endif /* JACL_HAS_C99 */

/* ============================================================= */
/* Absolute Value & Division                                     */
/* ============================================================= */
static inline int abs(int n) { return n < 0 ? -n : n; }
static inline div_t div(int n, int d) { return (n == INT_MIN && d == -1) ? (div_t){ INT_MIN, 0 } : (div_t){ n/d, n%d }; }
static inline long labs(long n) { return n < 0 ? -n : n; }
static inline ldiv_t ldiv(long n, long d) { return (n == LONG_MIN && d == -1) ? (ldiv_t){ LONG_MIN, 0 } : (ldiv_t){ n/d, n%d }; }

#if JACL_HAS_C99
static inline long long llabs(long long n) { return n < 0 ? -n : n; }
static inline lldiv_t lldiv(long long n, long long d) { return (n == LLONG_MIN && d == -1) ? (lldiv_t){ LLONG_MIN, 0 } : (lldiv_t){ n/d, n%d }; }
#endif

/* ============================================================= */
/* Pseudo-Random Number Generation                               */
/* ============================================================= */
#define RAND_MAX 0x7FFF  // Standard value (32767)

static unsigned __jacl_seed = 1;

static inline void srand(unsigned s) { __jacl_seed = s ? s : 1; }
static inline int rand(void) { __jacl_seed = __jacl_seed * 1103515245u + 12345u; return (int)((__jacl_seed >> 16) & RAND_MAX); }

/* ============================================================= */
/* Sorting & Searching                                           */
/* ============================================================= */

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*,const void*));

/* ============================================================= */
/* Program Termination & Exit                                    */
/* ============================================================= */
void exit(int status);
int atexit(void (*func)(void));

#if JACL_HAS_C11
_Noreturn void quick_exit(int status);
int at_quick_exit(void (*func)(void));
#endif

#if JACL_HAS_C99
static inline void _Exit(int status) { _exit(status); }
#endif

static inline void abort(void) {
	raise(SIGABRT);  // Standard attempt

	#if JACL_ARCH_X64 || JACL_ARCH_X86
		__asm__ volatile ("ud2");  // Invalid opcode
	#elif JACL_ARCH_ARM64 || JACL_ARCH_ARM32
			__asm__ volatile ("brk #0");	// Breakpoint
	#elif JACL_ARCH_RISCV
			__asm__ volatile ("ebreak");	// Environment break
	#elif JACL_ARCH_WASM
			__asm__ volatile ("unreachable");  // WASM trap
	#else
			*(volatile int*)0 = 0;	// Universal fallback
	#endif

	for(;;) {}	// The heat death of the universe
}

/* ============================================================= */
/* Environment & System                                          */
/* ============================================================= */

static inline char* getenv(const char* name) {
	extern char **environ;
	size_t len = strlen(name);

	for (char **env = environ; *env; ++env) {
		if (!strncmp(*env, name, len) && (*env)[len] == '=') return &((*env)[len+1]);
	}

	return NULL;
}
static inline int system(const char* command) {
	#if JACL_OS_WINDOWS
		return _spawnlp(_P_WAIT, "cmd.exe", "cmd.exe", "/C", command, NULL);
	#elif JACL_OS_JSRUN || JACL_OS_WASM
		(void)command;
		return -1;
	#else
		if (!command) return 1;

		pid_t pid = fork();
		if (pid < 0) return -1;
		if (pid == 0) {
			execl("/bin/sh", "sh", "-c", command, (char*)NULL);
			_exit(127);
		}

		int status;
		while (waitpid(pid, &status, 0) < 0) {
			if (errno != EINTR) return -1;
		}

		if (WIFEXITED(status)) return WEXITSTATUS(status);
		if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);

		return -1;
	#endif
}

#if JACL_HAS_POSIX
static inline int putenv(char *string) {
	extern char **environ;

	if (!string || !strchr(string, '=')) { errno = EINVAL; return -1; }

	const char *eq = strchr(string, '=');
	size_t namelen = eq - string;

	for (char **ep = environ; *ep; ep++) {
		if (strncmp(*ep, string, namelen) == 0 && (*ep)[namelen] == '=') {
			*ep = string;

			return 0;
		}
	}

	size_t count = 0;

	for (char **ep = environ; *ep; ep++) count++;

	char **new_environ = (char**)malloc((count + 2) * sizeof(char*));

	if (!new_environ) { errno = ENOMEM; return -1; }

	for (size_t i = 0; i < count; i++) new_environ[i] = environ[i];

	new_environ[count] = string;
	new_environ[count + 1] = NULL;

	environ = new_environ;

	return 0;
}
static inline int setenv(const char *name, const char *value, int overwrite) {
	if (!name || !*name || strchr(name, '=')) { errno = EINVAL; return -1; }
	if (!overwrite && getenv(name)) return 0;

	size_t len = strlen(name) + strlen(value) + 2;
	char *str = (char*)malloc(len);

	if (!str) { errno = ENOMEM; return -1; }

	strcpy(str, name);
	strcat(str, "=");
	strcat(str, value);

	return putenv(str);
}
static inline int unsetenv(const char *name) {
	extern char **environ;

	if (!name || !*name || strchr(name, '=')) { errno = EINVAL; return -1; }

	size_t len = strlen(name);
	char **ep = environ;

	while (*ep) {
		if (strncmp(*ep, name, len) == 0 && (*ep)[len] == '=') {
			char **sp = ep;
			do { *sp = *(sp + 1); } while (*sp++);
		} else {
			ep++;
		}
	}

	return 0;
}
#endif

/* ============================================================= */
/* String Duplication                                            */
/* ============================================================= */

static inline char *strdup(const char *s) { size_t n = strlen(s) + 1; char *p = (char *)malloc(n); return p ? (char *)memcpy(p, s, n) : NULL; }

#if JACL_HAS_C99
static inline char *strndup(const char *s, size_t n) {
		size_t len = strnlen(s, n);
		char *p = (char *)malloc(len + 1);

		if (!p) return NULL;

		memcpy(p, s, len);
		p[len] = '\0';

		return p;
}
#endif

/* ============================================================= */
/* Temporary Files (POSIX)                                       */
/* ============================================================= */

#if JACL_HAS_POSIX

static inline int mkstemp(char *template) {
	if (!template) {
		errno = EINVAL;
		return -1;
	}

	size_t len = strlen(template);
	if (len < 6) {
		errno = EINVAL;
		return -1;
	}

	char *suffix = template + len - 6;
	if (strcmp(suffix, "XXXXXX") != 0) {
		errno = EINVAL;
		return -1;
	}

	static unsigned int counter = 0;
	unsigned int attempts = 0;

	while (attempts++ < TMP_MAX) {
		unsigned int seed = (unsigned int)time(NULL)
		                  ^ (unsigned int)getpid()
		                  ^ __jacl_seed
		                  ^ __sync_fetch_and_add(&counter, 1);

		for (int i = 0; i < 6; i++) {
			seed = seed * 1103515245u + 12345u;
			int r = (seed >> 16) % 62;
			if (r < 10)
				suffix[i] = '0' + r;
			else if (r < 36)
				suffix[i] = 'A' + (r - 10);
			else
				suffix[i] = 'a' + (r - 36);
		}

		int fd = open(template, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) return fd;

		if (errno != EEXIST) return -1;
	}

	errno = EEXIST;
	return -1;
}

#endif /* JACL_HAS_POSIX */


#ifdef __cplusplus
}
#endif

#endif /* STDLIB_H */
