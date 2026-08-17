#!/bin/bash

# Compile and run the wildcard scan test against the locally built frida-gum.
# Run this from WSL in the frida-gum repo root.

set -e

SRCDIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILDDIR="$SRCDIR/build"
DEPSDIR="$SRCDIR/deps"
SDK="$DEPSDIR/sdk-linux-x86_64"
TOOLCHAIN="$DEPSDIR/toolchain-linux-x86_64"

export PKG_CONFIG_PATH="$BUILDDIR/meson-uninstalled:$DEPSDIR/sdk-linux-x86_64/lib/pkgconfig"
PKG_CONFIG="python3 $BUILDDIR/frida-pkg-config-linux-x86_64.py"

echo "Resolving build flags ..."
CFLAGS="$($PKG_CONFIG --cflags frida-gumjs-1.0) -DGUM_STATIC"
LIBS="$($PKG_CONFIG --libs frida-gumjs-1.0) -lm -lpthread -lrt -lresolv -ldl"

echo "Compiling test-wildcard-scan.c ..."
c++ -o "$BUILDDIR/test-wildcard-scan" \
  "$SRCDIR/tools/test-wildcard-scan.c" \
  $CFLAGS \
  $LIBS \
  -static-libgcc -static-libstdc++ \
  -Wl,-z,noexecstack -Wl,-z,relro -Wl,--gc-sections

echo ""
echo "Running test-wildcard-scan ..."
echo "=============================="
"$BUILDDIR/test-wildcard-scan"
