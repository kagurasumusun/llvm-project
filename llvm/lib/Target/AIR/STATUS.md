# Metal (AIR) Implementation Status & Plan

Last updated: 2026-07-28.

## Current State

### Kept: Metal language / frontend support

The clang Metal frontend and LLVM target plumbing are **kept** and unchanged:

- `Language::Metal`, `-x metal`, `.metal` input detection.
- Metal keywords (`kernel`, `vertex`, `fragment`, `device`, `constant`,
  `threadgroup`, `thread`, `mesh`, `tile`, `ray_data`, ...), `[[...]]`
  attribute parsing, Sema validation.
- `air32` / `air64` target triples (`llvm/lib/TargetParser/Triple.cpp`,
  `Triple::isAIR()`), AIR data layouts (`TargetDataLayout.cpp`), and
  `clang/lib/Basic/Targets/AIR.h` (`AIRTargetInfo` with device=AS1,
  constant=AS2, threadgroup=AS3, thread=AS0).
- AIR metadata / intrinsic emission in CodeGen (`CGMetalBuiltins.cpp`,
  686 builtins mapped), `!air.kernel` / `!air.version` /
  `!air.language_version` metadata.
- Metal standard library headers (`clang/lib/Headers/metal/**`) and the
  generator utilities (`clang/utils/metal/*.py`).
- Metal tests (`clang/test/{Parser,Sema,CodeGen,AST,Preprocessor,Frontend}/metal-*`).

### Kept: Metal runtime library stubs (cleanroom)

`llvm/lib/Target/AIR/Runtime/**`
(`libair_rt`, `libmetal_rt`, `libmetal_math_rt`, `libtracepoint_rt`, ...).

These are cleanroom C implementations derived from
[metal-info](https://github.com/kagurasumusun/metal-info)
(`research/spec/RTLIB_CLEANROOM_MAP.md`, 13,067 symbols).
They are **not** built by LLVM's CMake (no CMakeLists); they are compiled
by the `metal-arm64` workflows.

### Removed (2026-07-28): incomplete typed-pointer bitcode writer

The following **incomplete, non-compiling, work-in-progress** code was
removed to restore a clean state:

| Piece | Why it was removed |
|-------|--------------------|
| `BitcodeEmitMode` 3-mode hack in `llvm/lib/Bitcode/Writer/BitcodeWriter.{h,cpp}` | Contaminated the standard (opaque-pointer) writer with Metal-specific modes; did not even compile (`BitcodeEmitMode::Normal` vs `Opaque` mix-up). The opaque bitcode path must stay pristine upstream. |
| `-mllvm -air-bitcode-mode=` interception in `clang/lib/CodeGen/BackendUtil.cpp` | Same: mixed Metal emission into the standard `Backend_EmitBC` path. |
| `AIRBitcodeEmitMode` enum in `clang/Basic/CodeGenOptions.{h,def}` | Only existed for the above. |
| `llvm/lib/Target/AIR/AIRWriter/` (`AIRBitcodeWriter`, `AIRPointerTypeAnalysis`, `AIRValueEnumerator`) | Incomplete standalone typed-pointer writer, unverified against real Metal runtime. |
| `llvm/lib/Target/AIR/AIRPrepare.{h,cpp}` + `AIR.h` | Attribute-strip / bitcast-insert pass that only served the removed writer (it `#include`d `AIRWriter/AIRPointerTypeAnalysis.h`). The attribute whitelist research is preserved in metal-info (`research/spec/IR_GROUND_TRUTH.md` §6.4) and in git history for reuse in the proper implementation. |
| `llvm/tools/llvm-metallib/` | Incomplete `.air` / `.metallib` container tool (symbol table was a stub: single `"default"` symbol, fake SHA-256 UUID, never linked into the build). |
| `llvm/CMakeLists.txt` `AIR` in `LLVM_ALL_EXPERIMENTAL_TARGETS`, `llvm/lib/Target/AIR/CMakeLists.txt` | Registry entries for the removed library. |

The standard LLVM bitcode writer (`llvm/lib/Bitcode/Writer/`) is now
**identical to upstream llvmorg-22.1.8** — opaque pointers only, no Metal
modes. `clang -emit-llvm-bc` for `air64-*` triples therefore emits ordinary
opaque-pointer bitcode for now.

## Plan: proper AIR implementation (to be done)

AIR emission will be **reimplemented properly** as a separate component. The
design constraints learned from the failed attempt:

1. **Never** add modes to the upstream `BitcodeWriter`. The opaque path
   stays untouched.
2. Metal needs `air` bitcode as Apple emits it. Per metal-info ground truth
   (`research/spec/IR_GROUND_TRUTH.md`), Apple's real AIR bitcode parses
   with vanilla modern LLVM (opaque pointers, bitcode module version 2);
   old metalfe-era containers use typed-pointer bitcode with version 1.
   The exact encoding target must be verified against golden `.air` files in
   `metal-info/research/golden/**` before writing code.
3. Container format is specified in `research/spec/METALLIB_WRITER_SPEC.md`
   (machine-verified on real hardware): `MTLB` slice header (88 bytes),
   symbol tags (`NAME`/`TYPE`/`HASH`/`OFFT`/`VERS`/`MDSZ`/`ENDT`),
   `HDYN`/`RLST`/`UUID`, bitcode wrapper (`0x0B17C0DE`), fat binary
   (`0xCAFEBABE` 64-bit) for `.metallib`.
4. Attribute whitelist for old metalfe readers:
   `research/spec/IR_GROUND_TRUTH.md` §6.4.
5. Runtime libraries to link against: `llvm/lib/Target/AIR/Runtime/**`
   (already present, see above).

Note: `.github/workflows/metal-arm64.yml`, `workflows/metal-arm64.yml` and
`.github/workflows/linux-build.yml` still reference an `llvm-metallib`
binary in their CI pipeline stages. Those stages describe the intended
future pipeline (Metal source → .bc → .air → .metallib) and will need to be
rewritten once the proper AIR writer / container tool lands; they are
expected to fail until then.

## References (metal-info)

- `research/spec/AIR_VOCABULARY.md` — AIR intrinsic vocabulary
- `research/spec/IR_GROUND_TRUTH.md` — ground-truth analysis of real Apple AIR bitcode
- `research/spec/METALLIB_WRITER_SPEC.md` — .air / .metallib container spec
- `research/spec/LEGACY_METAL_SUPPORT.md` — Metal 1.0–4.1 std flags ↔ AIR versions mapping
- `research/spec/CLANG_FRONTEND_IMPL_MAP.md` — frontend implementation map
- `clang/utils/metal/fetch-metal-info-resource.py` — helper to pull these into tree

## File map (current)

```
clang/lib/CodeGen/CGMetalBuiltins.cpp          ← AIR intrinsic emission (kept)
clang/lib/CodeGen/CodeGenFunction.{h,cpp}      ← AIR metadata emission (kept)
clang/lib/Basic/Targets/AIR.h                  ← AIRTargetInfo (kept)
clang/lib/Headers/metal/**                     ← Metal stdlib headers (kept)
llvm/lib/TargetParser/Triple.cpp               ← air32/air64 triples (kept)
llvm/lib/Target/AIR/Runtime/**                 ← runtime stubs (kept)
llvm/lib/Target/AIR/STATUS.md                  ← this file
llvm/lib/Bitcode/Writer/BitcodeWriter.{h,cpp}  ← PRISTINE upstream (no Metal code)
clang/lib/CodeGen/BackendUtil.cpp              ← upstream (no -air-bitcode-mode)
```
