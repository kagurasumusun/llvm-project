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
