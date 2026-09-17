# PLANS.md — Windows CE LLVM bring-up plan

## Mission

Bring up a reproducible Windows CE target using the two-layer model:

- `cellvm-sdk` = CE development interface (`-dev`-package equivalent): headers, declarations, import/link metadata and compatibility declarations.
- `llvm-project` = compiler and language/runtime implementation.
- `wince-docs-corpus` = evidence and provenance store.

The final result must allow LLVM to compile and link CE-targeted C/C++ programs against `cellvm-sdk`, while the selected LLVM runtime libraries build without accidentally depending on desktop Windows or an unrelated CE generation.

This plan is deliberately evidence-first. It is not acceptable to fill gaps by guessing from desktop Win32/W64 behavior.

---

## 0. Ground truth and initial observations

The repositories currently have distinct responsibilities and must remain distinct.

The current `cellvm-sdk` Makefile already describes CE deployment checkpoints `0x420`, `0x500`, and `0x600` and lists a large header inventory. Treat those checkpoints as existing implementation facts to verify, not as proof that every API is correct.

The documentation corpus README describes itself as a Microsoft Learn/archived MSDN Windows CE 1.0–6.0 corpus. This is useful as the primary local evidence store, but its coverage must be inventoried rather than assumed complete.

Microsoft's archived CE documentation explicitly records CE-specific requirements such as header and link library. For example, the CE 5.0 CRT documentation identifies `coredll.dll` for functions such as `malloc`, `setjmp`, and `getchar`, demonstrating why a desktop CRT assumption is unsafe.

Microsoft's CE documentation also shows that CE's system-library composition differs from desktop Windows. CE 6.0 documentation describes `coredll.dll` and other CE-specific system libraries rather than the desktop `kernel32.dll`/`user32.dll`/`gdi32.dll` split.

Current upstream libc++ documentation officially lists modern Windows support, but does not list Windows CE as an officially supported platform. Therefore CE support must be treated as a new target/integration problem rather than inferred from the existing Windows implementation.

Current LLVM libc documentation likewise states that Windows support is partial and does not guarantee backward compatibility for obsolete operating systems. Do not assume LLVM libc is automatically suitable for Windows CE; establish the CE dependency model first.

---

## 1. Phase A — Repository and baseline audit

### A1. Repository state

Record for all three repositories:

- default branch;
- current commit;
- dirty/clean state in the local workspace;
- submodule relationships;
- existing CE branches/configuration;
- existing CE patches and tests;
- build-system entry points.

### A2. Existing instruction audit

Search all repositories for:

- `AGENTS.md`;
- `PLANS.md`;
- CE-specific instructions;
- generated files;
- TODO/FIXME notes concerning CE;
- comments that prescribe old SDK conventions.

Remove or replace obsolete header comments only after their historical status has been verified.

### A3. SDK inventory

Produce a machine-readable inventory of every `cellvm-sdk/include` header and every definition/import-library artifact.

For each header record:

- declarations;
- included headers;
- exported symbols;
- version macros;
- CE generation claims;
- library/DLL association;
- suspicious desktop-only declarations;
- duplicate declarations;
- unresolved dependencies.

Do not use filename spelling as proof of API ownership.

### A4. LLVM inventory

Identify all existing CE-related changes and every location where OS-specific behavior enters LLVM:

- `llvm::Triple` OS/environment handling;
- Clang `TargetInfo`;
- predefined macros;
- driver/toolchain selection;
- linker and assembler command construction;
- CMake runtime selection;
- compiler-rt;
- libunwind;
- libc++abi;
- libc++;
- libc;
- tests.

Create a dependency graph from target triple → driver → compiler runtime → system libraries → C++ runtime → application.

**Checkpoint A:** baseline inventory is complete and stored in the plan/research records.

---

## 2. Phase B — Build the Windows CE evidence corpus

### B1. Source collection

Prioritize Microsoft Learn and archived MSDN pages. Use Wayback only to recover official Microsoft material that is no longer directly accessible.

Do not collect from:

- Shared Source;
- Visual Studio implementation material;
- Platform Builder;
- leaks/dumps;
- private material.

CEGCC `mingwrt`/`w32api` CE portions are permitted only as secondary cross-checks.

### B2. Topic coverage

Collect evidence for at least:

- base CE types and fundamental types;
- calling conventions;
- exception/unwind behavior;
- process/thread APIs;
- memory allocation;
- file and I/O APIs;
- synchronization;
- time/date;
- Unicode/ANSI/generic text;
- locale/CRT functions;
- sockets/networking;
- DLL loading and imports;
- synchronization primitives;
- TLS if supported by the target generation;
- atomic capabilities;
- registry;
- COM/OLE where relevant;
- graphics/windowing where relevant;
- multimedia only where the SDK exposes it;
- compiler/runtime-specific requirements;
- headers and link libraries;
- availability by CE generation.

### B3. Generation matrix

Build a matrix with one row per API family and columns for:

`CE generation | header | symbol | import DLL/lib | signature | ABI | availability | source | status`

Never merge rows across generations merely because the symbol name is identical.

### B4. Conflict resolution

When sources disagree:

1. identify whether the disagreement is caused by generation;
2. check the original Microsoft page/version;
3. check archived Microsoft copies;
4. check multiple official pages describing the same API;
5. use secondary sources only as a cross-check;
6. record the unresolved ambiguity rather than silently choosing a value.

### B5. Corpus storage

Store all collected evidence in `kagurasumusun/wince-docs-corpus` with provenance and generation metadata.

**Checkpoint B:** every SDK surface being implemented has an evidence row or an explicitly documented evidence gap.

---

## 3. Phase C — Close cellvm-sdk gaps before LLVM integration

### C1. Header correctness

Compare the complete header inventory against the evidence matrix.

Fix:

- missing declarations;
- wrong signatures;
- wrong typedefs;
- wrong macros;
- wrong constants;
- wrong structure layouts;
- wrong include relationships;
- wrong header ownership;
- wrong library associations;
- duplicate/conflicting declarations;
- generation contamination.

### C2. Legacy comments

Find comments that tell a future agent to follow obsolete conventions or old plans. Remove or rewrite them using current evidence.

Do not delete useful historical provenance merely because it is old; delete/rewrite only instructions that could incorrectly control current implementation.

### C3. Import/link surface

Verify that every CE API required by the target runtime has the correct import/link representation.

Pay special attention to the distinction between:

- header declaration;
- import library/definition;
- system DLL provided by CE;
- implementation that belongs in LLVM;
- compatibility implementation that belongs in a separate POSIX layer.

### C4. Host validation

Compile every SDK header in a representative matrix of C/C++ modes where possible.

Add tests for:

- C89/C90-sensitive legacy headers where applicable;
- C99/C11 declarations;
- C++ compatibility;
- packing/alignment assertions;
- pointer/integer assumptions;
- duplicate inclusion;
- include-order independence.

Host validation is syntax/type validation only. It is not CE ABI proof.

**Checkpoint C:** `cellvm-sdk` has no known missing/inconsistent declaration within the declared target scope, and every remaining limitation is documented with evidence.

---

## 4. Phase D — Define the LLVM Windows CE target contract

### D1. Triple

Determine the canonical LLVM triple spelling and environment representation for the CE target.

Do not reuse a desktop Windows environment merely because both use PE/COFF.

Define explicitly:

- architecture;
- vendor field;
- OS field;
- environment/sub-environment;
- ABI;
- version semantics.

Add parser/printer/normalization tests.

### D2. Clang TargetInfo

Implement only CE-specific properties supported by evidence:

- predefined CE macros;
- object/data layout;
- wchar/TCHAR-related assumptions where applicable;
- calling convention defaults;
- TLS support;
- exception support;
- atomic capabilities;
- alignment rules;
- visibility/linkage behavior.

Every non-default value must have a reason and evidence.

### D3. Driver/toolchain

Make the driver select `cellvm-sdk` headers and libraries without falling through to desktop Windows paths.

Verify:

- include search paths;
- library search paths;
- system libraries;
- startup/runtime objects;
- linker flags;
- architecture-specific flags;
- import library handling;
- C and C++ runtime selection.

Add command-line `-###` tests so the exact toolchain selection is regression-tested.

**Checkpoint D:** a trivial CE C program can be lowered to a CE object with the intended triple and no accidental desktop runtime dependency.

---

## 5. Phase E — compiler-rt and low-level runtime

### E1. Inventory required compiler-rt builtins

Determine which compiler-generated helper functions are required by each supported architecture and CE ABI.

Separate:

- compiler builtins that LLVM must provide;
- OS services already provided by CE;
- functions that require a compatibility layer.

### E2. ABI-sensitive helpers

Verify integer arithmetic, division, floating-point helpers, atomics, stack protection, and other compiler-generated calls against the actual target ABI.

Do not copy desktop Windows implementations when they depend on unavailable APIs or incompatible CRT conventions.

### E3. Test

Compile representative C programs that force compiler-rt builtins and inspect the undefined-symbol set before linking.

**Checkpoint E:** all required compiler-generated helper symbols resolve through the intended LLVM runtime or CE-provided interface.

---

## 6. Phase F — libunwind / libc++abi / exceptions

### F1. Establish CE exception model

Use CE-specific evidence and the actual compiler/ABI behavior to determine whether C++ exceptions are supported for the target and what unwind personality/ABI is valid.

Do not infer this from desktop Windows.

### F2. libunwind

Select or implement the unwind backend appropriate to the target architecture/ABI.

Verify:

- register numbering;
- unwind information format;
- personality routines;
- stack unwinding;
- exception object ABI;
- setjmp/longjmp interactions where relevant.

### F3. libc++abi

Remove dependencies on unavailable desktop runtimes and connect required operations to CE/LLVM facilities.

### F4. Tests

Add compile/link tests and, where a CE execution environment is available, runtime exception tests.

**Checkpoint F:** C++ exception-related components either build and link correctly for CE or are explicitly and correctly disabled where the target contract proves they cannot be supported.

---

## 7. Phase G — libc++

### G1. Dependency audit

Audit every libc++ dependency on:

- CRT functions;
- threads;
- synchronization;
- filesystem;
- locale;
- time;
- random devices;
- dynamic loading;
- environment variables;
- terminal/stdio behavior;
- wide-character support.

Map each dependency to one of:

1. CE API;
2. compiler-rt/LLVM runtime;
3. `cellvm-sdk` compatibility layer;
4. unsupported feature that must be gated.

### G2. Avoid the desktop Windows path

Do not enable the ordinary Windows implementation simply because `WIN32` is true. CE needs an explicit target/environment condition where semantics differ.

### G3. ABI and configuration

Verify:

- `_LIBCPP_*` feature macros;
- thread API selection;
- filesystem availability;
- locale availability;
- wide character support;
- exception support;
- ABI library selection;
- allocator behavior;
- dynamic/static runtime assumptions.

### G4. Tests

Build libc++ headers and libraries against `cellvm-sdk`. Add targeted tests for each CE-specific adaptation.

**Checkpoint G:** libc++ builds without an accidental dependency on MSVC STL, desktop CRT, or unsupported Windows APIs.

---

## 8. Phase H — LLVM libc decision

LLVM libc is a separate implementation of the C standard library, not a synonym for `cellvm-sdk`.

Before enabling it, answer:

- Is LLVM libc appropriate for the CE target?
- Which OS primitives does it require?
- Which required functions are implemented?
- Which CE-specific operations must be provided externally?
- Does using LLVM libc conflict with the project's desired model of consuming the CE-provided CRT/API?

If evidence shows that LLVM libc is not the correct layer for this target, do not force it into the build merely to satisfy a checklist item. Document the reason and ensure the supported C library path is complete.

**Checkpoint H:** the C library ownership model is explicit and consistent.

---

## 9. Phase I — POSIX compatibility layer

Keep POSIX compatibility separate from the CE API.

For every POSIX function added:

- identify the POSIX semantics;
- identify the CE primitive(s) used underneath;
- document semantic differences;
- add a separate namespace/file/component where practical;
- test the compatibility layer independently.

Do not alter CE declarations to make them look POSIX-like.

**Checkpoint I:** POSIX support is additive and cannot silently change the Windows CE API contract.

---

## 10. Phase J — End-to-end build integration

### J1. Configure

Create a clean CE cross-build configuration that points at `cellvm-sdk`.

### J2. Bootstrap

Build the minimum compiler/toolchain needed to compile CE objects.

### J3. Runtime build

Build the supported combination of:

- compiler-rt;
- libunwind;
- libc++abi;
- libc++;
- selected C library implementation.

### J4. Link

Build a minimal C program and a minimal C++ program against the CE import surface.

### J5. Symbol audit

Inspect linked objects/libraries for accidental references to:

- desktop `kernel32`/`user32`/`gdi32` style dependencies;
- MSVC CRT/STL runtime names;
- unsupported POSIX symbols;
- APIs not present in the selected CE generation.

### J6. Negative tests

Intentionally request a desktop-only facility and verify that the CE target does not silently accept it through a wrong toolchain path.

**Checkpoint J:** clean CE cross-build succeeds from compiler invocation through final link for the supported test programs.

---

## 11. Phase K — Comprehensive verification loop

Run the required cycle repeatedly:

`information collection → investigation → verification → implementation/fix → information collection → ...`

When implementation deltas reach zero, switch to:

`test → verify → investigate → fix → test → ...`

For every failure:

1. save the exact diagnostic;
2. identify the first incorrect assumption;
3. trace it to source/header/build-system/runtime;
4. verify against CE evidence;
5. fix the root cause;
6. add or strengthen a regression test;
7. rerun the failed and adjacent tests;
8. re-check the documentation matrix.

Never mark a failure as harmless merely because another configuration builds.

---

## 12. Final generation-pollution audit

Before completion, search the complete diff and generated build metadata for cross-generation leakage.

Examples of things to detect:

- CE 5-only APIs exposed to CE 4.x without a version gate;
- CE 6 memory/ABI assumptions applied to older CE kernels;
- newer library names copied into older targets;
- mobile/Windows Mobile-specific APIs treated as universal CE APIs;
- desktop Windows macros controlling CE behavior;
- `_WIN32` used where an explicit CE condition is required;
- W64/W32 structure/layout assumptions imported into CE code.

Every suspicious match must be either corrected or explicitly justified with evidence.

---

## 13. Final completeness matrix

The final review must contain a checklist with at least these dimensions:

| Area | Evidence | SDK state | LLVM state | Build test | Runtime test | Generation review | Status |
|---|---|---|---|---|---|---|---|
| Types/ABI | required | required | required | required | where possible | required | |
| Headers | required | required | n/a | required | n/a | required | |
| Import libraries | required | required | required | required | n/a | required | |
| Triple | required | n/a | required | required | n/a | required | |
| Clang target | required | n/a | required | required | n/a | required | |
| Driver/toolchain | required | n/a | required | required | n/a | required | |
| compiler-rt | required | n/a | required | required | where possible | required | |
| libunwind | required | n/a | required | required | where possible | required | |
| libc++abi | required | n/a | required | required | where possible | required | |
| libc++ | required | n/a | required | required | where possible | required | |
| C library | required | n/a | required | required | where possible | required | |
| POSIX layer | required | required if present | required if used | required | where possible | required | |
| Docs corpus | required | required | required | n/a | n/a | required | |

A blank status is a failure of the completion process, not a reason to declare success.

---

## 14. Definition of done

Completion requires all of the following:

- [ ] User requirements are implemented.
- [ ] `cellvm-sdk` has no known missing/inconsistent item within the declared target scope.
- [ ] Windows CE public evidence has been reconciled with the implementation.
- [ ] CE generations are explicitly separated wherever behavior/availability differs.
- [ ] Legacy SDK comments that could mislead future agents have been removed or rewritten.
- [ ] LLVM target/triple/driver support is complete for the declared target.
- [ ] compiler-rt requirements are complete.
- [ ] libunwind requirements are complete.
- [ ] libc++abi requirements are complete.
- [ ] libc++ requirements are complete.
- [ ] The selected C library path is complete and its ownership is explicit.
- [ ] POSIX compatibility, if provided, is isolated from the CE API.
- [ ] CE-targeted C and C++ test builds succeed.
- [ ] Relevant runtime/link tests succeed or are explicitly unavailable with a documented infrastructure reason.
- [ ] No desktop Windows API has leaked into the CE contract without an evidence-backed reason.
- [ ] No debug code remains.
- [ ] No unintended files were modified.
- [ ] The complete diff has been reviewed.
- [ ] The documentation corpus contains the evidence used for the implementation.
- [ ] Submodule/reference updates are synchronized after repository pushes.
- [ ] The original task has been re-read and every requirement checked again.

A push/merge is only a transport step. It is not the completion condition.
