# opaque pointer に関する外部記録の追跡と、Phase 6 判定の訂正 (Phase 8)

実施日: 2026-07-29
契機: 「開発者のリバースエンジニアリング記録に、metal コンパイラが opaque を
受け付けたというものがあったはず」というご指摘。

---

## 1. 結論の要約

外部の一次記録を追跡した結果、**二つのことが同時に真**であると判明した。
この区別が、記憶にある「opaque が通った」という記録の正体である。

| 層 | opaque pointer の受理 | 根拠 |
|---|---|---|
| **metal コンパイラ (frontend)** | **入力として受理する** | `metal kernel.ll` / `kernel.bc` が動作する (Metal.jl #7)。任意の LLVM IR を入力にできる |
| **metallib ローダ (runtime)** | **拒否する** | LLVM RFC 実測。「rejects opaque-pointer type records in everything I have tested」 |

つまり「metal がコンパイルを受け付けた」という体験と、
「AIR/metallib は typed pointer でなければ動かない」という事実は**両立する**。
前者はフロントエンドの入力受理、後者は出力形式とランタイムの制約である。

---

## 2. 一次情報源

### 2-1. LLVM 公式 RFC (2026-05, discourse.llvm.org/t/.../90936)

Apple GPU 向け LLVM バックエンドを実装した開発者による、
**現時点で最も信頼できる公開一次情報**。逐語:

> AIR is a **typed-pointer bitcode encoding**, and the supported producers are
> Apple's closed Metal toolchain compiling MSL source.

> Apple's metallib loader **rejects opaque-pointer type records in everything
> I have tested**; its behavior is consistent with a bitcode reader forked
> before the opaque-pointer migration. Emitting standard bitcode with the
> in-tree writer is **refused by the loader**. The target therefore carries its
> own writer that emits the older typed-pointer encoding, which means tracking
> a pointee type for every pointer value (`PointeeTypeMap`).
> **This is the single largest piece of the target (~3,500 LOC)**

スコープ外項目にも明記:

> - **Opaque-pointer (LLVM 17+) bitcode output. Apple's loader predates the
>   opaque-pointer migration**

さらにスレッド内で別の開発者 (jdoerfert, LLVM/OpenMP 開発者) が:

> I've been playing around emitting AIR from IR, but the **version difference
> (e.g., opaque pointers), has caused me to stop for now.**

→ 独立した二人の開発者が、opaque では通らないことを実体験として報告している。

**この RFC は本 fork の判断 (typed pointer で出力する) を完全に支持する。**
逆に「3,500 LOC を費やしてでも typed pointer writer を自作する必要があった」
という事実は、**隠し opaque モードが存在しないことの強い傍証**でもある。
もし `-opaque-pointers` 相当の隠しフラグがあれば、誰もそんな実装はしない。

### 2-2. threedots.ovh ブログ (2022-04) — ご提示のもの

opaque には言及していないが、**別の重要事実**を含む:

> _metal_, the compiler, supports two input languages under the hood. But only
> one of which is exposed through public API, the Metal Shading Language.
> That second input language is _OpenCL_, usable through
> **`--driver-mode=openclc -x cl`**. That's what Apple's OpenCL implementation
> on M1 uses under the hood.

これは §3 の訂正に直結する。

### 2-3. Metal.jl issue #7 (2022-04, JuliaGPU)

**「metal コンパイラが LLVM IR を受け付ける」記録の出所として最有力**:

> it's possible to just re-use XCode's Clang-based metal compiler for this:
> ```
> $ .../usr/metal/macos/bin/metal kernel.(ll|bc)
> warning: overriding the module target triple with air64-apple-macosx12.0.0
> ```
> The problem is that we still need to **downgrade the LLVM IR** (…`metal` will
> complain about unsupported `dereferenceable` arguments, `spFlags` MDNode
> contents, etc)

要点:
- `metal` は `.ll` / `.bc` を**入力として受理する**（フロントエンド層）
- ただし **IR を「ダウングレード」する必要がある**（新しい LLVM の機能は拒否される）
- Metal.jl は自前の `metallib-as` で **LLVM 5 相当の bitcode** に落としていた

「受け付けた」という記憶はこの入力受理を指している可能性が高い。
しかし同じ記録が「ダウングレードが必要」とも述べており、
**新しい形式がそのまま通るわけではない**ことを示している。

---

## 3. Phase 6 の判定を訂正する: `test/*.bc` は「偽物」ではない

Phase 6 で `test/add.bc` / `test/AAPLShaders.bc` を
「第三者実装の出力＝偽物」と判定したが、**これは行き過ぎた結論だった**。
LLVM 22 で逆アセンブルして精査した結果を訂正する。

### 3-1. 純正である証拠（Phase 6 で見落としていた）

これらのファイルは **Apple の `!air.*` メタデータスキーマを完全に備えている**:

```
!air.version          = !{i32 2, i32 7, i32 0}
!air.language_version = !{!"Metal", i32 3, i32 0, i32 0}
!air.compile_options  = !{denorms_disable, fast_math_enable,
                          framebuffer_fetch_disable}
!air.kernel           = !{...}
!9 = !{i32 0, !"air.buffer", !"air.location_index", i32 0, i32 1,
       !"air.read_write", !"air.arg_type_size", i32 4,
       !"air.arg_type_align_size", i32 4, !"air.arg_type_name", !"float",
       !"air.arg_name", ...}
!12 = !{i32 3, !"air.thread_position_in_grid", !"air.arg_type_name",
        !"uint", !"air.arg_name", !"index"}
```

引数 operand の並び順・キー名が golden P01 と**逐語で一致**する。
第三者がゼロから再現したにしては一致しすぎており、
`framebuffer_fetch_disable` という macOS 固有の分岐まで正しい。

`source_file_name` は:
```
clang/test/AppleMetalSamples/PerformingCalculationsOnAGPU/add.metal
clang/test/AppleMetalSamples/UsingARenderPipelineToRenderPrimitives/AAPLShaders.metal
```
これは **Apple 公式サンプルコード**の名前そのもの
（Apple Developer の "Performing Calculations on a GPU" と
"Using a Render Pipeline to Render Primitives"）。

### 3-2. しかし純正 metalfe の出力でもない

| 観点 | 純正 metalfe (1,756 モジュール) | `test/*.bc` |
|---|---|---|
| producer | IDENTIFICATION 無し | `LLVM22.1.8` |
| ポインタ | typed | opaque |
| datalayout | `i128` 無し | `i128:128:128` あり |
| triple | `air64_v27-...` | `air64-apple-ios26.2`（`_vNN` 無し） |
| CC | 0 (C) | `spir_kernel` |
| AS マングリング | `PU11MTLconstant` | `PU3AS2` |
| エントリ関数 | 非マングル | `_Z12vertexShader...` |
| `air.*` 呼出 | あり | **0 件** |

### 3-3. 訂正後の判定

**`test/*.bc` は「LLVM 22 ベースの Metal 対応実装が、Apple 公式サンプルを
コンパイルして生成したテスト出力」である。**

- 「偽物」「捏造」ではない。**開発中の別実装による正当な成果物**
- おそらく本プロジェクトと同種の試み、または §2-1 の RFC 実装の初期版
- `air.*` 呼出が 0 件なのは、builtin lowering が未実装の段階だからと考えられる
- `spir_kernel` / `PU3AS2` / `__clang_ocl_kern_imp_` は、
  **OpenCL 経路（`--driver-mode=openclc`、§2-2 が述べる Apple 公式の第二言語）
  を経由した**か、その経路を模したことを示す

**そしてこれは opaque 仮説にとって極めて重要な意味を持つ:**
このファイルは **opaque pointer のまま `!air.*` メタデータを載せた AIR**
であり、「誰かが opaque で AIR を作ろうとした実記録」そのものである。
ご指摘の「opaque を受け付けた記録」とは、こうした成果物を指している
可能性が高い。

ただし **生成できることと、Metal ランタイムがロードできることは別問題**であり、
後者については §2-1 の RFC が「拒否される」と明確に否定している。

---

## 4. 「隠し opaque モード」仮説の最終判定

| 問い | 判定 | 根拠 |
|---|---|---|
| metal frontend は opaque IR を入力に取れるか | **取れる** | Metal.jl #7、`test/*.bc` の存在 |
| metal frontend は opaque で AIR を出力できるか | **記録上できない** | 全 23,538 実測が typed。`-no-opaque-pointers` を 3,654 回明示 |
| Apple 基盤 clang に opaque 能力はあるか | **ある** | わざわざ無効化しているのだから、能力自体は存在する |
| metallib ローダは opaque を受理するか | **しない** | LLVM RFC 実測。独立に 2 名が報告 |
| 「隠しモード」で opaque metallib を作れるか | **否** | 作れるなら RFC の 3,500 LOC は不要 |

**結論: 本 fork は typed pointer で出力すべきであり、現行の判断は正しい。**

ただし Phase 6 で述べた「Apple の attribute kind 上限が 71 だから LLVM 13〜15
世代」という推論は、**AIR bitcode の互換性ターゲットの話であって、
metalfe 本体の LLVM ベース版の話ではない**点に注意が必要。
metalfe 自身は新しい clang でありうる（実際 `-no-opaque-pointers` という
LLVM 15+ にしか存在しないフラグを解釈している）。
出力形式だけを意図的に古い世代に固定している、と理解するのが正確である。

---

## 5. 本 fork への反映事項

1. **判断は変更不要。** typed pointer 出力 (`-no-opaque-pointers`) を維持する。
2. `05-AUDIT.md` §2 の「偽物データ」という表現は**訂正が必要**。
   §3-3 の通り「別実装による正当なテスト出力」が正しい。
   ただし「Apple 仕様の根拠として使ってはならない」という結論は変わらない
   （純正 metalfe の出力ではないため）。
3. `audit_bitcode.py` の "NOT Apple-produced" というラベルは
   "not produced by Apple's metalfe" に改めるべき。
4. 新たに判明した実装上の含意:
   - 将来 opaque 対応を検討する場合、**ローダ側が拒否する**ため無意味。
     RFC が 3,500 LOC の typed writer を書いた理由がこれである。
   - `metal` が `.ll`/`.bc` を入力に取れることは、
     本 fork の検証手段として使える可能性がある（実機がある場合）。
