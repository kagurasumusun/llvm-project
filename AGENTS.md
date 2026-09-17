# AGENTS.md — Windows CE / LLVM cross-repository work

## 1. Scope

This is the operating contract for work spanning:

- `kagurasumusun/llvm-project` — compiler, target support, runtime libraries, C++ standard library, unwinder, and build/test integration.
- `kagurasumusun/cellvm-sdk` — Windows CE development interface surface: headers, declarations, import/link metadata, compatibility declarations, and related build metadata. Treat this as the Windows-CE equivalent of a Linux `-dev` package, **not** as an SDK that reimplements the operating system and **not** as an OS/runtime implementation.
- `kagurasumusun/wince-docs-corpus` — the evidence store for public Windows CE documentation used to derive and verify the implementation.

The end goal is a coherent Windows CE target that can be built with LLVM using `cellvm-sdk`, with no unexplained API/header/library mismatch and with the required LLVM runtimes and C++ libraries building successfully.

## 2. Non-negotiable architecture boundaries

### cellvm-sdk

`cellvm-sdk` supplies the development interface to the existing Windows CE platform:

- public CE headers and declarations;
- CE-specific constants, structures, macros, typedefs, calling conventions and annotations;
- import-library / symbol-definition material needed to link against CE system DLLs;
- compatibility declarations that belong to the development interface;
- build metadata and verification material.

Do **not** turn `cellvm-sdk` into an OS reimplementation. Do not add implementations of Windows CE kernel/system services merely because a header declares them.

### llvm-project

`llvm-project` supplies the compiler and language/runtime libraries. Its Windows CE work must adapt LLVM components to the interfaces actually exposed by Windows CE and `cellvm-sdk`.

For `libc`, `libc++`, `libc++abi`, `libunwind`, `compiler-rt`, and related runtime pieces, prefer connecting to facilities already supplied by Windows CE through the CE API/import libraries. Do not silently substitute desktop Windows, Win32, POSIX, or another CRT/runtime model.

### POSIX

POSIX compatibility is a separate compatibility layer. It must not be achieved by mutating the Windows CE API model into a POSIX API.

If POSIX functionality is needed, implement the compatibility layer separately in `cellvm-sdk` (or another explicitly scoped compatibility component) and document which CE primitives it maps to. Never make a POSIX assumption simply because desktop Windows happens to expose an analogous function.

## 3. Evidence policy

Implementation decisions must be evidence-driven.

### Source priority

1. Microsoft Learn / archived MSDN pages published by Microsoft.
2. Microsoft public historical documentation preserved by the Internet Archive / Wayback Machine.
3. Other large, reputable, lawful public sources, only when Microsoft material is unavailable or insufficient.
4. CEGCC `mingwrt` / `w32api` Windows CE portions may be consulted only as a secondary reference or for value/symbol cross-checking.

### Explicitly forbidden evidence

Do not use or reproduce information from:

- Shared Source;
- Visual Studio-specific implementation material;
- Platform Builder documentation/material;
- leaked or dumped material;
- private/non-public information;
- undocumented assumptions presented as facts.

### Desktop Windows separation

Windows CE is not desktop Windows. Do not use W32/W64 documentation as evidence for CE behavior, even when names look identical.

A desktop Windows result may only be used to explain LLVM's existing generic implementation mechanics; it must **not** establish the Windows CE API, ABI, availability, DLL ownership, header location, calling convention, structure layout, version availability, or runtime behavior.

Every CE-specific fact must be traceable to CE-specific evidence.

## 4. Generation discipline

Never collapse different Windows CE generations into one undocumented compatibility target.

Every discovered API/symbol/header/library fact must be recorded with:

- CE generation/version;
- product family where relevant;
- minimum/maximum supported version if documented;
- header;
- link library/import DLL;
- calling convention / ABI constraints when documented;
- availability notes and exclusions;
- evidence source.

If a declaration exists in multiple generations but its behavior or availability differs, keep separate records. Do not copy a newer declaration backward into an older CE target merely because the name matches.

The current `cellvm-sdk` build metadata already distinguishes CE deployment checkpoints such as `0x420`, `0x500`, and `0x600`; preserve this generation-aware approach and improve it rather than replacing it with a single generic `_WIN32` model.

## 5. Legacy comments in cellvm-sdk

Comments embedded in `cellvm-sdk` headers that describe old conventions, old plans, or historical implementation intentions are **not normative requirements**.

Treat such comments as legacy text. If they can cause an agent to follow an obsolete convention, remove or replace them with concise evidence-backed comments. Do not preserve obsolete instructions merely because they are already in a header.

The authoritative contract is the current implementation plus the evidence recorded in `wince-docs-corpus`.

## 6. Research-before-edit rule

Before changing a CE-facing declaration or LLVM CE integration:

1. Locate the existing declaration/implementation.
2. Identify its generation(s).
3. Search the corpus for the corresponding official documentation.
4. Search Microsoft Learn / archived MSDN when the corpus is incomplete.
5. Record the evidence and any conflicts.
6. Compare the evidence against the current implementation.
7. Define an explicit checkpoint for the discovered delta.
8. Only then implement the change.

After editing, repeat the evidence comparison. A patch is not complete merely because it compiles.

## 7. Required LLVM work areas

The plan must inspect and, where applicable, implement CE support across at least:

- target triple / OS environment identification;
- Clang target information and predefined macros;
- Clang driver/toolchain selection;
- assembler/linker invocation and CE import-library handling;
- compiler-rt;
- `libunwind`;
- `libc++abi` where required;
- `libc++`;
- LLVM libc only where it is actually appropriate to the CE target and evidence supports it;
- CMake/runtime build selection;
- feature/availability macros;
- headers and ABI assumptions;
- tests for compile, link and runtime-facing interfaces.

Do not enable a runtime component simply because its desktop Windows path builds. Establish the CE dependency model first.

## 8. Required cellvm-sdk work areas

Inventory the complete SDK and verify, at minimum:

- public headers and include relationships;
- duplicate/conflicting declarations;
- typedefs and fundamental CE types;
- structure packing/alignment where documented;
- integer/pointer-width assumptions;
- calling conventions;
- Unicode/ANSI and generic-text mappings;
- CE-specific macros and version gates;
- exported symbol declarations;
- DLL ownership and import-library mapping;
- library names and link requirements;
- API availability by CE generation;
- comments that encode obsolete rules;
- missing declarations and accidental declarations that belong to another generation or another Windows family.

Do not assume that a complete-looking header tree means the SDK is complete.

## 9. Required work cycle

For every substantial unit of work, execute this loop until the checkpoint is clean:

1. **Inspect** current code and repository state.
2. **Research** official CE evidence.
3. **Compare** evidence with code and record deltas.
4. **Plan** the smallest coherent implementation.
5. **Implement** the change.
6. **Build/test** the affected surface.
7. **Diagnose** every failure.
8. **Fix** the root cause.
9. **Re-test**.
10. **Re-review** against the evidence matrix.
11. Continue until no unexplained delta remains.

Never stop after the first successful build if the documentation comparison is incomplete.

If the same failed action fails twice without new information, stop repeating it, form a new hypothesis, and use a materially different diagnostic/implementation path.

## 10. Parallelism and waiting

Do not deliberately idle while a long build/test is running. When the available execution environment supports safe parallel work, use build/test time for independent research, source indexing, matrix preparation, static inspection, or documentation review.

Do not claim that parallel work occurred unless the execution environment actually permitted it.

## 11. Build/test expectations

A successful host build is not proof of CE correctness.

Required validation layers are:

1. host-side syntax/type/header checks;
2. LLVM target/triple/driver compile tests;
3. CE-targeted object generation;
4. CE-targeted linking against `cellvm-sdk` import libraries;
5. runtime-library builds for every component claimed to support CE;
6. negative tests proving desktop-only APIs are not accidentally selected;
7. generation-specific tests for the supported CE versions;
8. final clean build from a clean checkout/configuration.

For each failure, preserve the diagnostic, root cause, fix, and regression test.

## 12. Source corpus requirements

All collected documentation used as implementation evidence must be stored in `kagurasumusun/wince-docs-corpus`.

At minimum, maintain:

- source URL/reference;
- source title;
- Microsoft/archival provenance;
- CE generation;
- API/header/library topic;
- relevant extracted facts;
- conflicts/ambiguities;
- verification status.

The corpus is evidence, not executable SDK content.

## 13. Git discipline

Keep changes scoped and reviewable. Do not modify unrelated LLVM code.

Before each commit:

- inspect the complete diff;
- check for generated/debug files;
- check submodule state if applicable;
- verify no generation-specific files leaked into another generation;
- run the relevant tests.

A push is **never** the definition of done. After pushing a repository change, refresh/update the corresponding submodule/reference and continue the remaining verification work.

If a PAT is required for an operation, use the credential mechanism explicitly provided by the execution environment. Never print, log, or copy the token into source files, logs, plans, or documentation.

## 14. Completion gate

Do not declare completion until all of the following are true:

- the requested CE target is represented correctly in LLVM;
- `cellvm-sdk` is complete for the documented target scope;
- no unexplained SDK/LLVM declaration or ABI mismatch remains;
- `libc`/`libc++`/`compiler-rt`/`libunwind`/`libc++abi` applicability has been explicitly resolved;
- CE generation separation has been reviewed;
- documentation evidence has been reconciled with implementation;
- CE-targeted build/tests succeed for the declared target matrix;
- the final diff has been reviewed;
- the source corpus contains the evidence used for the implementation;
- no unrelated files were changed;
- the original requirements have been checked again.

If a genuine blocker remains, document the exact missing permission, missing public evidence, unavailable toolchain, or infrastructure limitation. Do not manufacture a success claim.
