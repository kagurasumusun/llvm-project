# Typed Pointer 移植マッピング (LLVM 16.0.6 → LLVM 22.1.8)

このドキュメントは、LLVM 16 (typed pointer 時代最終世代) の実装を現行
LLVM 22.1.8 へ移植するにあたり、**移植元と移植先の対応関係を網羅的にマッピング**
したものである。移植作業はこの表に基づいて実施した。

設計原則(ユーザー指示):

1. **typed pointer bitcode のフォーマット自体は一切変更しない**
   (`TYPE_CODE_POINTER` レコード構成 `[pointee typeidx, addrspace]`、
   abbrev 構成、モジュールバージョン 2 = LLVM 16 と同一)。
2. 切り替えは LLVM 16 世代と同じフラグ:LLVM 側 `-opaque-pointers=0`、
   clang 側 `-Xclang -fno-opaque-pointers`。
3. API・設計は**移植先 (22) の構造に合わせて作り替える**。
   LLVM 22 の IR は opaque pointer 前提で維持し、typed pointer は
   **bitcode 書き出し境界のみ**で合成型 (`llvm::TypedPointerType`) として現れる。

## 対応マップ

| 機能 | LLVM 16.0.6 における実装 | LLVM 22.1.8 現状 | 移植方針 |
|---|---|---|---|
| 切替フラグ (LLVM) | `llvm/lib/IR/LLVMContextImpl.cpp` に `cl::opt<bool> OpaquePointersCL("opaque-pointers", init(true))` | 削除済み (`grep OpaquePointers llvm/lib/IR` = 0件) | **そのまま移植** (22 の LLVMContextImpl.cpp に同 cl::opt を復活) |
| コンテキスト状態 | `LLVMContextImpl.h`: `std::optional<bool> OpaquePointers` + `getOpaquePointers()` / `setOpaquePointers(bool)` (変更不可 assert 付き) | 削除済み | **そのまま移植** (22 の LLVMContextImpl.h 末尾に追加) |
| コンテキスト API | `LLVMContext.h`: `setOpaquePointers(bool) const`, `supportsTypedPointers()` | 削除済み | **そのまま移植** |
| clang 切替フラグ | `clang/include/clang/Driver/Options.td`: `defm opaque_pointers : BoolOption<"", "opaque-pointers", CodeGenOpts<"OpaquePointers">, DefaultTrue ...>` → `-Xclang -f[no-]opaque-pointers` | 削除済み | **そのまま移植** (Options.td の BoolOption 機構は 22 でも同一) |
| clang CodeGenOpt | `CodeGenOptions.def`: `CODEGENOPT(OpaquePointers, 1, 0)` | 削除済み | **そのまま移植** |
| clang→LLVMContext 接続 | `clang/lib/CodeGen/CodeGenAction.cpp`: `VMContext->setOpaquePointers(CI.getCodeGenOpts().OpaquePointers)` × 2箇所 (BackendAction 双方) | 削除済み | **接続先を 22 の現行コードに合わせて移植** |
| Writer 型テーブル | `BitcodeWriter.cpp` `writeTypeTable()`:`TYPE_CODE_POINTER` abbrev を先に emit し、`PointerType::isOpaque()` で POINTER/OPAQUE_POINTER を振り分け | `TYPE_CODE_OPAQUE_POINTER` abbrev のみ。`TypedPointerTyID` は unreachable。「opaque しか書けない」 | **16 の writeTypeTable 分岐を移植**。ただし 22 では実 IR 上の型は全て opaque → VE の型変換 hook (後述) で TypedPointerType に写像してから 16 と同一パスに通す |
| Writer per-value 型 | 型自体が Value に付属 (`I.getType()` が typed ptr) | Value の型は opaque ptr のみ | **設計変更**: `ValueEnumerator` に型変換 hook を追加し、値の走査時に TypedPointerType を合成 (再構築解析 `TypedPointerAnalysis` 参照) |
| ValueEnumerator | `ValueEnumerator.{h,cpp}` (16) | 22 とほぼ同一 (差分は `computeBitsRequiredForTypeIndices` の typo 修正 1 行のみ)。`EnumerateValue`/`EnumerateType`/`getTypeID` シグネチャ同一 | 22 側に hook を追加 (軽微) |
| ポイント先型の再構築 | 不要 (IR が型を保持) | 唯一の難所。alloca/GlobalValue/GetElementPtrInst の source element type/Load/Store/CallBase の FunctionType 等、各命令が保持する型情報から復元する解析が要る | **新規ファイル** `llvm/lib/Bitcode/Writer/TypedPointerAnalysis.{h,cpp}`。LLVM 15 時代の reader 側 opaque-upgrade (`upgradeValueTypes`/`int8 fallback`) と同じ fallback 思想で設計 |
| Bitcode Reader | typed pointer module を読み、typed context なら保持 / opaque context なら upgrade | `TYPE_CODE_POINTER` を読み **opaque へ自動アップグレード** するコードが既存 (`BitcodeReader.cpp:2629`) | **移植不要** (そのまま往復検証に使える) |
| AsmParser typed ptr (`i32*`) | `LLLexer.cpp`: `ptr` は opaque mode のみ; LLParser が `ty *` 構文を受理 | typed ptr 構文完全削除 | **移植しない** (方針③により .ll の IR 表現は opaque のまま。bitcode 境界のみの機能とする) |
| Type 出力 (AssemblyWriter) | typed ptr 印字対応 | opaque のみ | **移植しない** (同上) |
| Verifier | typed ptr 検証 | opaque のみ想定 | **移植しない** (合成 TypedPointerType はモジュールに混入させないため検証対象外) |
| Verifier / opt 等の `-opaque-pointers` 受理 | LLVMContextImpl の cl::opt に起因 (全ツール共通) | 現状フラグ自体が無く unknown option エラー | cl::opt 復活により自動復帰 |
| モジュール version | LLVM 16 writer は常に `MODULE_CODE_VERSION = 2` | 同じく 2 | 変更なし (Apple 旧形式の version 1 は将来の AIR 層の責務) |

## TypedPointerAnalysis の復元規則 (優先度順)

| 優先度 | ソース | pointee 型 |
|---|---|---|
| 1 | `AllocaInst` | `getAllocatedType()` |
| 1 | `GlobalVariable` 定数 | `getValueType()` |
| 1 | `Function` 定数 | 解決済み `FunctionType` (引数・戻り値は本体内使用箇所 + 呼出側から制約伝播) |
| 1 | `LoadInst` の ptr オペランド | load 結果型 (結果が ptr ならその合成型リンク) |
| 1 | `StoreInst` の ptr オペランド | 格納値の型 |
| 1 | `GetElementPtrInst` の ptr オペランド | `getSourceElementType()` |
| 1 | `AtomicRMWInst`/`AtomicCmpXchgInst` の ptr オペランド | 交換値の型 |
| 2 | `CallBase` の引数/戻り値 | callee の解決済み FunctionType ↔ 双方向伝播 |
| 3 | `BitCast`/`PHINode`/`SelectInst` | オペランド/被使用者間で制約伝播 (不整合時は fallback) |
| 3 | アドレス空間キャスト | アドレス空間だけ差し替え |
| 4 | 構造体/配列/ベクトル型中の ptr 要素 | GEP の source element 参照からモジュール全体で制約集約 |
| 5 | `inttoptr` / 未解決 | **i8 (fallback)** — LLVM 15 upgrade と同一思想 |

集約には不動点反復を使い、収束後も未解決のものは `i8*` へフォールバック
(release では remark、debug では statistics)。**値本来の pointee が一意に
定まらない場合でも、bitcode フォーマットとしては常に正しい**。

## 検証方法

- `llvm-as -opaque-pointers=0 in.ll -o out.bc` で `llvm-bcanalyzer -dump`
  に `TYPE_CODE_POINTER` レコードが 16 互換構造で現れること。
- 生成 .bc が現行 `llvm-dis` で読めること (reader 自動アップグレード経路)。
- 既定 (opaque) 経路は既存テストが無変更で通ること (挙動不変)。
- 参照: reader 側フォーマット定義 `llvm/lib/Bitcode/Reader/BitcodeReader.cpp`
  `TYPE_CODE_POINTER: // POINTER: [pointee type] or [pointee type, address space]`。
