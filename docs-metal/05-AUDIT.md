# Metal 対応 — 情報源の独立監査とバイナリ実証 (Phase 6)

実施日: 2026-07-29
動機: 「AIR は typed pointer bitcode ベースではないのか」「AIR 情報に偽物データが
紛れ込んでいるかもしれない」という指摘。

## 0. 監査方法

これまでの実装は metal-info の**要約文書**（`spec/*.md`）と**機械生成 CSV** に
依存していた。今回は依存を断ち、**バイナリ実物を直接パース**して事実を再確立した。

環境に LLVM ツール（`llvm-bcanalyzer` / `llvm-dis`）が無いため、
LLVM Bitstream 仕様に基づく**ビットストリームリーダーを自作**した
（`docs-metal/verify/bcread.py`）。これにより:

- Darwin bitcode wrapper (`de c0 17 0b`) の除去
- ブロック構造・abbrev 定義・char6 エンコードの復元
- TYPE / MODULE / PARAMATTR_GROUP / STRTAB ブロックの読み出し

が可能になり、**要約を介さず生バイトから**検証できるようになった。

検査対象: `.air` 5,134 / `.bc` 1,756 / `.metallib` 278。

---

## 1. 結論: AIR は typed pointer である（指摘は正しい）

`research/golden/P01/metal32_macosx26/probe.air` の TYPE ブロック実測:

```
NUMENTRY       [29]
POINTER        [1, 1]      <- TYPE_CODE_POINTER=8, [pointee_type_id, addrspace]
STRUCT_NAME    ['struct._texture_2d_t']
OPAQUE         [0]
POINTER        [9, 1]      <- texture handle を device(1) で指す
STRUCT_NAME    ['struct._sampler_t']
OPAQUE         [0]
POINTER        [11, 2]     <- sampler handle を constant(2) で指す
```

`TYPE_CODE_POINTER` (8) は **pointee 型 ID を持つ**。
`TYPE_CODE_OPAQUE_POINTER` (25) は**一つも存在しない**。

### 全コーパス走査

| 対象 | typed pointer を含む | opaque pointer を含む |
|---|---:|---:|
| 純正 `.air`/`.bc` 399 件（無作為抽出） | 399 | **0** |
| `test/add.bc` | 0 | **1** |

`libtracepoint_rt_osx.metallib` に埋め込まれた出荷ランタイムの bitcode も
`typed POINTER=42 / OPAQUE_POINTER=0` で typed pointer。

**したがって `-no-opaque-pointers` を採用した実装判断は正しい。**
ただし従来の根拠は cc1 起動引数の文字列のみで弱かった。
今回バイト列で直接立証した。

---

## 2. 非 metalfe 由来のデータを 1 組特定した

> **【Phase 8 で訂正】** 当初この 2 ファイルを「偽物／第三者実装の出力」と
> 判定したが、精査の結果これは行き過ぎだった。これらは Apple の `!air.*`
> メタデータスキーマを逐語で備えた**別実装による正当なテスト出力**である。
> 詳細と訂正後の判定は `07-OPAQUE-EVIDENCE.md` §3 を参照。
> 「Apple 仕様の根拠として使ってはならない」という結論のみ変わらない。


`test/add.bc` と `test/AAPLShaders.bc` は **Apple 純正ではない**。
以下の 5 点がすべて純正と食い違う。

| 観点 | 純正 (1,756 モジュール) | `test/*.bc` (2 件) |
|---|---|---|
| ポインタ表現 | typed pointer | **opaque pointer** |
| IDENTIFICATION ブロック | 無し | **`LLVM22.1.8`** |
| datalayout | `i128` を**含まない** | **`i128:128:128` を含む** |
| triple | `air64_v28-apple-macosx26.0.0` | **`air64-apple-ios26.2`**（`_vNN` 無し、パッチ版付き） |
| アドレス空間マングリング | `PU9MTLdevice` / `PU11MTLconstant` | **`PU3AS2`**（OpenCL 様式の数値 AS） |
| エントリ関数名 | `probe_p01_kernel`（マングルなし） | **`_Z12vertexShaderjPU3AS2...`（マングルあり）** |

`PU3AS2` 様式は純正コーパス全体で**出現数ゼロ**。
`_Z...` 形式のエントリ関数も純正 400 件中**ゼロ**。

**判定**: これらは upstream LLVM 22 系ベースの**第三者実装（おそらく本プロジェクトの
ような Metal 対応の試作）の出力**であり、Apple の仕様を示す資料ではない。
ファイル内の文字列 `clang/test/AppleMetalSamples/...` もテストスイートの成果物を示唆する。

**幸い、本実装はこの 2 ファイルを一切参照していない**（golden と reference のみ使用）。
混入は `test/` ディレクトリに限局しており、他の 1,756 モジュールは完全に一貫している。

### 情報源自身も過去に汚染を排除している

`spec/IR_GROUND_TRUTH.md` §2.3 に
「**msl_analysis の捏造 datalayout (`n32:64-S128` 等) は実測と不一致 → 廃棄が確定**」
という記録がある。実際に `msl_analysis` ディレクトリは現存せず、
捏造 datalayout もデータ側には残っていない（言及する文章のみ）。
情報源は汚染を認識し排除する運用がなされている。

---

## 3. AIR は素の LLVM Bitcode である（独立に確認）

152 モジュールを走査し、**未知のブロック ID・未知の TYPE コードは 0 件**。
`MODULE_CODE_VERSION` は全件 `2`（LLVM 3.9 以降の現行エピック）。

→ Bitcode Reader/Writer に Apple 独自拡張を実装する必要はない、という
設計前提が生バイトから裏付けられた。

---

## 4. metallib コンテナ構造を実物から解読

`libtracepoint_rt_osx.metallib` (34,564 B):

```
fat header (big-endian!):
  magic  cb fe ba be
  nfat   2
  slice0 cputype=0x01000017 subtype=0x7 off=48    size=17074
  slice1 cputype=0x01000017 subtype=0x9 off=17136 size=17428

MTLB slice (little-endian):
  +0   'MTLB'
  +4   version 32769, 2
  +16  file_size          = 17074 (スライス長と一致)
  +24  HEADERS off=88   len=2754   ("NAME" タグで始まる)
  +40  section2 off=2938 len=144    ("ENDT")
  +56  section3 off=3082 len=144    ("ENDT")
  +72  BITCODE  off=3226 len=13648  (de c0 17 0b = Darwin wrapper)
```

BITCODE 領域を切り出して自作パーサに通したところ **1,428 レコードを正常に解析**でき、
`air64_v23-apple-macosx11.0.0` / typed pointer であることを確認した。

**注意**: fat header は**ビッグエンディアン**、MTLB 内部は**リトルエンディアン**。
`METALLIB_WRITER_SPEC.md` はこの点を明示していない。writer 実装時の重要事項。

---

## 5. 自分の実装のバグを 3 件発見

監査の主目的は情報源の検証だったが、結果として**自分の実装誤り**が見つかった。

### バグ1: `air.arg_type_name` が pointee 型でなく全体型

純正 (golden P01、ソースは `device float* b`, `constant Params& p`):
```
air.arg_type_name = "float"      <- pointee
air.arg_type_name = "Params"     <- pointee
```
私の `CGMetal.cpp:446` は `Ty.getAsString(...)` で `device float *` を出す。**誤り。**

さらに型名は **MSL 表記**でなければならない。実測の出現頻度上位は
`float`(4220) `uint`(3726) `int`(1393) `sampler`(1283) `half`(570) `float4`(343)
`texture2d<float, sample>`(295) で、C++ の `vector<float,4>` 形式ではない。
MSL 型名プリンタが別途必要。

### バグ2: `air.compile.framebuffer_fetch_*` を常に enable にしていた

実測の分岐規則:

| プラットフォーム | 条件 | 値 |
|---|---|---|
| iOS / tvOS | 常時 | `framebuffer_fetch_enable` |
| macOS | `-std >= macos-metal2.3` | `framebuffer_fetch_enable` |
| macOS | `-std <= macos-metal2.2` | `framebuffer_fetch_disable` |

境界は macOS で 2.2/2.3 の間（各 1,283 / 1,262 件で明瞭）。
私の実装は常に enable なので、macOS の MSL 2.2 以前で誤る。

### バグ3: watchOS の AIR 版マッピングが誤り

実測:
```
watchOS 10.3 => v26    watchOS 11.4 => v27    watchOS 26.0 => v28
```
私の実装は `Major >= 10 ? 26 : ...` なので **watchOS 11.4 で v26 を返す（正解は v27）**。

### 全 OS の AIR 版対応表（実測・完全版）

| macOS | AIR | iOS / tvOS | AIR | watchOS | AIR |
|---|---|---|---|---|---|
| 10.13 | v20 | 10.3 | v111 | 10.3 | v26 |
| 10.14 | v21 | 11.4 | v20 | 11.4 | v27 |
| 10.15 | v22 | 12.5 | v21 | 26.0 | v28 |
| 11.x | v23 | 13.7 | v22 | | |
| 12.x | v24 | 14.8 | v23 | | |
| 13.x | v25 | 15.8 | v24 | | |
| 14.x | v26 | 16.7 | v25 | | |
| 15.x | v27 | 17.7 | v26 | | |
| 26.x | v28 | 18.7 | v27 | | |
| | | 26.0 | v28 | | |

規則: iOS/tvOS は `AIR = major + 9`（13 以降）、watchOS は `AIR = major + 16`。
macOS のみ独自系列。iOS の `v111` は 10.3 期の旧式表記。

私の実装は macOS と iOS/tvOS は正しく、**watchOS のみ誤り**。

---

## 6. 実装判断が正しかったことを再確認した項目

バイナリから独立に再検証し、いずれも従来の判断を支持した。

| 項目 | 検証方法 | 結果 |
|---|---|---|
| 特殊 calling convention は無い | `MODULE_CODE_FUNCTION` の callingconv フィールドを 408 関数分読み出し | **全件 0 (=C)**。実装は正しい |
| datalayout は一意 | 203 モジュールの `MODULE_CODE_DATALAYOUT` を比較 | **1 種類のみ**。実装値と一致 |
| air32 の datalayout | air32 バイナリを直接読み出し | `e-p:32:32:32-...` 実装値と一致 |
| `__HAVE_*` はコンパイラが定義しない | 全 11 版の `-dM` 実測を集計 | **全版 0 件**。実装は正しい |
| threadgroup 引数は `air.buffer`+as3 | golden P01 の operand を読み出し | 実装と一致 |
| golden の `.ll` は捏造でない | `.air` バイナリの文字列と `.ll` のトークンを突合 | 13/13 一致。`air-buffer-no-alias` は PARAMATTR_GROUP の char6 から復元して確認 |

---

## 7. 新たに判明した未実装仕様

### 7-1. エントリ関数はマングルしない（重要）

純正 400 件のエントリ関数すべてが**マングルされていない**。
同一モジュール内でヘルパーはマングルされる:

```
define float @_Z10helper_mulff(float, float)   <- ヘルパー: マングルあり
define i32   @visible_fn(i32)                  <- [[visible]]: マングルなし
define void  @k_using_add(...)                 <- kernel:     マングルなし
```

→ `[[kernel]]/[[vertex]]/[[fragment]]/[[visible]]/[[stitchable]]` が付いた関数は
**C リンケージ相当**として扱う必要がある。現実装は未対応。

### 7-2. `air.fast_*` の語彙は 27 種

`fast_` 形のみ存在するもの 24 種（acos, asin, atan, ceil, clamp, cos, cosh,
exp, exp2, floor, fmax, fmin, fract, log, log2 …）、
`fast_` と素形の両方が存在するもの 3 種（fabs, rsqrt, sqrt）。

→ fast-math 時に単純接頭辞を付けるのではなく、**op ごとに fast 形の有無が異なる**。
対応表の air 名をそのまま使うのが安全で、独自に `fast_` を合成してはならない。

---

## 8. 監査の総括

- **「typed pointer ではないのか」→ その通り。** バイト列で確定した。実装は正しかったが根拠を強化した。
- **「偽物データが紛れ込んでいるかも」→ その通り。** `test/*.bc` 2 件が別実装の出力と判明。ただし本実装は未参照で影響なし。
- 副産物として**自分の実装バグを 3 件**発見した（型名・framebuffer_fetch・watchOS）。
- **未実装の重要仕様を 2 件**発見した（エントリ関数の非マングル・fast 形の個別性）。

要約文書を信じずバイナリに当たるという方針は、実際に誤りを検出できた点で有効だった。
今後 `builtin_to_air_map.v2.csv` の `low` 45 件など、確度の低い行についても
同様のバイナリ照合を行うべきである。
