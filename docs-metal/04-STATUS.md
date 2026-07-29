# Metal 対応 — 実装状況 (Phase 5: 検証と残作業)

最終更新: 2026-07-29

## 検証結果

`docs-metal/verify/verify_against_metal_info.py` — **26 項目全緑**

```
git clone --depth 1 https://github.com/kagurasumusun/metal-info /tmp/metal-info
python3 docs-metal/verify/verify_against_metal_info.py /tmp/metal-info
```

このスクリプトは metal-info から値を再抽出し、本ツリーの実装値と突合する。
「情報源にこう書いてある」ではなく「情報源から再計算した値と一致する」ことを確認する。

| 検証項目 | 内容 |
|---|---|
| datalayout | golden P01 の `target datalayout` と逐語一致 |
| アドレス空間 7 種 | 実測 addrspace 番号 (0/1/2/3/4/7/9) と一致 |
| opaque 型 37 種 | AST dump の implicit typedef と**順序込みで**一致 |
| opaque 型の IR 名 | golden corpus の `%struct._X_t` と一致 (35 種で照合) |
| 予約マクロ 143 種 | 実測 `-E -dM` 出力と逐語一致 |
| `-std=` 綴り | 実測で存在する全綴りを実装済 |
| C++ ベース 13 版 | 実測 `__cplusplus` と一致 |
| リソース上限 6 種 | golden P01 の module flags と一致 |
| 属性 88 種 | MSL 仕様書から抽出した全 spelling を実装済 |
| AST クラス 54 種 | 実測 `Metal*Attr` を全て網羅 |
| 版ゲート 19 件 | 実測診断文から抽出した最小版と一致 |
| builtin 686 名 | Apple 純正 stdlib が使う `__metal_*` を全て宣言済 |
| AIR intrinsic 650 件 | 対応表の air 名と一致 |
| `air.*` metadata キー | 発行する全キーが実測語彙 (15,643 件辞書) に存在 |
| AIR 版表 20 ターゲット | driver `-###` 実測の triple と一致 |

## 実装済み

| 段階 | 内容 |
|---|---|
| S1 | LLVM Triple: `air32`/`air64`、`getAIRVersion()` |
| S2 | 言語モード: `Language::Metal`、29 の `-std=` 綴り、C++ ベース |
| S3 | `AIRTargetInfo`: datalayout / AS map / 143 マクロ / リソース上限 |
| S4 | Lexer: キーワード 13 種 (`KEYMETAL`) |
| S5 | アドレス空間: `LangAS` 7 種、Parser、`asMetalLangAS()`、SemaType |
| S6 | 型: opaque builtin 37 種を AST 全域へ配線 |
| S7 | 属性: 89 spelling / Attr.td / 版ゲート / index 上限検査 |
| S8 | Sema: エントリ検査・アドレス空間規則・禁止構文 |
| S9 | マングリング: `U9MTLdevice` 等 6 種、型は bare source-name |
| S10 | pragma: `METAL internals`、`METAL fp math_mode` |
| S11 | Builtin: 650 種 + AIR 対応表 |
| S12 | 診断: 実測文言 18 種 |
| S13 | Driver: `TY_Metal`、`-fmetal-*`、AIR triple 自動決定 |
| S14 | CodeGen: opaque 型 → 名前付き opaque struct |
| S15 | CodeGen: builtin → `air.*` 呼出 (tail call / fast) |
| S16 | CodeGen: module + entry metadata |

## 重要な設計判断とその根拠

情報源内に矛盾があった箇所は、すべて**一次実測を優先**した。詳細は
`01-ANALYSIS.md` §2。

1. **呼出規約はデフォルト C**。`METAL_CODEGEN_IMPL_MAP.md` は
   `fastcc`/`air_kernel` と書くが、golden P01/P02 と 701 モジュール全走査が
   否定する。エントリ識別は metadata で行う。
2. **AIR 版は deployment target のみの関数**。`-std=` とは独立。
   driver `-###` を 20 ターゲットで実測して確定 (§1.8)。
3. **`__HAVE_*` はコンパイラが定義しない**。純正 `metal_config` が
   `__METAL_VERSION__` から導出する。ここを誤ると能力マクロが全滅する。
4. **`__METAL__` と `__METAL_MACOS__` は排他**。MSL 3.0 以降が前者、
   1.x/2.x が後者。純正ヘッダの `#ifdef` 構造がこれに依存している。
5. **builtin の型は捏造しない**。情報源が F-4 で「型付けは未実測」と
   明記しているため custom typechecking とした。

## バイナリ監査で判明した実装バグ (未修正)

`05-AUDIT.md` で生バイト検証を行った結果、自分の実装に 3 件の誤りが見つかった。
いずれも未修正。

1. **`air.arg_type_name` が pointee 型でなく全体型** (`CGMetal.cpp:446`)
   純正は `device float*` に対し `"float"` を出すが、実装は `"device float *"`。
   加えて型名は MSL 表記 (`float4`, `texture2d<float, sample>`) が必要で、
   C++ 表記ではない。MSL 型名プリンタが別途要る。
2. **`air.compile.framebuffer_fetch_*` を常に enable にしている**
   実測では iOS/tvOS は常時 enable、macOS は `-std >= macos-metal2.3` で
   enable、それ未満は disable。
3. **watchOS の AIR 版マッピングが誤り** (`Driver.cpp`)
   watchOS 11.4 は v27 が正しいが v26 を返す。正しい規則は
   iOS/tvOS が `major+9`、watchOS が `major+16`。

## 監査で判明した未実装の重要仕様

1. **エントリ関数はマングルしてはならない。** 純正 400 件すべてで
   `[[kernel]]/[[vertex]]/[[fragment]]/[[visible]]` の関数は非マングル
   (`k_using_add`)、同一モジュール内のヘルパーはマングル (`_Z10helper_mulff`)。
2. **`air.fast_*` は op ごとに存在有無が異なる** (fast 形のみ 24 種、
   両形あり 3 種)。単純に接頭辞を合成してはならない。
3. **metallib の fat header はビッグエンディアン**、MTLB 内部はリトル
   エンディアン。`METALLIB_WRITER_SPEC.md` には明記がない。

## Phase 10: 修正作業の結果 (2026-07-29)

情報源の読み込みをさらに進め、判明した欠陥を修正した。
検証は **conformance 47/47 / verify 26/26 で全緑**。

### 新たに読み込んだ情報源

| 対象 | 規模 | 得たもの |
|---|---|---|
| `reference/**/*.err` | **191,617 ファイル全走査** | Metal 固有診断 **82 種**を重複なく確定 (`docs-metal/data/measured_diagnostics_all.txt`) |
| `meta/intrinsics-catalog.csv` | 111 件 | 実シグネチャ付き intrinsic。照合の結果、真の欠落は `air.convert` と `air.fast_*` のみと判明 |
| `ir/*.ll` の数学 intrinsic 全走査 | — | `fast_` 接頭の**真の条件**を発見 (下記) |
| `src/abi_layout_all.metal` | — | ABI 検証 probe の内容 |

### 修正した項目

1. **メンバ関数の末尾アドレス空間修飾** — Parser は既存経路で動作しており、
   欠けていたのは Sema 側だった。`SemaType.cpp` に Metal 分岐を追加。
2. **`air.convert.*`** — `CGExprScalar` に `emitMetalConvert` を追加。
   命名規則が実測 57 変種すべてを再現することを検証。
3. **`air.arg_type_name`** — pointee を MSL 表記で出す `getMetalTypeName`。
4. **`air.compile.framebuffer_fetch`** — iOS/tvOS は常時 enable、
   macOS は `-std >= 2.3` のみ enable。
5. **AIR 版マッピング** — watchOS は `major+16`、iOS/tvOS は `major+9`。実測 21 点と一致。
6. **エントリ関数の非マングル** — `Mangle.cpp` で 8 種の属性を非マングル扱い。
7. **禁止 C++ 構文・型の拒否** — virtual / 派生クラス / union / double / long long。
8. **エントリ引数の binding 属性必須** — `t parameter must have texture attribute`。
9. **`__packed_vector_type__`** — `VectorType::MetalPackedVector` を追加。
   仕様書の size/align 表 21 件すべてを再現。
10. **AIR 型サフィックスと `fast_` 接頭** — 下記の新発見を実装。

### 新発見: `fast_` 接頭は要素型で決まる

従来「fast-math が有効なら付く」と理解していたが**誤り**だった。実測:

```
air.sqrt.f16        ← f16 には決して付かない
air.fast_sqrt.f32   ← f32 には常に付く
```

コーパス全体で `air.fast_*.f16` も素の `air.sin.f32` も存在しない。
ドライバフラグ名 `-fmetal-math-fp32-functions=fast`（単精度のみを名指し）
とも整合する。接頭を取る stem は実測 27 種。

## Phase 7 の突き合わせで判明した最優先の欠落

`06-CROSSCHECK.md` に詳細。純正 StdLib / 公式 PDF / gz 資料を全て読み込んだ結果:

1. **メンバ関数の末尾アドレス空間修飾が未実装（最優先）**
   `sampler(const device coherent(device) sampler &) thread = default;` のように
   MSL は仮引数リストの後ろにアドレス空間を置く。純正 StdLib で 7,668 箇所。
   これが無いと純正 StdLib は一行もパースできない。
2. `__packed_vector_type__` 未実装（StdLib 82 箇所、公式にサイズ表あり）
3. 公式が禁止する C++ 機能 12 項目のうち 11 項目が未実装
4. lambda は Metal 3.2 以降のみ許可（公式・実測・AST の三者で確認）
5. 実測 Metal 固有診断 28 種のうち 10 種が未カバー

## opaque pointer 仮説の最終決着 (Phase 8)

外部一次情報 (LLVM 公式 RFC、Metal.jl、threedots.ovh) を追跡した結果、
**二つが同時に真**であることが判明した。これが「opaque が通った」という
記録の正体である。

| 層 | opaque の受理 |
|---|---|
| metal コンパイラ (frontend) | **入力としては受理する** (`metal kernel.ll` が動く) |
| metallib ローダ (runtime) | **拒否する** |

LLVM RFC の実装者は「Apple's metallib loader rejects opaque-pointer type
records in everything I have tested」と報告し、そのために **3,500 LOC の
typed-pointer writer を自作**している。隠しモードがあればこの労力は不要であり、
これ自体が仮説を否定する強い傍証となる。

**本 fork の typed pointer 出力という判断は変更不要。**
詳細は `07-OPAQUE-EVIDENCE.md`。

## Phase 9: LLVM 公式 RFC / 参照実装との突き合わせ

`08-RFC-CROSSCHECK.md` に詳細。主な収穫:

1. **metallib コンテナ仕様を実測で確定** — RFC の `AIRLibFormat.rst` の
   タグ構造 (NAME/TYPE/HASH/MDSZ/OFFT/VERS/ENDT) は golden P01 の実バイトと
   完全一致。ただし**ヘッダのフィールド位置は RFC 文書が誤り**で、
   FileSize は +20 ではなく **+16**、セクション表は +28 ではなく **+24** から。
   Apple 純正 10 本すべてで検証済み。これで S18 が着手可能になった。

2. **`air.convert.*` という設計漏れを発見（重要）** — 純正 golden P02 に
   `air.convert.f.f32.u.i32` が実在し、コーパス全体で **57 種**。
   これは `__metal_*` builtin 経由ではなく、通常の型変換
   (`float(vid)` など) が CodeGen で落ちる層。686 builtin の対応表にも
   本 fork にも無く、**設計から完全に漏れていた**。影響範囲が広い。

3. `double` / `long long` / `long double` の拒否が三者一致
   （公式 PDF「Metal does not support the double, long long, ...」/
   実測診断 `'double' is not supported in Metal` / RFC の AIRDemoteF64）。

4. RFC は本 fork の判断（typed pointer / triple / AS 0-3 / bitcode ラッパ）を
   すべて追認。逆に本 fork は AS 7 種・MSL 全 13 版・グラフィックス・
   Clang FE で RFC より広い。両者は相補的。

## AIR との一致度

`docs-metal/verify/conformance.py` による静的照合で **35/39 (89.7%)**。
不一致 4 件はいずれも Phase 6 で検出した既知欠陥。
なおこれは静的に照合できる範囲の値であり、ビルド検証ができていない以上
真の出力一致率ではない。

## 未完了・制約

### ビルド検証ができていない (重要)

本サンドボックスは **CPU 2 / RAM 3 GB / cmake・ninja なし / 空き 5.7 GB** で、
LLVM+Clang のビルドは物理的に不可能。したがって:

- **コンパイル・リンクによる動作確認は行えていない**
- 型エラーや TableGen エラーが残っている可能性がある
- `clang/test/Metal/` の 6 本の lit テストは**未実行**

実装値の正しさは検証スクリプト (26 項目) と情報源との逐語照合で担保しているが、
これはビルド検証の代替にはならない。ビルド可能な環境での最初の作業は:

```
cmake -G Ninja -S llvm -B build -DLLVM_ENABLE_PROJECTS=clang \
      -DCMAKE_BUILD_TYPE=Release
ninja -C build clang
./build/bin/llvm-lit -v clang/test/Metal
```

### 情報源が OPEN としている項目 (推測で埋めていない)

- AS 6 / AS 8 の意味 (corpus 未出現)
- mesh / object / tile エントリ metadata の完全スキーマ (`INFO_SET.md` A-2 残件)
- `[[function_constant]]` の IR 表現 (`INFO_SET.md` A-6 = 未調査)
- builtin 各々の正確なプロトタイプ (`INFO_SET.md` F-4)
- air op を持たない builtin 9 種のうち 7 種の lowering
  (get_sampler / get_control_point / struct_has_render_target / tensor 系)

### 未着手

- **S18 metallib writer**。`METALLIB_WRITER_SPEC.md` と
  `metallib_structure.csv` でタグ構造は判明しているが、`INFO_SET.md` C-2 が
  「現サンプルは tracepoint ライブラリのみ、通常シェーダのエントリ系タグは
  未採取」と明記している。golden P01/P02 に実物の `.metallib` があるので、
  まずそれをパースして通常シェーダのタグを確定させるのが次の一手。
- Metal 専用 ToolChain (`.air` → `.metallib` のリンク段の駆動)
- `air.vertex` / `air.fragment` の出力側 metadata
  (`generated(...)` 接続 ID、補間修飾) — 読み取りは済んでいるが未実装
- `__packed_vector_type__` 属性 (純正ヘッダが 66 箇所で使用、FE 拡張が必要)
- fast-math 時の `air.fast_*` 接頭切り替え
