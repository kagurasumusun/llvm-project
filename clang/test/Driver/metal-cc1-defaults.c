// The cc1 options the Metal driver seeds for every Metal compilation.
//
// Each CHECK below is transcribed from a measured `metal -cc1` invocation
// (reference/metal-ast-macos-air64/meta/metal-cc1-invocations.txt; the set
// is uniform across all 18 target/OS tuples):
//
//   warning configuration   (27 flags, emitted before user -W options)
//   -fno-verbose-asm -no-integrated-as
//   -fvisibility-inlines-hidden-static-local-var
//   -fno-new-infallible      (operator new may fail in device code)
//   -fno-autolink            (off even though the target is Darwin)
//   -fencode-extended-block-signature
//   -fregister-global-dtors-with-atexit
//   -mllvm -treat-scalable-fixed-error-as-warning
//   -mframe-pointer=all -faligned-alloc-unavailable -fno-strict-return
//   -fcommon -no-opaque-pointers -ferror-limit 19
//   -fmetal-math-fp32-functions=fast
//
// RUN: %clang -target air64_v28-apple-macosx26.0.0 -x metal -std=metal4.0 \
// RUN:   -fsyntax-only -### %s 2>&1 | FileCheck %s --check-prefix=METAL
// RUN: %clang -target air64_v28-apple-macosx26.0.0 -x metal -std=metal4.0 \
// RUN:   -fsyntax-only -ferror-limit 7 -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LIMIT
// The Metal driver contract is `-c input.metal -o output.air` -> one cc1
// invocation that emits raw LLVM bitcode.  In particular it must not schedule
// a generic backend job after the frontend: that job consumes the bitcode and
// overwrites the requested .air output with an object file.
// RUN: %clang --target=air64-apple-ios26.0 -miphoneos-version-min=26.0 -x metal -std=metal3.2 \
// RUN:   -c -o %t.air -### %s 2>&1 | FileCheck %s --check-prefix=AIR
// RUN: %clang --target=air64-apple-ios26.0 -miphoneos-version-min=26.0 -x metal -std=metal3.2 \
// RUN:   -c -o %t.air %s && od -An -tx1 -N4 %t.air | FileCheck %s --check-prefix=AIR-MAGIC

// METAL: "-cc1"

// METAL-DAG: "-Wdeprecated-objc-isa-usage"
// METAL-DAG: "-Werror=deprecated-objc-isa-usage"
// METAL-DAG: "-Werror=implicit-function-declaration"
// METAL-DAG: "-Wuninitialized"
// METAL-DAG: "-Wunused-variable"
// METAL-DAG: "-Wunused-value"
// METAL-DAG: "-Wunused-function"
// METAL-DAG: "-Wsign-compare"
// METAL-DAG: "-Wreturn-type"
// METAL-DAG: "-Wmissing-braces"
// METAL-DAG: "-Wformat-nonliteral"
// METAL-DAG: "-Wno-reorder-init-list"
// METAL-DAG: "-Wno-implicit-int-float-conversion"
// METAL-DAG: "-Wno-c99-designator"
// METAL-DAG: "-Wno-final-dtor-non-final-class"
// METAL-DAG: "-Wno-extra-semi-stmt"
// METAL-DAG: "-Wno-misleading-indentation"
// METAL-DAG: "-Wno-quoted-include-in-framework-header"
// METAL-DAG: "-Wno-implicit-fallthrough"
// METAL-DAG: "-Wno-enum-enum-conversion"
// METAL-DAG: "-Wno-enum-float-conversion"
// METAL-DAG: "-Wno-elaborated-enum-base"
// METAL-DAG: "-Wno-reserved-identifier"
// METAL-DAG: "-Wno-gnu-folding-constant"
// METAL-DAG: "-Wno-objc-load-method"
// METAL-DAG: "-Wmtl-shader-return-type"
// METAL-DAG: "-Werror=mtl-shader-return-type"

// METAL-DAG: "-fno-verbose-asm"
// METAL-DAG: "-no-integrated-as"
// METAL-DAG: "-fvisibility-inlines-hidden-static-local-var"
// METAL-DAG: "-fno-new-infallible"
// METAL-DAG: "-fno-autolink"
// METAL-DAG: "-fencode-extended-block-signature"
// METAL-DAG: "-fregister-global-dtors-with-atexit"
// METAL-DAG: "-mllvm" "-treat-scalable-fixed-error-as-warning"
// METAL-DAG: "-mframe-pointer=all"
// METAL-DAG: "-faligned-alloc-unavailable"
// METAL-DAG: "-fno-strict-return"
// METAL-DAG: "-fcommon"
// METAL-DAG: "-no-opaque-pointers"
// METAL-DAG: "-ferror-limit" "19"
// METAL-DAG: "-fmetal-math-fp32-functions=fast"

// !air.source_file_name carries the base name of the input file, which is
// seeded through -main-file-name.
// METAL-DAG: "-main-file-name" "metal-cc1-defaults.c"

// A user supplied -ferror-limit wins over the Metal default.
// LIMIT: "-ferror-limit" "7"
// LIMIT-NOT: "-ferror-limit" "19"

// AIR-COUNT-1: "-cc1"
// AIR: "-triple" "air64_v28-apple-ios26.0.0"
// AIR: "-x" "metal"
// AIR: "-emit-llvm-bc"
// AIR: "-o" "{{.*}}.air"
// AIR-MAGIC: 42 43 c0 de
