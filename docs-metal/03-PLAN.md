# Metal 対応 — 実装計画 (Phase 3)

`01-ANALYSIS.md` (解析) → `02-DESIGN.md` (設計) を受けた作業計画。

## 環境上の制約 (重要)

本サンドボックスは **CPU 2 / RAM 3 GB / cmake・ninja 無し / ディスク空き 5.7 GB**。
LLVM+Clang のビルドは物理的に不可能 (最小構成でも RAM 8 GB・数十 GB のディスクを要する)。

したがって本作業では:
- **ソース実装とテストの投入までを行う**
- **コンパイル/リンクによる動作検証は行えない** — この点は成果物に明記する
- 代わりに、実装値の**正しさ**は情報源 (`golden/*.ll`, `reference/**/ast`, 実測 CSV) との
  逐語照合スクリプトで検証する

## 段階別 実装項目

| # | 段階 | 主なファイル | 根拠 |
|---|---|---|---|
| S1 | LLVM Triple | `llvm/include/llvm/TargetParser/Triple.h`, `llvm/lib/TargetParser/Triple.cpp` | §1.8 実測 triple 群 |
| S2 | 言語モード | `Basic/LangStandard.h`, `LangStandards.def`, `LangOptions.{h,def}`, `Frontend/CompilerInvocation.cpp` | §1.9 C++ 世代実測 |
| S3 | Target | `Basic/Targets/AIR.{h,cpp}`, `Basic/Targets.cpp` | §1.3 datalayout, §1.4 AS, §1.13 マクロ |
| S4 | Lexer | `Basic/TokenKinds.def`, `Basic/IdentifierTable.cpp` | §1.10 キーワード実測 |
| S5 | AddressSpace | `Basic/AddressSpaces.h`, `AST/TypePrinter.cpp`, `Sema/SemaType.cpp` | §1.4 |
| S6 | 型 | `Basic/MetalTypes.def`, `Basic/BuiltinTypes.def` 相当, `AST/ASTContext.cpp` | §1.11 AST 実測 37 型 |
| S7 | 属性 | `Basic/Attr.td`, `Sema/SemaDeclAttr.cpp` | §1.12 実測 30 クラス/90 spelling/版ゲート |
| S8 | Sema | `Sema/SemaMetal.cpp` (新規) | §1.14 診断カタログ, sema-rule-catalog |
| S9 | Mangling | `AST/ItaniumMangle.cpp` | §1.5 実測 |
| S10 | Pragma | `Parse/ParsePragma.cpp` | §1.10 純正ヘッダ実測 |
| S11 | Builtin | `Basic/BuiltinsMetal.def` (生成) | 686 builtin 実測 |
| S12 | 診断 | `Basic/DiagnosticSemaKinds.td` ほか | §1.14 |
| S13 | Driver | `Driver/Types.def`, `Driver/ToolChains/Metal.*` | §1.15 cc1 実測 |
| S14 | CodeGen 型 | `CodeGen/CodeGenTypes.cpp` | `type_layout_map.csv` |
| S15 | CodeGen builtin | `CodeGen/CGBuiltin.cpp` | `builtin_to_air_map.v2.csv` |
| S16 | CodeGen metadata | `CodeGen/CGMetal.cpp` (新規) | golden P01/P02 逐語 |
| S17 | 検証 | `docs-metal/verify/` | golden 突合 |
| S18 | metallib | `Object/` | `METALLIB_WRITER_SPEC.md` (一部 OPEN) |

## 実施結果

各段階の完了状況と検証結果は `04-STATUS.md` を参照。

## 検証方法

ビルド不可のため、以下で代替する:
1. **データ検証**: 実装に埋め込んだ定数表 (triple/AS/型/属性/マクロ/builtin) を
   情報源 CSV/golden から再抽出して差分ゼロを確認するスクリプト
2. **構文検証**: `.td` / `.def` / `.cpp` の構造的整合性チェック
3. **golden 突合**: 生成すべき `.ll` の期待値を golden から抽出し、
   実装コードの生成規則と照合
