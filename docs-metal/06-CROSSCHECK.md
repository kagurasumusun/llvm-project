# Metal 対応 — 全情報源の突き合わせ (Phase 7)

実施日: 2026-07-29
導入ツール: llvmlite 0.48 (LLVM 22 実体を同梱) / pypdf 6.14

対象: metal-info 全体（バイナリ 7,168 / gz 圧縮資料 / 公式 PDF 11 版 2,690 頁 /
純正 StdLib 81,473 行）。

---

## 1. 「最新 metal コンパイラに opaque の隠しモードがあるのでは」 → 否定

### 1-1. まず判明した方法論上の落とし穴

導入した LLVM 22 で Apple の `.air` を読ませると、逆アセンブル結果は
`ptr addrspace(1)` と **opaque pointer 表記になる**。

```
define void @probe_p01_kernel(ptr addrspace(1) noundef captures(none) ...)
```

これは LLVM 15 以降が **読み込み時に typed → opaque へ自動アップグレード**する
ためで、ファイルの中身とは無関係である。
**テキスト出力で typed/opaque を判定してはならない。**
ディスク上のビットを直接読む必要がある（Phase 6 の自作パーサが正しい道具だった）。

### 1-2. 17,070 行 × 18 ターゲットの実測データ

`meta/pointer-mode-info.csv.gz`（今回展開）は Apple 出力の pointer_mode を
fixture 単位で記録している。全ターゲット集計:

| pointer_mode | 件数 |
|---|---:|
| typed | **23,538** |
| opaque | **0** |
| unknown（ポインタを含まない fixture） | 118,000 超 |

macOS / iOS / tvOS / watchOS とその simulator、air32 / air64 の
**全 18 ターゲットで opaque は 1 件も無い**。

### 1-3. 隠しモードが存在し得ないことの構造的証明

Apple の bitcode が使用する **enum attribute kind の最大値は 71**
(`ATTR_KIND_NO_CALLBACK`、LLVM 13 導入)。使用属性は
`ARGMEMONLY`(45) / `WRITEONLY`(52) / `READONLY` といった **LLVM 15 以前の旧形式**で、
LLVM 16 がこれらを置き換えた `ATTR_KIND_MEMORY`(86) は**一度も現れない**。

opaque pointer (`TYPE_CODE_OPAQUE_POINTER` = 25) は **LLVM 15 で追加**された機能。
Apple の AIR bitcode は機能的に LLVM 13〜15 世代に留まっており、
かつ cc1 起動では **3,654 回すべてで `-no-opaque-pointers` を明示**している。

**結論**: 隠し opaque モードは存在しない。仮説は否定される。
ただし `-no-opaque-pointers` を明示的に渡している事実は、
Apple の基盤 clang 自体には opaque 能力があり、AIR 出力では
**意図的に無効化している**ことを示す。これは AIR のバイナリ互換性
（既存 metallib / rtlib が typed 前提）を保つための選択と解釈できる。

---

## 2. Apple の AIR と本実装の一致度

`docs-metal/verify/conformance.py` で静的に検証可能な観測項目を機械照合した。

**35 / 39 項目 (89.7%) を再現。**

| 分類 | 一致 | 内容 |
|---|---|---|
| module | 14/14 | datalayout、リソース上限 6 種、named metadata 7 種 |
| addrspace | 7/7 | 0/1/2/3/4/7/9 すべて |
| types | 1/1 | opaque 型 37 種と順序 |
| builtins | 2/2 | 純正 stdlib が要求する 650 種、AIR 名 650 件 |
| attributes | 1/1 | 仕様書由来 88 種 |
| entry metadata | 10/10 | buffer / texture / sampler / stage builtin の operand キー列 |
| **既知欠陥** | **0/4** | 下記 |

不一致 4 件（Phase 6 で検出済み・未修正）:
1. `air.arg_type_name` が pointee 型でなく全体型、かつ MSL 表記でない
2. `air.compile.framebuffer_fetch_*` の分岐条件
3. watchOS の AIR 版マッピング
4. エントリ関数の非マングル（未実装）

**この 89.7% は「静的照合できる範囲」の値であり、実際の出力一致率ではない。**
ビルドできていない以上、真の一致度は測定不能である。

---

## 3. 公式 PDF との突き合わせ（11 版 2,690 頁を全文抽出）

### 3-1. C++ ベース: 公式と実測が一致（1 版を除く）

| MSL | 公式 PDF の記述 | 実測 `__cplusplus` | 判定 |
|---|---|---|---|
| 2.0〜3.2 | 「C++14 Specification」 | 201402L | 一致 |
| 4.0 / 4.1 | 「C++17-based specification」 | 201703L | 一致 |
| **1.2** | **「Metal and C++14」** | **201103L (C++11)** | **矛盾** |

MSL 1.2 の PDF は目次に「1.4 Metal and C++14」と書くが、
実際のコンパイラは C++11 を名乗る。**実測を採用すべき事例**であり、
本実装は実測どおり C++11 としている（正しい）。

### 3-2. 公式が列挙する「Metal で使えない C++17 機能」12 項目

MSL 4.1 §1.6.1 より逐語:

```
• lambda expressions (section 5.1.2) prior to Metal 3.2
• dynamic_cast operator (section 5.2.7)
• type identification (section 5.2.8)
• new and delete operators (5.3.4, 5.3.5). Metal 4.1 and later supports placement new.
• noexcept operator (section 5.3.7)
• goto statement (section 6.6)
• register, thread_local storage attributes (section 7.1.1)
• virtual function attribute (section 7.1.2)
• derived classes (section 10, 11)
• exception handling (section 15)
```

**本実装がカバーしているのは `thread_local` のみ。残り 11 項目は未実装。**

### 3-3. 新発見: lambda は Metal 3.2 以降でのみ許可

公式が「prior to Metal 3.2」と明記。三者で裏付けた:

| 検証源 | 結果 |
|---|---|
| 公式 PDF 4.0 / 4.1 | 「prior to Metal 3.2」と明記 |
| 実機診断ログ | metal2.4 / 3.0 / 3.1 で `error: lambda expressions are not supported in Metal` |
| 実機 AST | `LambdaExpr` は metal3.2 / 4.0 でのみ出現、それ以前は 0 |

本実装は lambda を無条件許可しており誤り。
なお `METAL_CXX_MASTER_ATLAS.md` は lambda を「全版で Active」と書いており、
**要約文書が公式・実測の双方と食い違う**例である。

### 3-4. 新発見: packed vector の正確なサイズ / アライン

公式 PDF に完全な表がある（本実装は `__packed_vector_type__` 自体が未実装）:

| 型 | size | align | | 型 | size | align |
|---|---:|---:|---|---|---:|---:|
| packed_uchar2/3/4 | 2/3/4 | 1 | | packed_half2/3/4 | 4/6/8 | 2 |
| packed_ushort2/3/4 | 4/6/8 | 2 | | packed_bfloat2/3/4 | 4/6/8 | 2 |
| packed_uint2/3/4 | 8/12/16 | 4 | | packed_float2/3/4 | 8/12/16 | 4 |
| packed_long2/3/4 | 16/24/32 | 8 | | | | |

規則: **align = 要素型のサイズ**（`ext_vector_type` は N を 2 の冪に切り上げ）。
`packed_float3` は size 12 / align 4（`float3` は size 16 / align 16）。
datalayout の `v24:32:32` / `v48:64:64` はこの packed 3 要素配置の裏付け。

---

## 4. 純正 StdLib の徹底読み込み（68 ファイル 81,473 行）

### 4-1. 致命的な未実装を発見: メンバ関数の末尾アドレス空間修飾

MSL は C++ に無い構文を持つ。仮引数リストの後ろにアドレス空間を書く:

```cpp
METAL_FUNC constexpr sampler(const device coherent(device) sampler &) thread = default;
METAL_FUNC thread sampler &operator=(const device coherent(device) sampler &) thread = default;
```

純正ヘッダでの出現数:

| 修飾子 | 出現数 | | 修飾子 | 出現数 |
|---|---:|---|---|---:|
| `) ... thread` | 2,370 | | `) ... ray_data` | 1,285 |
| `) ... device` | 1,931 | | `) ... object_data` | 929 |
| `) ... constant` | 1,001 | | `) ... threadgroup` | 93 |
| | | | `) ... threadgroup_imageblock` | 59 |

**合計 7,668 箇所**。AST 実測でも `void () thread` / `void () const constant` /
`void (device Uniforms &&) thread` という QualType として現れる
（`meta/builtin-signatures.csv.gz`）。

**これが無いと純正 StdLib は一行もパースできない。**
本実装は `ParseMetalQualifiers` を DeclSpec と型修飾子リストにしか繋いでおらず、
メンバ関数の ref-qualifier 位置には未対応。**最優先の欠落**。

### 4-2. `coherent(device)` は 3,121 箇所

引数形は `coherent(device)` の一形のみ（`coherent(threadgroup)` は
ヘッダ内には存在しない）。本実装の Parser は括弧付き引数に対応済みで正しい。

### 4-3. StdLib が要求するコンパイラ機能の全量

| 機能 | 出現 | 実装状況 |
|---|---:|---|
| `__ext_vector_type__` | 84 | upstream 既存 |
| `__packed_vector_type__` | 82 | **未実装（Apple 独自拡張）** |
| `#pragma METAL fp math_mode(safe)` | 90 | 実装済 |
| `#pragma METAL internals : enable/disable` | 104 | 実装済 |
| メンバ関数 AS 修飾 | 7,668 | **未実装** |
| `__attribute__((format(metal_os_log, …)))` | 1 | **未実装（format 種別の追加が必要）** |
| `__attribute__((__maybe_undef__))` | 1 | upstream 既存 |
| `enable_if` / `pure` / `const` / `nothrow` 等 | 各 1 | upstream 既存 |

`kernel` / `vertex` / `fragment` / `stitchable` は StdLib ヘッダ内では
**一度も使われない**（ユーザーコード専用）。`visible` は 1 回のみ。

---

## 5. gz 圧縮資料の展開で得た新事実

### 5-1. `sema-metal-rules.csv.gz`（7,024 行の実測診断）

Metal 固有の error メッセージ **28 種**を確定。本実装は 18 種なので
**10 種が未カバー**。新たに判明した文言:

```
Metal does not support the 'X' storage class specifier   (register 用)
variables in function scope cannot be declared static
C-style cast from 'X' to 'X' converts between mismatching address spaces
invalid prototype; variadic arguments are not allowed in Metal qualified functions
kernel function cannot be a member function
parameter may not be qualified with an address space
invalid type 'X' for input declaration in a kernel function
'X' attribute only applies to fragment functions
zero-length arrays are not permitted in C++
invalid return type 'X' (vector of N 'X' values) for kernel function
```

### 5-2. `ir-function-attr-groups.csv.gz`（関数属性の実測）

エントリ関数に付く keyword 属性の典型パターン:

```
argmemonly;mustprogress;nofree;norecurse;nosync;nounwind;willreturn;writeonly
mustprogress;nofree;nosync;nounwind;readnone;willreturn
argmemonly;nounwind;readonly;willreturn
```

文字列属性は fast-math 系 8 種が 18,666 件で一定
（`approx-func-fp-math` `unsafe-fp-math` `no-infs-fp-math` `no-nans-fp-math`
`no-signed-zeros-fp-math` `no-trapping-math` `min-legal-vector-width`
`frame-pointer` `stack-protector-buffer-size`）。

注: golden P01 には `no-builtins` があるが、この CSV の集計では 0 件。
採取方法の差と思われ、**要確認**。

### 5-3. `builtin-signatures.csv.gz`

AST が持つ関数シグネチャの全量。ここから
メンバ関数 AS 修飾（§4-1）が QualType に現れることを確認した。
builtin の**正確なプロトタイプ**を将来ここから導出できる可能性がある
（`INFO_SET.md` が F-4 で OPEN としている項目）。

---

## 6. 偽物データの追加証拠

Phase 6 で特定した `test/*.bc` について、LLVM 22 での逆アセンブルにより
**決定的な追加証拠**が得られた:

```
define spir_kernel void @add_arrays(...)
declare ... @__clang_ocl_kern_imp_add_arrays(...)
```

- **`spir_kernel` calling convention** — 純正 AIR には CC=0 (C) しか存在せず、
  純正 `.ll` 全体で `spir_kernel` の出現は **0 件**
- **`__clang_ocl_kern_imp_` 接頭辞** — upstream Clang の **OpenCL** カーネル
  生成が付けるもの

`test/*.bc` は Metal ではなく **OpenCL 経路でコンパイルされた別物**である。
判定は確定。

---

## 7. 全体の突き合わせ結果

| 情報源 | 相互整合 | 備考 |
|---|---|---|
| 公式 PDF ↔ 実測マクロ | 一致（1.2 を除く） | 1.2 は PDF が C++14、実測は C++11 |
| 公式 PDF ↔ 実測診断（lambda） | 完全一致 | 3.2 境界を三者で確認 |
| 要約文書 ↔ 公式 PDF（lambda） | **不一致** | ATLAS は「全版 Active」と誤記 |
| 要約文書 ↔ 実測（CC） | 不一致 | CODEGEN_IMPL_MAP の fastcc 説は誤り（既知） |
| 要約文書 ↔ 実測（AIR 版） | 不一致 | legacy_metal_support_map の std 依存説は誤り（既知） |
| バイナリ ↔ `.ll` テキスト | 完全一致 | golden は忠実な逆アセンブル |
| gz 資料 ↔ バイナリ | 完全一致 | pointer_mode / attr group とも |
| StdLib ↔ builtin 対応表 | 完全一致 | 686 名すべて説明可能 |

**metal-info の一次データ（バイナリ・AST・gz）は極めて信頼できる。**
誤りは一貫して**人間が書いた要約文書**の側にあり、
今回も 1 件（lambda）を新たに検出した。

---

## 8. 本実装の未実装項目（優先度順）

| # | 項目 | 根拠 | 影響 |
|---|---|---|---|
| 1 | **メンバ関数の末尾 AS 修飾** | StdLib 7,668 箇所 | **純正 StdLib が全く読めない。最優先** |
| 2 | `__packed_vector_type__` | StdLib 82 箇所、公式にサイズ表 | packed 型が使えない |
| 3 | エントリ関数の非マングル | 純正 400/400 | リンク不能 |
| 4 | 禁止 C++ 機能 11 項目 | 公式 §1.6.1 | 不正コードを受理 |
| 5 | lambda の 3.2 未満での禁止 | 公式＋実測＋AST | 版間の互換性 |
| 6 | 未カバー診断 10 種 | `sema-metal-rules.csv.gz` | Apple 互換性 |
| 7 | `arg_type_name` の pointee + MSL 表記 | golden | metadata 不正 |
| 8 | `framebuffer_fetch` の分岐 | 実測 | metadata 不正 |
| 9 | watchOS AIR 版 | 実測 | triple 不正 |
| 10 | `format(metal_os_log)` | StdLib 1 箇所 | os_log が使えない |

これらは**すべて未修正**である。作業指示があれば優先度 1 から着手する。
