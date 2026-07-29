# Metal 対応 — 設計 (Phase 2: 設計)

前提: `docs-metal/01-ANALYSIS.md` の確定事実。
方針: Apple 純正 StdLib / Runtime はそのまま利用する。再実装しない。
本 fork は **`.metal → Clang → LLVM IR(AIR) → metallib`** を自前で完結させる。

ベース: LLVM/Clang 16.0.6 (本リポジトリ)。
情報源は「Apple の Metal 用 LLVM は 15 系ベースの可能性が高い」とするが、
決定的な要素は bitcode 互換性であり、実測で upstream で読める (§1.1) ため 16 で問題ない。
特に **typed pointer (`-no-opaque-pointers`) が LLVM 16 でまだ選択可能**であり、
Apple 実測出力 (typed pointer) と揃えられる点で有利。

---

## A. 全体アーキテクチャ

```
                       .metal
                         │
          ┌──────────────▼──────────────┐
          │ Driver (metal 互換動作)      │   ToolChains/Metal.cpp (新規)
          │  -x metal / -std=metalX.Y    │   Driver/Types.cpp: TY_Metal
          │  → triple air{32,64}_vNN-... │   Driver/ToolChains/Darwin.cpp 連携
          └──────────────┬──────────────┘
                         │ -cc1
          ┌──────────────▼──────────────┐
          │ Clang Frontend               │
          │  LangOptions::Metal          │  Basic/LangOptions.def
          │  Lexer: 13 keyword           │  Basic/TokenKinds.def (KEYMETAL)
          │  Types: 37 opaque BuiltinType│  Basic/MetalTypes.def (新規)
          │  Attrs: 90 spelling/30 class │  Basic/Attr.td
          │  Sema: MSL 制約              │  Sema/SemaMetal.cpp (新規)
          │  Builtins: 686 __metal_*     │  Basic/BuiltinsMetal.def (新規)
          │  Preproc: __METAL_* 定義群   │  Basic/Targets/AIR.cpp
          │  Mangling: U9MTLdevice 等    │  AST/ItaniumMangle.cpp
          └──────────────┬──────────────┘
                         │
          ┌──────────────▼──────────────┐
          │ CodeGen                      │
          │  air.* intrinsic 発行        │  CodeGen/CGBuiltin.cpp
          │  opaque struct lowering      │  CodeGen/CodeGenTypes.cpp
          │  !air.* metadata 発行        │  CodeGen/CGMetal.cpp (新規)
          └──────────────┬──────────────┘
                         │  LLVM IR = AIR
          ┌──────────────▼──────────────┐
          │ LLVM                         │
          │  Triple: air32 / air64       │  TargetParser/Triple.{h,cpp}
          │  Bitcode Writer (既存で可)    │  — 変更不要 (§1.1)
          └──────────────┬──────────────┘
                         │  .air (bitcode)
          ┌──────────────▼──────────────┐
          │ metallib writer              │  Object/MetallibWriter (新規)
          └──────────────────────────────┘
                       .metallib
```

---

## B. LLVM 側設計

### B-1. Triple

`llvm::Triple::ArchType` に `air32`, `air64` を追加。
- 綴り: `air32`, `air64`, および **バージョン付き** `air64_v20` … `air64_v28`,
  旧式 `air64_v18` / `air64_v111`。
- `parseArch` はプレフィックス一致 (`air64` で始まれば `air64`) とし、
  `_vNN` 部はサブアーキ文字列として保持 (実測の triple 文字列をそのまま再現するため)。
- `getArchPointerBitWidth`: air32→32, air64→64。
- OS は既存の `MacOSX / IOS / TvOS / WatchOS / XROS` と `-macabi` / `-simulator` 環境をそのまま使う
  (`datasets/os_triple_map.csv` の全形が既存 Triple 機構で表現できることを確認済)。

### B-2. DataLayout

`clang/lib/Basic/Targets/AIR.h` の `resetDataLayout` で §1.3 の文字列を設定。
LLVM 側に Target を登録する必要はない (**バックエンドは実装しない**、
情報源も「Apple GPU 命令セット実装は目的ではない」と明記)。
ただし `unable to create target` 相当を避けるため、
`-emit-llvm` / `-emit-llvm-bc` 経路のみを通す構成にする。

### B-3. Bitcode

変更不要 (§1.1)。`-no-opaque-pointers` で typed pointer 出力し実測と揃える。

---

## C. Clang Frontend 設計 (最優先)

### C-1. 言語モード

- `Language::Metal` を `LangStandard.h` に追加、`LangFeatures::Metal = (1<<16)`。
- `LangStandards.def` に **実測で存在が確認できる綴りのみ**登録:
  ```
  macos-metal1.0/1.1/1.2, macos-metal2.0..2.4      (C++11 / C++11 / C++11 / C++14 x5)
  osx-metal1.0/1.1/1.2, osx-metal2.0..2.4          (別名。cc1 実測に osx-metal1.2 あり)
  ios-metal1.0..2.4                                 (同構造)
  metal3.0, metal3.1, metal3.2                      (C++14, prefixless)
  metal4.0, metal4.1                                (C++17)
  ```
- `LangOptions`:
  - `LANGOPT(Metal, 1, 0, "Metal")`
  - `ENUM_LANGOPT(MetalVersion, MetalLangStd, 16, Metal_Unset, "Metal Version")`
    値は `__METAL_VERSION__` と同じ整数 (100,110,120,200,…,320,400,410)。
- C++ ベースの設定 (§1.9 が根拠):
  - `__METAL_VERSION__ < 200` → C++11
  - `200 <= v < 400` → C++14
  - `v >= 400` → C++17
- backport (§1.9 FEAT-07〜09): C++11/14 モードでも
  `if constexpr` / 構造化束縛 / fold 式を許可するフラグを立てる
  (Sema/Parser の該当箇所で `LangOpts.Metal` を or 条件に加える)。
- 恒久禁止 (FEAT-10〜14): exceptions=0, RTTI=0, `new/delete` 禁止, `virtual` 禁止,
  `thread_local` 禁止 (診断文言は §1.14)。

### C-2. Lexer / キーワード

`TokenKinds.def` に `KEYMETAL` 修飾で 13 個 (`clang_frontend_impl_map.csv` FE-0001〜0013):

| キーワード | 分類 |
|---|---|
| `device` `constant` `threadgroup` `thread` `ray_data` `object_data` `threadgroup_imageblock` | address space |
| `coherent` | 修飾子 (`coherent(device)` の形。引数を取る) |
| `kernel` `vertex` `fragment` `visible` `stitchable` | function class |

`constant` は C++ で識別子として使われうるため、Metal モードでのみキーワード化する
(`KEYMETAL` = `LangOpts.Metal` のときだけ有効)。

### C-3. アドレス空間

`LangAS` に追加:
```
metal_device, metal_constant, metal_threadgroup, metal_thread,
metal_threadgroup_imageblock, metal_ray_data, metal_object_data
```
`AIRAddrSpaceMap` (§1.4 実測):
```
Default(thread)               -> 0
metal_device                  -> 1
metal_constant                -> 2
metal_threadgroup             -> 3
metal_threadgroup_imageblock  -> 4
metal_object_data             -> 7
metal_ray_data                -> 9
metal_thread                  -> 0
```
`UseAddrSpaceMapMangling` は使わず、**専用の Metal mangling** (C-7) を使う。

### C-4. 型システム

`clang/include/clang/Basic/MetalTypes.def` (新規) に §1.11 の 37 型を **その順序で** 定義。
各エントリ: `METAL_TYPE(Name, Id, IRStructName)`
例: `METAL_TYPE(__metal_texture_2d_t, MetalTexture2d, "_texture_2d_t")`

- `BuiltinType::Kind` に 37 個追加
- `ASTContext::InitBuiltinTypes` で `LangOpts.Metal` のとき implicit TypedefDecl を注入
  (AST dump が実測と一致するようにする)
- CodeGen: `%struct.<IRStructName> = type opaque` を生成
- `__attribute__((__packed_vector_type__(N)))` を新規実装 (§1.10)
  — `ext_vector_type` と同様だが align = 要素 align (packed)。
  datalayout の `v24:32:32` / `v48:64:64` はこの packed 3 要素を裏付ける。

### C-5. 属性

`Attr.td` に **30 クラス**を追加し、**90 spelling** をそこへ割り当てる。
- spelling は `CXX11<"", "buffer">` 形式 (MSL は名前空間なしの `[[buffer(0)]]`)
- 引数あり: buffer/texture/sampler/threadgroup/color/attribute/id/function_constant/
  user/raster_order_group/max_total_threads_per_threadgroup/… (`IntegerLiteral` 等)
- 版ゲート: `Attr.td` に `MetalMinVersion` フィールドを持たせ、
  `SemaDeclAttr` で §1.12 の表に基づき
  `err_metal_attribute_requires_std` を出す。
- 上限検査 (`SemaDeclAttr::CheckMetalResourceIndexBounds`):
  buffer/constant/threadgroup ≤ 30 (max 31), texture ≤ 127 (max 128), sampler ≤ 15 (max 16)
  — 値は module flags 実測 (§1.7) と一致させる。

### C-6. Sema

`clang/lib/Sema/SemaMetal.cpp` (新規)。`meta/sema-rule-catalog.csv` の 200 シナリオを
カテゴリ単位で実装:
- **address_space**: 非 generic 間の暗黙変換禁止、`constant`→`thread` 代入禁止、
  ポインタは明示アドレス空間必須 (`pointer type must have explicit address space qualifier`)、
  program scope 変数は `constant` 必須
- **entry constraints**: kernel は `void` 返却、vertex の返却型検査、
  fragment 返却型検査、エントリ引数の属性必須
- **attribute**: stage 不整合 (`[[vertex_id]]` を fragment で使う等)、重複 index、
  型不一致 (`type 'X' is not valid for attribute 'Y'`)
- **restriction**: virtual / 例外 / RTTI / new・delete / thread_local の拒否

### C-7. マングリング

`ItaniumMangle.cpp` の `mangleQualifiers` に Metal 分岐を追加 (§1.5 実測):
```
metal_device                 -> "U9MTLdevice"
metal_constant               -> "U11MTLconstant"
metal_threadgroup            -> "U14MTLthreadgroup"
metal_ray_data               -> "U10MTLraydata"
metal_object_data            -> "U13MTLobjectdata"
metal_threadgroup_imageblock -> "U24MTLthreadgroupimageblock"
coherent                     -> "U18MTLcoherent..."   (詳細は要追加実測: OPEN)
```
`thread`(=default) は修飾子を出さない (実測 `_Z11read_threadP10AddressBox`)。

### C-8. Builtin

`clang/include/clang/Basic/BuiltinsMetal.def` (新規)。
`datasets/builtin_to_air_map.v2.csv` (686 行, confirmed 641) を機械変換して生成する。
各行: builtin 名 / 型文字列 / 属性 / **対応 air intrinsic 名**。

CodeGen (`CGBuiltin.cpp`) は `__metal_X` を対応する `air.Y` の `declare` 呼出に落とす。
命名の実測則 (`IR_GROUND_TRUTH.md` §6.4/6.5):
- FP 系は fast-math 有効時 `air.fast_*` 接頭 (`air.fast_fmax3.f32`)
- 整数系は `.s` / `.u` 接尾
- 型接尾は LLVM 風 (`.f32`, `.v4f32`, `.i32`, `.v4i64`, `.p1i8`)
- 一部はドットでなくアンダースコア連結 (`air.abs_diff`, `air.add_sat`, `air.mad_hi`)
- **air op が存在しない** builtin もある (§6.9): divide/select は native 命令、
  get_sampler は module 定数化 (`@__air_sampler_state`) — 表の `status` を尊重する

### C-9. プリプロセッサ

`AIRTargetInfo::getTargetDefines` で §1.13 の Metal 固有マクロを出す。
**`__HAVE_*` は出さない** (純正 `metal_config` が `__METAL_VERSION__` から導出する)。

- `__METAL__ 1`
- `__METAL_VERSION__` = std 由来 (100/110/120/200/210/220/230/240/300/310/320/400/410)
- `__METAL_MACOS__` / `__METAL_IOS__` / `__METAL_TVOS__` / `__METAL_WATCHOS__` / `__METAL_XROS__`
  = triple の OS 由来
- `__AIR64__` (air64) / `__AIR32__` (air32)
- `__AIR_VERSION__` = AIR version × 10000 + minor×100 (実測 `20800` = 2.8.0)
- `__AIR_ABI__` / `__AIR_PB_ABI__ 0` / `__AIR_MB_ABI__ 1` / `__AIR_VB_ABI__ 2`
- 数学モードと enum 値マクロ群 (実測 688 行から Metal 固有分を抽出したリストを使用)

### C-10. Pragma

- `#pragma METAL internals : enable|disable` — 内部 API の可視化トグル
- `#pragma METAL fp math_mode(safe|fast|precise)` — FP モードのスコープ制御

いずれも `Parse/ParsePragma.cpp` にハンドラを追加。純正ヘッダが使うため**必須**。

### C-11. CodeGen — metadata

`clang/lib/CodeGen/CGMetal.cpp` (新規)。§1.6 / §1.7 の逐語スキーマを生成:
- module: `!air.version`, `!air.language_version`, `!air.compile_options`,
  `!air.source_file_name`, `!llvm.ident`, module flags 6 種 + SDK Version
- entry: `!air.kernel` / `!air.vertex` / `!air.fragment` (+ `!air.object`,
  `!air.visible`, `!air.sampler_states`, `!air.function_constants` を版に応じて)
- 引数 operand の生成規則を §1.6 の通りに実装
  (buffer は address_space 有 / texture・sampler は無 / builtin は location_index 無 /
   constant 参照は buffer_size + struct_type_info / vertex output は `generated(...)`)
- `air-buffer-no-alias` 文字列属性、`air-alias-scope-*` の alias.scope metadata

版依存 (`air-metadata-version-changes.csv`):
`air.sampler_states` は metal2+、`air.object`/`air.visible`/
`air.visible_function_references` は metal3+ でのみ発行。

### C-12. Driver

`clang/lib/Driver/ToolChains/Metal.{h,cpp}` (新規) + `Types.def` に `TY_Metal`。
- `.metal` 拡張子 → `TY_Metal`
- `-std=` から `__METAL_VERSION__` を決定
- **triple の `_vNN` は deployment OS から決定** (§1.8 の表を実装)
- cc1 引数は実測 (§1.15) に合わせる: `-finclude-default-header`,
  fast-math 一式, `-fmetal-math-fp32-functions=fast`, `-no-opaque-pointers`,
  `-fmodules -fmodule-map-file=.../metal/module.modulemap`
- リンク段: `.air` → `.metallib`

Metal 固有フラグ (実測 `metal-help.txt`):
`-fmetal-enable-logging`, `-fmetal-math-fp32-functions={fast,precise}`,
`-fmetal-math-mode=<value>`, `-fmodules={all,stdlib,none}`

### C-13. Diagnostics

`DiagnosticSemaKinds.td` / `DiagnosticDriverKinds.td` に §1.14 の実測文言をそのまま定義。
警告グループ `-Wmtl-shader-return-type` (実測で `-Werror` 化されている) も追加。

---

## D. metallib writer 設計

`spec/METALLIB_WRITER_SPEC.md` + `datasets/metallib_structure.csv` (実サンプル 10 本) が根拠。
確定しているタグ: `NAME` `TYPE` `HASH` `VERS` `MDSZ` `ENDT` `UUID` (+ `OFFT` `HDYN` `AIRR`)。
fat ラッパ: magic `cb fe ba be`, 20 バイト slice エントリ, cputype `0x01000017`。

**ただし** `INFO_SET.md` C-2 は「現サンプルは tracepoint ライブラリのみ、
通常シェーダのエントリ系タグは未採取 (🧪)」と明記する。
→ 通常シェーダ向けタグの一部は **OPEN**。golden `P01/P02` の `.metallib` 実物が
リポジトリにあるので、これを一次資料としてパース・照合してから実装する。

---

## E. 実装順序 (依存関係順)

指示に従い **Clang Frontend が十分な完成度に達するまで Backend に進まない**。

```
S1  LLVM Triple (air32/air64)                    ← 全ての前提
S2  Clang: LangOptions / LangStandards / Language::Metal
S3  Clang: AIRTargetInfo (datalayout, addrspace map, マクロ)
S4  Clang: Lexer キーワード 13
S5  Clang: LangAS + アドレス空間 Parser/Sema
S6  Clang: 37 opaque BuiltinType + packed_vector_type
S7  Clang: Attr.td 30 クラス / 90 spelling + 版ゲート + 上限検査
S8  Clang: Sema 制約 (address space / entry / restriction)
S9  Clang: mangling (U9MTLdevice 等)
S10 Clang: pragma METAL internals / fp math_mode
S11 Clang: 686 builtin 定義
S12 Clang: Diagnostics 実測文言
S13 Clang: Driver (TY_Metal, triple 決定, cc1 引数)
--- ここまでで Frontend が形になる ---
S14 CodeGen: opaque 型 lowering
S15 CodeGen: builtin → air.* intrinsic
S16 CodeGen: !air.* metadata (module + entry)
S17 検証: golden P01/P02 との .ll 差分
S18 metallib writer
```

各段で `research/golden` および `reference/metal-ast-*` と突合して検証する。
