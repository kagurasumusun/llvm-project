# C++ ABI / runtime: cellvm (Clang for arm-pc-wince) vs eVC++ (MSVC)

Status: audited 2026-09-01. Regression coverage:
`clang/test/CodeGen/ARM/wince-cpp-abi.cpp` (mangling + size probes +
driver flags).

The cellvm toolchain is **GNU/Itanium ABI + libc++/libc++abi** on
Windows CE. eVC++ 4.0 (the CE-era MSVC) is a **MSVC ABI** toolchain.
They are object-incompatible; source-level porting is the supported
path.

## Where the ABIs agree (source ports usually work unchanged)

| Aspect | cellvm (arm-pc-wince) | eVC++ 4.0 | Notes |
|---|---|---|---|
| `wchar_t` | `unsigned short` (16-bit) | `unsigned short` (16-bit) | driver forces `-fwchar-type=short -fno-signed-wchar` |
| `long`, pointers | 32-bit | 32-bit | |
| `bool`, `char` | 1 byte / signed | 1 byte / signed | |
| default struct alignment | 4 | 4 (ARM) | no `/Zp` differences in practice |
| Win32 API surface | w32api CE headers, stdcall imports via `__stdcall` | same API | `__stdcall` accepted (calling convention on ARM is single) |
| CE CRT semantics | mingwrt (CeGCC lineage), `_MT` with `-pthread` | eVC++ CRT | `WinMainCRTStartup`/`DllMainCRTStartup` entry, subsystem 9 |

The driver also passes `-fms-extensions` (w32api headers use
`__declspec`, `#pragma pack` etc.) and `-fgnuc-version=14.2` (CeGCC
era source compatibility), and for C: `-fgnu89-inline -fcommon`.

## Where the ABIs differ (porting breakage)

| Aspect | cellvm | eVC++ | Porting action |
|---|---|---|---|
| name mangling | Itanium (`_Z3addii`) | MSVC (`_?add@@YAHHH@Z`) | link-level interop impossible; recompile the source |
| C++ exceptions | ARM EHABI + winh CE-compressed pdata (Itanium runtime) | SEH-based MSVC C++ EH | `#pragma`-level differences; `__except`/`__try` in C++ → SEH intrinsics unavailable |
| RTTI | Itanium (`typeinfo`, `dynamic_cast`) | MSVC RTTI + `__RTCastToVoid`/`__RTCastTo`/`__RTtypeid` intrinsics | replace `__RT*` intrinsics with `dynamic_cast`/`typeid` |
| `__asm` | not supported | supported (ARM assembler intrinsics) | rewrite as inline-asm C++ (`__asm volatile` clang form) |
| CRT internals | mingwrt: `errno` single shared static, `_errno` n/a | thread-local `_errno()` | code reading `errno` after API calls works; direct `_errno()` does not port |
| `set_se_translator` / `<eh.h>` | unavailable | available | MSVC-specific EH plumbing; not needed for plain try/catch |
| `new`/`delete` | Itanium `operator new` (libc++abi) | MSVC CRT (`_new_mode`, `_set_new_mode`) | avoid CRT EH mode APIs |
| `std::thread` | OFF (documented; CE thread primitives not wired) | N/A (C++03) | use native `CreateThread`/winh |
| `std::filesystem` | OFF | N/A | use Win32 file APIs |
| debug info | DWARF (default) or CodeView (`-gcodeview`, embedded; no PDB) | PDB | see README "PDB / CodeView" |

## Object-level interop

Not possible and not planned: an eVC++ `.obj`/`.lib` and a cellvm
`.obj` cannot be linked together (mangling, EH tables, CRT symbols all
differ). Porting means compiling the original source with the cellvm
toolchain; that is exactly the path already taken for
TECLIB/glpi-wince-agent and MaxSignal/Player.

## What this means for "C++ ABI / runtime differences"

No MSVC-ABI compatibility layer is implemented (mangler + MSVC-ABI
libc++ + CRT compat would be required; rejected as out of scope —
source porting covers the two target apps). The ABI choices that make
source porting smooth (16-bit wchar_t, -fms-extensions, _MT, gnu89
inline, common, gnu-version 14.2) are pinned by the lit test above.
