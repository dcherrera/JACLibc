#!/bin/sh
# (c) 2025 FRINKnet & Friends – MIT licence

set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"

rm -f "$ROOT/include/x/"config_has.h
rm -f "$ROOT/include/x/"linux*
rm -f "$ROOT/include/x/"darwin*
rm -f "$ROOT/include/x/"freebsd*
rm -f "$ROOT/include/x/"openbsd*
rm -f "$ROOT/include/x/"netbsd*
rm -f "$ROOT/include/x/"dragonfly*

# OS versions
linux_ver="v6.17"
darwin_ver="main"
freebsd_ver="main"
netbsd_ver="HEAD"
openbsd_ver="HEAD"
dragonfly_ver="master"

# Base URLs
linux_url="https://raw.githubusercontent.com/torvalds/linux/${linux_ver}"
darwin_url="https://raw.githubusercontent.com/apple-oss-distributions/xnu/${darwin_ver}"
freebsd_url="https://cgit.freebsd.org/src/plain"
netbsd_url="http://cvsweb.netbsd.org/bsdweb.cgi/src/sys/kern"
openbsd_url="https://cvsweb.openbsd.org/cgi-bin/cvsweb/~checkout~/src"
dragonfly_url="https://gitweb.dragonflybsd.org/dragonfly.git/blob_plain/${dragonfly_ver}:"

# get X Files
xfiles() {
  os="$1"
  arch="$2"
  shift 2

  file="${os}_${arch}.h"
  gen="${ROOT}/include/x/${file}"
  tmp="${gen}.tmp"
  title=$(echo "$os" | tr '[:lower:]' '[:upper:]')
  base_url="$(eval echo "\$${os}_url")"

  printf "%-40s %s\n" "Parsing $os $arch syscalls..." "$file"

  # Clear temp file
  : > "$tmp"

  # Fetch and concatenate all source files
  for rel in "$@"; do
    url="${base_url}${rel}"
    if ! curl -fsSL "$url" >> "$tmp" 2>/dev/null; then
      echo "ERROR: Failed to fetch $url" >&2
      rm -f "$tmp"
      exit 1
    fi
  done

  # Generate header with all source URLs listed
  {
    echo "/**"
    echo " * $title SYSCALLS IMPORTED  //  last updated: $(date -u +"%Y-%m-%d %H:%M:%S UTC")"
    for rel in "$@"; do
      echo " * ${base_url}${rel}"
    done
    echo " */"
    echo ""
    awk -varch="$arch" -f "$ROOT/utils/get_syscalls_${os}.awk" "$tmp"
  } > "$gen"

  if [ ! -s "$gen" ]; then
    echo "ERROR: awk failed $file from $base_url" >&2
    #rm -f "$tmp"
    exit 1
  fi

  rm "$tmp"
  sleep 1 && true
}

# gen X Stubs
xstubs() {
  os="$1"
  arch="$2"
  file="${os}_${arch}.h"

  {
    printf "%-40s %s\n" "Linking $os $arch syscalls..." "$file"
    cd "$ROOT/include/x"
    ln -sf ${os}_generic.h $file
  }
}

# find X syscalls
xfinds() {
  name="$1"
  config="$ROOT/include/x/config_has.h"

  # First call: initialize the header
  if [ ! -f "$config" ]; then
    {
      echo "/**"
      echo " * JACLIBC SYSCALLS MATRIX  //  last updated: $(date -u +"%Y-%m-%d %H:%M:%S UTC")"
      echo " */"
    } > "$config"
  fi

  printf "%-40s %s\n" "Exporting for JACL_HASSYS(${name})..."
  printf "\n/* check SYS_%s */\n" $name >> "$config"

  # Search all generated headers for this syscall
  for header in "$ROOT/include/x/"*_*.h; do
    os_arch=$(basename "$header" .h)

    if grep -q "SYS_${name}," "$header" 2>/dev/null; then
      printf "#define %-20s %s\n" __${os_arch}_has_${name} 1 >> "$config"
    else
      printf "#define %-20s %s\n" __${os_arch}_has_${name} 0 >> "$config"
    fi
  done
}

# Linux
xfiles  linux      generic    "/scripts/syscall.tbl"
xfiles  linux      x64        "/arch/x86/entry/syscalls/syscall_64.tbl"
xfiles  linux      x86        "/arch/x86/entry/syscalls/syscall_32.tbl"
xfiles  linux      arm32      "/arch/arm/tools/syscall.tbl"
xfiles  linux      mips32     "/arch/mips/kernel/syscalls/syscall_n32.tbl"
xfiles  linux      mips64     "/arch/mips/kernel/syscalls/syscall_n64.tbl"
xfiles  linux      hppa       "/arch/parisc/kernel/syscalls/syscall.tbl"
xfiles  linux      powerpc32  "/arch/powerpc/kernel/syscalls/syscall.tbl"
xfiles  linux      powerpc64  "/arch/powerpc/kernel/syscalls/syscall.tbl"
xfiles  linux      s390x      "/arch/s390/kernel/syscalls/syscall.tbl"
xfiles  linux      sparc32    "/arch/sparc/kernel/syscalls/syscall.tbl"
xfiles  linux      sparc64    "/arch/sparc/kernel/syscalls/syscall.tbl"
xfiles  linux      sh4        "/arch/sh/kernel/syscalls/syscall.tbl"
xfiles  linux      xtensa     "/arch/xtensa/kernel/syscalls/syscall.tbl"
xstubs  linux      alpha
xstubs  linux      arc
xstubs  linux      arm64
xstubs  linux      csky
xstubs  linux      hexagon
xstubs  linux      loongarch64
xstubs  linux      m68k
xstubs  linux      microblaze
xstubs  linux      nios2
xstubs  linux      or1k
xstubs  linux      riscv32
xstubs  linux      riscv64

# Darwin
xfiles  darwin     generic   "/bsd/kern/syscalls.master"
xstubs  darwin     x64
xstubs  darwin     arm64

# FreeBSD
xfiles  freebsd    generic   "/sys/kern/syscalls.master"
xstubs  freebsd    x64
xstubs  freebsd    x86

# NetBSD
xfiles  netbsd     generic   "/syscalls.master?rev=${netbsd_ver}&content-type=text/plain"
xstubs  netbsd     alpha
xstubs  netbsd     x64
xstubs  netbsd     x86
xstubs  netbsd     arm32
xstubs  netbsd     arm64
xstubs  netbsd     powerpc32
xstubs  netbsd     powerpc64
xstubs  netbsd     sparc32
xstubs  netbsd     sparc64
xstubs  netbsd     riscv32
xstubs  netbsd     riscv64
xstubs  netbsd     ia64
xstubs  netbsd     hppa
xstubs  netbsd     vax

# OpenBSD
xfiles  openbsd    generic   "/sys/kern/syscalls.master"
xstubs  openbsd    x64
xstubs  openbsd    x86

# Dragonfly
xfiles  dragonfly  generic   "/sys/kern/syscalls.master"
xstubs  dragonfly  x64
xstubs  dragonfly  x86

# Windows
xstubs  windows    x64
xstubs  windows    x86
xstubs  windows    arm64

# Config
grep -R "if JACL_HASSYS" "$ROOT" \
  | sed -n 's/.*JACL_HASSYS(\([^)]*\)).*/\1/p' \
  | sort -u \
  | while read name; do
    xfinds "$name"
  done
