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


## Update 2026-07-24 JST — platform-qualified old standards

Implemented in commit `2f1bfccc505b72df7eda4e44855f5c1491367022`:

- Restored the Metal implementation tree on `metal-test` from implementation base `a148c4b1aec75b4a464c453b6921db87a557e077` after workflow-only branch drift.
- Added `def Metal : LangOpt<"Metal">;` in `Attr.td` to fix the TableGen error from `let LangOpts = [Metal]`.
- Replaced old unqualified `metal1.*` / `metal2.*` standard definitions with platform-qualified standards:
  - `ios-metal1.0` ... `ios-metal2.4`
  - `macos-metal1.1` ... `macos-metal2.4`
  - aliases `osx-metal1.1`, `osx-metal1.2`, `osx-metal2.0`.
- Kept `metal3.*` / `metal4.*` unqualified.
- Updated focused tests and cached smoke workflows to use `macos-metal2.0`, `macos-metal1.1`, and `ios-metal1.0`.
- Added `docs/metal/MetalVersionAvailability.md`.
- Added platform-family compatibility checks for prefixed standards in `CompilerInvocation.cpp`.

Workflow update on default branch `metal`: `81432f973c55e3fb75033262cb806c440c7c88a9`.

Latest CI run dispatched after this update:

- `30051384548` — https://github.com/kagurasumusun/llvm-project/actions/runs/30051384548
- Status at dispatch-time check: `in_progress`.

Next agent should check run `30051384548`; if it fails, download job log/artifact and fix the first compile/configure/smoke-test error.


## Update 2026-07-24 JST — Re-applied Attr.td standard-attribute version fix

Latest cached workflow run `30051384548` failed in clang-tblgen with many errors like:

```text
Attr.td:1709:20: error: Standard attributes must have valid version information.
  let Spellings = [CXX11<"", Name>];
```

The earlier `def Metal : LangOpt<"Metal">` fix was present, but the global namespace `CXX11<"", ...>` spellings in the Metal attribute block still lacked explicit C++ attribute version numbers after the platform-standard restoration. Re-applied the fix by changing all Metal global CXX11 spellings to include `202600`, e.g. `CXX11<"", "buffer", 202600>` and `CXX11<"", Name, 202600>`.

Static review after patch:

- no undefined `LangOpts` references in `Attr.td`
- no version-less `CXX11<"", ...>` spellings in the Metal attribute block

Next agent: rerun cached smoke workflow v5 and inspect the next compiler/test error.


## Update 2026-07-24 JST — Attr generated class inheritance fix

Latest run `30052906006` failed after the CXX11 version fix with generated `Attrs.inc` errors:

```text
tools/clang/include/clang/AST/Attrs.inc:8045:73: error: expected class-name before '{' token
../clang/include/clang/AST/AttrVisitor.h:29:54: error: invalid static_cast from Attr* to Metal*Attr*
```

Cause: The Metal attribute definitions used custom TableGen classes such as `class MetalResourceBindingAttr<string Name> : InheritableParamAttr` and then concrete defs like `def MetalBuffer : MetalResourceBindingAttr<"buffer">;`. The generated C++ attempted to use these TableGen helper classes as C++ base classes. Fix: expand the Metal resource and builtin-input attributes into direct `def ... : InheritableParamAttr` definitions instead of deriving concrete attrs from custom Metal*Attr TableGen classes.

Static review after patch:

- no `class Metal*Attr` helper remains in the Metal block
- no version-less global-scope `CXX11<"", ...>` spelling remains in the Metal block

Next agent should rerun cached workflow v5 and inspect the next compiler/test error.


## Update 2026-07-24 JST — half prelude collision fix

Workflow run `30054210146` built clang successfully and then failed in the first Metal smoke test:

```text
<built-in>:422:16: error: cannot combine with previous 'half' declaration specifier
  typedef __fp16 half;
<built-in>:516:9: error: expected unqualified-id
  using ::half;
```

Cause: `LangOpts.Half` is enabled for Metal, so `half` is already a language/builtin type spelling. The lightweight prelude must not typedef or `using` it. Fix: remove the `half` row from `MetalAIRTypes.def` and the generator table; define `half2/half3/half4` directly from `__fp16`; keep CodeGen mangle cases for `half`/`__fp16` as `Dh`.

Next agent should rerun cached workflow v5 and inspect the next smoke/build error.


## Update 2026-07-24 JST — faster cached workflow v6

Added workflow commits:

- default branch `metal`: `75d0099166c364074a0ea6adb85e11d0302bd4e5`
- working branch `metal-test`: `0ae5b132bf0b81a7805ce8894b5ee92efd552852`

New workflow files:

- `.github/workflows/metal-clang-smoke-fast.yml`
- `.github/workflows/metal-clang-smoke-v6.yml`

Purpose: reduce clang-only CI time. Changes include clang/clang++ host compiler, lld, ccache restore/save, O2 Release flags, assertions off, tests/docs/examples/benchmarks/static analyzer/ARCMT disabled, X86-only LLVM target (LLVM target group includes x86_64), and grep-based CodeGen smoke checks.

Previous latest completed error before this workflow tune was run `30054210146`: clang build succeeded, smoke failed due to `typedef __fp16 half;` prelude collision. Fixed by commit `2aa377e04581c50ccf216771531d5ace7289cbc5`.

Next agent should prefer dispatching `metal-clang-smoke-v6.yml` from default branch `metal` with input `ref=metal-test`.


## Update 2026-07-24 JST — component-fast workflow dispatched

Added component-only fast workflow commits:

- default branch `metal`: `541b8f4738240f7ae3778c61eb480538c33826a5`
- working branch `metal-test`: `ce3080002e6b0c16ff3244f38f12a279f19c8ce9`

Workflow files:

- `.github/workflows/metal-clang-components-fast.yml`
- `.github/workflows/metal-clang-components-v1.yml`

This workflow compiles selected Clang component libraries instead of linking the `clang` executable, intended for faster TableGen/C++ compile-error feedback. It was dispatched as run `30059551713`: https://github.com/kagurasumusun/llvm-project/actions/runs/30059551713 . Existing full smoke workflow runs were not stopped.

Use full `metal-clang-smoke-v6.yml` only when a clang binary and smoke tests are needed; use component workflow for fast implementation iteration.

## Update 2026-07-24 JST — Metal stage function attrs and thread-local address-space Sema

Current `metal-test` head before this update was `c7d66e7eebbea5ee8445843f88a30909b37b2bf2`. Latest full smoke run `30054210146` had already built clang, then failed syntax-only Metal smoke with:

```text
clang/test/Parser/metal-basic-keywords.metal:5:14: error: automatic variable qualified with an address space
clang/test/Parser/metal-basic-keywords.metal:10:1: error: 'vertex' attribute cannot be applied to a declaration
clang/test/Parser/metal-basic-keywords.metal:11:1: error: 'fragment' attribute cannot be applied to a declaration
```

Implementation update in this commit:

- `clang/lib/Sema/SemaDeclAttr.cpp`: attach `MetalVertexAttr` and `MetalFragmentAttr` with `handleSimpleAttribute<>`, so custom-keyword function qualifiers `vertex` and `fragment` are real function stage attrs instead of parsed-but-dropped attrs.
- `clang/lib/Sema/SemaDecl.cpp`: allow Metal automatic variables explicitly qualified with the `thread`/`opencl_private` address space. This preserves the existing diagnostic for non-private automatic address-space objects, so the implementation does not simply silence the check globally.
- `clang/test/Sema/metal-address-spaces.metal`: added focused coverage for accepted `thread int local` and rejected `device int invalid`.
- Existing `clang/test/Parser/metal-basic-keywords.metal` intentionally keeps `thread int local`, `vertex void v() {}`, and `fragment void f() {}` as smoke coverage for this implementation.

Next actions:

1. Dispatch `.github/workflows/metal-clang-components-v1.yml` with `ref=metal-test` for fast compile feedback.
2. If component build passes, dispatch `.github/workflows/metal-clang-smoke-v6.yml` with `ref=metal-test` to rerun the parser/CodeGen smoke tests.
3. Inspect the first new CI error before making further broad changes.

## Update 2026-07-24 JST — Metal function constants may be uninitialized

Commit `ede4c79b88517c2f8a3c70d625a3bd133aeb200f` fixed the previous smoke blocker by implementing:

- `vertex` / `fragment` custom-keyword function attrs in `SemaDeclAttr.cpp`.
- Metal `thread` / `opencl_private` automatic local variables in `SemaDecl.cpp`.
- `clang/test/Sema/metal-address-spaces.metal`.

Validation after that commit:

- Component-only workflow run `30062376523` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30062376523
- Full smoke workflow run `30062542551` built clang and passed the previous `metal-basic-keywords.metal` smoke step, then failed at `metal-prelude-types.metal` with:

```text
clang/test/Parser/metal-prelude-types.metal:20:15: error: variable in constant address space must be initialized
  constant bool fc_bool [[function_constant(0)]];
clang/test/Parser/metal-prelude-types.metal:21:14: error: variable in constant address space must be initialized
  constant int fc_int [[function_constant(1)]];
```

Implementation update in this commit:

- `clang/lib/Sema/SemaDecl.cpp`: keep Clang's OpenCL-derived rule that ordinary non-extern `constant` globals need initializers, but exempt Metal globals carrying `MetalFunctionConstantAttr` because MSL function constants are supplied by specialization data and can be declared without an initializer.
- `clang/test/Sema/metal-function-constants.metal`: added focused coverage for uninitialized `constant ... [[function_constant(n)]]` declarations and a negative ordinary `constant int` declaration.

Next actions:

1. Dispatch component-fast workflow again for `metal-test`.
2. If it succeeds, dispatch full smoke v6 again and inspect the next failure.

## Update 2026-07-24 JST — AST-reference builtin smoke names avoid prelude aliases

Commit `c12884d7eb4ea4df10a64a7b12e4b778bccb35c8` allowed uninitialized Metal function constants and added Sema coverage. Validation after that commit:

- Component-only workflow run `30062728900` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30062728900
- Full smoke workflow run `30062869752` built clang, passed `metal-basic-keywords.metal`, `metal-prelude-types.metal`, and `metal-generated-prelude-table.metal`, then failed at `clang/test/Parser/metal-ast-reference-builtins.metal` with:

```text
clang/test/Parser/metal-ast-reference-builtins.metal:4:20: error: redefinition of 'sampler' as different kind of symbol
  __metal_sampler_t *sampler;
<built-in>:489:27: note: previous definition is here
  typedef __metal_sampler_t sampler;
clang/test/Parser/metal-ast-reference-builtins.metal:7:19: error: redefinition of 'tensor' as different kind of symbol
  __metal_tensor_t *tensor;
<built-in>:508:26: note: previous definition is here
  typedef __metal_tensor_t tensor;
```

This is a smoke-test issue exposed by the prelude object aliases: `sampler` and `tensor` are intentionally type aliases now, so the test should not use the same identifiers as global variable names. Updated the AST-reference builtin smoke declarations to use suffixed variable names (`texture_obj`, `sampler_obj`, `vft_obj`, `ias_obj`, `tensor_obj`) while preserving the checked `__metal_*` builtin object types.

Next action: rerun full smoke v6 for `metal-test` and inspect the next failure.

## Update 2026-07-24 JST — Metal IO attrs now have Sema handlers

Commit `79fb78875a1aa39a4b9927a5e4b3a1fc422e2fda` renamed variables in `metal-ast-reference-builtins.metal` to avoid colliding with intentional prelude aliases. Full smoke run `30063035302` then progressed further and failed in `clang/test/Sema/metal-builtin-input-validation.metal`:

```text
error: 'expected-error' diagnostics seen but not expected:
  File clang/test/Sema/metal-builtin-input-validation.metal Line 3: 'attribute' attribute cannot be applied to a declaration
```

Root cause: many Metal IO/builtin attributes existed in `Attr.td` but did not have `SemaDeclAttr.cpp` cases, so `[[attribute(0)]]` on a vertex input field was parsed but not attached.

Implementation update in this commit:

- Added handlers for missing Metal param/field/function attrs:
  - parameter attrs: `MetalLocalIndex`, `MetalId`, `MetalObject`, `MetalMesh`, `MetalPayload`, `MetalIntersection`, `MetalVisible`
  - IO attrs: `MetalAttribute`, `MetalPointSize`, `MetalRenderTargetArrayIndex`, `MetalViewportArrayIndex`, `MetalUser`, `MetalDepth`, `MetalFlat`, `MetalCenterPerspective`, `MetalCenterNoPerspective`, `MetalCentroidPerspective`, `MetalCentroidNoPerspective`, `MetalSamplePerspective`, `MetalSampleNoPerspective`, `MetalEarlyFragmentTests`
- Added `handleMetalAttributeAttr`, `handleMetalUserAttr`, and `handleMetalDepthAttr` for attrs with arguments.
- Added `clang/test/Sema/metal-io-attrs.metal` as focused coverage for vertex input attributes, vertex/fragment output IO attrs, interpolation attrs, depth/color attrs, and `early_fragment_tests`.

Next actions:

1. Dispatch component-fast workflow because this commit changes C++ Sema code.
2. If it succeeds, dispatch full smoke v6 and inspect the next failure.

