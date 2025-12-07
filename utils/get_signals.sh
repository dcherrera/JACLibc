#!/bin/sh
# (c) 2025 FRINKnet & Friends – MIT licence

set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"

# clean old generated signal X-files
rm -f "$ROOT/include/x/"signals*

# OS versions (keep in sync with syscall fetcher)
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
netbsd_url="http://cvsweb.netbsd.org/bsdweb.cgi/src"
openbsd_url="https://cvsweb.openbsd.org/cgi-bin/cvsweb/~checkout~/src"
dragonfly_url="https://gitweb.dragonflybsd.org/dragonfly.git/blob_plain/${dragonfly_ver}:"

xfiles() {
  os="$1"
  out="${ROOT}/include/x/signals_${os}.h"
  title=$(echo "$os signals" | tr '[:lower:]' '[:upper:]')
  base_var="${os}_url"
  base="$(eval echo "\$$base_var")"
  shift

  tmp="${out}.tmp"
  : > "$tmp"

  printf "%-40s %s\n" "Parsing $os signals..." "signals_${os}.h"

  # Fetch and concat all given relative paths
  for rel in "$@"; do
    url="${base}${rel}"
    if ! curl -fsSL "$url" >> "$tmp" 2>/dev/null; then
      echo "ERROR: Failed to fetch $url" >&2
      rm -f "$tmp"
      exit 1
    fi
  done

  {
    echo "/**"
    echo " * ${title} IMPORTED  //  last updated: $(date -u +"%Y-%m-%d %H:%M:%S UTC")"
    for rel in "$@"; do
      echo " * ${base}${rel}"
    done
    echo " */"
    echo
    if [ "$os" = "linux" ]; then
      awk -f "$ROOT/utils/get_signals_linux.awk" "$tmp"
    else
      awk -f "$ROOT/utils/get_signals_bsd.awk" "$tmp"
    fi
  } > "$out"

  if [ ! -s "$out" ]; then
    echo "ERROR: awk failed for $os" >&2
    rm -f "$tmp" "$out"
    exit 1
  fi

  rm -f "$tmp"
  sleep 1 || true
}

xfiles  linux      "/include/uapi/asm-generic/signal.h" \
                   "/include/uapi/asm-generic/signal-defs.h" \
                   "/include/uapi/asm-generic/siginfo.h"
xfiles  darwin     "/bsd/sys/signal.h"
xfiles  freebsd    "/sys/sys/signal.h"
xfiles  netbsd     "/sys/sys/signal.h?rev=${netbsd_ver}&content-type=text/plain"
xfiles  openbsd    "/sys/sys/signal.h" \
                   "/sys/sys/siginfo.h"
xfiles  dragonfly  "/sys/sys/signal.h"
