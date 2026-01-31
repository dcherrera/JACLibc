/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef CORE_STDLIB_H
#define CORE_STDLIB_H

#include <stdlib.h>
#include <assert.h>
#include <core/format.h>

#if JACL_OS_WINDOWS
  #include <windows.h>
#endif

/* Heap Growth Configuration */
#ifndef JACL_HEAP_INIT
#define JACL_HEAP_INIT (256u<<10)
#endif

#ifndef JACL_HEAP_MAX
#define JACL_HEAP_MAX (1u<<30)
#endif

#ifndef JACL_HEAP_SEGMENT
#define JACL_HEAP_SEGMENT (1u<<20)
#endif

/* Configuration */
#ifndef JACL_SMALL_MAX
#define JACL_SMALL_MAX 128u
#endif

#ifndef JACL_TLS_CHUNK
#define JACL_TLS_CHUNK 4096u
#endif

#ifndef JACL_ALIGNMENT
#define JACL_ALIGNMENT 8u
#endif

#define JACL_ALIGN_UP(x,a) (((x)+((a)-1u))&~((a)-1u))
#define JACL_HDR_ALLOC 1u
#define JACL_HDR_ARENA 2u

#if JACL_HAS_C11
	#include <stdatomic.h>
#else
	#define memory_order_relaxed 0
	#define memory_order_acquire 0
	#define memory_order_release 0
	#define memory_order_seq_cst 0

	#define atomic_load(ptr) (*(ptr))
	#define atomic_load_explicit(ptr, order) (*(ptr))
	#define atomic_store(ptr, val) (*(ptr) = (val))
	#define atomic_store_explicit(ptr, val, order) (*(ptr) = (val))
	#define atomic_fetch_add(ptr, val) ((*(ptr))++)
	#define atomic_fetch_add_explicit(ptr, val, order) ((*(ptr)) += (val), (*(ptr)) - (val))
	#define atomic_compare_exchange_strong(ptr, expected, desired) \
		((*(ptr) == *(expected)) ? (*(ptr) = (desired), 1) : (*(expected) = *(ptr), 0))

	#warning "WARNING: Before C11 your memory is single-threaded mode only"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Internal State */
typedef struct {
	size_t size, prev_size, flags;
} __jacl_hdr_t;

typedef struct {
	_Atomic uint_fast32_t next, owner;
} __jacl_lock_t;

typedef struct __jacl_segment {
	uint8_t* base;
	size_t size, bins[32];
	uint_least32_t bitmap;
	struct __jacl_segment* next;
} __jacl_segment_t;

// Initial static heap
static _Alignas(16) uint8_t __jacl_static_heap[JACL_HEAP_INIT];

// Segment tracking
static __jacl_segment_t __jacl_initial_segment;
static __jacl_segment_t* __jacl_segment_head = NULL;
static size_t __jacl_heap_current = JACL_HEAP_INIT;
static size_t __jacl_heap_max = JACL_HEAP_MAX;
static _Atomic uint32_t __jacl_once = 0;
static __jacl_lock_t __jacl_lock = {0,0};

// TLS state
static _Atomic uint32_t __jacl_tls_cursor = 0;
_Thread_local uint32_t __jacl_tls_off = 0;
_Thread_local uint32_t __jacl_tls_end = 0;

// Quicklist
#define QUICKLIST_MAX 7
_Thread_local _Alignas(64) struct {
	void* slots[QUICKLIST_MAX];
	int count;
} __jacl_quicklist[4];

/* Internal Functions */
static inline void __jacl_lock_acquire(__jacl_lock_t* l) {
	uint32_t ticket = atomic_fetch_add_explicit(&l->next, 1, memory_order_relaxed);

	while (atomic_load_explicit(&l->owner, memory_order_acquire) != ticket) {
		#if JACL_OS_WINDOWS
			YieldProcessor();
		#else
			sched_yield();
		#endif
	}
}

static inline void __jacl_lock_release(__jacl_lock_t* l) {
	atomic_fetch_add_explicit(&l->owner, 1, memory_order_release);
}

#define __jacl_bin_safety(seg, off, where) do { \
	if ((off) >= (seg)->size || (off) + sizeof(__jacl_hdr_t) > (seg)->size) { \
		return 0; \
	} \
} while (0)

#define __jacl_bin_safety_void(seg, off, where) do { \
	if ((off) >= (seg)->size || (off) + sizeof(__jacl_hdr_t) > (seg)->size) { \
		return; \
	} \
} while (0)

static inline int __jacl_bin_idx(size_t sz) {
	if (sz <= 128) return (sz + 7) >> 3;

	int i = 16;
	size_t q = sz >> 7;

	while (q > 1 && i < 31) { q >>= 1; i++; }

	return i;
}

static inline size_t __jacl_bin_pop(__jacl_segment_t* seg, size_t need) {
	int i = __jacl_bin_idx(need);
	uint32_t mask = (~0u) << i;
	uint32_t m = seg->bitmap & mask;

	if (!m) return 0;

	i = __jacl_ctz32(m);

	size_t off = seg->bins[i];

	if (!off) { seg->bitmap &= ~(1u << i); return 0; }

	__jacl_bin_safety(seg, off, "pop");

	uint8_t *ptr = seg->base + off + sizeof(__jacl_hdr_t);

	if (ptr < seg->base + sizeof(__jacl_hdr_t) ||
		ptr > seg->base + seg->size - sizeof(size_t)) {
			seg->bins[i] = 0;
			seg->bitmap &= ~(1u << i);

			return 0;
		}

	seg->bins[i] = *(size_t*)ptr;

	if (!seg->bins[i]) seg->bitmap &= ~(1u << i);

	return off;
}

static inline void __jacl_bin_push(__jacl_segment_t* seg, size_t off) {
	int i = __jacl_bin_idx(((__jacl_hdr_t*)(seg->base + off))->size);
	__jacl_bin_safety_void(seg, off, "push");

	*(size_t*)(seg->base + off + sizeof(__jacl_hdr_t)) = seg->bins[i];
	seg->bins[i] = off;
	seg->bitmap |= 1u << i;
}

static inline void __jacl_bin_remove(__jacl_segment_t* seg, size_t off) {
	int i = __jacl_bin_idx(((__jacl_hdr_t*)(seg->base + off))->size);
	size_t cur = seg->bins[i];

	__jacl_bin_safety_void(seg, off, "remove");

	if (cur == off) {
		seg->bins[i] = *(size_t*)(seg->base + off + sizeof(__jacl_hdr_t));
	} else {
		while (cur) {
			size_t next = *(size_t*)(seg->base + cur + sizeof(__jacl_hdr_t));

			if (next) __builtin_prefetch(seg->base + next, 0, 1);

			if (next == off) {
				*(size_t*)(seg->base + cur + sizeof(__jacl_hdr_t)) = *(size_t*)(seg->base + off + sizeof(__jacl_hdr_t));
				break;
			}
			cur = next;
		}
	}

	if (!seg->bins[i])
		seg->bitmap &= ~(1u << i);
}

static inline __jacl_segment_t* __jacl_find_segment(void* ptr) {
	if (!ptr) return NULL;

	__jacl_segment_t* seg = __jacl_segment_head;

	while (seg) {
		if (ptr >= (void*)seg->base && ptr < (void*)(seg->base + seg->size)) return seg;

		seg = seg->next;
	}

	return NULL;
}

static inline void __jacl_init_once(void) {
	static int canary = 0;
	canary++;

	if (atomic_load_explicit(&__jacl_once, memory_order_acquire)) return;

	uint32_t expect = 0;

	if (!atomic_compare_exchange_strong(&__jacl_once, &expect, 1)) return;

	__jacl_initial_segment.base = __jacl_static_heap;
	__jacl_initial_segment.size = JACL_HEAP_INIT;
	__jacl_initial_segment.next = NULL;

	memset(__jacl_initial_segment.bins, 0, sizeof(__jacl_initial_segment.bins));

	__jacl_initial_segment.bitmap = 0;
	__jacl_segment_head = &__jacl_initial_segment;

	atomic_store_explicit(&__jacl_tls_cursor, 0, memory_order_relaxed);

	size_t base = JACL_ALIGN_UP(JACL_HEAP_INIT/4, JACL_ALIGNMENT);

	if (base + sizeof(__jacl_hdr_t) < JACL_HEAP_INIT) {
		__jacl_hdr_t* h = (__jacl_hdr_t*)(__jacl_static_heap + base);

		h->size = JACL_HEAP_INIT - base;
		h->prev_size = 0;
		h->flags = 0;

		__jacl_bin_push(&__jacl_initial_segment, base);
	}
}

static inline int __jacl_tls_refill(void) {
	uint32_t cur = atomic_fetch_add_explicit(&__jacl_tls_cursor, JACL_TLS_CHUNK, memory_order_relaxed);
	uint32_t limit = JACL_HEAP_INIT / 4;

	if (cur >= limit) return 0;

	uint32_t available = (cur + JACL_TLS_CHUNK > limit) ? (limit - cur) : JACL_TLS_CHUNK;

	if (available < 64) return 0;

	__jacl_tls_off = JACL_ALIGN_UP(cur, JACL_ALIGNMENT);
	__jacl_tls_end = cur + available;

	return 1;
}

static inline size_t __jacl_round_size(size_t sz) {
	if (sz <= 128) return (sz + 7) & ~7u;
	if (sz <= 512) return (sz + 15) & ~15u;

	return JACL_ALIGN_UP(sz, 32);
}

static inline int __jacl_can_grow(void) {
	return (__jacl_heap_max != JACL_HEAP_INIT && __jacl_heap_current < __jacl_heap_max);
}

static inline __jacl_segment_t* __jacl_grow_heap(size_t min_needed) {
	if (!__jacl_can_grow()) return NULL;

	// Calculate segment size
	size_t seg_size = (min_needed > JACL_HEAP_SEGMENT)
		? JACL_ALIGN_UP(min_needed, 4096)
		: JACL_HEAP_SEGMENT;

	// Check against max
	if (__jacl_heap_current + seg_size > __jacl_heap_max) {
		seg_size = __jacl_heap_max - __jacl_heap_current;

		if (seg_size < min_needed) return NULL;
	}

	// Allocate memory for segment + metadata
	void* mem = NULL;

#if JACL_OS_WINDOWS
	mem = VirtualAlloc(NULL, seg_size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

	if (!mem) return NULL;

#elif JACL_ARCH_WASM
	size_t pages = (seg_size + 65535) / 65536;
	int old_pages = __builtin_wasm_memory_grow(0, pages);

	if (old_pages < 0) return NULL;

	mem = (void*)(old_pages * 65536);
#elif JACL_HAS_POSIX
	mem = mmap(NULL, seg_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	if (mem == MAP_FAILED) return NULL;
#else
	return NULL;
#endif

	__jacl_segment_t* seg = (__jacl_segment_t*)mem;
	seg->base = (uint8_t*)mem + sizeof(__jacl_segment_t);
	seg->size = seg_size - sizeof(__jacl_segment_t);
	seg->next = NULL;
	memset(seg->bins, 0, sizeof(seg->bins));
	seg->bitmap = 0;

	// Place initial free block at a non-zero offset to avoid sentinel confusion
	size_t initial_off = sizeof(__jacl_hdr_t);

	__jacl_hdr_t* h = (__jacl_hdr_t*)(seg->base + initial_off);
	h->size = seg->size - initial_off;
	h->prev_size = 0;
	h->flags = 0;

	__jacl_bin_push(seg, initial_off);

	__jacl_segment_t* tail = __jacl_segment_head;

	while (tail->next) tail = tail->next;

	tail->next = seg;

	__jacl_heap_current += seg_size;

	return seg;
}

/* Public API */
void* malloc(size_t n) {
	if (n == 0) n = 1;
	if (n > SIZE_MAX - sizeof(__jacl_hdr_t)) return NULL;

	__jacl_init_once();

	size_t need = __jacl_round_size(n + sizeof(__jacl_hdr_t));

	/* Quicklist */
	if (n <= 40 && n >= 16) {
		int qidx = (n >> 3) - 2;

		if (qidx >= 0 && qidx < 4 && __jacl_quicklist[qidx].count > 0) {
			void* p = __jacl_quicklist[qidx].slots[--__jacl_quicklist[qidx].count];
			__jacl_hdr_t* h = (__jacl_hdr_t*)p - 1;

			h->flags = JACL_HDR_ALLOC;

			return p;
		}
	}

	/* TLS arena */
	if (JACL_LIKELY(n <= JACL_SMALL_MAX)) {
		if (JACL_UNLIKELY(__jacl_tls_off == 0 || __jacl_tls_off + need > __jacl_tls_end) && JACL_UNLIKELY(!__jacl_tls_refill())) goto large_path;

		__jacl_hdr_t* h = (__jacl_hdr_t*)(__jacl_static_heap + __jacl_tls_off);

		h->size = need;
		h->prev_size = (__jacl_tls_off > 0) ? ((__jacl_hdr_t*)(__jacl_static_heap + __jacl_tls_off - h->size))->size : 0;
		h->flags = JACL_HDR_ALLOC | JACL_HDR_ARENA;

		__jacl_tls_off += need;

		return (void*)(h + 1);
	}

large_path:
assert(__jacl_tls_off <= JACL_HEAP_INIT / 4);
assert(__jacl_tls_end <= JACL_HEAP_INIT / 4);
assert(__jacl_tls_off <= __jacl_tls_end);

	__jacl_lock_acquire(&__jacl_lock);

	__jacl_segment_t* seg = __jacl_segment_head;

	while (seg) {
		size_t off = __jacl_bin_pop(seg, need);

		if (off) {
			__jacl_hdr_t* h = (__jacl_hdr_t*)(seg->base + off);

			if (h->size >= need + sizeof(__jacl_hdr_t) + 8) {
				size_t rem_off = off + need;
				__jacl_hdr_t* rem = (__jacl_hdr_t*)(seg->base + rem_off);

				rem->size = h->size - need;
				rem->prev_size = need;
				rem->flags = 0;

				__jacl_bin_push(seg, rem_off);

				h->size = need;
			}

			size_t next_off = off + h->size;

			if (next_off < seg->size) ((__jacl_hdr_t*)(seg->base + next_off))->prev_size = h->size;

			h->flags = JACL_HDR_ALLOC;

			__jacl_lock_release(&__jacl_lock);

			return (void*)(h + 1);
		}

		seg = seg->next;
	}

	seg = __jacl_grow_heap(need);

	if (seg) {
		size_t off = __jacl_bin_pop(seg, need);

		if (off) {
			__jacl_hdr_t* h = (__jacl_hdr_t*)(seg->base + off);

			if (h->size >= need + sizeof(__jacl_hdr_t) + 8) {
				size_t rem_off = off + need;
				__jacl_hdr_t* rem = (__jacl_hdr_t*)(seg->base + rem_off);

				rem->size = h->size - need;
				rem->prev_size = need;
				rem->flags = 0;

				__jacl_bin_push(seg, rem_off);

				h->size = need;
			}

			h->flags = JACL_HDR_ALLOC;

			__jacl_lock_release(&__jacl_lock);

			return (void*)(h + 1);
		}
	}

	__jacl_lock_release(&__jacl_lock);

	return NULL;
}

void free(void* p) {
	if (JACL_UNLIKELY(!p)) return;

	__jacl_hdr_t* h = (__jacl_hdr_t*)p - 1;

	// Find which segment this belongs to
	__jacl_segment_t* seg = __jacl_find_segment((void*)h);

	// If not found, might be aligned allocation
	if (!seg) {
		// Try to recover original pointer stored before the aligned pointer
		void** stored_ptr = (void**)((uint8_t*)p - sizeof(void*));
		void* original = *stored_ptr;

		// Validate original pointer looks reasonable
		if (!original || ((uintptr_t)original & (JACL_ALIGNMENT - 1))) return;  // Invalid

		h = (__jacl_hdr_t*)original - 1;
		seg = __jacl_find_segment((void*)h);

		if (!seg) return;  // Still invalid

		p = (void*)(h + 1);  // Use original pointer for the rest of free
	}

	if (JACL_UNLIKELY(!(h->flags & JACL_HDR_ALLOC))) return;

	size_t size = h->size - sizeof(__jacl_hdr_t);

	/* Quicklist */
	if (size <= 40 && size >= 16 && !(h->flags & JACL_HDR_ARENA)) {
		int qidx = (size >> 3) - 2;

		if (qidx >= 0 && qidx < 4 && __jacl_quicklist[qidx].count < QUICKLIST_MAX) {
			h->flags = 0;

			__jacl_quicklist[qidx].slots[__jacl_quicklist[qidx].count++] = p;

			return;
		}
	}

	/* Arena blocks leak */
	if (h->flags & JACL_HDR_ARENA) {
		h->flags = 0;

		return;
	}

	h->flags = 0;

	size_t off = (size_t)((uint8_t*)h - seg->base);

	/* Coalescing */
	if (h->size >= 1024) {
		__jacl_lock_acquire(&__jacl_lock);

		/* Coalesce next */
		size_t next_off = off + h->size;

		if (next_off < seg->size) {
			__jacl_hdr_t* next = (__jacl_hdr_t*)(seg->base + next_off);

			if (!(next->flags & JACL_HDR_ALLOC)) {
				__jacl_bin_remove(seg, next_off);

				h->size += next->size;

				uint32_t after_off = off + h->size;

				if (after_off < seg->size) ((__jacl_hdr_t*)(seg->base + after_off))->prev_size = h->size;
			}
		}

		/* Coalesce prev */
		if (off > 0 && h->prev_size > 0) {
			size_t prev_off = off - h->prev_size;

			__jacl_hdr_t* prev = (__jacl_hdr_t*)(seg->base + prev_off);

			if (!(prev->flags & JACL_HDR_ALLOC)) {
				__jacl_bin_remove(seg, prev_off);

				prev->size += h->size;

				size_t after_off = prev_off + prev->size;

				if (after_off < seg->size) ((__jacl_hdr_t*)(seg->base + after_off))->prev_size = prev->size;

				off = prev_off;
			}
		}

		__jacl_bin_push(seg, off);
		__jacl_lock_release(&__jacl_lock);
	} else {
		__jacl_lock_acquire(&__jacl_lock);
		__jacl_bin_push(seg, off);
		__jacl_lock_release(&__jacl_lock);
	}
}

void* calloc(size_t nmemb, size_t size) {
	if (nmemb && size > SIZE_MAX / nmemb) return NULL;

	size_t total = nmemb * size;
	void* p = malloc(total);

	if (p) memset(p, 0, total);

	return p;
}

void* realloc(void* ptr, size_t size) {
	if (!ptr) return malloc(size);
	if (size == 0) { free(ptr); return NULL; }

	__jacl_hdr_t* h = (__jacl_hdr_t*)ptr - 1;
	size_t old_size = h->size - sizeof(__jacl_hdr_t);

	if (size <= old_size) return ptr;

	void* new_ptr = malloc(size);

	if (!new_ptr) return NULL;

	size_t copy_size = old_size < size ? old_size : size;

	memcpy(new_ptr, ptr, copy_size);

	if (!(h->flags & JACL_HDR_ARENA)) free(ptr);

	return new_ptr;
}

void* aligned_alloc(size_t alignment, size_t size) {
	if (alignment == 0 || (alignment & (alignment-1)) != 0) return NULL;
	if (size % alignment != 0) return NULL;
	if (alignment <= JACL_ALIGNMENT) return malloc(size);

	size_t extra = alignment + sizeof(void*);
	void* raw = malloc(size + extra);

	if (!raw) return NULL;

	union { void* ptr; uintptr_t addr; } raw_conv, aligned_conv;

	raw_conv.ptr = raw;

	aligned_conv.addr = JACL_ALIGN_UP(raw_conv.addr + sizeof(void*), alignment);
	*(void**)((uint8_t*)aligned_conv.ptr - sizeof(void*)) = raw;


	return aligned_conv.ptr;
}

/* ============================================================= */
/* Exit Handler Registry                                         */
/* ============================================================= */

#define __JACL_ATEXIT_MAX 32

static struct {
	void (*func)(void);
	int used;
} __jacl_exit_handlers[__JACL_ATEXIT_MAX];

static int __jacl_exit_count = 0;

#if JACL_HAS_C11
static struct {
	void (*func)(void);
	int used;
} __jacl_quick_exit_handlers[__JACL_ATEXIT_MAX];

static int __jacl_quick_exit_count = 0;
#endif

int atexit(void (*func)(void)) {
	if (!func) return -1;

	for (int i = 0; i < __JACL_ATEXIT_MAX; i++) {
		if (!__jacl_exit_handlers[i].used) {
			__jacl_exit_handlers[i].func = func;
			__jacl_exit_handlers[i].used = 1;
			__jacl_exit_count++;

			return 0;
		}
	}

	return -1;
}

static inline void __jacl_exit_run_handlers(void) {
	for (int i = __JACL_ATEXIT_MAX - 1; i >= 0; i--) {
		if (__jacl_exit_handlers[i].used && __jacl_exit_handlers[i].func) {
			__jacl_exit_handlers[i].func();
		}
	}
}

#if JACL_HAS_C11

int at_quick_exit(void (*func)(void)) {
	if (!func) return -1;

	for (int i = 0; i < __JACL_ATEXIT_MAX; i++) {
		if (!__jacl_quick_exit_handlers[i].used) {
			__jacl_quick_exit_handlers[i].func = func;
			__jacl_quick_exit_handlers[i].used = 1;
			__jacl_quick_exit_count++;
			return 0;
		}
	}

	return -1;
}

static inline void __jacl_exit_run_quick_handlers(void) {
	for (int i = __JACL_ATEXIT_MAX - 1; i >= 0; i--) {
		if (__jacl_quick_exit_handlers[i].used && __jacl_quick_exit_handlers[i].func) {
			__jacl_quick_exit_handlers[i].func();
		}
	}
}

_Noreturn void quick_exit(int status) {
	__jacl_exit_run_quick_handlers();
	_Exit(status);

	for(;;);
}

#endif /* JACL_HAS_C11 */

void exit(int status) {
	__jacl_exit_run_handlers();

	#if JACL_HAS_C99
		_Exit(status);
	#else
		(void)status;

		abort();
	#endif
}


/* ============================================================= */
/* Sorting & Searching                                           */
/* ============================================================= */

static inline void __jacl_sort_swap(unsigned char* a, unsigned char* b, size_t size) {
	while (size--) {
		unsigned char tmp = *a;

		*a++ = *b;
		*b++ = tmp;
	}
}

static void __jacl_sort_again(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {
	if (nmemb < 2) return;

	unsigned char* arr = (unsigned char*)base;

	// Median-of-three pivot selection
	size_t low = 0;
	size_t high = nmemb - 1;
	size_t mid = low + (high - low) / 2;

	// Sort low, mid, high
	if (compar(arr + low * size, arr + mid * size) > 0) __jacl_sort_swap(arr + low * size, arr + mid * size, size);
	if (compar(arr + mid * size, arr + high * size) > 0) __jacl_sort_swap(arr + mid * size, arr + high * size, size);
	if (compar(arr + low * size, arr + mid * size) > 0) __jacl_sort_swap(arr + low * size, arr + mid * size, size);

	// Allocate pivot storage
	void* pivot = malloc(size);

	if (!pivot) {
		// Fallback to bubble sort
		for (size_t i = 0; i < nmemb - 1; i++) {
			for (size_t j = 0; j < nmemb - i - 1; j++) {
				if (compar(arr + j * size, arr + (j + 1) * size) > 0) __jacl_sort_swap(arr + j * size, arr + (j + 1) * size, size);
			}
		}

		return;
	}

	memcpy(pivot, arr + mid * size, size);

	// Partition
	size_t i = 0;
	size_t j = nmemb - 1;

	while (1) {
		while (i < nmemb && compar(arr + i * size, pivot) < 0) i++;
		while (j > 0 && compar(arr + j * size, pivot) > 0) j--;

		if (i >= j) break;

		__jacl_sort_swap(arr + i * size, arr + j * size, size);

		i++;

		if (j > 0) j--;
	}

	free(pivot);

	// Recursively sort partitions
	if (j > 0) __jacl_sort_again(arr, j + 1, size, compar);
	if (i < nmemb - 1) __jacl_sort_again(arr + (i + 1) * size, nmemb - i - 1, size, compar);
}

void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {
	if (!base || !compar || size == 0 || nmemb < 2) return;

	__jacl_sort_again(base, nmemb, size, compar);
}

void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {
	if (!key || !base || !compar || size == 0 || nmemb == 0)
		return NULL;

	const unsigned char* arr = (const unsigned char*)base;
	size_t low = 0;
	size_t high = nmemb;

	while (low < high) {
		size_t mid = low + (high - low) / 2;
		const void* mid_elem = arr + mid * size;
		int cmp = compar(key, mid_elem);

		if (cmp == 0) return (void*)mid_elem;
		else if (cmp < 0) high = mid;
		else low = mid + 1;
	}

	return NULL;
}

/* ============================================================= */
/* String Conversion                                             */
/* ============================================================= */
#define __jacl_ato_int(name, type, len, usig) type ato##name(const char* str) { \
	const char* p = str; \
	int read = 0, ch = 0; \
	uintmax_t val = 0; \
	__jacl_fmt_t spec = usig? 10 : 10 | JACL_FMT_VAL(FLAG, sign); \
	__jacl_read_skip(ch, NULL, &p, &read); \
	if (__jacl_input_int(NULL, &p, &read, JACL_FMT_SET(LENGTH, spec, len), INT_MAX, &val)) return (type)val; \
	return 0; \
}
#define __jacl_strto_int(suf, type, len, usig) type strto##suf(const char* restrict str, char** restrict endptr, int base) { \
	if (base != 0 && (base < 2 || base > 36)) { \
		errno = EINVAL; \
		if (endptr) *endptr = (char*)str; \
		return 0; \
	} \
	const char* p = str; \
	int read = 0, ch = 0; \
	uintmax_t val = 0; \
	__jacl_fmt_t spec = usig? base : base | JACL_FMT_VAL(FLAG, sign); \
	__jacl_read_skip(ch, NULL, &p, &read); \
	if (__jacl_input_int(NULL, &p, &read, JACL_FMT_SET(LENGTH, spec, len), INT_MAX, &val)) { \
		if (endptr) *endptr = (char*)p; \
		return (type)val; \
	} \
	if (endptr) *endptr = (char*)str; \
	return 0; \
}
#define __jacl_strto_float(suf, type, len) type strto##suf(const char* restrict str, char** restrict endptr) { \
	const char* p = str; \
	int read = 0, ch = 0; \
	long double val = 0.0; \
	__jacl_read_skip(ch, NULL, &p, &read); \
	__jacl_fmt_t spec = 10; \
	JACL_FMT_SET(LENGTH, spec, len); \
	if (__jacl_input_special(NULL, &p, &read, spec, INT_MAX, &val) || __jacl_input_float(NULL, &p, &read, spec, INT_MAX, &val)) { \
		if (endptr) *endptr = (char*)p; \
		return (type)val; \
	} \
	if (endptr) *endptr = (char*)str; \
}


__jacl_ato_int(i, int, 0, 0)
__jacl_ato_int(l, long, l, 0)

__jacl_strto_int(l, long, l, 0)
__jacl_strto_int(ul, unsigned long, l, 1)

#if JACL_HAS_C99
__jacl_ato_int(ll, long long, ll, 0)

__jacl_strto_int(ll, long long, ll, 0)
__jacl_strto_int(ull, unsigned long long, ll, 1)

__jacl_strto_float(f, float, 0)
__jacl_strto_float(d, double, l)
__jacl_strto_float(ld, long double, L)
#endif

double atof(const char* str) {
	const char* p = str;
	int _read = 0;
	long double val = 0.0;

	if (__jacl_input_float(NULL, &p, &_read, JACL_FMT_BASE_exp, INT_MAX, &val)) return (double)val;

	return 0.0;
}

#ifdef __cplusplus
}
#endif

#endif /* CORE_STDLIB_H */
