#!/bin/sh
# Install CeGCC/mingw32ce tool *names* that third-party Makefiles already use.
# Each name only binds --target=arm-pc-wince (or is a binutils alias).
# Do not add extra flags. Do not patch the application.
set -eu
BIN=${1:?usage: bind-cegcc-names.sh <toolchain-bin-dir>}
cd "$BIN"

clang_target() {
  tgt=$1
  src=$2
  cat >"$tgt" <<EOF
#!/bin/sh
exec "\$(dirname "\$0")/$src" --target=arm-pc-wince "\$@"
EOF
  chmod +x "$tgt"
}

clang_target arm-mingw32ce-gcc clang
clang_target arm-mingw32ce-g++ clang++
clang_target arm-mingw32ce-cc clang
clang_target arm-mingw32ce-c++ clang++

cat >arm-mingw32ce-cpp <<'EOF'
#!/bin/sh
exec "$(dirname "$0")/clang" --target=arm-pc-wince -E "$@"
EOF
chmod +x arm-mingw32ce-cpp

cat >arm-mingw32ce-windres <<'EOF'
#!/bin/sh
# CeGCC's arm-mingw32ce-windres is already a target-specific binary.
exec "$(dirname "$0")/llvm-windres" --target=arm-pc-wince "$@"
EOF
chmod +x arm-mingw32ce-windres

ln -sf llvm-ar arm-mingw32ce-ar
ln -sf llvm-ranlib arm-mingw32ce-ranlib
ln -sf llvm-nm arm-mingw32ce-nm
ln -sf llvm-objdump arm-mingw32ce-objdump
if [ -e llvm-strip ]; then
  ln -sf llvm-strip arm-mingw32ce-strip
elif [ -e llvm-objcopy ]; then
  cat >arm-mingw32ce-strip <<'EOF'
#!/bin/sh
exec "$(dirname "$0")/llvm-objcopy" --strip-unneeded "$@"
EOF
  chmod +x arm-mingw32ce-strip
fi

# Also the arm-wince-mingw32ce spelling some Makefiles use.
for t in gcc g++ cc c++ cpp windres ar ranlib nm objdump strip; do
  [ -e arm-mingw32ce-$t ] && ln -sf arm-mingw32ce-$t arm-wince-mingw32ce-$t
done
