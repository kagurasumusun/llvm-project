# LLVM 公式 RFC / 参照実装との突き合わせ (Phase 9)

実施日: 2026-07-29
対象:
- RFC 本文 [discourse.llvm.org/t/rfc-add-an-apple-metal-air-backend-target/90936](https://discourse.llvm.org/t/rfc-add-an-apple-metal-air-backend-target/90936)（全文）
- 参照実装 `imperatormk/llvm-project` branch `metal-target-poc`
  （`llvm/lib/Target/AIR` 9,870 行 + `llvm/docs/AIR/*.rst`）

この実装は **本 fork と同じ問題を独立に解いた唯一の公開実装**であり、
かつ **実機 Apple GPU でロード・実行まで検証済み**という点で価値が高い。
本 fork が Clang フロントエンド側、RFC が LLVM バックエンド側という
相補的な関係にある。

---

## 1. metallib コンテナ仕様を実測で検証した（S18 の突破口）

RFC は `AIRLibFormat.rst` に完全なコンテナ仕様を公開している。
これを **Apple 純正 metallib の実バイトと突合**した。

### 1-1. タグ構造は完全に正しい

golden `P01/probe.metallib` の Section 0 を実際にパースした結果:

```
EntryCount=1
entry0: EntrySize=134
  NAME len=17  "probe_p01_kernel"
  TYPE len=1   02                       (2 = kernel)
  HASH len=32  46 9a 4a ba ... (SHA-256)
  MDSZ len=8   c0 10 00 00 00 00 00 00  (= 4288, bitcode section size)
  OFFT len=24  全ゼロ (単一エントリのため)
  VERS len=8   02 00 08 00 03 00 02 00  (air 2.8 / metal 3.2)
  ENDT
```

RFC が記す **タグ名・順序・ペイロード長・意味がすべて実測と一致**。
`VERS` が `u16{air_major, air_minor, metal_major, metal_minor}` である点も、
`02 00 08 00 03 00 02 00` = AIR 2.8 / Metal 3.2 で確認できた
（この metallib は `-std=metal3.2` でコンパイルされたもの）。

### 1-2. ただしヘッダのフィールド位置は RFC 文書が誤っている

RFC の `AIRLibFormat.rst` は FileSize を `+20`、セクション表を `+28` からと
記すが、**実測では FileSize が `+16`、セクション表が `+24` から**である。
8 バイトずれている。

Apple 純正 10 本すべてで検証:

| ファイル | slice 長 | `+16` の値 | 一致 |
|---|---:|---:|---|
| libtracepoint_rt_ios.metallib | 17027 | 17027 | ✅ |
| libtracepoint_rt_iosmac.metallib | 17093 | 17093 | ✅ |
| libtracepoint_rt_iossim.metallib | 17062 | 17062 | ✅ |
| libtracepoint_rt_osx.metallib | 17074 | 17074 | ✅ |
| libtracepoint_rt_tvos.metallib | 17028 | 17028 | ✅ |
| golden P01 | 4534 | 4534 | ✅ |
| golden P02 | 10519 | 10519 | ✅ |

`+20` は全件で `377957122048`（意味を成さない値）。

### 1-3. 実測に基づく MTLB ヘッダの正しいレイアウト（88 バイト）

```
 +0   4  Magic          "MTLB"
 +4   2  FormatMinor    0x8001
 +6   2  FormatMajor    0x0002
 +8   1  Reserved       0x09 (golden/macOS26) / 0x05 (rtlib/macOS11) ← 版で変わる
 +9   2  (zero)
 +11  1  Platform       0x81 = macOS (rtlib では 0x00)
 +12  1  OSMajor        26   (rtlib では 0)
 +13  3  pad
 +16  8  FileSize       ← RFC 文書は +20 としているが誤り
 +24  8  Sec0Offset     (常に 88 = ヘッダ長)
 +32  8  Sec0Size
 +40  8  Sec1Offset
 +48  8  Sec1Size
 +56  8  Sec2Offset
 +64  8  Sec2Size
 +72  8  Sec3Offset     (wrapped bitcode)
 +80  8  Sec3Size
```

`+8` / `+11` / `+12` が版によって異なる点は RFC も触れていない発見。
`Reserved` は AIR 版に連動している可能性が高い（0x05 ↔ AIR2.3、
0x09 ↔ AIR2.8）が、2 点のみの観測なので断定は避ける（**OPEN**）。

**含意**: 本 fork の S18（metallib writer）は、この実測レイアウトに基づけば
実装可能な段階に達した。RFC 文書をそのまま信じると壊れる。

---

## 2. RFC が実測で裏付けた「AIR 契約」の追加事項

### 2-1. `air.convert.*` — 本 fork に欠落していた層を発見

RFC 実装は `air.convert` を持つ。調べたところ、**純正 golden P02 にも実在**する:

```
%6 = tail call fast float @air.convert.f.f32.u.i32(i32 %5)
```

純正コーパス全体を走査すると **57 種**の変種がある:

```
air.convert.f.f32.u.i32     uint  -> float
air.convert.f.f16.s.i16     short -> half
air.convert.f.v4f32.f.v4f16 half4 -> float4
air.convert.f.v3f16.u.v3i32 uint3 -> half3
...
```

命名規則: `air.convert.<dst種別>.<dst型>.<src種別>.<src型>`
（種別は `f`=float / `s`=signed int / `u`=unsigned int）。

**これは `__metal_*` builtin 経由ではない。** 通常の C++ の型変換
（`float(vid)` のような暗黙・明示キャスト）が CodeGen でこの intrinsic に
落ちる。したがって `builtin_to_air_map.v2.csv`（686 builtin）にも
本 fork の `BuiltinsMetal.def` にも存在せず、**設計から漏れていた**。

`docs-metal/02-DESIGN.md` の CodeGen 設計に「型変換 → `air.convert.*`」の
層を追加する必要がある。sitofp / uitofp / fptrunc / fpext / fptosi / fptoui
の各命令を、この命名規則の呼出に置き換える処理である。

### 2-2. `double` 非対応が三者で一致

RFC は `AIRDemoteF64` パス（double → float 降格）を持ち、
Known Limitations に「f64 in device memory is not supported」と記す。

これは独立に確認できた:

| 情報源 | 記述 |
|---|---|
| 公式 PDF 4.1 | 「Metal does not support the **double**, long long, unsigned long long, and long double data types」 |
| 実機診断 | `error: 'double' is not supported in Metal` |
| RFC 実装 | `AIRDemoteF64` パスで降格 |

本 fork は `double` / `long long` / `long double` を拒否していない（未実装）。
`06-CROSSCHECK.md` §3-2 で挙げた「禁止 C++ 機能 11 項目未実装」に加えて、
**型レベルの禁止も未実装**である。

### 2-3. `air.indirect_argument` / `air.indirect_buffer` / `air.argbuf`

RFC 実装が持つ argument buffer 関連の metadata キー。
純正の metadata キー census にも `air.indirect_argument` /
`air.indirect_buffer` が存在する（Phase 6 で収集済み）。
本 fork の CGMetal は argument buffer を未実装。

### 2-4. デバイスメモリの alias metadata と CAS の相互作用

RFC 本文より:

> One pattern the metadata does not cover is a device load in a loop that
> reaches an atomic compare-and-swap: **without further intervention the kernel
> miscompiles on-device**, consistent with the load being hoisted across the
> CAS. A separate pass marks those loads `volatile` to prevent it.

これは Apple の**オンデバイス JIT の実挙動**に関する知見で、
静的解析では絶対に得られない。実装は `AIRDeviceLoadsVolatile.cpp`。

本 fork は Clang フロントエンドまでが範囲なので直ちには影響しないが、
`air-buffer-no-alias` と `!alias.scope` を出す以上、
**それらが JIT の最適化判断に実際に使われる**という事実は記録に値する。

---

## 3. RFC が本 fork の判断を追認した事項

| 事項 | RFC の記述 | 本 fork |
|---|---|---|
| typed pointer | 「AIR is a typed-pointer bitcode encoding」「loader cannot consume opaque pointers」 | 一致（`-no-opaque-pointers`） |
| triple | `air64-apple-macosx26.0.0` / `air64_v28` | 一致 |
| アドレス空間 0/1/2/3 | `AIRAddressSpaces.h` に同値 | 一致 |
| bitcode ラッパ | LLVM 標準 20 バイト wrapper | 一致 |
| コンテナは Mach-O 系ではない | 独自 MTLB | 一致 |

RFC は AS を 0〜3 しか定義していないが、本 fork は golden 実測から
4（imageblock）/ 7（object_data）/ 9（ray_data）も確定させている。
**この点では本 fork の方が網羅的**である（RFC は compute カーネル限定のため）。

---

## 4. RFC から学ぶべき方法論

### 4-1. 検証戦略

> The lit tests validate the IR transforms and the container structure, and run
> anywhere. The part that needs a real Apple GPU is the final "does the runtime
> accept and execute this" check; LLVM CI can't do that.
> If the project wants a stronger in-tree guarantee, options are **golden-file
> container tests and a checked-in byte-comparison against recorded toolchain
> output**.

本 fork がビルドできない状況で採った「情報源との逐語照合」は、
この「golden-file byte-comparison」と同じ発想である。方向性は妥当だった。

### 4-2. コミュニティの反応（重要）

RFC はレビュアから厳しい指摘を受けている:

> - No guarantee of correctness, with best effort for code and binary generation.
> - **No Apple involvement** while the hardware is still very much supported elsewhere.
> - No guarantee of continued support, with Apple changing things and the author
>   not able to reverse engineer.
> - Not enough integration on the existing LLVM infrastructure.

> Just remember, **releasing reverse engineered code into random Github repos is
> one thing, integrating it into LLVM is a whole different game.**

また AI 利用についても質問が出ている:

> Can you please clarify whether or to what degree AI was used when implementing
> the target?

**本 fork への含意**: この種の実装を upstream に持ち込む場合、
(a) 出所の明示、(b) Apple の関与が無いことの明記、(c) 検証範囲の限界の明示、
(d) AI 利用の開示 が求められる。本 fork は `docs-metal/` で
(a)〜(c) を実践しているが、upstream を狙うなら (d) も必要になる。

RFC 著者は「one human + one AI + one M1 Pro」で開発したと明言している。

### 4-3. 版対応の方針

RFC は **Metal 4 / macOS 26 のみ**に絞り、旧版は
「Apple has not published the AIR bitcode version / container header mapping
for prior AIR versions」として範囲外にしている。

本 fork は逆に **MSL 1.0〜4.1 の 13 版すべて**を実装対象にしており、
AIR 版 ↔ OS 版の対応表も実測で確定済み（`06-CROSSCHECK.md`）。
**この点では本 fork の方が進んでいる**。

---

## 5. 本 fork の未実装項目に追加すべきもの

`06-CROSSCHECK.md` §8 の一覧に以下を追加する。

| # | 項目 | 根拠 | 影響 |
|---|---|---|---|
| 11 | **`air.convert.*` による型変換 lowering（57 種）** | golden P02 実測 + RFC 実装 | 型変換を含む全シェーダで IR 不正 |
| 12 | `double` / `long long` / `long double` の拒否 | 公式 PDF + 実測診断 + RFC | 不正コードを受理 |
| 13 | argument buffer (`air.indirect_argument` 等) | metadata census + RFC | argument buffer 非対応 |
| 14 | metallib writer（実測レイアウト確定済み） | 本書 §1 | `.metallib` を出力できない |

特に **#11 は影響範囲が広い**。`float(vid)` のような何気ない変換が
すべて対象になるため、実質すべてのシェーダで必要になる。

---

## 6. 総括

RFC とその参照実装は、本 fork にとって次の価値があった。

1. **opaque 仮説の決着**（Phase 8 で反映済み）
2. **metallib コンテナ仕様の入手** — ただし文書のヘッダ位置は誤りで、
   実測で訂正した。これにより S18 が着手可能になった
3. **`air.convert.*` という設計漏れの発見** — 単独では気づけなかった
4. **`double` 拒否など型レベル制約の裏付け**
5. **upstream 化に必要な作法の把握**

一方、本 fork が RFC より進んでいる点もある:
- MSL 全 13 版対応（RFC は Metal 4 のみ）
- アドレス空間 7 種（RFC は 4 種）
- グラフィックスシェーダ対応（RFC は compute のみ）
- Clang フロントエンド全体（RFC はバックエンドのみ）

両者は競合ではなく**相補的**である。
