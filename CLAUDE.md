# JACLibc Development Guidelines

## Project Overview
JACLibc is a header-only C standard library targeting WASM-first with full POSIX/Windows support. Version 1.83.

## Coding Style

### File Structure
```c
/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef HEADER_H
#define HEADER_H
#pragma once

#include <config.h>
// ... other includes

#ifdef __cplusplus
extern "C" {
#endif

// ... content

#ifdef __cplusplus
}
#endif

#endif /* HEADER_H */
```

### Section Dividers
Use 61-character dividers for major sections:
```c
/* ============================================================= */
/* Section Name                                                  */
/* ============================================================= */
```

### Function Style
- **Always use `static inline`** for header-only implementation
- **Pointer declarations**: Space before asterisk: `char* name`, `void* ptr`
- **Restrict keyword**: Use `restrict` after type: `char* restrict dest`
- **Cast style**: Space after cast: `(unsigned char* )ptr`

### Error Handling
```c
// Use JACL_UNLIKELY for error paths
if (JACL_UNLIKELY(!ptr || !src)) { errno = EINVAL; return NULL; }

// Single-line conditionals for simple returns
if (!stream) { errno = EBADF; return EOF; }
```

### Variable Declarations
- Declare close to first use
- Blank line after declarations when followed by logic
```c
static inline int func(void) {
	int result = 0;
	char* ptr = buffer;

	// logic starts here
	for (int i = 0; i < n; i++) {
		// ...
	}

	return result;
}
```

### Conditional Compilation
```c
// Feature detection - use JACL_HAS_* macros
#if JACL_HAS_C99
// C99 code
#endif

#if JACL_HAS_POSIX
// POSIX-specific code
#endif

// Multi-condition checks - use #if defined()
#if defined(__ARCH_CLONE) && JACL_OS_LINUX
// NOT: #ifdef __ARCH_CLONE && JACL_OS_LINUX (invalid!)
#endif
```

### X-Macro Pattern for Syscalls
Syscall files use X-macros for enum generation:
```c
// In x/os_arch.h - each syscall MUST have unique name
X(SYS_read, 63, read)
X(SYS_write, 64, write)

// For aliases (same syscall, different names), use different SYS_* names:
X(SYS_fcntl64, 25, fcntl64)
X(SYS_fcntl, 25, fcntl)      // OK: different enum name, same number

// NEVER duplicate the enum name - this breaks compilation:
// X(SYS_nosys, 32, nosys)   // WRONG if SYS_nosys defined elsewhere
// X(SYS_nosys, 97, nosys)   // WRONG - redefinition!
```

### Indentation & Spacing
- **Tabs** for indentation (not spaces)
- **Blank line** between function definitions
- **Blank line** after opening brace of function with declarations
- **No blank line** before closing brace

### Loop Style
```c
// Simple loops on one line when body is short
for (size_t i = 0; i < n; i++) d[i] = s[i];

// Multi-line for complex logic
for (size_t i = 0; i < n; i++) {
	if (x[i] != y[i]) return x[i] < y[i] ? -1 : 1;
}
```

### Comments
- Minimal inline comments - code should be self-documenting
- Use `//` for short notes, `/* */` for documentation blocks
- No trailing comments on code lines except for critical notes

### Return Statements
```c
// Ternary for simple value selection
return (*a < *b) ? -1 : (*a > *b);

// Direct return for simple cases
return dest;

// Set errno before returning error
errno = EINVAL;
return -1;
```

## Architecture-Specific Files

### Location: `include/arch/*.h`
Each file handles multiple phases via `#ifdef` guards:
- `__ARCH_CONFIG` - Architecture detection and macros
- `__ARCH_SYSCALL` - Syscall implementation
- `__ARCH_START` - Program entry point
- `__ARCH_TLS` - Thread-local storage
- `__ARCH_CLONE` - Thread creation (Linux only)

### Location: `include/x/*.h`
Syscall number tables using X-macro format:
- `darwin_arm64.h`, `linux_arm64.h`, etc.
- Format: `X(SYS_name, number, symbol)`

### Location: `include/os/*.h`
OS-specific implementations:
- `darwin.h`, `linux.h`, `windows.h`, `wasi.h`

## Build & Test

### Compilation
```bash
# Standard compilation with JACLibc
cc -I./include -o program source.c jaclib.c -lm

# Using all-in-one header (define JACL_MAIN in main file)
#define JACL_MAIN
#include <jaclibc.h>
```

### Testing
Tests use `<testing.h>` framework:
```c
#include <testing.h>

TEST_TYPE(unit);
TEST_UNIT(header.h);
TEST_SUITE(function_name);

TEST(specific_test_case) {
	// Setup
	char buf[256] = {0};

	// Action
	snprintf(buf, sizeof(buf), "%d", 42);

	// Assert
	ASSERT_STR_EQ("42", buf);
}

TEST_MAIN()
```

## Common Patterns

### NULL/Error Checking
```c
if (JACL_UNLIKELY(!ptr)) { errno = EINVAL; return NULL; }
```

### Buffer Initialization
```c
char buf[256] = {0};  // Zero-initialize fixed buffers
```

### Size Calculations
```c
size_t count = sizeof(array) / sizeof(array[0]);
```

## Key Macros (from config.h)
- `JACL_HAS_C99`, `JACL_HAS_C11`, `JACL_HAS_C23` - C standard detection
- `JACL_HAS_POSIX` - POSIX availability
- `JACL_LIKELY(x)`, `JACL_UNLIKELY(x)` - Branch prediction hints
- `JACL_ARCH_*` - Architecture flags (ARM64, X64, WASM, etc.)
- `JACL_OS_*` - OS flags (LINUX, DARWIN, WINDOWS, WASI)

## Known Issues to Fix
1. `include/x/darwin_arm64.h` - Duplicate `SYS_nosys` enum values
2. `include/arch/arm64.h:79` - Invalid `#ifdef X && Y` syntax
