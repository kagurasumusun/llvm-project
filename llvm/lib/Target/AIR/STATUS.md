# Metal (AIR) Implementation Status & Verification Plan

## Current State: What Works

### BitcodeWriter (3 modes)
| Mode | Pointer | Attributes | Version | Status |
|------|---------|-----------|---------|--------|
| `Opaque` (default) | TYPE_CODE_OPAQUE_POINTER | All latest | 2 | ✅ Production |
| `Typed` (legacy) | TYPE_CODE_POINTER | All latest | 2 | ✅ Implemented |
| `AIR` (Metal) | TYPE_CODE_POINTER | Stripped | 1 | ⚠️ Needs verification |

### Tool: llvm-metallib
| Subcommand | Status |
|------------|--------|
| `assemble` (.bc → .air) | ⚠️ Header structure unverified on real hardware |
| `link` (.air → .metallib) | ⚠️ Fat 64 format unverified |
| `info` (inspect) | ✅ Basic |

### AIRPrepare Pass
| Function | Status |
|----------|--------|
| Strip DEAD_ON_UNWIND (91) | ✅ Fixes reported error |
| Strip other unsupported attrs | ⚠️ Whititelist needs real-device verification |
| No-op bitcast insertion | ✅ Implemented |

---

## Gaps Requiring Work

### 1. Bitcode Compatibility (CRITICAL)
- [ ] Verify bitcode module version 1 is correct for metalfe 32023.883
- [ ] Verify TYPE_CODE_POINTER encoding matches Apple's format exactly
- [ ] Test with real Metal runtime (macOS + Xcode)
- [ ] Verify all AIR intrinsics survive bitcode round-trip
- [ ] Verify metadata (!air.kernel etc.) encoding

### 2. Metal Parser/Lexer/Sema (HIGH PRIORITY)
Based on metal-info's `CLANG_FRONTEND_IMPL_MAP.md` and `clang_frontend_impl_map.csv`:

| Component | Status | Detail |
|-----------|--------|--------|
| **Lexer** | | |
| Metal keywords (kernel, vertex, fragment, device, constant, threadgroup) | ✅ | Registered in IdentifierTable when `LangOpts.Metal` |
| Metal 4.0+ keywords (ray_data, mesh, tile) | ✅ | In parser |
| **Parser** | | |
| [[buffer(N)]] attribute syntax | ✅ | C++11 attribute parsing |
| [[texture(N)]], [[sampler(N)]] | ✅ | |
| [[thread_position_in_grid]] etc. | ✅ | |
| [[color(N)]], [[flat]], [[perspective]] | ⚠️ | Partial - needs verification |
| [[mesh]] payload attributes | ⚠️ | Needs verification |
| [[raytracing]] attributes | ⚠️ | Needs verification |
| **Sema** | | |
| Address space conversion checking | ✅ | device→generic, constant→generic allowed |
| Metal function qualifier validation (kernel/vertex/fragment) | ✅ | |
| Metal overload resolution for builtins | ⚠️ | Needs verification |
| Metal type checking for textures/samplers | ⚠️ | Needs verification |
| **CodeGen** | | |
| !air.kernel metadata generation | ✅ | |
| !air.version metadata | ✅ | |
| !air.language_version | ✅ | |
| AIR intrinsic emission (__metal_*) | ✅ | 686 builtins mapped |
| Texture/sampler intrinsic emission | ⚠️ | Partial |
| Raytracing intrinsic emission | ⚠️ | Partial |
| Mesh shader intrinsic emission | ⚠️ | Not started |

### 3. Metal stdlib / Runtime (CLEANROOM IMPLEMENTATION NEEDED)

From metal-info's `stdlib_cleanroom_complete_map.csv` (113 modules) and
`rtlib_cleanroom_map.csv` (13,067 symbols):

#### Runtime Libraries (libmetal-rt etc.)
| Library | Symbols | Status |
|---------|---------|--------|
| libair_rt | ~2000 | ❌ Not started |
| libmetal_math_rt | ~5000 | ❌ Not started |
| libmetal_rt | ~3000 | ❌ Not started |
| libtracepoint_rt | ~500 | ❌ Not started |

#### Standard Library Modules
| Module | Status |
|--------|--------|
| metal_math | ❌ Not started |
| metal_relational | ❌ Not started |
| metal_geometric | ❌ Not started |
| metal_matrix | ❌ Not started |
| metal_atomic | ❌ Not started |
| metal_texture | ❌ Not started |
| metal_sampler | ❌ Not started |
| simd (simd.h) | ❌ Not started |
| Metal标准ライブラリ (全113モジュール) | ❌ Not started |

### 4. Metal-Specific Features Not Yet Implemented
| Feature | Status | Reference |
|---------|--------|-----------|
| Imageblocks (MSL4) | ❌ | metal-info golden probes |
| Mesh shaders | ❌ | metal-info golden probes |
| Ray tracing (intersection queries) | ❌ | AIR_VOCABULARY.md §5 |
| Tensor operations (MSL4) | ❌ | AIR_VOCABULARY.md §6.3 |
| Visible function tables | ❌ | AIR_VOCABULARY.md |
| Indirect command buffers | ❌ | AIR_VOCABULARY.md |
| Sparse textures | ❌ | |
| Atomic texture operations | ❌ | AIR_VOCABULARY.md §4 |

---

## Verification Plan

### Phase 1: Bitcode Compatibility (NOW)
1. Build clang with AIR support
2. Compile simple Metal kernel
3. Verify bitcode with `llvm-dis` (should round-trip)
4. Verify with `llvm-metallib info`
5. Test on real hardware (macOS + Xcode)

### Phase 2: Frontend Completeness
1. Run existing Metal test suite:
   ```
   llvm-lit clang/test/Parser/metal-*.metal
   llvm-lit clang/test/Sema/metal-*.metal
   llvm-lit clang/test/CodeGen/metal-*.metal
   ```
2. Compare output with Apple's golden IR from metal-info
3. Add missing test cases for each feature gap

### Phase 3: Runtime Implementation (CLEANROOM)
1. Create stub implementations for each rt library
2. Implement metal_math functions first (most commonly used)
3. Verify against Apple's reference implementations
4. Build comprehensive test suite

---

## How to Update for New LLVM Versions

When LLVM adds new attributes or changes bitcode format:

1. **Opaque mode**: No changes needed (standard LLVM behavior)
2. **Typed mode**: No changes needed (attributes pass through)
3. **AIR mode**: Update `AIRPrepare::isValidForAIR()` in `llvm/lib/Target/AIR/AIRPrepare.cpp`
   - Add new attributes to the whitelist if Metal runtime supports them
   - The default is to STRIP unknown attributes (safe)

To find which attributes Metal supports:
1. Compile with Apple's metalfe
2. Use `llvm-bcanalyzer` on the output .air
3. Compare attribute kinds

---

## File Map

```
llvm/include/llvm/Bitcode/BitcodeWriter.h     ← BitcodeEmitMode enum
llvm/lib/Bitcode/Writer/BitcodeWriter.cpp     ← 3-mode writer
llvm/lib/Target/AIR/AIRPrepare.cpp            ← Attribute filter
llvm/lib/Target/AIR/AIRWriter/                ← Standalone AIR writer (optional)
llvm/lib/Target/AIR/Runtime/                  ← Runtime stubs (future)
llvm/tools/llvm-metallib/                     ← metallib tool
clang/lib/CodeGen/BackendUtil.cpp             ← Integration
clang/include/clang/Basic/CodeGenOptions.h    ← AIRBitcodeEmitMode enum
```
