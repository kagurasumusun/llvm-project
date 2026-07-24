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

## Update 2026-07-24 JST — Fix IdentifierLoc access in Metal depth handler

Commit `20e4b45a111810bddc4e8f60d7ab8b81757f34d7` added the remaining Metal IO attr handlers, but component-fast workflow run `30063311103` failed to compile `SemaDeclAttr.cpp`:

```text
../clang/lib/Sema/SemaDeclAttr.cpp:5610:19: error: 'Loc' is a private member of 'clang::IdentifierLoc'
    S.Diag(Depth->Loc, diag::warn_attribute_type_not_supported)
                  ^
```

Fix in this commit: use the public `IdentifierLoc::getLoc()` accessor in `handleMetalDepthAttr` instead of accessing `Depth->Loc` directly.

Next actions: rerun component-fast workflow; if it succeeds, rerun full smoke v6.

## Update 2026-07-24 JST — Suppress cascading constant-init diag for invalid function constants

Commit `a921fde5cff30bac9cc31272b07c1c171a883978` fixed the component compile error; component-fast workflow run `30063475275` completed `success`. Full smoke workflow run `30063626208` then progressed through builtin input validation and failed in `clang/test/Sema/metal-resource-validation.metal`:

```text
error: 'expected-error' diagnostics seen but not expected:
  File clang/test/Sema/metal-resource-validation.metal Line 5: variable in constant address space must be initialized
```

Line 5 intentionally tests an invalid `constant uint4 [[function_constant(2)]]` because Metal function constants must be scalar. The desired diagnostic is the Metal function-constant type error; the ordinary constant-address-space initializer diagnostic was a cascade because `handleMetalFunctionConstantAttr` rejected the attr without marking the declaration invalid.

Fix in this commit: when `handleMetalFunctionConstantAttr` diagnoses an out-of-range index or invalid variable type, mark the declaration invalid (`D->setInvalidDecl()`) so later uninitialized-constant checks do not emit a second unrelated diagnostic.

Next actions: rerun component-fast, then full smoke v6.

## Update 2026-07-24 JST — Mark unavailable Metal attrs invalid to avoid cascades

Commit `285020a4bef67de3dcf654f499242d6c71b94200` marked invalid `[[function_constant]]` declarations invalid after type/index diagnostics. Component-fast run `30063801527` then completed `success`. Full smoke run `30063917870` progressed further and failed in `clang/test/Sema/metal-availability.metal`:

```text
error: 'expected-error' diagnostics seen but not expected:
  File clang/test/Sema/metal-availability.metal Line 4: variable in constant address space must be initialized
```

Line 4 is:

```metal
constant int fc [[function_constant(0)]];
// expected-error@-1 {{'function_constant' attribute requires Metal 1.2 or later}}
```

Root cause: `checkMetalAttributeAvailability` diagnosed the unavailable `function_constant` attribute and returned before the attr was attached, but left the variable valid. Later `ActOnUninitializedDecl` saw an ordinary uninitialized `constant int` and emitted a cascading initializer diagnostic.

Fix in this commit:

- Change `checkMetalAttributeAvailability` to receive `Decl *D`.
- After emitting `err_metal_attribute_requires_version`, call `D->setInvalidDecl()` when available. This treats unavailable Metal attrs as invalid declarations and suppresses later unrelated declaration checks.

Next actions: rerun component-fast for C++ compile validation, then full smoke v6.

## Update 2026-07-24 JST — Availability-cascade fix validated

Commit `b5eabafc6f9e88de9612a6d59a43f72b432e6b67` implemented the latest error fix:

- `checkMetalAttributeAvailability` now receives `Decl *D`.
- When a Metal attr is unavailable for the selected Metal language version, the decl is marked invalid after emitting `err_metal_attribute_requires_version`.
- This suppresses the unrelated follow-on `variable in constant address space must be initialized` diagnostic for the `macos-metal1.1` `[[function_constant]]` availability test.

Validation:

- Component-fast workflow run `30065249380` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30065249380
- Full smoke v6 workflow run `30065387233` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30065387233

Result: the latest reported CI blocker (`metal-availability.metal` line 4 cascading constant-address-space initializer diagnostic) is fixed. The current Metal smoke workflow passes at commit `b5eabafc6f9e88de9612a6d59a43f72b432e6b67`; any later docs-only handoff commit should be treated as equivalent for implementation state.

Next recommended implementation work:

1. Continue beyond smoke coverage: connect `MetalStdlibBuiltins.def` to builtin registration/lowering or expand AIR metadata parity.
2. Use component-fast first for C++ compile feedback, then full smoke v6 after each functional change.
3. Keep platform-qualified old standards (`ios-metal*`, `macos-metal*`, `osx-metal*`) and do not reintroduce unqualified `metal1.*`/`metal2.*`.

## Update 2026-07-24 JST — Continue implementation: early_fragment_tests validation and stage_in user metadata

Starting point for this implementation batch:

- `metal-test` was at docs-only head `adca35ebbc8143070f993f083cbc770527049696`.
- Previous implementation head `b5eabafc6f9e88de9612a6d59a43f72b432e6b67` had passing validation:
  - component-fast `30065249380` success
  - full smoke v6 `30065387233` success

Implementation changes in this batch:

- `clang/lib/Sema/SemaDeclAttr.cpp`: `[[early_fragment_tests]]` is now semantically restricted to fragment functions. A vertex/kernel function with the attr emits `err_metal_attribute_wrong_stage` instead of accepting the attr solely because the TableGen subject is any function.
- `clang/test/Sema/metal-builtin-input-validation.metal`: added a negative vertex-function `[[early_fragment_tests]]` test. This file is part of the focused smoke workflow.
- `clang/lib/CodeGen/CodeGenFunction.cpp`: `[[stage_in]]` record fields using `[[user(name)]]` now emit AIR input metadata with `user(name)` instead of falling back to a generated field name. This aligns stage input handling with existing vertex output `[[user(name)]]` metadata.
- `clang/test/CodeGen/metal-air-stage-in-struct.metal`: updated expected metadata for a `[[user(shade_id), flat]]` stage input field.
- `.github/workflows/metal-clang-smoke-v6.yml`: the focused smoke grep for `metal-air-stage-in-struct.metal` now also checks `user(shade_id)`. The workflow file must be pushed to both `metal-test` and default workflow branch `metal` so manual dispatch sees the new grep.

Next validation steps:

1. Push implementation to `metal-test`.
2. Push the workflow-only update to branch `metal`.
3. Dispatch component-fast (`metal-clang-components-v1.yml`) with `ref=metal-test`.
4. Dispatch full smoke v6 (`metal-clang-smoke-v6.yml`) with `ref=metal-test`.

## Update 2026-07-24 JST — early_fragment_tests/stage_in user metadata validated

Implementation commit on `metal-test`:

- `b7543c0cd3ebd875444b5d48342df2973db5e4d8` — `[Metal] Validate early fragment tests and user stage-in metadata`

Default workflow branch update:

- `96b7f42e2f255809cf1f7a156b70cf08b0f4ba73` — `[ci][Metal] Check user stage-in metadata in fast smoke`

Validation results:

- Component-fast workflow run `30065730546` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30065730546
- Full smoke v6 workflow run `30065841031` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30065841031

What passed in this batch:

- `[[early_fragment_tests]]` is now rejected on non-fragment Metal stage functions and covered by `clang/test/Sema/metal-builtin-input-validation.metal`.
- `[[stage_in]]` record fields with `[[user(name)]]` now produce `user(name)` AIR input metadata. `clang/test/CodeGen/metal-air-stage-in-struct.metal` and the fast smoke workflow grep now check `user(shade_id)`.

Current known-good implementation state before this docs-only update: `b7543c0cd3ebd875444b5d48342df2973db5e4d8`. Any subsequent docs-only handoff commit should be treated as equivalent for code.

Next recommended implementation directions:

1. Add more Sema validation for Metal IO attrs (duplicate locations/colors, output-only/input-only stage constraints, point_size/position/depth/color type checks).
2. Continue expanding AIR metadata parity for resource/object/mesh/raytracing attrs.
3. Start connecting `MetalStdlibBuiltins.def` to a real builtin declaration/lowering path rather than leaving it as data only.

## Update 2026-07-24 JST — Continue implementation: duplicate Metal resource/IO index validation

Starting point:

- Code implementation head before this batch: `b7543c0cd3ebd875444b5d48342df2973db5e4d8`, full smoke v6 `30065841031` success.
- Current `metal-test` may have a later docs-only handoff commit; preserve the implementation tree.

Implementation changes in this batch:

- Added Metal-specific duplicate diagnostics in `clang/include/clang/Basic/DiagnosticSemaKinds.td`:
  - duplicate indexed attrs across parameters/stage input fields/stage output fields/function constants
  - duplicate non-indexed stage IO attrs such as `[[depth]]`, `[[position]]`, `[[point_size]]`
- `clang/lib/Sema/SemaDeclAttr.cpp` now validates:
  - duplicate resource binding indices within a stage function for each resource kind (`buffer`, `texture`, `sampler`, `threadgroup`)
  - duplicate `[[function_constant(n)]]` global indices across the translation unit
  - duplicate `[[attribute(n)]]` fields in a `[[stage_in]]` record
  - duplicate fragment output `[[color(n)]]` / `[[depth(... )]]` fields
  - duplicate vertex output singleton attrs (`position`, `point_size`, `render_target_array_index`, `viewport_array_index`)
- Focused smoke-covered tests updated:
  - `clang/test/Sema/metal-resource-validation.metal` covers duplicate function constants and duplicate resource bindings.
  - `clang/test/Sema/metal-builtin-input-validation.metal` covers duplicate stage input attributes.
  - `clang/test/Sema/metal-color-validation.metal` covers duplicate fragment color/depth outputs.

Next validation steps:

1. Push to `metal-test`.
2. Dispatch component-fast (`metal-clang-components-v1.yml`) with `ref=metal-test`.
3. Dispatch full smoke v6 (`metal-clang-smoke-v6.yml`) with `ref=metal-test`.
4. If passing, record commit IDs and run IDs below.

## Update 2026-07-24 JST — duplicate resource/IO validation passed

Implementation commit on `metal-test`:

- `37a592b55b83f8e0ca54faa1aacfc13b0a006878` — `[Metal] Validate duplicate resource and IO indices`

Validation results:

- Component-fast workflow run `30066128180` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30066128180
- Full smoke v6 workflow run `30066697140` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30066697140

What passed in this batch:

- Duplicate `[[function_constant(n)]]` indices are diagnosed across global Metal function constants.
- Duplicate resource binding indices are diagnosed within a Metal stage function for `buffer`, `texture`, `sampler`, and `threadgroup` namespaces.
- Duplicate `[[attribute(n)]]` fields in a `[[stage_in]]` struct are diagnosed when the struct is used as a stage input parameter.
- Duplicate fragment output `[[color(n)]]` and `[[depth(... )]]` fields are diagnosed.
- Duplicate vertex output singleton attrs (`[[position]]`, `[[point_size]]`, `[[render_target_array_index]]`, `[[viewport_array_index]]`) are now checked.
- Smoke-covered tests updated: `metal-resource-validation.metal`, `metal-builtin-input-validation.metal`, and `metal-color-validation.metal`.

Current known-good implementation state before this docs-only update: `37a592b55b83f8e0ca54faa1aacfc13b0a006878`. Any later docs-only handoff commit should be treated as equivalent for code.

Next recommended implementation work:

1. Add type validation for Metal IO fields (`position` float4, `point_size` float, fragment depth float, render target/viewport indices uint, etc.) and update CodeGen tests away from intentionally loose `int` field types.
2. Add stage-compatibility validation for IO field attrs (e.g. fragment-only `color/depth`, vertex-output-only `point_size`, stage-input-only `attribute`).
3. Continue AIR metadata parity or wire `MetalStdlibBuiltins.def` into actual builtin declaration/lowering.

## Update 2026-07-24 JST — Continue implementation: IO field type/context validation and stdlib builtin declarations

Starting point:

- Code implementation head before this batch: `37a592b55b83f8e0ca54faa1aacfc13b0a006878`, full smoke v6 `30066697140` success.
- `metal-test` may have a later docs-only handoff commit.

Implementation changes in this batch:

- `clang/lib/Sema/SemaDeclAttr.cpp` / `DiagnosticSemaKinds.td`:
  - added field-specific Metal diagnostics for wrong IO field type and wrong field context.
  - validates `[[position]]` fields as `float4` for vertex outputs and stage-input records.
  - validates `[[point_size]]` and fragment `[[depth(... )]]` fields as `float`.
  - validates `[[render_target_array_index]]` and `[[viewport_array_index]]` fields as `uint`.
  - validates fragment color output fields as arithmetic scalar/vector values.
  - rejects fragment-output-only `[[color]]`/`[[depth]]` on vertex outputs.
  - rejects vertex-output-only attrs (`position`, `point_size`, render target/viewport array index) on fragment outputs as appropriate.
  - rejects output-only attrs (`color`, `depth`, `point_size`, render target/viewport array index) on `[[stage_in]]` record fields.
- `clang/lib/Frontend/InitPreprocessor.cpp`:
  - connects `MetalStdlibBuiltins.def` to the lightweight Metal prelude by emitting generic `extern "C" int __metal_*(...);` declarations for all collected Apple stdlib builtin entry-point names.
  - This is a bootstrap declaration/lowering path: calls now parse and CodeGen as external calls; precise overload signatures/lowering remain future work.
- Tests:
  - updated `metal-air-io-metadata.metal` to use valid IO field types.
  - expanded `metal-color-validation.metal` and `metal-builtin-input-validation.metal` for type/context failures.
  - added `clang/test/Parser/metal-stdlib-builtin-decls.metal`.
  - added `clang/test/CodeGen/metal-stdlib-builtin-calls.metal`.
- `.github/workflows/metal-clang-smoke-v6.yml` now runs the new stdlib syntax smoke and greps CodeGen for `@__metal_abs`; push this workflow to default branch `metal` as well as `metal-test`.

Next validation steps:

1. Push implementation to `metal-test`.
2. Push workflow update to branch `metal`.
3. Dispatch component-fast and full smoke v6.
4. If passing, record commit IDs and run IDs below.

## Update 2026-07-24 JST — Fix stdlib prelude type-name collisions

After commit `c628f825ed7f848de099428868bb566a548a8ef7`, component-fast run `30067687702` completed `success`, but full smoke v6 run `30068094427` failed in `clang/test/Parser/metal-ast-reference-builtins.metal` before reaching the new stdlib smoke:

```text
clang/test/Parser/metal-ast-reference-builtins.metal:3:1: error: must use 'struct' tag to refer to type '__metal_texture_2d_t' in this scope
<built-in>:1154:16: note: struct '__metal_texture_2d_t' is hidden by a non-type declaration of '__metal_texture_2d_t' here
  extern "C" int __metal_texture_2d_t(...);
```

Root cause: `MetalStdlibBuiltins.def` currently contains some `__metal_*_t` names that are opaque builtin object type names, not callable stdlib entry points. Emitting generic extern declarations for every row hid the struct tags for types such as `__metal_texture_2d_t`, `__metal_sampler_t`, and `__metal_tensor_t`.

Fix in this commit:

- `clang/lib/Frontend/InitPreprocessor.cpp` now filters generic stdlib extern declarations through `IsMetalBuiltinTypeName`.
- The filter recognizes builtin object/type names from `MetalASTReference.def` and `MetalBuiltinObjects.def` and skips those rows while still declaring callable `__metal_*` functions such as `__metal_abs` and `__metal_select`.

Next validation steps: rerun component-fast, then full smoke v6.

## Update 2026-07-24 JST — Fix MetalBuiltinObjects macro parameter collision

The first stdlib type-name filter fix (`ad1a33e1e7676718f6b386d9b2f3499988f0e5d6`) failed component-fast run `30068682537` while compiling `InitPreprocessor.cpp`:

```text
../clang/include/clang/Basic/MetalBuiltinObjects.def:22:33: error: use of undeclared identifier '__metal_texture_1d_t'
METAL_BUILTIN_OBJECT(texture1d, __metal_texture_1d_t, texture)
```

Root cause: the local macro definition used `BuiltinName` as both the lambda parameter name and the macro parameter name:

```cpp
#define METAL_BUILTIN_OBJECT(Alias, BuiltinName, Kind) \
  if (BuiltinName == #BuiltinName) return true;
```

The unstringized `BuiltinName` token was macro-expanded to `__metal_texture_1d_t`, causing a C++ identifier lookup instead of comparing the lambda argument.

Fix in this commit: rename the lambda variable to `CandidateName` and the macro parameter to `TypeName`, comparing `CandidateName == #TypeName`.

Next validation steps: rerun component-fast, then full smoke v6.

## Update 2026-07-24 JST — IO type/context validation and stdlib builtin bootstrap passed

Implementation commits on `metal-test`:

- `c628f825ed7f848de099428868bb566a548a8ef7` — `[Metal] Validate IO field types and declare stdlib builtins`
- `ad1a33e1e7676718f6b386d9b2f3499988f0e5d6` — `[fix][Metal] Filter stdlib declarations for builtin type names`
- `0a9505a034a6e11d6bf3866cd75647a73c7aad3c` — `[fix][Metal] Correct stdlib type filter macro comparison`

Default workflow branch update:

- `c08565abd6ec59cbd26561900284515a1746bf74` — `[ci][Metal] Add stdlib builtin smoke coverage`

Validation results:

- Component-fast `30067687702` passed for the initial implementation but full smoke `30068094427` failed because generic stdlib declarations hid builtin type names such as `__metal_texture_2d_t`.
- Component-fast `30068682537` failed because the first filter fix had a macro parameter collision in `METAL_BUILTIN_OBJECT`.
- Component-fast `30068880975` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30068880975
- Full smoke v6 `30069031637` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30069031637

What passed in this batch:

- IO field type validation:
  - `[[position]]` field -> `float4`
  - `[[point_size]]` field -> `float`
  - fragment `[[depth(... )]]` field -> `float`
  - `[[render_target_array_index]]` / `[[viewport_array_index]]` fields -> `uint`
  - fragment `[[color(n)]]` field -> arithmetic scalar/vector
- IO field stage/context validation:
  - rejects `[[color]]` / `[[depth]]` on vertex outputs
  - rejects vertex-output-only attrs on fragment outputs
  - rejects output-only attrs on `[[stage_in]]` record fields
- `MetalStdlibBuiltins.def` is now used by the Metal prelude to emit bootstrap generic declarations for callable `__metal_*` stdlib builtins.
  - The declaration path filters out builtin object/type names from `MetalASTReference.def` and `MetalBuiltinObjects.def` so opaque types are not hidden by function declarations.
  - Calls such as `__metal_abs(-7)` now parse and CodeGen as external calls; exact overload signatures/lowering remain future work.
- Fast smoke workflow now checks:
  - `clang/test/Parser/metal-stdlib-builtin-decls.metal`
  - `clang/test/CodeGen/metal-stdlib-builtin-calls.metal` with grep for `@__metal_abs`

Current known-good implementation state before this docs-only update: `0a9505a034a6e11d6bf3866cd75647a73c7aad3c`. Any later docs-only handoff commit should be treated as equivalent for code.

Next recommended implementation work:

1. Refine stdlib builtin declarations from generic `int (...)` toward generated signatures/overload sets using Apple headers from `metal-info`.
2. Expand exact AIR ABI metadata for object/mesh/raytracing resources and texture/sampler access methods.
3. Add more stage/context validation for less common IO attrs (`payload`, `intersection`, mesh/object attrs, raytracing attrs).

## Update 2026-07-24 JST — Continue implementation: typed stdlib prototypes, texture methods, mesh/ray metadata

Starting point:

- Known-good code before this batch: `0a9505a034a6e11d6bf3866cd75647a73c7aad3c`, full smoke v6 `30069031637` success.

Implementation changes in this batch:

1. Stdlib builtin declarations:
   - `InitPreprocessor.cpp` now emits exact bootstrap prototypes for selected common stdlib entry points instead of the generic `int (...)` fallback:
     - `extern "C" int __metal_abs(int);`
     - `extern "C" int __metal_select(int, int, bool);`
     - `extern "C" float __metal_sin(float);`
     - `extern "C" float __metal_cos(float);`
     - `extern "C" float __metal_floor(float);`
   - Remaining callable rows in `MetalStdlibBuiltins.def` still use the generic fallback.
   - `metal-stdlib-builtin-decls.metal` now checks function-pointer assignment for these exact prototypes.

2. Texture/sampler method path:
   - Opaque Metal texture/depth builtin object structs emitted by the prelude now include bootstrap method declarations:
     - `uint get_width() const;`
     - `uint get_height() const;`
     - `uint get_depth() const;`
     - `uint get_array_size() const;`
   - Added parser and CodeGen smoke tests for `texture2d::get_width()` / related methods.
   - This is still external-call lowering rather than exact AIR texture intrinsic lowering, but it creates a frontend/IR path for method syntax.

3. Mesh/object/raytracing attrs:
   - `SemaDeclAttr.cpp` now type-checks placeholder attrs:
     - `[[local_index]]` / `[[id]]` require `uint`
     - `[[object]]`, `[[payload]]`, `[[intersection]]` require record types
     - `[[mesh]]` requires a mesh object type spelling
     - `[[visible]]` requires visible/intersection function table object type spelling
   - `CodeGenFunction.cpp` now emits preliminary AIR metadata tags for these attrs:
     - `air.local_index`, `air.id`, `air.object`, `air.mesh`, `air.payload`, `air.intersection`, `air.visible`
   - Added Sema and CodeGen tests: `metal-mesh-raytracing-attrs.metal`, `metal-air-mesh-raytracing-attrs.metal`.

4. Workflow:
   - `.github/workflows/metal-clang-smoke-v6.yml` now includes the new stdlib prototype checks, texture method checks, and mesh/raytracing metadata grep smoke. Push workflow changes to branch `metal` too.

Next validation steps:

1. Push implementation to `metal-test`.
2. Push workflow update to `metal`.
3. Run component-fast.
4. Run full smoke v6 and fix the first new failure.

## Update 2026-07-24 JST — Fix texture method smoke grep

Commit `5341a47f81fad89904255681dad014296e0ab888` passed component-fast run `30069379992`, but full smoke run `30069514270` failed in the new texture method CodeGen smoke. The `get_width` grep succeeded, then the workflow looked for `!air.texture` and failed:

```text
+ grep -F get_width /tmp/metal-texture-methods.metal.ll
+ grep -F '!air.texture' /tmp/metal-texture-methods.metal.ll
```

Root cause: AIR metadata strings in LLVM IR are printed as `!"air.texture"`, so the literal substring `!air.texture` is not present. Existing smoke checks use the plain string name style.

Fix in this commit:

- `clang/test/CodeGen/metal-texture-methods.metal`: FileCheck now expects `!"air.texture"`.
- `.github/workflows/metal-clang-smoke-v6.yml`: grep now checks `air.texture` instead of `!air.texture`.

Next validation step: rerun full smoke v6. Component-fast already passed for the code changes.

## Update 2026-07-24 JST — typed stdlib/texture methods/mesh metadata validated

Implementation commits on `metal-test`:

- `5341a47f81fad89904255681dad014296e0ab888` — `[Metal] Add typed stdlib prototypes and mesh AIR metadata`
- `c35fa55d0c448fe85304a289eda0877017edcdba` — `[test][Metal] Fix texture metadata smoke grep`

Default workflow branch updates:

- `7fc41720a68cf1d928e77bc1d1553aaa1dbfb7f9` — `[ci][Metal] Add texture and mesh/ray smoke coverage`
- `89d418899f57541bf5cf235e0498e1885a4238a0` — `[ci][Metal] Fix texture metadata smoke grep`

Validation results:

- Component-fast `30069379992` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30069379992
- Full smoke v6 `30069514270` failed only because the new texture method smoke grepped for literal `!air.texture`; the IR contains `!"air.texture"`, so the grep was changed to `air.texture`.
- Full smoke v6 `30069713086` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30069713086

What passed in this batch:

- Selected typed stdlib prototypes now replace generic `int (...)` fallback for `__metal_abs`, `__metal_select`, `__metal_sin`, `__metal_cos`, and `__metal_floor`.
- Texture/depth opaque builtin structs expose bootstrap method declarations (`get_width`, `get_height`, `get_depth`, `get_array_size`) and texture method syntax/CodeGen smoke passes.
- Mesh/object/raytracing-style parameter attrs now have Sema type checks and preliminary AIR metadata tags for `air.local_index`, `air.id`, `air.object`, `air.mesh`, `air.payload`, `air.intersection`, and `air.visible`.

Current known-good implementation state before this docs-only update: `c35fa55d0c448fe85304a289eda0877017edcdba`. Any later docs-only handoff commit should be treated as equivalent for code.

Next recommended implementation work:

1. Generate broader stdlib overload/signature declarations from Apple headers instead of the current small hand-written exact-prototype subset plus generic fallback.
2. Lower texture methods to exact AIR/Metal intrinsics or metadata patterns rather than external C++ method calls.
3. Add stage-specific mesh/object/raytracing function modeling when the frontend has explicit object/mesh/intersection stage entry points.

## Update 2026-07-24 JST — Continue implementation: broader prototypes, texture wrapper lowering, mesh-like stage attrs

Starting point:

- Known-good code before this batch: `c35fa55d0c448fe85304a289eda0877017edcdba`, full smoke v6 `30069713086` success.

Implementation changes in this batch:

1. Broader stdlib prototype bootstrap:
   - Added exact one-lane float prototypes for more common `__metal_*` stdlib entries in `InitPreprocessor.cpp`: `acos`, `asin`, `atan`, `ceil`, `exp`, `exp2`, `log`, `log2`, `rsqrt`, `sqrt`, `trunc`, `pow`, `fmin`, `fmax`, and `clamp`, in addition to the earlier `abs`, `select`, `sin`, `cos`, and `floor` subset.
   - `metal-stdlib-builtin-decls.metal` now checks additional function pointer assignments and calls.

2. Texture method lowering path:
   - Texture/depth opaque structs no longer leave `get_width()`/etc. as undefined C++ methods. The prelude now defines inline wrappers that call C-style helper entry points:
     - `__metal_texture_get_width(this)`
     - `__metal_texture_get_height(this)`
     - `__metal_texture_get_depth(this)`
     - `__metal_texture_get_array_size(this)`
   - This moves CodeGen from an external C++ method symbol toward a stable Metal helper builtin call while still stopping short of true AIR intrinsic lowering.
   - `metal-texture-methods.metal` now checks calls to `@__metal_texture_get_width` / `@__metal_texture_get_height` and `air.texture` metadata.

3. Mesh/object/raytracing stage modeling:
   - `Attr.td` changed `MetalObject`, `MetalMesh`, `MetalIntersection`, and `MetalVisible` from `InheritableParamAttr` to `InheritableAttr` and expanded their subjects to `[ParmVar, Function]`.
   - `CodeGenFunction.cpp` now treats functions carrying those attrs as Metal stage-like functions and emits named metadata nodes:
     - `!air.object`
     - `!air.mesh`
     - `!air.intersection`
     - `!air.visible`
   - Existing parameter metadata for `air.object`, `air.mesh`, `air.payload`, `air.intersection`, and `air.visible` remains.
   - Sema/CodeGen tests now cover `[[object]]`, `[[mesh]]`, `[[intersection]]`, and `[[visible]]` functions.

4. Workflow:
   - Fast smoke greps texture CodeGen for `__metal_texture_get_width` and mesh/object/ray stage metadata. Push workflow update to default branch `metal`.

Next validation steps:

1. Push implementation to `metal-test`.
2. Push workflow update to `metal`.
3. Run component-fast and full smoke v6.
4. Fix first CI error and update this handoff again.

## Update 2026-07-24 JST — broader stdlib prototypes, texture helper lowering, and mesh-like stage attrs validated

Implementation commit on `metal-test`:

- `e7bcc7c9ba6bab94a21ba8122b463c702155cde7` — `[Metal] Broaden stdlib prototypes and mesh-like stages`

Default workflow branch update:

- `6f713317c0cf354cbdee9ccb76d822f769fc5d52` — `[ci][Metal] Check texture helper and mesh-like stage metadata`

Validation results:

- Component-fast run `30070063106` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30070063106
- Full smoke v6 run `30071361260` completed `success`: https://github.com/kagurasumusun/llvm-project/actions/runs/30071361260

What passed in this batch:

1. Broader stdlib bootstrap prototypes:
   - Added exact float/int prototypes for a larger common subset: `__metal_acos`, `__metal_asin`, `__metal_atan`, `__metal_ceil`, `__metal_exp`, `__metal_exp2`, `__metal_log`, `__metal_log2`, `__metal_rsqrt`, `__metal_sqrt`, `__metal_trunc`, `__metal_pow`, `__metal_fmin`, `__metal_fmax`, `__metal_clamp`, plus previous `abs`, `select`, `sin`, `cos`, `floor`.
   - `metal-stdlib-builtin-decls.metal` checks exact function pointer assignments and calls for the expanded subset.

2. Texture method lowering path:
   - Texture/depth opaque structs now define inline wrappers for `get_width`, `get_height`, `get_depth`, `get_array_size` that call C-style helper builtins (`__metal_texture_get_width`, etc.).
   - CodeGen smoke now checks helper calls such as `@__metal_texture_get_width`, rather than external C++ method symbols, plus `air.texture` metadata.

3. Mesh/object/raytracing-like stage attrs:
   - `MetalObject`, `MetalMesh`, `MetalIntersection`, and `MetalVisible` now allow both `ParmVar` and `Function` subjects.
   - `EmitKernelMetadata` treats functions with those attrs as Metal stage-like functions and emits preliminary named metadata nodes `!air.object`, `!air.mesh`, `!air.intersection`, and `!air.visible`.
   - Parameter metadata for object/mesh/payload/intersection/visible attrs remains covered.

Current known-good implementation state before this docs-only update: `e7bcc7c9ba6bab94a21ba8122b463c702155cde7`. Any later docs-only handoff commit should be treated as equivalent for code.

Next recommended implementation work:

1. Generate the expanded stdlib prototypes from `metal-info` Apple headers instead of keeping the exact-prototype subset hand-authored.
2. Replace texture helper external calls with actual AIR intrinsic/lowering once the target ABI mapping is clearer.
3. Refine object/mesh/intersection function syntax and exact AIR metadata against Apple AST/IR examples.

