#!/bin/sh
# Build MaxSignal/Player 0.6.2.3-wince (no audio) with the LLVM/Clang
# WinCE toolchain. Overlay Makefiles only; do not patch app C++.
#
# Official Player zip only. Deps: zlib, libpng, pixman, libiconv,
# SDL 1.2.15 (GDI), liblcf 0.6.2.
set -eu

OVERLAY=$(CDPATH= cd -- "$(dirname "$0")/easyrpg-player" && pwd)
JOBS=${JOBS:-$(nproc)}
PREFIX=${PREFIX:?set PREFIX to the deps install prefix}
PLAYER_ZIP_URL=${PLAYER_ZIP_URL:-https://github.com/MaxSignal/Player/archive/refs/tags/0.6.2.3-wince.zip}
WORK=${WORK:-${TMPDIR:-/tmp}/easyrpg-wince}
CROSS=${CROSS:-arm-mingw32ce}

export PATH
mkdir -p "$PREFIX" "$WORK"
cd "$WORK"

fetch() {
  url=$1
  out=$2
  if [ ! -f "$out" ]; then
    curl -fsSL -o "$out" "$url"
  fi
}

need() {
  command -v "$1" >/dev/null || {
    echo "missing $1" >&2
    exit 1
  }
}

need "$CROSS-gcc"
need "$CROSS-g++"
need "$CROSS-ar"
need make
need curl

export CC="$CROSS-gcc"
export CXX="$CROSS-g++"
export AR="$CROSS-ar"
export RANLIB="$CROSS-ranlib"
export CPPFLAGS="-I$PREFIX/include ${CPPFLAGS:-}"
export LDFLAGS="-L$PREFIX/lib ${LDFLAGS:-}"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
export PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig"

if [ ! -f "$PREFIX/lib/libcecompat.a" ]; then
  mkdir -p "$PREFIX/include" "$PREFIX/lib"
  cp -a "$OVERLAY/ce-strerror.h" "$PREFIX/include/"
  "$CC" -O2 -c -o "$WORK/ce-strerror.o" "$OVERLAY/ce-strerror.c"
  "$AR" rcs "$PREFIX/lib/libcecompat.a" "$WORK/ce-strerror.o"
  "$RANLIB" "$PREFIX/lib/libcecompat.a"
fi

# --- zlib ---
if [ ! -f "$PREFIX/lib/libz.a" ]; then
  fetch https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz zlib-1.3.1.tar.gz
  rm -rf zlib-1.3.1
  tar -xzf zlib-1.3.1.tar.gz
  # zlib gzlib.c uses _lseeki64 on all _WIN32; CE has lseek. Do not patch zlib.
  # Build libz.a only — the gcc makefile's `all` also links example_d.exe.
  make -C zlib-1.3.1 -f win32/Makefile.gcc -j"$JOBS" \
    PREFIX="$CROSS-" \
    LOC="-D_lseeki64=lseek" \
    SHAREDLIB= SHAREDLIBV= SHAREDLIBM= \
    libz.a
  mkdir -p "$PREFIX/include" "$PREFIX/lib" "$PREFIX/lib/pkgconfig"
  cp -a zlib-1.3.1/zconf.h zlib-1.3.1/zlib.h "$PREFIX/include/"
  cp -a zlib-1.3.1/libz.a "$PREFIX/lib/"
  cat >"$PREFIX/lib/pkgconfig/zlib.pc" <<EOF
prefix=$PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include
Name: zlib
Version: 1.3.1
Libs: -L\${libdir} -lz
Cflags: -I\${includedir}
EOF
fi

# --- libpng ---
if [ ! -f "$PREFIX/lib/libpng16.a" ] && [ ! -f "$PREFIX/lib/libpng.a" ]; then
  fetch https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.43.tar.gz libpng-1.6.43.tar.gz
  rm -rf libpng-1.6.43
  tar -xzf libpng-1.6.43.tar.gz
  # Their scripts/makefile.gcc. Do not patch libpng C. libpng.a only (skip pngtest).
  make -C libpng-1.6.43 -f scripts/makefile.gcc -j"$JOBS" \
    CC="$CC" AR="$AR" RANLIB="$RANLIB" \
    ZLIBINC="$PREFIX/include" ZLIBLIB="$PREFIX/lib" \
    CFLAGS="-O2 -Wall -include $OVERLAY/ce-strerror.h" \
    libpng.a
  mkdir -p "$PREFIX/include" "$PREFIX/include/libpng16" "$PREFIX/lib"
  cp -a libpng-1.6.43/libpng.a "$PREFIX/lib/libpng.a"
  ln -sf libpng.a "$PREFIX/lib/libpng16.a"
  cp -a libpng-1.6.43/png.h libpng-1.6.43/pngconf.h libpng-1.6.43/pnglibconf.h \
    "$PREFIX/include/"
  cp -a libpng-1.6.43/png.h libpng-1.6.43/pngconf.h libpng-1.6.43/pnglibconf.h \
    "$PREFIX/include/libpng16/"
fi

# --- pixman (soft-float: no ARM SIMD/NEON) ---
if [ ! -f "$PREFIX/lib/libpixman-1.a" ]; then
  fetch https://www.cairographics.org/releases/pixman-0.42.2.tar.gz pixman-0.42.2.tar.gz
  rm -rf pixman-0.42.2
  tar -xzf pixman-0.42.2.tar.gz
  (
    cd pixman-0.42.2
    ./configure --host="$CROSS" --prefix="$PREFIX" \
      --disable-shared --enable-static \
      --disable-gtk --disable-libpng \
      --disable-arm-simd --disable-arm-neon --disable-arm-a64-neon \
      --disable-arm-iwmmxt --disable-mmx --disable-sse2 --disable-ssse3 \
      --disable-vmx --disable-mips-dspr2 --disable-loongson-mmi \
      pixman_cv_have_tls=no ac_cv_tls=none
    # `make all` also links test EXEs (CreateMutexA). Install the library only.
    make -C pixman -j"$JOBS"
    make -C pixman install
  )
fi

# --- libiconv (liblcf encoder without ICU) ---
if [ ! -f "$PREFIX/lib/libiconv.a" ]; then
  fetch https://ftp.gnu.org/gnu/libiconv/libiconv-1.17.tar.gz libiconv-1.17.tar.gz
  rm -rf libiconv-1.17
  tar -xzf libiconv-1.17.tar.gz
  (
    cd libiconv-1.17
    ./configure --host="$CROSS" --prefix="$PREFIX" \
      --disable-shared --enable-static --disable-nls
    make -j"$JOBS"
    make install
  )
fi

# --- SDL 1.2.15 ---
if [ ! -f "$PREFIX/lib/libSDL.a" ]; then
  fetch https://github.com/libsdl-org/SDL-1.2/archive/refs/tags/release-1.2.15.tar.gz SDL-1.2.15.tar.gz
  rm -rf SDL-1.2-release-1.2.15
  tar -xzf SDL-1.2.15.tar.gz
  cp -a "$OVERLAY/SDL_config.h" SDL-1.2-release-1.2.15/include/SDL_config.h
  cp -a "$OVERLAY/Makefile.sdl" SDL-1.2-release-1.2.15/Makefile
  make -C SDL-1.2-release-1.2.15 -j"$JOBS" CROSS="$CROSS" PREFIX="$PREFIX"
  make -C SDL-1.2-release-1.2.15 install CROSS="$CROSS" PREFIX="$PREFIX"
fi

# --- liblcf 0.6.2 ---
if [ ! -f "$PREFIX/lib/liblcf.a" ]; then
  fetch https://github.com/EasyRPG/liblcf/archive/refs/tags/0.6.2.tar.gz liblcf-0.6.2.tar.gz
  rm -rf liblcf-0.6.2
  tar -xzf liblcf-0.6.2.tar.gz
  cp -a "$OVERLAY/liblcf-config.h" liblcf-0.6.2/config.h
  cp -a "$OVERLAY/Makefile.liblcf" liblcf-0.6.2/Makefile
  make -C liblcf-0.6.2 -j"$JOBS" CROSS="$CROSS" PREFIX="$PREFIX"
  make -C liblcf-0.6.2 install CROSS="$CROSS" PREFIX="$PREFIX"
fi

# --- Player (official zip) ---
fetch "$PLAYER_ZIP_URL" player-0.6.2.3-wince.zip
rm -rf Player-0.6.2.3-wince
unzip -qo player-0.6.2.3-wince.zip
test -d Player-0.6.2.3-wince
cp -a "$OVERLAY/Makefile" Player-0.6.2.3-wince/Makefile
make -C Player-0.6.2.3-wince -j"$JOBS" CROSS="$CROSS" PREFIX="$PREFIX"

EXE=Player-0.6.2.3-wince/easyrpg-player.exe
test -s "$EXE"
DEST=${DEST:-$PREFIX/../player-wince}
mkdir -p "$DEST"
cp -a "$EXE" "$DEST/"
if command -v llvm-readobj >/dev/null; then
  llvm-readobj --file-headers "$EXE" > "$DEST/easyrpg-player.exe.headers.txt"
fi
{
  echo "upstream: MaxSignal/Player 0.6.2.3-wince"
  echo "zip: $PLAYER_ZIP_URL"
  echo "audio: off (no SUPPORT_AUDIO)"
  echo "ui: SDL 1.2 WINDIB"
  echo "player C++: unmodified (Makefile overlay)"
  sha256sum "$DEST/easyrpg-player.exe"
} > "$DEST/SHA256SUMS"
echo "built $EXE"
ls -l "$EXE"
