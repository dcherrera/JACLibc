# JACLibc

Lean, portable, header-only C standard library. WASM-first, works on desktop, mobile, edge, IoT, bare metal, and browsers.

## Project Structure

```
JACLibc/
├── CLAUDE.md
├── README.md
├── STANDARDS.md             # C standards compliance notes
├── LEGAL.md                 # Copyright assignment policy
├── jaclib.c                 # Single-file implementation (alternative to header-only)
├── include/
│   ├── jaclibc.h            # Main umbrella header
│   ├── config.h             # Platform/compiler detection
│   ├── static.h             # Cherry-pick inclusion helper
│   ├── jsio.h               # WASM <-> JavaScript interchange objects
│   ├── testing.h            # Built-in test framework
│   ├── immintrin.h          # SIMD abstraction (SSE2/NEON/WASM SIMD)
│   ├── arch/                # Architecture-specific headers
│   ├── core/                # Core internal headers
│   ├── os/                  # OS-specific backends (linux, darwin, windows, wasm, none)
│   ├── sys/                 # System headers (types, stat, mman, socket, etc.)
│   ├── fmt/                 # Formatting internals
│   ├── x/                   # Extended/experimental headers
│   └── <standard headers>   # stdlib.h, stdio.h, string.h, math.h, pthread.h, etc.
├── syslib/                  # System library helpers
├── tests/                   # Test suite
├── utils/                   # Utility scripts
├── web/                     # WASM hosting/loader
├── build.sh                 # Unix build script
└── build.bat                # Windows build script
```

## Tech Stack

- **Language**: C (C89-C23 compatibility)
- **Type**: Header-only library (or link `jaclib.c`)
- **Targets**: x86, x64, ARM32, ARM64, RISC-V, WASM
- **OSes**: Linux, BSD, macOS, Windows, WASI, browsers, bare metal
- **Compilers**: clang, GCC, TCC, XCC (small compilers supported)

## Usage Patterns

### Header-only (recommended)

```c
#define JACL_MAIN    // In the file containing main()
#include <jaclibc.h>
```

### Link against single .c file

```c
#include <jaclibc.h>
// Then compile and link jaclib.c
```

### Cherry-pick headers

```c
#include <static.h>  // See this file for selective inclusion
```

## Build Commands

```bash
# Build tests
./build.sh

# Run tests
./build.sh test

# Cross-compile for WASM
clang --target=wasm32 -nostdlib -I include -c myapp.c
```

## Testing

Uses built-in `<testing.h>` framework:

```c
#include <testing.h>

void test_example(void) {
    ASSERT(1 + 1 == 2);
}

int main(void) {
    RUN_TEST(test_example);
    return 0;
}
```

Tests live in `tests/` directory.

## Key Features

- **Complete libc**: malloc, string, math, stdio, pthreads, atomics
- **WASM-first**: No Emscripten dependency, targets browser and WASI natively
- **jsio.h**: Bidirectional WASM <-> JavaScript object exchange (not just JSON)
- **SIMD abstraction**: `<immintrin.h>` works across SSE2, NEON, and WASM SIMD
- **Bare metal**: `os/none.h` provides stubs for freestanding environments
- **Testing framework**: `<testing.h>` for TDD

## Bare Metal Notes

On bare metal (`__STDC_HOSTED__ == 0`):
- Uses `os/none.h` which stubs syscalls to `-ENOSYS`
- `__jacl_grow_heap()` returns NULL (no mmap/VirtualAlloc)
- `printf` -> `fputc` -> `write()` -> syscall -> `-ENOSYS`
- Consumer must wire up I/O (e.g., BootAI overrides stdout to UEFI ConOut)

## Coding Conventions

- Header-only: all functions `static inline` where possible
- 1K line cap per header file
- Trust the compiler for link-time optimization
- Polyfills and fallbacks for small/baby compilers
- Source code IS the documentation — comments inline, grep-friendly
- C89-C23 compatibility range

## Commit Rules

- Never mention Claude, AI, or generated code in commits
- All commits are authored by the developer - no co-author tags
- Write commit messages in first person as the developer
- Keep commit messages concise and descriptive of the changes

## Communication Rules

**Always explain before acting.** Before running any command or making changes:
- Give a short, concise explanation of WHAT you're about to do and WHY
- Wait for approval on anything destructive

## Relationship to RWKVx

JACLibc is a foundational dependency used by:
- **BootAI** — provides libc for bare-metal UEFI (no glibc available)
- **Foundry** — used for WASM compilation target
- **Pantheon** (planned) — zero-dependency C inference engine

It is a standalone project with its own repo (`dcherrera/JACLibc`), included as a git submodule.
