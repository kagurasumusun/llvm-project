# Metal clang bring-up handoff

Last updated: 2026-07-24 JST.

## Repository / branch

- Repository: `kagurasumusun/llvm-project`
- Primary working branch: `metal-test`
- Default branch used to host GitHub Actions workflows: `metal`
- Work style constraint: do **not** full-clone `metal-info`; consume it through GitHub API / raw single-file downloads. For `llvm-project`, prefer GitHub blob/tree/commit API updates or shallow/blobless/sparse checkout only when absolutely needed.

## Important branch-history note

During CI workflow edits, `metal-test` was temporarily moved onto a workflow-only tree. It must be kept restored on top of the Metal implementation tree. The known implementation base used for restoration was:

- `a148c4b1aec75b4a464c453b6921db87a557e077` — preliminary AIR function constant metadata, with prior Metal frontend/AIR work in its ancestry.

Current/next commits should preserve the implementation files, not only workflow files.

## Major implementation already done

### Language / target plumbing

- `Language::Metal` added.
- `-x metal` and `.metal` input detection added.
- `air32`, `air64`, `air32_v...`, `air64_v...` target triples recognized.
- Minimal `AIRTargetInfo` added with AIR datalayouts and address spaces:
  - `device` / global -> addrspace(1)
  - `constant` -> addrspace(2)
  - `threadgroup` -> addrspace(3)
  - `thread` / private -> addrspace(0)

### Standards / versions

- Metal 3+ remains unqualified: `metal3.0`, `metal3.1`, `metal3.2`, `metal4.0`, `metal4.1`.
- Requested change in progress: old unqualified `metal1.*` / `metal2.*` are being removed in favor of platform-prefixed spelling:
  - iOS-family: `ios-metal1.0` ... `ios-metal2.4`
  - macOS-family: `macos-metal1.1` ... `macos-metal2.4`
  - legacy macOS aliases: `osx-metal1.1`, `osx-metal1.2`, `osx-metal2.0`
- `-std=metal` and `-std=metal4` map to Metal 4.0.

### Keywords / attributes

Implemented or partially implemented:

- stage keywords: `kernel`, `vertex`, `fragment`
- address spaces: `device`, `constant`, `threadgroup`, `thread`
- resources: `[[buffer(n)]]`, `[[texture(n)]]`, `[[sampler(n)]]`, `[[threadgroup(n)]]`
- builtin inputs: `[[thread_position_in_grid]]`, `[[thread_position_in_threadgroup]]`, `[[threadgroup_position_in_grid]]`, `[[threads_per_grid]]`, `[[threads_per_threadgroup]]`, `[[thread_index_in_threadgroup]]`, `[[thread_index_in_simdgroup]]`, `[[simdgroup_index_in_threadgroup]]`, `[[simdgroups_per_threadgroup]]`, `[[vertex_id]]`, `[[instance_id]]`, `[[amplification_id]]`, `[[base_vertex]]`, `[[base_instance]]`, `[[front_facing]]`, `[[position]]`, `[[sample_id]]`, `[[sample_mask]]`, `[[primitive_id]]`, `[[barycentric_coord]]`
- IO: `[[position]]`, `[[point_size]]`, `[[color(n)]]`, `[[depth(any|greater|less)]]`, `[[user(name)]]`, interpolation attributes, `[[render_target_array_index]]`, `[[viewport_array_index]]`, `[[early_fragment_tests]]`
- other initial entries: `[[function_constant(n)]]`, mesh/object/raytracing placeholder attrs.

### Sema validation

Implemented initial validation for:

- resource kind checks: buffer must be pointer/reference; texture must be texture/depth object; sampler must be sampler object.
- sampler index range `0..15`.
- color index range `0..7`.
- function constant must be global scalar bool/integer/enum/floating type.
- builtin input type checks: `uint`, `uintN`, `bool`, `float4`, record type for `stage_in`.
- delayed stage validation after function attributes are known, because parameter attrs may be processed before `kernel`/`vertex`/`fragment` is attached.
- Metal attr availability through `MetalAttrAvailability.def`.

### CodeGen / AIR metadata

Preliminary metadata emitted:

- `!air.kernel`
- `!air.vertex`
- `!air.fragment`
- `!air.version`
- `!air.language_version`
- `!air.compile_options`
- `!air.source_file_name`
- `!air.function_constants`
- resource argument records: `air.buffer`, `air.texture`, `air.sampler`, `air.location_index`, `air.read`, `air.read_write`
- stage inputs and IO outputs: `air.vertex_input`, `air.fragment_input`, `air.position`, `air.vertex_output`, `air.point_size`, `air.render_target`, `air.depth`, interpolation flags.

Still preliminary: exact Apple ABI global initializers, exact output name generation, struct layout metadata, texture/sampler method lowering, and builtin lowering are not complete.

## metal-info usage already added

### AST dump extraction

- Script: `clang/utils/metal/gen-metal-ast-support.py`
- Generated table: `clang/include/clang/Basic/MetalASTReference.def`
- Bulk run scanned 3894 AST dumps across iOS, macOS, tvOS, watchOS, simulator/non-simulator, air32/air64, Metal 1.0 through 4.0.
- Extracts:
  - `__metal_*_t` builtin object type names
  - Apple `Metal*Attr` names
  - alias hints from Apple AST attr names to current Clang attr names.

### stdlib header builtin extraction

- Script: `clang/utils/metal/gen-metal-stdlib-builtins.py`
- Generated table: `clang/include/clang/Basic/MetalStdlibBuiltins.def`
- Scanned Apple clang 32023.883 Metal headers from `metal-info/reference-apple/clang/32023.883/include/metal`.
- Extracted 686 unique `__metal_*` builtin entry point names.
- Not yet connected to Clang builtin registration/lowering.

## GitHub Actions / build workflow

### Workflows

- Preferred workflow: `.github/workflows/metal-clang-smoke-cached.yml` / `.github/workflows/metal-clang-smoke-v5.yml`
- Hosted on branch `metal` so it can be manually dispatched.
- Workflow input `ref` defaults to `metal-test` and checks out that branch.

### Cached workflow optimizations

- sparse checkout (`clang`, `llvm`, `cmake`, `third-party`)
- `filter: blob:none`
- `ccache` with `actions/cache`
- `lld`
- `LLVM_TARGETS_TO_BUILD=X86`
- `LLVM_INCLUDE_TESTS=OFF`
- no `FileCheck` build; CodeGen smoke tests use simple grep checks.

### Known CI runs

- `30048230924`: failed at CMake configure because `LLVM_INCLUDE_TESTS=ON` caused duplicate `check-clang-utils` target.
- `30049779751`: cached v5 run started; was later cancelled when a TableGen error was found/fixed.
- Latest known TableGen error: `Attr.td: Variable not defined: 'Metal'` from `let LangOpts = [Metal];`. Fix is to add `def Metal : LangOpt<"Metal">;` next to `def HLSL` in `Attr.td`.

## Current critical gotchas

1. Ensure `clang/include/clang/Basic/Attr.td` contains:

   ```td
   def HLSL : LangOpt<"HLSL">;
   def Metal : LangOpt<"Metal">;
   ```

2. Current `metal-test` branch may need restoration if workflow-only commits accidentally replaced the implementation tree. Preserve the implementation base and overlay workflow/docs changes.

3. Old Metal standards should no longer use unqualified `metal1.*` / `metal2.*`; use platform-qualified spelling.

4. `MetalStdlibBuiltins.def` is data only; it does not yet make `__metal_*` calls compile/lower.

## Next recommended work

1. Finish standard spelling migration:
   - remove `metal1.*` / `metal2.*` LangStandard definitions
   - add `ios-metal*`, `macos-metal*`, `osx-metal*`
   - update tests and workflows from `-std=metal2.0` to `-std=macos-metal2.0`
   - add platform mismatch validation.
2. Re-run cached v5 workflow.
3. Fix compile errors reported by CI.
4. Connect `MetalStdlibBuiltins.def` to actual builtin declarations/lowering.
5. Add platform availability table based on `docs/metal/MetalVersionAvailability.md`.

## How to continue without reading chat history

- Start from `metal-test` and inspect the most recent branch head.
- If implementation files are missing, restore from implementation commit `a148c4b1aec75b4a464c453b6921db87a557e077` and overlay later workflow/docs/fix changes.
- Use GitHub API blob/tree/commit/ref updates rather than full clone when possible.
- Before each final response, update this file with the latest status, new commits, CI run IDs, and remaining blockers.
