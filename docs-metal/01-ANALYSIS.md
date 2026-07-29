# Metal 対応 — 情報源解析結果 (Phase 1: 情報理解 / 解析)

第一級情報源: `https://github.com/kagurasumusun/metal-info`
(解析時点のクローン: 481,170 files / 12 GB, 2026-07-29 取得)

本書は **実装判断の根拠** をすべて情報源の実測データに紐付けて記録する。
推測で埋めた項目は存在しない。未確定は `OPEN` と明記する。

---

## 0. 情報源の構成と、本実装が根拠として用いた部分

| パス | 内容 | 本実装での用途 |
|---|---|---|
| `research/spec/*.md` (40) | 仕様・実装マップ | 設計の骨格。特に `INFO_SET.md` (マスター索引), `IR_GROUND_TRUTH.md` (実 IR 確定事実), `METAL_*_IMPL_MAP.md` (層別実装点) |
| `research/datasets/*.csv` (78) | 機械生成テーブル | Builtin 686 件, 属性 90 件, 型 35+ 件, triple 対応, C++ 世代対応 |
| `research/golden/**` (175) | Apple 実機コンパイラ出力 (`.metal`/`.ll`/`.air`/`.metallib`) | **CodeGen の正解データ**。metadata スキーマの逐語的根拠 |
| `reference/metal-ast-*/` (18 ターゲット) | AST dump / IR / driver / meta 実測 | 属性クラス名・opaque 型・マクロ・診断・triple 対応の一次実測 |
| `reference-apple/clang/32023.883/include/metal/` | **Apple 純正 StdLib ヘッダ実物** (~130) | コンパイラが提供すべき語彙の確定 (再実装はしない) |
| `metal-reference/*.pdf` (11 版: 1.2〜4.1) | MSL 公式仕様書 | 版別仕様差分 |

---

## 1. 最重要の確定事実 (情報源からの抽出)

### 1.1 AIR は素の LLVM Bitcode である

`research/spec/IR_GROUND_TRUTH.md` §1 (C-4):

> bitcode 読込 (llvmlite/LLVM 20) — **701 モジュール全成功**、独自レコード/enum 衝突は観測なし

さらに全 OS 展開で **1,764 unique modules / パースエラー 0**。

→ **設計含意**: Bitcode Writer / Container に Apple 独自拡張を実装する必要は**ない**。
AIR 固有性はすべて「IR の中身」= triple / datalayout / metadata / intrinsic 命名 / addrspace に閉じる。

### 1.2 呼出規約は特殊 CC ではない (A-3 確定)

> 701 モジュール全関数が **デフォルト C calling convention**。`spir_kernel` 等の特殊 CC は一切存在しない

golden `P01/probe.ll` でも `define void @probe_p01_kernel(...) local_unnamed_addr #0` — CC 指定なし。

→ **設計含意**: エントリの識別は CC ではなく **named metadata** (`!air.kernel` 等) で行う。
`spec/METAL_CODEGEN_IMPL_MAP.md` には "fastcc または air_kernel" という記述があるが、
これは golden 実測 (`P01`,`P02`) と**矛盾する**。**実測 golden を優先**する。

### 1.3 datalayout は全モジュールで一意

air64 (`IR_GROUND_TRUTH.md` §2.3, golden 全 54 `.ll` で一致):

```
e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-
v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-
v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-n8:16:32
```

air32 (本解析で `reference/metal-ast-macos-air32/ir/...probe_min_10_13_llvm-ir-O0.ll` から実測):
`e-p:32:32:32-` で始まり以降 air64 と同一。

要点: `v24:32:32` / `v48:64:64` = 3 要素ベクタの align 規定。`n8:16:32` = native int widths。

### 1.4 アドレス空間の実測確定値

`IR_GROUND_TRUTH.md` §2.4 + golden corpus の実測 (`addrspace(N)` 出現分布):

| AS | 意味 | 実測根拠 |
|---:|---|---|
| 0 | `thread` (default) | 引数 `ptr` 無印 |
| 1 | `device` | golden 1,841 出現。texture handle もここ |
| 2 | `constant` | golden 431 出現。sampler handle もここ |
| 3 | `threadgroup` | golden 16 出現 (`P01` の `float addrspace(3)*`) |
| 4 | `threadgroup_imageblock` | golden 44 出現。`%struct._imageblock_t addrspace(4)*` |
| 5 | (raytracing 内部 payload) | golden 80 出現。`air.intersect_direct_access` の戻り `i8 addrspace(5)*` |
| 7 | `object_data` (mesh) | golden 20 出現。`%struct._mesh_t addrspace(7)*` |
| 9 | `ray_data` | golden 177 出現。`%struct._intersection_result_t addrspace(9)*` |

注: `spec/METAL_TARGETINFO_IMPL_MAP.md` は「as4/as9 = ray_data/object_data」と大雑把に書くが、
golden 実測では **imageblock=4, mesh/object=7, ray_data(intersection result)=9** が明確。
実測を採用する。AS 6/8 は corpus 未出現 → `OPEN`。

### 1.5 マングリングはベンダ拡張修飾子方式

本解析で `reference/metal-ast-macos-air64/ir/*.ll` を全走査して抽出:

```
U9MTLdevice   U11MTLconstant   U14MTLthreadgroup
U10MTLraydata U13MTLobjectdata U24MTLthreadgroupimageblock
U18MTLcoherent
```

例: `@_Z11read_devicePU9MTLdevice10AddressBox` = `read_device(device AddressBox*)`

→ **設計含意**: Itanium mangler の `<extended-qualifier>` (`U <len> <name>`) を使う。
名前は `MTL` + 小文字連結。`coherent` も修飾子として mangle される。

### 1.6 エントリ metadata スキーマ (golden P01/P02 逐語)

`!air.kernel = !{!fn}` / `!fn = !{ptr @f, !{}, !args}`。
`!air.vertex = !{!{ptr @f, !outputs, !inputs}}`。
`!air.fragment = !{!{ptr @f, !outputs, !inputs (, !"early_fragment_tests")?}}`。

引数 operand 実例 (P01):
```
!12 = !{i32 0, !"air.buffer", !"air.location_index", i32 0, i32 1, !"air.read_write",
       !"air.address_space", i32 1, !"air.arg_type_size", i32 4,
       !"air.arg_type_align_size", i32 4, !"air.arg_type_name", !"float",
       !"air.arg_name", !"b"}
!15 = !{i32 2, !"air.texture", !"air.location_index", i32 0, i32 1, !"air.sample",
       !"air.arg_type_name", !"texture2d<float, sample>", !"air.arg_name", !"t"}
!18 = !{i32 5, !"air.thread_position_in_grid", !"air.arg_type_name", !"uint",
       !"air.arg_name", !"i"}
```
- buffer は `air.address_space` を持つ / texture・sampler は持たない
- builtin 入力は `air.location_index` を持たない
- `constant` 参照引数には `air.buffer_size` + `air.struct_type_info` が付く
- vertex user output は `generated(<mangled>)` 一意 ID で fragment 入力と接続
- 補間は `air.center` + `air.perspective|no_perspective` の文字列組
- 未使用引数に `air.arg_unused`

### 1.7 モジュール metadata / module flags (golden P01 逐語)

```
!llvm.module.flags: SDK Version [26,5], wchar_size 1→4, frame-pointer 7→2,
  air.max_device_buffers 31, air.max_constant_buffers 31,
  air.max_threadgroup_buffers 31, air.max_textures 128,
  air.max_read_write_textures 8, air.max_samplers 16
!air.version          = !{!{i32 2, i32 8, i32 0}}
!air.language_version = !{!{!"Metal", i32 3, i32 2, i32 0}}
!air.compile_options  = !{denorms_disable, fast_math_enable, framebuffer_fetch_enable}
!air.source_file_name = !{!"<絶対パス>"}
!llvm.ident           = !{!"Apple metal version 32023.883 (metalfe-32023.883)"}
```

`reference/.../meta/air-metadata-version-changes.csv` による版別存在:

| named md | metal1 | metal2 | metal3 | metal4 |
|---|---|---|---|---|
| air.kernel / vertex / fragment / version / language_version / compile_options / source_file_name / function_constants | ✅ | ✅ | ✅ | ✅ |
| air.sampler_states | ✗ | ✅ | ✅ | ✅ |
| air.object / air.visible / air.visible_function_references | ✗ | ✗ | ✅ | ✅ |

### 1.8 `-std=` ↔ AIR version ↔ triple の関係 (本解析で新規確定)

`spec/*.md` の複数箇所は「`-std` が `air64_vNN` を決める」と読める記述をしているが、
`reference/metal-ast-macos-air64/{ir,log}` の全走査で **これは誤り**であることを確認した。

実測 (`llvm-ir-O0.ll` の `target triple` と `!air.version`):

| `-std=` | 使用した min-OS | triple | `!air.version` | `!air.language_version` |
|---|---|---|---|---|
| macos-metal1.1 | 10.13 | `air64_v20-apple-macosx10.13.0` | 2.0.0 | Metal 1.1.0 |
| macos-metal1.2 | 10.14 | `air64_v21-apple-macosx10.14.0` | 2.1.0 | Metal 1.2.0 |
| macos-metal2.0 | 10.15 | `air64_v22-apple-macosx10.15.0` | 2.2.0 | Metal 2.0.0 |
| macos-metal2.1 | 11.7 | `air64_v23-apple-macosx11.7.0` | 2.3.0 | Metal 2.1.0 |
| macos-metal2.2 | 12.7 | `air64_v24-apple-macosx12.7.0` | 2.4.0 | Metal 2.2.0 |
| macos-metal2.3 | 13.7 | `air64_v25-apple-macosx13.7.0` | 2.5.0 | Metal 2.3.0 |
| macos-metal2.4 | 14.7 | `air64_v26-apple-macosx14.7.0` | 2.6.0 | Metal 2.4.0 |
| metal3.0 | 15.7 | `air64_v27-apple-macosx15.7.0` | 2.7.0 | Metal 3.0.0 |
| metal3.1 | 26.0 | `air64_v28-apple-macosx26.0.0` | 2.8.0 | Metal 3.1.0 |
| metal3.2 | 26.0 | `air64_v28-apple-macosx26.0.0` | 2.8.0 | Metal 3.2.0 |
| metal4.0 | 26.0 | `air64_v28-apple-macosx26.0.0` | 2.8.0 | Metal 4.0.0 |

そして **min-OS を変えると同一 `-std` でも triple が変わる** ことを直接確認 (driver `-###` ログ):

```
10_11 => air64_v18-apple-macosx10.11.0     ("v" + "1"+"8"  ← 10.11 由来)
10_12 => air64_v111-apple-macosx10.12.0    ("v" + "1"+"11" ← 10.12 由来)
10_13 => air64_v20-apple-macosx10.13.0
10_14 => air64_v21 ... 15_7 => air64_v27, 26_0..26_9 => air64_v28
```

→ **確定した規則**:
- `air64_vNN` の `NN` と `!air.version` は **deployment target OS 版のみ**の関数。
- `!air.language_version` は **`-std=` のみ**の関数。
- 2 者は独立。`-std=metal3.1` を macOS 15.7 に向ければ `air64_v27` + `language 3.1.0` になる。
- macOS 10.11/10.12 期は `major*10 + minor` を素朴連結する旧式 (→ `18`, `111`)。
  10.13 以降は表引き (`20,21,22,23,24,25,26,27,28`)。

これは `datasets/legacy_metal_support_map.csv` の `default_air_ver` 列 (std→air ver) と
食い違うが、同 CSV は「その std を使う典型的な最小 OS」を並べたものであり、
driver 実測 (`-###`) の方が一次情報として強い。**実測を採用**する。

### 1.9 C++ 言語ベース (指示書「C++ Language Base」への回答)

`spec/METAL_CXX_GENERATIONS_MAP.md` / `METAL_CXX_MASTER_ATLAS.md` (実機 `metalfe -E -dM` 実測):

| `-std=` | C++ 世代 | `__cplusplus` |
|---|---|---|
| macos-metal1.0 / 1.1 / 1.2 | **C++11** | 201103L |
| macos-metal2.0 〜 2.4 | **C++14** | 201402L |
| metal3.0 / 3.1 / 3.2 | **C++14** | 201402L |
| metal4.0 | **C++17** | 201703L |
| metal4.1 | **C++17** | 201703L |

重要な補足 (同アトラス §2, FEAT-07〜09): metalfe は C++11/14 モードでも
**`if constexpr` / 構造化束縛 / fold 式を backport して有効化**している
(stdlib ヘッダのメタプログラミングがそれに依存するため)。

MSL の恒久的制限 (FEAT-10〜14): concepts / virtual / 例外 / RTTI(dynamic_cast,typeid) / new・delete は
全版で拒否。

### 1.10 Apple 純正 StdLib が要求するコンパイラ機能 (ヘッダ実物からの抽出)

`reference-apple/clang/32023.883/include/metal/` を全走査:

- `__metal_*` builtin: **686 個** (本解析で独立に数え直して一致)
- `__attribute__((__ext_vector_type__(N)))`: 66 箇所 — upstream に既存
- `__attribute__((__packed_vector_type__(N)))`: 66 箇所 — **upstream に存在しない Apple 独自拡張。FE 実装必須**
- `#pragma METAL internals : enable|disable`: 各 52 箇所 — **独自 pragma。実装必須**
- `#pragma METAL fp math_mode(safe)`: 90 箇所 — **独自 pragma。実装必須**
- `__attribute__((format(metal_os_log, F, F+1)))` — format 種別 `metal_os_log` の追加が必要
- アドレス空間キーワードをヘッダは `#define` していない
  (`device` 9,596 / `constant` 3,210 / `thread` 6,049 / `threadgroup` 509 /
   `ray_data` 3,441 / `object_data` 3,057 / `threadgroup_imageblock` 406 出現)
  → `INFO_SET.md` F-2 の通り **コンパイラ組込みキーワード**であることが裏付けられる

### 1.11 opaque builtin 型は 37 個 (AST 実測)

`reference/.../ast/*_attributes_all_*_ast-text.txt` の TranslationUnitDecl 先頭に、
`-x metal` で常に注入される implicit TypedefDecl + BuiltinType として現れる。
**全 MSL 版 (1.1〜4.0) で 37 個・順序も同一**であることを確認した (版による増減なし)。

順序 (この順で BuiltinType を定義する必要がある):
```
texture_1d, texture_1d_array, texture_2d, texture_2d_array, texture_3d,
texture_cube, texture_cube_array, texture_2d_ms, texture_2d_ms_array,
texture_buffer_1d,
depth_2d, depth_2d_array, depth_cube, depth_cube_array, depth_2d_ms,
depth_2d_ms_array,
sampler, threadgroup_event, imageblock, patch_control_point,
command_buffer, render_pipeline_state, compute_pipeline_state,
interpolant, vertex_value, visible_function_table,
intersection_function_table, instance_acceleration_structure,
primitive_acceleration_structure, mesh, mesh_grid_properties,
intersection_query, function_handle, intersection_result,
depth_stencil_state, tensor, tensor_thread
```
CodeGen での lowering は `datasets/type_layout_map.csv` により
`__metal_texture_2d_t` → `%struct._texture_2d_t = type opaque` (先頭 `__metal` を除き `_` 始まり)。

### 1.12 AST 属性クラスは `Metal*Attr` (実測名)

`reference/.../meta/attr-class-topology.csv` に実測出現数付きで 30 クラス。
`datasets/clang_frontend_impl_map.csv` の attr 層に **spelling 90 種**。
版ゲートは実診断文から抽出 (本解析):

```
early_fragment_tests, function_constant            -> macos-metal1.2+
id, raster_order_group, simdgroup_index_in_threadgroup,
simdgroups_per_threadgroup, thread_index_in_simdgroup,
viewport_array_index                                -> macos-metal2.0+
max_total_threads_per_threadgroup                   -> macos-metal2.1+
amplification_id, barycentric_coord, host_name,
primitive_id                                        -> macos-metal2.2+
intersection, payload, stitchable, visible          -> macos-metal2.3+
mesh, object                                        -> metal3.0+
```
診断文言も実測: `'X' attribute requires Metal language standard <std> or higher`

### 1.13 プリプロセッサ定義 (実測 688 行)

`reference/.../meta/metal-predefined-macros.txt` (metal4.0 / macOS 26 実測)。
Metal 固有は以下の群:
- `__METAL__ 1`, `__METAL_VERSION__ 400`, `__AIR64__ 1`, `__AIR_VERSION__ 20800`
- ABI: `__AIR_ABI__ = __AIR_PB_ABI__`, `__AIR_PB_ABI__ 0`, `__AIR_MB_ABI__ 1`, `__AIR_VB_ABI__ 2`
- 数学モード: `__METAL_FAST_MATH__ 0`, `__METAL_HALF_MATH__ 1`, `__METAL_NATIVE_MATH__ 2`,
  `__METAL_PRECISE_MATH__ 3`, `__METAL_MATH_FP32_FUNCTIONS_FAST__ 1`
- enum 値マクロ群 (access / address / border_color / compare_func / coord / cull_mode /
  filter / memory_flags / memory_order / memory_scope / mip_filter / os_log_type /
  raytracing_* / reduction / rounding / simdgroup_load_store_bounds_check /
  texture_write_rounding / topology / vertex_index / winding / primitive_type /
  packed_numeric_format / coherence …) — 計 140 種前後
- `__METAL_VOTE_T__ long unsigned int`
- `__HAVE_*` 能力マクロは **ヘッダ側 (`metal_config`) が `__METAL_VERSION__` を見て定義**する
  (216 マクロ / `datasets/have_matrix.csv`)。**コンパイラは定義しない**。

これは重要: `__HAVE_*` を FE で定義してはならない。FE は `__METAL_VERSION__` と
`__METAL_IOS__`/`__METAL_MACOS__` 等を出し、純正 `metal_config` が展開する。

### 1.14 診断カタログ (実測)

`reference/.../meta/diagnostic-catalog.csv` (277 行) / `error-ids-complete.csv`。
Metal 固有で頻出かつ再現すべきもの:
- `pointer type must have explicit address space qualifier`
- `program scope variable must reside in constant address space`
- `'thread_local' is not supported in Metal`
- `invalid return type 'X' for vertex function`
- `invalid address space qualification for buffer pointee type 'X'`
- `type 'X' (aka 'Y') is not valid for attribute 'Z'`
- `valid address space qualifications are device and constant` (note)
- `'X' attribute requires Metal language standard <std> or higher`
- `unable to create target: 'No available targets are compatible with triple "air64_vNN-..."'`

`meta/sema-rule-catalog.csv` に 200 の Sema 検査シナリオが分類済 (address_space / attribute / …)。

### 1.15 Driver 実測 (cc1 起動引数)

`reference/.../meta/metal-cc1-invocations.txt.gz` に実物。要点:
- `-x metal` / `-triple air64_vNN-apple-macosxX.Y.0`
- `-finclude-default-header`
- `-fmetal-math-fp32-functions=fast` (既定)
- fast-math 一式: `-menable-no-infs -menable-no-nans -fapprox-func
  -menable-unsafe-fp-math -fno-signed-zeros -mreassociate -freciprocal-math
  -ffp-contract=fast -ffast-math -ffinite-math-only`
- `-std=osx-metal1.2` (= `macos-metal1.2` の別名。両綴りが存在)
- `-fmodules -fmodule-map-file=<...>/metal/module.modulemap` + `-fmodules-cache-path`
- `-no-opaque-pointers` ← **Apple は typed pointer で出力**している
- `-Wmtl-shader-return-type -Werror=mtl-shader-return-type` (Metal 固有 warning group)
- リンク段は `darwin::AIRLLD` (`metallib` 生成)

`-no-opaque-pointers` は LLVM 16 (本リポジトリ) ではまだ利用可能であり、
golden `.ll` も typed pointer (`float addrspace(1)*`) 形式である。LLVM 16 ベースは好都合。

---

## 2. 情報源内の矛盾と、その解決

| # | 矛盾 | 解決 (採用したもの) | 根拠 |
|---|---|---|---|
| 1 | CC が `fastcc`/`air_kernel` (`METAL_CODEGEN_IMPL_MAP.md`) vs デフォルト C CC (`IR_GROUND_TRUTH.md`, golden) | **デフォルト C CC** | golden `P01/P02` の `.ll` 逐語 + 701 モジュール全走査 |
| 2 | `-std` が `air64_vNN` を決める (`legacy_metal_support_map.csv`) vs deployment OS が決める | **deployment OS** | driver `-###` 実測を本解析で全走査 (§1.8) |
| 3 | as4/as9 = ray_data/object_data (`METAL_TARGETINFO_IMPL_MAP.md`) vs imageblock=4 / mesh=7 / intersection_result=9 (golden) | **golden 実測** | `type_layout_map.csv` + golden `addrspace(N)` 分布 |
| 4 | opaque 型 35 個 (`type_layout_map.csv`) vs 37 個 (AST) | **37 個** (AST が全量。golden に出現しない 2 個も型としては存在) | `ast-text` の implicit TypedefDecl |
| 5 | 属性 30 (AST 実測) vs 88〜90 (spec PDF / help) | **両方**: AST クラスは 30 種だが spelling は 90。複数 spelling が 1 クラスに集約される (`[[threadgroup(N)]]`→`MetalBufferIndexAttr` 等) | `attr-class-topology.csv` + `clang_frontend_impl_map.csv` |

---

## 3. 未確定 (OPEN) — 推測で埋めない

- AS 6, AS 8 の意味 (corpus 未出現)
- mesh/object/tile エントリ metadata の完全スキーマ (`INFO_SET.md` A-2 の残件)
- function_constant の IR 表現詳細 (`INFO_SET.md` A-6 = ❌未調査)
- metallib コンテナの一般シェーダ向けタグ全量 (`INFO_SET.md` C-2 = 🌐🔶)
- `.metallibsym` (C-5 = ❌ 未入手)
- fat スライス cpu subtype ↔ GPU family (S0-3 = 🔶)

これらに依存する実装は、確定部分のみ実装し、未確定部分は
「情報源に無い」ことを明示して据え置く (独自仕様を作らない)。
