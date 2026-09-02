# Windows CE × LLVM/Clang ツールチェーン — 完全引き継ぎ資料 (HANDOFF)

> 最終更新: 2026-09-01 (§16) / ブランチ: `llvm-wince` / HEAD: 参照 §16
>
> **NOTE (2026-08-30):** §6.1 was rewritten and §11 added during Phase 2.
> Everything dated before that describes an older tree: the
> `EncodingType::CE` triple emitter in `MCWin64EH.cpp` mentioned there was
> **removed** (`84aec8c1`), `EmitCE()` no longer exists, and ARM SEH is
> implemented through a completely different (per-function) route.  Read
> §11 first, then the rest. **Read §13 (Phase 3: external review fixes)
> before acting on anything else in this file** — it supersedes the
> "stage-1 CI build now succeeds" claim that propagated into
> WINCE-WINEH-STATUS.md and fixes several lld/COFF gaps.
> この文書はセッション引き継ぎ用。後続のエージェントまたは人間が**この文書 alone で**作業を継続できることを目標に書かれている。

---

## 0. 30秒サマリ

- **目的**: `kagurasumusun/llvm-project`(LLVM 22.1.8ベースのフォーク)を、**Windows CE (CE 6.0中心、5.0/4.x/3.0/2.xまで設定で対応) の完全なクロス開発ツールチェーン**にすること。
- **◎ 唯一の対象 = 32ビット ARM、ARMv5TE 級、`armel` ABI(リトルエンディアン・soft-float・AAPCS)、既定 CPU `arm926ej-s`(i.MX28)**。他CPUは**スコープ外(非目標)**。特に **x86(i386-mingw32ce/CEPC)、x86-64、AArch64(ARM64/ARM64EC/ARM64X)、armhf、ビッグエンディアン、MIPS/SH3/SH4/PPC の import thunk**は「実装・完了・拡張・検証」の対象にしない。評価/監査表に x86/64 項目が出てきても**コンテキスト注記**であり、作業項目ではない(詳細は `utils/wince/README.md` の「Scope and non-goals」)。
- **方針**: 独自ランタイムは作らない。mingwrt / w32api(CeGCC系) / pthread-win32 は **kagurasumusun の各リポジトリ**(WinCE 修正コミット適用済み)からサブモジュールとして消費し、それぞれ自身の configure/make で clang を使ってビルドする。LLVM/Clang/LLD 側の改修は「正規の機構」として本リポジトリ(in-tree)で行う。
- **リポジトリ構成(2026-08-31 リ-org 完了・§14)**: 本リポジトリ = **コンパイラ側のみ**(ドライバ / CMake キャッシュ / lld COFF / lit テスト / ドキュメント / コンパイラゲート CI)。ツールチェーンのフルパイプライン(sysroot・ランタイム・パッケージ化・アプリ)は **`kagurasumusun/cellvm-build`**(サブモジュール: llvm-project@llvm-wince + mingwrt + w32api + pthread-win32)に移管。cegcc-build と同型の「サブモジュール + ビルドスクリプト」構成。
- **現状**: アプリ/ドライバ/OAL 開発に必要な一式はほぼ実装済み・検証済み(下記)。OS イメージ(sysgen/makeimg/BIB)のビルド基盤は **AE600 ソース(ユーザー手元)が前提**で、実装計画は確定済み・未実装。
- **直近のブロッカー(解消済み・2026-08-30)**: `kagurasumusun/wince-source` への**トークンアクセスが復帰**(200 OK 確認、ce600 完整ソース 145,461 ファイル参照可能)。代替だった §4g 検証(実バイナリ .pdata の解析)と WINEH-ABI-FACTS.md §4d–4g の対照がそのまま有効。旧ブロッカーの残タスクは §6.1。
- **現在のブロッカー(Phase 3 時点)**: Stage 1 CI が全 5 ラン失敗中。失敗の進行は §13.2、直前に潰した原因は TableGen 検査(診断メッセージの大文字始まり、`warn_drv_wince_sysroot_missing`)。外部レビューで lld/COFF の P0/P1 不備が指摘され、§13 で修正・記録した(いずれも**ソースレベル修正で、CI による初回検証待ち**)。
- **2026-08-31 時点**: コンパイラ CI(Stage 1 + WinCE lit)は本リポジトリで継続。**フルパイプライン(Stage 2–5)は `kagurasumusun/cellvm-build` に移管済み**(§14)。Stage 5 の進行(zlib→libpng→pixman→libiconv→SDL→liblcf→Player)は cellvm-build 側 CI で継続中(直近の停止点は libpng のテスト exe 群 `CreateMutexA` undefined → cecompat で対処し pixman へ進行)。

---

## 1. リポジトリ/ブランチ/認証の現状

| 項目 | 値 |
|---|---|
| ワークスペース | なし(サンドボックス制約のためクローンを作らず GitHub REST API で作業。2026-08-30 時点) |
| ブランチ | `llvm-wince`(デフォルトブランチ。**作業対象はこれ**) |
| リモート | `origin` = `https://github.com/kagurasumusun/llvm-project.git` |
| HEAD | リ-org 後: `a9c0d082a`(CI 削減)→ `696f2a437`(wince-sysroot 削除)→ `43d0852d6`(スクリプト類移管)→ 文書更新。進捗は §14 |
| 注意 | `arena/wince-wineh-ce` は **diverged (ahead 7 / behind 35)**。SEH を別方式(ターゲット全体 WinEH)で実装した未統合分岐。詳細は §11.3 |
| プッシュ | **全コミット push 済み**。コンポーネントリポジトリ側の WinCE 修正も push 済み(§14) |
| upstream tag | `refs/tags/upstream-22.1.8`(差分確認用に fetch 済み) |

### 認証(重要な罠)

- ユーザー提供の fine-grained トークンは **kagurasumusun の全リポジトリに admin**(2026-08-31 確認: llvm-project / w32api / mingwrt / pthread-win32)。旧トークンの llvm-project 限定制約は解消済み。
  - 4 つのリポジトリすべてに push 可能(リ-org の push はすべてこのトークンで実施)。
  - `kagurasumusun/wince-source`(private): トークンの Repository access に含まれない場合あり(404)。
- **ルール(ユーザー指示)**: トークンは単一 bash コマンド内の一時的環境変数としてのみ使用。**ファイル・ログ・コミット・指示文に一切記録しない**。
- push の方法: `git push https://x:${GH_PAT}@github.com/...`(インメモリ URL)。**`.git/config` に token を書き込まない**(`origin` は token なしの通常 URL)。

---

## 2. ディレクトリマップ(追加・変更したもの)

```
# 本リポジトリ(kagurasumusun/llvm-project, branch llvm-wince)= コンパイラ側のみ
clang/lib/Basic/Targets/ARMCE.*          CE6/CE5/CE4/CE3/CE2 ターゲット定義
clang/lib/Driver/ToolChains/WinCE.{h,cpp}  ドライバ(sysroot は <prefix>/wince-sysroot を参照)
clang/cmake/caches/WinCE.cmake           ステージ1(ホストビルド)キャッシュ
clang/test/Driver/wince*.c               ドライバ lit テスト
clang/test/CodeGen/ARM/wince-*.c         EHABI テーブル / グローバル ctor / SEH テスト
clang/test/{Parser,Sema}/ms-*.c          MSVC 構文テスト
lld/COFF + lld/test/COFF/wince-*.s       lld の CE 挙動(image/ctors/relocs/thunk/exidx/pdata)
llvm/test/MC/ARM/wince-*.s               MC レベルの CE テスト
llvm/test/CodeGen/ARM/wince-*.ll         llc レベルの CE テスト
utils/wince/README.md                    ★全仕様・監査表・検証状況の一次資料(必読)
utils/wince/STATUS.md                    現状・do-not-revive 一覧
utils/wince/WINEH-ABI-FACTS.md           WinEH/CE 実 ABI の検証済み事実
utils/wince/WINCE-WINEH-*.md             WINEH 設計/状況
WINCE-HANDOFF.md                         本ドキュメント

# ツールチェーンのパイプラインは kagurasumusun/cellvm-build に移管(§14):
#   cellvm-build/
#   ├── .gitmodules        llvm-project@llvm-wince + mingwrt@master + w32api@wip + pthread-win32@master
#   ├── sysroot/gmon|posix|include-overlay   社内向け sysroot コード(gmon, execv/system/waitpid/popen/signal, SAL/intrin)
#   ├── build-wince-sysroot.sh / build-wince-runtimes.sh / build-easyrpg-player.sh / bind-cegcc-names.sh / audit-coredll.py
#   ├── armasm/armasm-convert.py
#   ├── easyrpg-player/    MaxSignal/Player の Makefile オーバーレイ
#   └── .github/workflows/ フルパイプライン CI(Stage 1–5)
```

削除したもの: `wince-sysroot/`(→ cellvm-build、§14)、`wince-crt/`(旧ベスポーク CRT)、`.gitmodules`+`third-party/*` サブモジュール(一時 in-tree 化)、`utils/wince/patches/`(パッチ不要化に伴い削除)、`utils/wince/` のビルドスクリプト群(→ cellvm-build、§14)。

---

## 3. 実装済み内容(コンポーネント別・検証状態つき)

### 3.1 ターゲット定義(clang/lib/Basic/Targets)

- **ARM**: `WinCEARMTargetInfo`(既存を強化)。GenericARM C++ ABI、TLS=emutls 有効、`_WIN32_WCE` 既定 **0x600**、トリプルのバージョン(wince5.0等)を反映、UNDER_CE/_WIN32_WCE/WINCE/__MINGW32CE__ を `addWinCEDefines()`(OSTargets.cpp)に集約。
- **X86-32**: `WinCETargetInfo` を新規追加(CeGCC i386/mingw32ce.h の移植)。`__stdcall` 等を cdecl 属性マクロへリライト、呼出規約は受けて無視、`__CEGCC_VERSION__` 等の定義。
- トリプル: `arm-pc-wince`、`arm-mingw32ce`(エイリアス)、`i386-pc-wince`、`i386-mingw32ce`、バージョン付き `winceN.M`。llvm/TargetParser/Triple.cpp で canonical 化。
- **TLS**: `Triple::hasDefaultEmulatedTLS()` に WinCE を追加 → `-femulated-tls` 既定。`__thread`/`thread_local` 動作(emutls は TlsAlloc 系を使用、def に記載済み)。
- CPU 既定: `arm926ej-s`(ARMv5TE / i.MX28系)。`-march=armv4t` 等で変更可。soft-float(armel 相当)。

### 3.2 ドライバ(clang/lib/Driver/ToolChains/WinCE.cpp)

GCC風オプションを lld-link へ翻訳。CeGCC の SPECS を再現:

- STARTFILE: `crt3.o`(EXE) / `dllcrt3.o`(DLL) / `-pg`で`gcrt3.o`
- ENTRY: EXE 常に `WinMainCRTStartup`(crt3.o はこれしか持たない。main は winmain_ce.o がブリッジ)。DLL は `DllMainCRTStartup`
- LIB順: `%{mthreads:-lmingwthrd} -lmingw32 -lgcc -lceoldname -lmingwex -lposix(追加) -lcoredll6|coredll`
  - `-lgcc` → `libclang_rt.builtins-<arch>.a`、`-mthreads/-pthread` → `libmingwthrd.a`+`libpthread.a` と `-D_MT`
  - **ライブラリ名は sysroot に GNU 名があれば `libX.a`、無ければ `X.lib` を probe**
- イメージ既定: `/subsystem:windowsce /base:0x10000 /fixed`(DLLは `0x10000000`、/fixed無し)
- `-wince` を lld-link に渡す(ctors/dtors ブラケット + `__text_start__` バインド用)
- `-auto-import -runtime-pseudo-reloc` を渡す(dllimportデータ経路)
- COREDLL 選択: トリプルバージョン < 6.0 なら `coredll`、それ以外 `coredll6`
- sysroot 不在時に分かりやすい警告(`warn_drv_wince_sysroot_missing`)
- C++ 標準ライブラリ: `-lc++ -lc++abi -lunwind` 相当(GNU名 probe)
- `-fgnu89-inline` を既定で付与(eVC/旧mingwrtヘッダの `extern __inline` 規約)

### 3.3 LLD(lld/COFF)

- `config->wince` フラグ(新設、`-wince`)で:
  - `.ctors/.dtors` を `__CTOR_LIST__/__DTOR_LIST__` にブラケット(-1ヘッド/0終端)。mingwrt の `__main`(gccmain.c)がこれを歩く → **グローバルC++コンストラクタ/デストラクタが動く**
  - `__text_start__/__text_end__` を .text 境界にバインド(pseudo-reloc.c がイメージベース導出に使用)
- `__exidx_start/__exidx_end` バインド(既存実装、参照駆動)
- `-auto-import` + `-runtime-pseudo-reloc` はドライバが明示指定 → dllimport データがリンク可能(`__RUNTIME_PSEUDO_RELOC_LIST__` を mingwrt の `_pei386_runtime_relocator` が処理)
- subsystem/entry/base の CE 既定(DriverUtils.cpp の windowsce 等、既存部分も維持)

### 3.4 llvm(MC/CodeGen)

- `ARMCOFFMasmParser.cpp`(新規、295→322行): armasm 方言のディレクティブを MC 拡張として実装。ARMCOFFMasmParser 詳細は §6.2
- `TargetLoweringObjectFileCOFF`: WinCE のグローバル ctor/dtor を `.ctors/.dtors`(GNU形式)に出力。**MSVCの`.CRT$XCU`は使わない**(CEランタイムに `__xc_a` が無いため。過去にここが壊れていた)
- llvm-dlltool: def ファイルの `;` コメント外文法は変えず、mingwrt 側で `-C` を外して解決

### 3.5 MSVC 互換(clang)

- **既定フラグ**(WinCE ターゲット時): `-fms-extensions -fms-compatibility -fdelayed-template-parsing -fms-compatibility-version=1900 -fgnu89-inline`
- プラグマ実装: `auto_inline(on/off)`→noinline範囲適用、`check_stack(on/off)`→no_stack_protector、`setlocale`/`conform` は完全文法解析+マッピング警告、`runtime_checks` は受入無視
- `extern extern` 等の重複ストレージクラスは拡張警告で許可(clang本体機能、テスト `ms-extern.c`)
- `__uuidof`/`__declspec(uuid)`: clang本体が対応済み(追加不要だった)
- SAL: `include-overlay/sal.h` を新規作成(現在は `cellvm-build/sysroot/include-overlay/sal.h`)(`_In_/_Out_` 系 + 旧 `__in` 系の no-op 定義)
- `<intrin.h>`: `include-overlay/intrin.h` 新規。**ARMv5TE では arm_acle.h を include しない**(LLVMのACLEヘッダの `__dmb` は v6+ 専用 intrinsic で v5 で codegen エラーになる)。v5 は `CacheSync` ベースの `__dmb/__dsb/__isb` を提供、v6+ は ACLE。`_CountLeadingZeros` も実装

### 3.6 sysroot(ステージ2)

`build-wince-sysroot.sh` が実施(検証済み・EXIT=0):

1. mingwrt を patch 無しで in-tree から configure/make(host_alias=arm-mingw32ce)。dlltool は `llvm-dlltool -m armce` への sh アダプタ(`--def`→`-d`、`--as`除去、`--output-def`は llvm-nm で合成)
2. 生成物: `crt3.o dllcrt3.o CRT_noglob.o crtmt.o crtst.o libmingw32.a libm.a libmingwex.a libceoldname.a libcoredll.a libcoredll6.a` + **`libmingwthrd.a`(crtmt.o を直接アーカイブ。mingwrt 自身のルールは DLLリンクを巻き込むため回避)**
3. ヘッダ: mingwrt install-headers → sysroot/include
4. w32api: libce の全 def→`lib*.a`(71個)+ ヘッダコピー
5. pthreads4w: `pthread.c` ジャンボビルド(`-D_MT -DPTW32_STATIC_LIB -D__CLEANUP_C -D__PTHREAD_JUMBO_BUILD__`)→ `libpthread.a`、ヘッダ5個
6. gmon: `gcrt3.o` + `libgmon.a`
7. posix: `libposix.a`(process/popen/signal)+ `sys/wait.h`

最終ライブラリ数 76、ヘッダ 426+。`include-overlay`(sal.h, intrin.h)もステージ済み。

### 3.7 ステージ3

`build-wince-runtimes.sh`: compiler-rt builtins(`libclang_rt.builtins-arm.a`)、libunwind/libc++abi/libc++ 静的(`LIBCXX_HAS_PTHREAD_API=ON`、filesystem OFF、WinCEロケールバックエンド `libcxx/src/support/wince/locale_wince.cpp` 既存使用)。**実ビルドは未実行**(ステージ1の clang が必要なため。CI で回る想定)。

### 3.8 -pg プロファイラ(実装済み)

- `gcrt3.o`: crt3 のライフサイクル + `__gmon_start/__gmon_stop`
- `libgmon.a`: BSD gmon.out(TIME_HIST)を書く。サンプラースレッド(10ms、低優先度、SuspendThread/GetThreadContext/ResumeThread)。`mcount` は no-op(clang の `-pg` は `bl mcount` のみで、EABI フレーム契約がないためアーク収集不可 — フラットプロファイルのみ)
- 使い方: `clang --target=arm-pc-wince -pg hello.c -o hello.exe` → 実行 → `gmon.out` を gprof で解析

### 3.9 POSIX 層(libposix.a)

- `execv/execvp/execl/execlp`: CreateProcess → 待機 → 子の終了コードで自exit(POSIX近似)
- `system()`: CreateProcess 直(CEにシェルなし)。戻り値は waitpid エンコーディング
- `waitpid`: ランタイム子テーブル + WNOHANG
- `popen/pclose`: **"r"は子完了後にテンポラリファイルを開く(CEは子stdoutリダイレクト不可という文書化済み限界)、"w"は子をpcloseまで遅延**
- `signal/raise/alarm`: レジストリ+協調配送、SIGALRM はタイマスレッド。フォルト起因のSIGSEGV等は要VEH(def未記載、カーネル側は存在の可能性→要確認)
- `fork`: **不可能**(CEにCOWなし。CygwinモデルはOSレイヤープロジェクト)

### 3.10 coredll.def 完全化

- アーカイブ済み CE5/WM6 dumpbinダンプ(1,799関数)**全9チャンクを照合、欠落30名を追加**(CeMapArgument, allPrivilege, C++マングル28個)
- `TlsAlloc/TlsFree` を追加(MSDN確認: CE 1.0+に存在。CeGCC def の漏れ)
- `audit-coredll.py`: 実機 dumpbin 出力との照合ツール
- 注意: `wcsdup` は `_wcsdup` と記載されており名前不一致 → mingwex/wince/wcsdup.c でローカル提供

### 3.11 armasm 対応(2経路)

- **Path A(フォールバック)**: `utils/wince/armasm/armasm-convert.py`。AREA/PROC/ENDP/EXPORT/IMPORT/DCB/DCD/EQU/IF/MACRO($パラメータ)/GET 等を GNU へ変換。CEドライバ風ソースとマクロテストの両方で assemble 検証済み
- **Path B(正規・実装済み)**: `llvm/lib/MC/MCParser/ARMCOFFMasmParser.cpp`(新規)。MASM系パーサ拡張として AREA/PROC/ENDP/EXPORT/IMPORT/EXPORTAS/ALIGN/ENTRY/END + 無視ディレクティブ群を実装。CMake 登録、`createARMCOFFMasmParser()` を MCAsmParser.h に宣言。cc1as に `Opts.MasmDialect`(=2でarmasm)を追加し、ARM/Thumb トリプルで拡張をアタッチ。**clang ドライバは WinCE 向け `.s` 入力に `-masm=armasm` を既定転送**
- これで `clang --target=arm-pc-wince -c driver.asm` が直接動くはず(**実機検証はステージ1ビルド待ち**)

### 3.12 MSVC PRAGMA・その他の言語機能

- `ms-pragmas-full.c` テスト参照。Sema 経由の auto_inline/check_stack は `SemaAttr.cpp` に実装
- `__declspec` 系、`__int64` 等の型マッピング、`__uuidof` は clang 本体機能でカバー

---

## 4. 検証の現状(何が済んでいて何が済んでいないか)

### 検証済み(zig bundled clang 18/21 をホストスタンドインとして使用)

- sysroot フルビルド(76 ライブラリ、426+ ヘッダ、libpthread/libposix/libgmon 含む)
- シンボルレベル監査: 301オブジェクトの未解決参照を自作パーサで抽出 → **残存ゼロ**(リンカ提供シンボルと `__aeabi_*` を除く)
- クライアント TU(windows.h/pthread.h/tchar.h)がステージ済み sysroot のみでコンパイル可能
- 厳格 C17 スイープ(`-std=c17`/`-std=gnu17`): **警告ゼロ**。言語世代更新は完了
- armasm 変換器 → zig clang で ARM COFF オブジェクト生成まで
- lit テスト群 → **CI で実走済み**(2026-08-31 以降、全パイプライン緑のランで WinCE lit ゲート 30 ファイルが PASS。zig スタンドイン時代の記述は以下に歴史として残す)

### 未検証/未実施

1. **ステージ1(LLVM/Clang自体のビルド)** — このサンドボックス(2コア/3GB)では不可能。CI(`.actions/build-wince-llvm.yml` → ユーザーが `.github/workflows/main.yml` に移動済み)で実行される想定
2. **ステージ3の実ビルド** — 同上
3. **実機テスト** — 一切未実施。旧 `wince-crt/docs/DEVICE-TESTING.md` は削除済み(必要なら履歴から復元)
4. coredll dump の残チャンク2/3/5/6の `audit-coredll.py` 検収(チャンク0/1/4/7は手動照合済み。2/3/5/6も現在は全部照合済みという記載に更新するのを忘れていた可能性 → **要確認**: 実際には §3.10 の通り全9チャンク照合・欠落30追加が完了している)

---

## 5. 重要な設計判断と理由(後続者が再検討しないよう記録)

1. **ベスポークCRTを廃止した理由**: ユーザー指摘どおり、mingwrt/w32api をそのまま使う方が正しい。wince-crt は削除済み
2. **サブモジュール廃止 → in-tree vendoring**: ユーザー指摘。LLVMの構造ではランタイムが自身のソースを所有(compiler-rt と同型)
3. **`third-party/` ではない場所に置く**: ユーザー指摘で `wince-sysroot/` ランタイムプロジェクト直下に移動(★歴史的決定: 2026-08-31 のリ-org でこの in-tree vendoring 自体を取りやめ、コンポーネントは `cellvm-build` のサブモジュールになった)
4. **global ctor/dtor を GNU 形式にした理由**: `.CRT$XCU` は MSVC CRT の `__xc_a` 起動オブジェクトを要求し、CEランタイムに存在しない。mingwrt の `__main` が歩くのは `__CTOR_LIST__`
5. **EXE エントリを常に WinMainCRTStartup にした理由**: crt3.o が定義する唯一のエントリ。`mainCRTStartup` はどこにも存在しない(過去に -mconsole リンクが必ず失敗していた)
6. **libmingwthrd.a を crtmt.o 直アーカイブにした理由**: mingwrt の正規ルールは def 生成→DLLリンクを巻き込む。静的リンクに必要なのは crtmt.o のみ
7. **`__declspec` を clang 側でマクロ定義しない**: 当初 clang 側ブリッジを試したが、ユーザー指示で mingwrt 側修正(`__clang__` をプローブに追加)に変更し、clang 側は revert 済み
8. **pthread-win32 に直接コミットしない**: ユーザー指示(他人リポジトリのため)。修正はベンダーツリー内に適用済み(注: ユーザーが後に `kagurasumusun/pthread-win32` フォークを作成したので、将来的にそちらへ upstream するのが望ましい)
9. **C17 ピン留め**: C23 はキーワード吸収(bool等)リスクがあるため C17 で停止。`-fgnu89-inline` 併用
10. **ARMv5TE で arm_acle.h を include しない**: clangのACLEヘッダの `__dmb` は v6+ 専用 intrinsic で、v5 では codegen が fallback しない(検証済みエラー)。v5 は CacheSync バリア

---

## 6. 未完了作業(引き継ぎタスク)

### 6.1 【一部進展】wince-source アクセス → ARM32 WinEH CE 実装

> **⚠ 更新 (2026-08-30): この節の「実装計画」は失効。** ARM WinEH は
> 計画された `EncodingType::CE` ルートではなく、**関数単位の WinCFI 切り替え**
> (`llvm/lib/Target/ARM/ARMWinCFI.h` の `functionUsesWinCFI()`)＋
> `ARMWinCOFFStreamer::CEEmitUnwindInfo` の圧縮 CE `.pdata` で実装済み。
> `MCWin64EH.cpp` の `EmitCE()` は削除された(`84aec8c1`)。以下の記述は
> 「なぜその経路を採らなかったか」の記録として残してある。**現状は §11 を読むこと。**
> また §4 の「CE カーネルに SEH 相当の OS サポートは無い」という前提は誤りで、
> 訂正済み(coredll は `__C_specific_handler` を export している)。

**更新 (2026-08-28)**: トークンの `wince-source` アクセスは解決済み(200 OK)。
`DLL/ARM/unwind.c` と `CORELIBC/CRTW32/EH/*.cpp` を参照し、構造的事実(レイアウト・
オフセット・マジックナンバーのみ、アルゴリズムコードはコピー禁止)を
`utils/wince/WINEH-ABI-FACTS.md` に抽出した。**重要な訂正**:
`ARMMCTargetDesc.cpp` に残っていた「CE カーネルに SEH 相当の OS サポートは無い」という
コメントは誤り。実際には coredll が `__C_specific_handler`/`__CxxFrameHandler` を
export しており(coredll6.def 1213-1214行、確認済み)、ARM 版アンワインダは
`RUNTIME_FUNCTION{BeginAddress, PrologEndAddress, EndAddress}` という3ワード形式を
使い、**xdata もパックドアンワインドコードも一切使わず、実際のプロローグ命令列を
逆順再実行してレジスタを復元する**という、NT ARM WinEH とは全く異なる方式である
(詳細は WINEH-ABI-FACTS.md §3)。コメントは訂正済み。C++ 例外(EHABI 経由)は
この節と無関係で、影響を受けない。

**未解決**: `ehdata.h`/`ehstate.h`(`FuncInfo`/`TryBlockMap` 等のレイアウトを宣言する
ヘッダ)は今回の抽出(AE600 サブセット)に含まれていない。`__CxxFrameHandler` v1 の
構造体レイアウトは依然ブロック中(優先度は元々低い — C++ 例外は EHABI で足りるため)。

**当初の実装計画(調査済み・詳細。ARM WinEH 部分は上記の訂正済み事実を踏まえて読むこと)**:

参考にする既存 in-tree 資産(全て存在確認済み):

| 資産 | 場所 | 状態 |
|---|---|---|
| `EncodingType::CE` enum | llvm/include/llvm/MC/MCAsmInfo.h:48 | 宣言のみ、エミッタなし |
| `ARMCOFFMCAsmInfoMicrosoft` | ARMMCAsmInfo.cpp:115 | WinEH + Itanium エンコーディング設定済み |
| `ARMWinCOFFStreamer` + `Win64EH::ARMUnwindEmitter` | ARMWinCOFFStreamer.cpp | .pdata/.xdata 出力、**パックドアンワインド命令コード実装済み**(`ARMEmitUnwindInfo` with TryPacked, MCWin64EH.cpp:2517) |
| ARMAsmParser の `.seh_*` ディレクティブ | ARMAsmParser.cpp | 存在 |
| `WinEHPrepare` パス | TargetPassConfig.cpp:943 | ターゲット非依存で存在 |
| `WinException` | WinException.cpp | isThumb 対応、!isAArch64 パスあり |

**実装ステップ**:

1. **wince-source から CE 形式の実データを確認**:
   - `PRIVATE\WINCEOS\COREOS\CORE\` 配下の coredll 関連、SEH 使用箇所
   - CE の RUNTIME_FUNCTION / UNWIND_INFO 形式(ARM NT とどこが違うか。恐らく同一またはサブセット)
   - `__CxxFrameHandler`(v1)が期待する FuncInfo/TryBlockMap レイアウト
2. **`EncodingType::CE` の実装**(訂正: ARM NT とほぼ同一という当初の想定は誤りだったと判明。WINEH-ABI-FACTS.md §3 参照):
   - ✅ **実装済み(未ビルド検証)**: `llvm::Win64EH::ARMUnwindEmitter::EmitCE()`(`llvm/lib/MC/MCWin64EH.{h,cpp}`)。`{BeginAddress, PrologEndAddress, EndAddress}` の3ワード triple を xdata 無しで `.pdata` に出す、CE 専用の独立したパス。既存の NT ARM 用 `ARMEmitUnwindInfo`/`ARMEmitRuntimeFunction` とはコードを共有せず(低レベルの `EmitSymbolRefWithOfs` ヘルパのみ共用)、新規実装。`.seh_endprologue` が既に汎用MCインフラとして `FrameInfo::PrologEnd` をセットする仕組みを持っていたため、ラベル基盤は新規追加不要だった(WINEH-ABI-FACTS.md §4a に詳細)
   - ⬜ **未実装**: `WinEHEncodingType = EncodingType::CE` をどこからも選択していない。C++ 例外は EHABI のまま維持する必要があるため、ターゲット単位ではなく**関数単位**(`__try` 使用関数のみ)での選択機構が必要 — これは未設計
   - ⬜ **未実装**: `ARMWinCOFFStreamer::emitWindowsUnwindTables()` は `Emit()` のみ呼んでおり `EmitCE()` を呼んでいない。`AsmPrinter.cpp:647` の switch にも `CE` ケース無し(ARM NT 自体もこの switch には来ていない点に注意 — 正しい接続点は要調査)
   - ⬜ **未確認**: clang の `__try/__except` (MS SEH) が ARM ターゲットでそもそも Sema/CodeGen レベルで動くのか未調査。これが無いと上記は宙に浮く
   - **実装リスクの所在(エンコーダ実装は完了したので、今後のリスクはここに移った)**: codegen 側。CE のアンワインダはプロローグ命令列をパターンマッチで解釈する best-effort インタプリタなので、バックエンドが実際に生成するプロローグが認識対象のイディオム集合に収まっているかどうかが正しさを左右する(WINEH-ABI-FACTS.md §3 のイディオム一覧参照)
3. **SEH(`__try/__except`)の配線**:
   - coredll6.def に `__C_specific_handler` **存在確認済み**(行1213)。SEH はこれで動く
   - `WinCEARMTargetInfo` / ドライバで CE 時に `ExceptionHandling::WinEH` を選択するフラグを追加(現状は `ExceptionHandling::ARM`=EHABI。**注意**: C++ 例外(EHABI)と SEH は別経路。カーネルソースの `__try` は SEH なので WinEH が必要だが、libc++ の C++ 例外は EHABI を使い続ける設計。両立方法は `WinException` 側で SEH C フレームのみ処理する形を検討)
   - 実装の詳細な分岐設計は wince-source の実データを見てから
4. **C++ 例外(`__CxxFrameHandler` v1)**:
   - coredll6.def 行1214 に `__CxxFrameHandler` あり(`3`は無し)
   - clang の WinException は `__CxxFrameHandler3` 用テーブルを出す。**v1 形式への対応 or 互換サンク**(coredll に v1 ラッパーを差し込む)のどちらか。AE600 の `__CxxFrameHandler` 実装を参照して決定
   - なお本ツールチェーンの C++ 例外は libc++ + EHABI(GenericARM)が既定なので、**SEH が動けば C++ の WinEH 化は必須ではない**。優先度は低い
5. **lld**: `.pdata` の FixupV2 等の CE 向け調整
6. **検証**: AE600 の SEH 使用ソース(カーネル or ドライバ)を実際に `clang --target=arm-pc-wince -c` し、`llvm-readobj` で unwind 情報を確認

**工数の目安**: SEH のみなら数日。`__CxxFrameHandler` v1 までだと 1-2週。

### 6.2 【すぐできる】小タスク

- [x] `utils/wince/README.md` の「COREDLL def completeness」節 → **済み**(2026-09-02。壊れていたテーブル断片の除去と、`_errno` 記述の実態への同期も同時に実施)
- [x] `.actions/build-wince-llvm.yml` と `.github/workflows/main.yml` の重複解消 → **済み**(`a9c0d082a` で両コピーを削除。HEAD に `.actions` は存在しない)
- [x] `kagurasumusun/mingwrt` への push リマインド → **済み**(§14.2 で `69043bc` として push、以後も継続 push 中)
- [x] `cellvm-build/sysroot/posix/` の `signal.c` VEH 検討 → **否決済みとして解決**(CeGCC の coredll.def はベクトル例外 API を export しないという設計事実のため、フォルト起因 SIGSEGV は配送不可。signal.c 冒頭コメントに方針を記録済み。明示的な `raise()` 経由の配送のみ対応)
- [x] errno のスレッド対応 → **実装済み**(mingwrt `coredll_stubs.c` の `_errno` は TlsAlloc ベースのスレッド毎スロット。「要承認」のランタイム意味論変更は当時の実装時に承認済み)

### 6.3 【将来】OS ビルド基盤(AE600 前提で実装可能と結論済み)

- `build.exe` クローン(sources/dirs ツリーウォーカ)
- `sysgen` クローン(SYSGEN_XXX → def フィルタ。**これが AE600 のフル def と我々の完全 def を繋ぐ役割**)
- `makeimg`/BIB/REG/DAT コンパイラ(形式は文書化済みで安定)
- 詳細は `utils/wince/README.md` の「OS build platform」節

### 6.4 【継続的】

- 実機テスト(デバイスなし)。`gweslab/cerf`(CEエミュレータ)が検証に使える可能性あり
- 上流 LLVM へのフィードバック(ARMCOFFMasmParser、EncodingType::CE 等)

---

## 7. ビルド & テスト手順(後続者がそのまま実行できる形)

### ステージ1(ホストツールチェーン)

```bash
cmake -G Ninja -S llvm -B build -C clang/cmake/caches/WinCE.cmake \
  -DCMAKE_BUILD_TYPE=Release
ninja -C build clang lld llvm-ar llvm-ranlib llvm-dlltool llvm-nm llvm-mc \
  llvm-readobj llvm-objdump FileCheck
ninja -C build install   # install/wince-llvm に入る
```

マシン要件: 8コア/16GB+ 推奨(このサンドボックスでは不可能だった)。CI で実行推奨。

### ステージ2(sysroot)

```bash
# (cellvm-build リポジトリのルートで実行)
sh build-wince-sysroot.sh \
  --toolchain <install>/bin --target arm-pc-wince \
  --prefix <install>/wince-sysroot --jobs $(nproc)
```

スモークスタンドイン(本ツールチェーン無しで動作確認するトリック):
zig bundled clang を `/tmp/fake-tc/bin/{clang,llvm-ar,llvm-ranlib,llvm-dlltool}` にラッパーとして置き、`/tmp/wince-defines.h`(CEプリディファイン集)を `-include` する。過去の検証はすべてこの方法。詳細は git 履歴の会話ログではなく、スクリプト自体を見れば分かる構造。

### ステージ3(ランタイム)

```bash
# (cellvm-build リポジトリのルートで実行)
bash build-wince-runtimes.sh --toolchain <install>/bin \
  --sysroot <install>/wince-sysroot
```

### lit テスト

```bash
ninja -C build check-clang-driver check-lld-coff  # 関連のみ
# 個別:
build/bin/llvm-lit -sv clang/test/Driver/wince.c lld/test/COFF/wince-ctors.s
```

---

## 8. 罠・学び(同じ轍を踏まないために)

1. **zig cc は `--target=arm-pc-wince` を解釈しない** → `arm-freestanding-eabi -mcpu=arm926ej_s -fms-extensions -fshort-wchar` に書き換えるラッパーで検証する
2. **zig の builtin ヘッダ(float.h/stdarg.h/arm_acle.h)が本家 clang と異なる**。特に arm_acle.h の `__dmb` は v5 で codegen 死。CE ソース側では ACLE に直接依存しない
3. **mingwrt の def 生成規則は `-C -E -P`** — C コメントが def に残り llvm-dlltool が死ぬ。`-C` 除去で解決済み(マージ済み)
4. **`llvm-dlltool -m armce`**: `armce` は llvm 独自の別名(ARM 0x01c0)。`-m arm` は ARMNT になるので混同しない
5. **posix ラッパーソースの先頭コメントに `exec*/system` と書くと `*/` でコメントが閉じる** → `exec* / system` と書くこと(実際にやった)
6. **cc1as に新しい Opts を足すときは Opts 構造体(cc1as_main.cpp 内)とパース両方**
7. **fine-grained トークンは Repository access に入っていないリポを404で隠す**
8. **ドキュメントの分かりやすさのため、README(utils/wince/README.md)が一次資料**。本資料と矛盾したら README を疑う(README の方が細かい)
9. ユーザーの指示は日本語。返信は日本語。コミットメッセージは英語(リポ慣習)
10. ユーザーは「勝手に他人のリポジトリを触るな」「パッチファイルより直接修正」→「他人のは直接触るな」と方針が進化した。**最終方針: 自分たちのリポは直接修正、他人のリポは修正しない**

---

## 9. ユーザーとの約定事項・経緯メモ

1. 最初は CeGCC 参照の wince 対応 → wince-crt 作成(他人のセッション)
2. ユーザー: 「mingwrt/w32api を使え。wince-crt は不適切」→ wince-crt 廃止
3. ユーザー: 「mingwrt 自体を clang でビルドできるように(パッチではなく)」→ mingwrt に直接コミット
4. ユーザー: 「勝手に他人のリポを触るな。kagurasumusun だけにしろ」→ pthread はパッチ方式へ
5. ユーザー: 「サブモジュールではなく正規の llvm&clang の一部として」→ in-tree vendoring(third-party は不適切と指摘され wince-sysroot/ へ)→ さらに 2026-08-31 の方針転換で「サードパーティツリーは本リポに内蔵しない」= cellvm-build サブモジュールへ(§14)
6. ユーザー: 「断線経路を全部確認」「SEH/デストラクタが壊れている」→ 各種修正
7. ユーザー: 「-pg 実装して」「posix 実装して」「armasm 正規実装して」「C17 更新して」→ 実施
8. ユーザー: 「AE600 があるので OS ビルド基盤も自前実装可能では?」→ 実装計画策定(未実装)
9. ユーザー: 「wince-source から参考ソースを取得して WinEH 実装して」→ **トークン権限不足でブロック中**

---

## 10. 関連ドキュメント

- `utils/wince/README.md` — 一次資料(最も詳細)。矛盾があればこちらを優先し本資料を更新
- 各コンポーネントリポジトリ(kagurasumusun/{mingwrt,w32api,pthread-win32})の WinCE 修正コミット — ベンダー証跡(in-tree の `README.llvmvendor.md` は §14 のリ-org で削除)
- `clang/cmake/caches/WinCE.cmake` — ステージ1設定
- 各テストファイル — 期待動作の仕様書を兼ねる

(以上)

---

## 11. Phase 2 (2026-08-30): 調査で見つかった問題の修正

Phase 1(環境構築 + 全量調査)の結果。**ビルド・lit 実行は一切しておらず、
すべてソースレベルの解析に基づく**。根拠は各コミットメッセージに記載。

### 11.1 修正したバグ(いずれも実行未確認・既存テスト未実行)

| # | 場所 | 内容 | 状態 |
|---|---|---|---|
| B1 | `clang/include/clang/Options/Options.td` | `-masm=` に `CC1AsOption` が無く、cc1as が unknown argument でエラー → **WinCE の全 `.s`/`.S` アセンブルが失敗** | 修正 |
| B2 | `lld/COFF/Chunks.cpp` | PROLOG 疑似再配置が `off-8`(FUNCLEN 用)を共用し、**別ワードを書き換え** → PrologLen が常に 0 | 修正 |
| B3 | `lld/COFF/Chunks.cpp` | 疑似再配置で **inline addend を未加算** → FuncLen/PrologLen が `.text` 先頭基準の不正値 | 修正 |
| B4 | `llvm/lib/Target/ARM/.../ARMWinCOFFStreamer.cpp` | pFuncStart に Thumb マーカー(bit 0)が載らない(一時ラベルはセクション再配置になるため) → CE カーネルが Thumb/ARM デコーダを選べない | 修正(式に +1 を畳み込み) |
| B5 | `llvm/lib/MC/MCParser/AsmParser.cpp` | ドット無しディレクティブがディスパッチされず、armasm 拡張が**完全にデッドコード** | 修正(+ 大文字小文字を無視) |
| B6 | `llvm/tools/llvm-mc/llvm-mc.cpp` | `llvm-mc` に armasm 方言オプションが無く、MC テストから到達不能 | 修正(`-masm-armasm`) |
| B7 | `llvm/lib/MC/MCParser/ARMCOFFMasmParser.cpp` | `AREA \|.text\|` のパイプ形式(Platform Builder の標準記法)を読めない | 修正 |
| B8 | `llvm/lib/TargetParser/Triple.cpp` | `getDefaultFormat` の `case WinCE:` のインデント違反(clang-format) | 修正 |
| B9 | `lld/COFF/Writer.cpp` | `insertEXIdxBoundsSymbols()` が無条件呼び出し(WinCE 専用ヘルパ) | WinCE 時のみに限定 |
| B10 | `clang/include/clang/Basic/TargetInfo.h` | `isSEHTrySupported()` が x86 CE でも SEH を許可 → ARM 専用の CE `.pdata` が無くデスクトップ形式を出し「暗黙の誤生成」 | ARM/Thumb のみに限定 |

### 11.2 プロセス修正

* **CI が lit を実行していなかった**: `.github/workflows/main.yml` に WinCE
  テスト一式を実行するステップを追加(+ `split-file` のビルド追加)。これが
  B2/B3 が長期間見過ごされた直接の原因。

### 11.3 `arena/wince-wineh-ce` 分岐(未統合・要判断)

`llvm-wince` と **diverged (ahead 7 / behind 35)**。分岐側だけにある差分:

* `AsmPrinter.cpp` に `WinEH::EncodingType::CE` → `WinException` を割当
  (= **ターゲット全体**を WinEH に切り替える方式。WINEH-ABI-FACTS §4c で
  「EHABI を壊すので危険」と結論済みの方針。`llvm-wince` は関数単位方式を採用)
* `EHPersonalities.cpp` の v1 `__CxxFrameHandler` 対応(+4)
* `ARMMCAsmInfo.{h,cpp}` の `WinEHEncodingType = CE` 設定
* `lld/COFF/{Writer,Chunks}.cpp` と `ARMWinCOFFStreamer.cpp` の別実装
* `utils/wince/WINCE-WINEH-{DESIGN,STATUS}.md` (llvm-wince 側に**存在しない**設計資料)
* `clang/lib/Driver/ToolChains/WinCE.h` / `TargetInfo.h` / `DiagnosticDriverKinds.td`

**判断が必要**: (a) 設計資料だけ取り込む / (b) v1 `__CxxFrameHandler`
personality も取り込む(優先度低: C++ 例外は EHABI 経路)/ (c) 分岐を廃止。
`llvm-wince` の方式は分岐の方式を概ね supersede しているため、最低限
**設計資料の取り込み**と**分岐の扱いの明記**が必要。

**判断 (2026-08-30)**: (a) を実施 — `utils/wince/WINCE-WINEH-{DESIGN,STATUS}.md` を
provenance バナー付きで取り込み(分岐自体は放置。削除はユーザー判断)。
コードは取り込まない(superseded)。附带調査として `frame.cpp` で
`__CxxFrameHandler` = `__CxxFrameHandler3` への一行パススルー(v3 形式
`FuncInfo` を `pDC->FunctionEntry->HandlerData` から読む)を確認済み
(WINEH-ABI-FACTS §4g 参照) — 「v1 形式」は def の名前からの推測にすぎず、
C++ 例外が EHABI を使う限り C++ WinEH 対応は不要。

### 11.4 残課題(優先度順)

1. **実ビルド + lit 実行**(本セッションでは対象外)。CI を回して WinCE
   テスト一式を通すことが最優先。特に `lld/test/COFF/wince-pdata.test` の
   byte 期待値は**手計算で再導出したもの**であり要検証。
2. armasm Path B の完全化 → **第 3 弾で完了(§15)**。`%`/`&`/`n_` 数値リテラル、
   `DCFS`/`DCFD`、`GBLA`/`SETA`、`IF`/`ELSEIF`/`ELSE`/`ENDIF`、`DCBU`〜`DCFDU`
   は実装・llvm-mc 実走で検証済み。残るのはマクロ処理系
   (`MACRO`/`MEND`、`WHILE`/`WEND`、`GET`/`INCLUDE`、`:DEF:`、`SETS`)だけで、
   それは **converter 側で既に実装済み**(`cellvm-build:armasm/armasm-convert.py`。
   本リポジトリの `llvm/utils/wince/...` という経路は実在しないので注意)。
3. x86 CE の SEH → **非目標**。B10 は診断のみ(そのまま維持)。x86 は本ツールチェーンの対象外。
4. `WINEH-ABI-FACTS.md` §4g の `dwSlot` 値など、private リポ
   `wince-source` 由来の前提の一次確認(今回は未参照)。
5. `.actions/build-wince-llvm.yml` と `.github/workflows/main.yml` の重複解消
   (ユーザー判断事項として残置)。
6. 実機検証(デバイス無し)。

## 12. Phase 2 第 2 弾 (2026-08-30): armasm Path B のステートメント構文

Phase 2 第 1 弾で「ディスパッチが届かない」を直しただけでは armasm のソース
は読めなかったため、続けて**文(statement)構文**を実装した。
**繰り返しになるが、ビルドも lit 実行もしていない。すべてソースレベルの解析
に基づく。**

| # | ファイル | 内容 |
|---|---|---|
| C1 | `llvm/include/llvm/MC/MCParser/AsmLexer.h`<br>`llvm/lib/MC/MCParser/AsmLexer.cpp` | `;` を行コメント開始として追加受理する `setSemicolonComments()`。ARM の `MCAsmInfo::getCommentString()` は GNU 構文用の `@` なので、置換ではなく**追加**でなければならない |
| C2 | `llvm/include/llvm/MC/MCParser/MCAsmParserExtension.h`<br>`llvm/lib/MC/MCParser/MCAsmParserExtension.cpp` | `emitMasmLabel()` を追加。既定実装はラベルを出力し、`doBeforeLabelEmit()`/`onLabelParsed()`(ARM は pending IT ブロックのフラッシュと `.thumb_func` に使う)も呼ぶ |
| C3 | `llvm/include/llvm/MC/MCParser/MCAsmParser.h`<br>`llvm/lib/MC/MCParser/AsmParser.cpp` | `setMasmLabelExtension()` / `takeMasmLabel()` と `parseMasmLabelStatement()`。ドット無し識別子の**直後が方言自身のディレクティブ**か行末なら、それをラベルとして扱う。登録した拡張が無ければ無効なので GNU 構文には一切影響しない |
| C4 | `llvm/lib/MC/MCParser/AsmParser.cpp` | ドット無しディレクティブ探索を `lookupMasmDirective()` に抽出(重複していたループを 1 か所に) |
| C5 | `llvm/lib/MC/MCParser/ARMCOFFMasmParser.cpp` | `PROC`/`ENDP` が前置ラベルを名前として受け取るようにした。`ENDP` は開いている `PROC` との名前一致も警告する |
| C6 | 同上 | `DCD`/`DCW`/`DCB`/`DCQ`(`DCB` は文字列も可)、`SPACE`、`FILL n{,value}`、`EQU` を追加 |
| C7 | 同上 | `emitMasmLabel()` を override し、`ENDP`/`ENDFUNC`/`EQU` の前置ラベルは**出力しない**(`PROC` で出したシンボルの再定義になるため) |
| C8 | 同上 | **バグ修正**: `EXPORT name` の後に `name PROC` を書くと `PROC` 側の `setExternal(false)` が輸出を取り消していた。armasm の標準形なので、`EXPORT` 済みの名前は local に戻さないようにした |
| C9 | `llvm/test/MC/ARM/wince-armasm-labels.s` | 新規テスト。ラベル構文・`;` コメント・データ定義を固定 |
| C10 | `.github/workflows/main.yml` | 新規テストを lit ステップに追加 |
| C11 | `AsmLexer.h` / `AsmLexer.cpp` | `setLexArmasmIntegers()`: `&FF` [16進] / `%1010` [2進] / `n_xxxx` [基数 n] を受理。`0x` と 10 進は従来どおり |
| C12 | `ARMCOFFMasmParser.cpp` | `DCFS`/`DCFD` を追加。`AsmParser` は浮動小数点リテラルを常に IEEE double として読むので、`DCFS` はここで単精度に丸める |

### 12.1 設計メモ: なぜ「カラム 0」ではなく「直後のトークン」か

本物の armasm は「ラベルは第 1 カラム開始」という規則だが、`AsmLexer` には
「このトークンは行頭で始まったか」を外部から取る手段が無い
(`IsAtStartOfLine` はトークン消費時に必ずクリアされ、字句解析器全体に触れる
変更が必要)。そのため**直後のトークンが方言のディレクティブか行末か**で判定
している。

* 扱える: `Foo PROC` / `Foo ENDP` / `Foo DCD 1` / `Foo EQU 4` / 行末の `Foo`
* 扱えない: `Foo mov r0, r1`(ラベルと命令が同一行)。Platform Builder の
  ソースはラベルを単独行か `PROC`/データ系の前に置く書き方が殆どなので
  実害は小さいと判断したが、**完全な armasm 互換ではない**。

### 12.2 未実装(Path B) — **2026-09-01 更新: 下記は第 2 弾時点の記述**

実装済み: `DCBU`/`DCWU`/`DCDU`/`DCQU`/`DCFU`/`DCFSU`/`DCFDU`(※LLVM MC は
そもそもデータ出力を整列しない。`emitValue` に整列引数は無く、
`MCObjectStreamer::emitValueImpl` は行位置と範囲検査のみ = 「アラインメント未調整」
という差は**存在しない**。よって U 版は同値の名前別として実装)、
`GBLA`/`LCLA` 系宣言と `SETA`/`SETL` 代入、`IF`/`ELSEIF`/`ELSE`/`ENDIF`
(汎用 `.if` 系統名)。
今も未実装: `MACRO`/`MEND`、`WHILE`/`WEND`、`GET`/`INCLUDE`、`IF :DEF:`、
`SETS`、`EQU` の文字列・論理演算子、`name!n` 配列要素。うちマクロ処理系は
**無視せず診断**する(§15)。このためドライバは既定で armasm に切り替えない
(第 1 弾の判断を維持)。

### 12.3 追加で気をつけたい点

* `n_xxxx` の判定は「数字の直後に `_` があること」で行っている。基数が
  2〜16 の範囲外なら従来どおりの字句解析に流すので、`0x1F` や `1_000`
  (基数 1 は不正)の挙動は変わらない。
* `&` と `%` は `AsmToken::Amp` / `Percent` としても使われ得るが、
  armasm 時のみ・直後が有効な数字のときだけ数値として読む。
* `DCFS` の単精度変換は `APFloat::convert()` に依存。`DCFD` は
  `AsmParser` が既に double で読むのでそのまま 8 バイト書くだけで正しい。

## 13. Phase 3 (2026-08-30): 外部レビュー対応・lld/COFF の CE machine 整備

### 13.1 背景

実装コード・テストを横断して追った外部レビューが提出され、`IMAGE_FILE_MACHINE_ARM`(CE 専用)が lld/COFF・llvm の各 dispatch で ARMNT と同列に扱えていない箇所が P0/P1 で列挙された。各項目は HEAD ソースで逐一検証し、同意・一部同意を区分して対応した(判定は §13.3)。**本節の全修正はソースレベルで、ビルド・lit 実行は CI が初回検証者**(§13.7)。

### 13.2 CI 失敗の進行(2026-08-29 23:54 → 08-30 00:44、全 5 ラン failure)

| # | 失敗点 | 修正 |
|---|---|---|
| 1 | configure: ccache 未導入 | `7fdc653522`(workflow に導入を追加) |
| 2 | configure: mold 未導入(WinCE.cmake が要求) | `e064a67c4c` |
| 3 | build: `ninja: unknown target 'clang-cl'`(シンボリックリンクは ninja ターゲットでない) | `86a67bce24` |
| 4 | build: `ARMCOFFMasmParser.cpp:327 StringRef::equals` + `AsmParser.cpp:1732 ExtensionDirectiveHandler`(両方 HEAD で修正済みとソース確認) | `6b5cc2feb0` |
| 5 | build: **clang-tblgen** `DiagnosticDriverKinds.td:518: Diagnostics should not start with a capital letter; 'Windows' is invalid` | **本セッション**(§13.4-1) |

5 が唯一の現行ブロッカー: `warn_drv_wince_sysroot_missing` のメッセージが大文字 W 始まり。fork 追加診断 3 件中唯一の違反(残 2 件は `'` 始まり)。lit テストにこの文言の FileCheck 依存なし(確認済み)→ 文言の小文字化のみで安全。**ninja は初回失敗で停止するため、背後に潜在エラーが残る可能性は排除できない**(CI で再確認する)。

### 13.3 レビュー項目の判定(全て HEAD ソースで確認)

| 項目 | 判定 | 根拠(ファイル:行) |
|---|---|---|
| P0 LTO: bitcode → ARMNT 固定 | **同意・修正** | `BitcodeFile::getMachineType`(InputFiles.cpp:1460)が arm/thumb → ARMNT 固定。aarch64 は OS 判定があるのに arm だけなかった |
| P0 CE long-branch thunk 欠如 | **同意・一部修正** | `isInRange`(Writer.cpp:426)は ARMNT/arm64 のみ、`finalizeAddresses`(:666)で CE は早期 return、`getThunk`(:458)は arch switch のみ。apply 側は範囲外で `error("relocation out of range")`(Chunks.cpp:222-255)。既存 `armThunk` は **Thumb-2 の movw/movt + `add pc,ip`** で CE 既定(ARMv5TE)に流用不可 → 新規 CE スタブ実装。**ただし Thumb 側(T1/T32)caller の thunk は未対応**(§13.6-1) |
| P0 PE entry の Thumb bit | **一部同意・テスト固定** | 誤解を解く必要がある: CE は「シンボル値が bit0 を持つ」convention(WinCOFFObjectWriter.cpp:426-429 が CE のみ `Local->Data.Value \|= 1`)で、`DefinedCOFF::getRVA() = sectionRVA + sym->Value`(Symbols.h:225)が bit を保持 → **通常オブジェクトの entry/export は現状のコードで正しく odd/even になる**。ARMNT の `|= 1` は「値は plain(全関数 Thumb)」convention への補正。CE に無条件 `|= 1` を足すと ARM entry を壊す → 修正はしない。代わりに convention をテストで固定(wince-thumb-bit.s)。残る穴: リンカ合成シンボル / 未マークの asm entry(bit 欠落の余地) |
| P0 export の Thumb bit | **一部同意・テスト固定** | 同上(DLL.cpp:659-666、CE では `bit=0` の no-op で値がそのまま書かれる) |
| P1 import library に CE 欠如 | **同意・修正** | `getImgRelRelocation`(COFFImportFile.cpp:122)に `IMAGE_FILE_MACHINE_ARM` 無し → llvm_unreachable。llvm-dlltool `-m armce` は 0x1c0 を書く(WindowsMachineFlag.cpp:32)→ 読み込み時にクラッシュした経路 |
| P1 delay-load に CE 欠如 | **同意・診断化** | `newThunkChunk`/`newTailMergeChunk`(DLL.cpp:1071/1040)に ARM 無し → llvm_unreachable。**stub 実装はしない**: CE ローダーに delay-import 機構(delay-load helper / VEH 解決)が無いため、実装してもデバイスで動かない。`llvm_unreachable` → 明示 Fatal(診断)|
| P1 `.pdata` compact 化の 16-byte 前提 | **同意(維持・文書化)** | `sortCEExceptionTable`(Writer.cpp)は `total % 16 != 0` で**すでに Fatal**(黙って壊さない)。LLVM 自己生成の intermediate 以外の 8-byte CE pdata(手書き asm / 他ツール)を受け入れると in-place compact が壊れる → **閉ループ前提を維持**し、LLVM→LLVM で閉じる。on-disk は 16-byte stride のまま(末尾ゼロ)+ PE ディレクトリ長を 8*N にする設計(writeHeader)は維持 |
| P1 pdata reloc は改善済み | **同意** | B2/B3/B4 修正済み、wince-pdata.test が固定 |
| P1 EHABI/SEH 併存の限定性 | **同意(設計どおり)** | `functionUsesWinCFI`(ARMWinCFI.h)は MSVC_TableSEH/X86SEH の personality のみ。ターゲット横断で EHABI を切ると既存動作が壊れる(§4c 検証済)ため per-function gate が唯一安全。funclet なし SEH / 手書き asm SEH は未サポート(実装コメントで明記) |
| P1 ARM/Thumb interworking 的不備 | **同意・未対応(依存あり)** | entry/export/relocation は符号値 convention で一貫するが、**相対分岐の mode bit はセクション RVA の偶奇に依存**する。lld はセクションを偶数 RVA に配置するため、**Thumb セクションを奇数 RVA に配置するレイアウトが未実装**。これが無い限り -mthumb 代码の相対分岐は mode bit を失う(§13.6-1 で最重要残課題) |
| P2 x86 CE | **同意(非目標)** | TargetInfo + driver のみ、CE 特化 linker/runtime 経路は ARM 中心。**x86 CE は本ツールチェーンの対象外(非目標)**で、完了作業として扱わない(詳細: `utils/wince/README.md`「Scope and non-goals」) |
| P2 machine dispatch の全探索 | **同意・実施** | §13.5 |

### 13.4 本セッションの修正(commit 単位)

1. `[clang]` 診断メッセージの小文字化(CI ブロッカー)
2. `[lld][COFF]` LTO bitcode を `IMAGE_FILE_MACHINE_ARM` に判定(CE-aware)+ import library の `getImgRelRelocation` に CE 追加(同 concern で 1 commit)
3. `[lld][COFF]` CE 用の branch range-extension thunk: `RangeExtensionThunkARMCE`(jmp_arm_bytes 系 `ldr ip,[pc]`/`ldr pc,[ip]` + 絶対アドレス literal + HIGHLOW baserel、12 バイト、4-byte align)。**A32 caller(BRANCH24)のみ対象**: スタブは偶数地址に置かれ ARM mode で到達可能。`isInRange`/`finalizeAddresses`/`getThunk` に CE 分岐追加。テスト: `wince-range-thunk.s`(36MB の data セクションで A32 の ±32MB 範囲を外す)
4. `[lld][COFF]` entry/export の Thumb bit convention を固定するテスト: `wince-thumb-bit.s`(.thumb_func 関数の entry → odd RVA、ARM 関数 → even RVA、export 表も同じ)
5. `[lld][COFF]` CE で /delayload を指定したら明示 Fatal(llvm_unreachable 脱却)
6. `[CI]` 上記 2 テストを main.yml の lit 一覧に追加

### 13.5 machine dispatch 監査(ARMNT / isAnyArm64 の各 dispatch を CE 観点で判定)

| 箇所 | CE での挙動 | 判定 |
|---|---|---|
| `isInRange`(Writer.cpp:426) | CE 分岐を追加(本セッション) | 修正済み |
| `finalizeAddresses`(:666) | CE を対象に追加(本セッション) | 修正済み |
| `getThunk`(:458) | CE → 新規スタブ(本セッション) | 修正済み |
| entry point `|= 1`(:1929) | CE は符号値 convention で不要(ARM entry を壊す) | 維持 + テスト固定 |
| export `bit=1`(DLL.cpp:659) | 同上 | 維持 + テスト固定 |
| import thunk 選択(InputFiles.cpp:1237) | CE 分岐既存(ImportThunkChunkARMCE) | OK |
| `getImgRelRelocation`(COFFImportFile.cpp:122) | CE 追加(本セッション) | 修正済み |
| delay-load(DLL.cpp:1040/1071) | 明示 Fatal(本セッション) | 診断化 |
| `DelayAddressChunk::writeTo`(DLL.cpp:588) | CE の thunk は偶数地址 → bit 0 で正しい。CE delay-load はそもそも Fatal | 維持 |
| `/dynamicbase:no` 拒否(Driver.cpp:2434) | ARMNT/arm64 限定。CE は fixed base が正規 → **追加しない** | 維持 |
| `sortExceptionTables`(Writer.cpp:2882) | CE はその前の `config.wince` 分岐で return | OK |
| `sx \|= 1`(Chunks.cpp:267) | CE は raw 値使用(convention) | OK |
| `Baserel::getDefaultType` | machine 非依存(HIGHLOW/DIR64) | OK |
| `machineFromStr`/`machineToStr`(WindowsMachineFlag.cpp) | armce ↔ 0x1c0 両方向済み | OK |
| PDB の machine → CPUType(PDB.cpp:1340) | CE は既存で `CPUType::ARM7` にマップ済み(確認済み・OK) | OK |
| LTO `BitcodeFile::getMachineType`(InputFiles.cpp:1460) | CE 追加(本セッション) | 修正済み |
| `getFileFormatName`(COFFImportFile.cpp:36) | CE 名を `"COFF-import-file-ARMCE"`(`machineToStr` の armce 表記に一致)として報告。`llvm-dlltool -m armce` 生成 import lib への表示対応 | 修正済み(`fb4d8843856b` + テスト `32d24c0ae710`) |

### 13.6 残課題(優先度順)

1. **【最重要】Thumb caller のランジ拡張 veneer(Thumb→ARM BLX 遠距離 / Thumb-1)。修正**: 旧記述の「Thumb セクションを奇数 RVA に配置する」(odd VMA)は **mingw-w64/ARM EABI の ELF 規約であり、COFF/PE には不適用**(lld はセクションを `config->align`=4096 で整列し、奇数 RVA は物理的に不可能)。COFF/PE の interworking は**シンボル値の bit0**(Thumb マーカー、`WinCOFFObjectWriter` → `DefinedRegular::getRVA()`)で運ばれ、Thumb→Thumb の T32 BL は `applyBranch24T` の半語スケール(`v>>1`)が bit0 を正しく吸収するため**既に成立**している。真の未対応は (a) Thumb→ARM の BLX(遠距離、H ビット/mode 切替)、(b) Thumb-1(ARMv5TE 既定)のランジ拡張 veneer(movw/movt が無いため literal-pool `ldr`+`bx` 型を要し、エンコード・整列・interworking がハードウェア版数依存)。**ARM mode 既定の CE 開発(既定は ARM、`+noarm` は WinCE に未適用)には影響しない**。詳細は `WINCE_EVALUATION_VERIFICATION.md` §7.2/§7.3。
2. armasm Path B の残構文(§12.2: IF/ENDIF、MACRO/MEND、GBLA/SETA、DCFU 系、EQU 文字列/論理)。CI 緑化後。
3. ~~x86 CE の SEH 実装~~ → **非目標(スコープ外)**。x86 CE は本ツールチェーンの対象ではないため実装しない(B10 の診断はそのまま)。詳細は「Scope and non-goals」。
4. ~~PDB 生成時の CE CPUType マッピング・`getFileFormatName` の CE 名~~ → 解決済み: CPUType は既存で ARM7 にマップ済み(§13.5 確認)、`getFileFormatName` は `fb4d8843856b` + テスト `32d24c0ae710` で対応。
5. ~~`.actions` と `.github/workflows/main.yml` の重複解消~~ → 解消済み(`a9c0d082a` で死んだコピー両方削除)。
6. ~~mingwrt `5ed3cc4` 相当の push~~ → 済み(2026-08-31、リ-org 時に kagurasumusun/mingwrt へ push、§14)。
7. デバイス検証(未実施・デバイスなし。gweslab/cerf エミュ可能性は調査済)。
8. CI 緑化後の最初の WinCE lit 実行: 既存テスト(wince-pdata.test の手計算期待値等)+ 本セッション追加 2 件を初検証。

### 13.7 検証状態(正直な区切り)

- **本セッションの全コード修正はソースレベルのみ**: ビルド・リンク・実行・lit 実行はこの環境では実施していない。CI(push 後)が初回検証者。
- 本セッションで実施した検証: レビュー指摘の全箇所を HEAD ソースで直接確認(ファイル:行で引用)、apply 系 reloc 関数の全コード読取、thunk 機構(createThunks/verifyRanges/margin loop)の読取、既存テスト構造の読取、符号値 convention の伝播経路(WinCOFFObjectWriter → DefinedCOFF::getRVA)の確認。
- インストラクタ定数(`ldr ip,[pc]` = 0xE59FC000 等)は importThunkARMCE と同一 pattern の手計算 + binutils jmp_arm_bytes との一致で確定。**実行による確認は未実施**。

---

## 14. Phase 4 (2026-08-31): リポジトリ再編 — パイプラインを `cellvm-build` へ分離

### 14.1 方針(ユーザー決定)

- `llvm-project` には**サードパーティツリーを内蔵しない**(cegcc と同じ形にする)。
  mingwrt / w32api / pthread-win32 を `kagurasumusun` 各リポジトリから分離。
- **新規リポジトリ `kagurasumusun/cellvm-build`**(public、cegcc-build 型): サブモジュール + 1 つのビルドパイプライン + CI。
  サブモジュールは **4 つ**: `llvm-project@llvm-wince` + `mingwrt@master` + `w32api@wip` + `pthread-win32@master`
  (cegcc-build の前例: w32api + mingw + binutils + gcc。mingwrt もビルドに必須のため同様にサブモジュール化)。
- 社内向けコード(gmon / posix / include-overlay)とビルドスクリプト・CI はすべて `cellvm-build` へ。
- `llvm-project` にはコンパイラ側の変更・テスト・ドキュメントだけを残し、CI は **Stage 1 + WinCE lit ゲート**に削減。

### 14.2 各リポジトリへの push(2026-08-31、すべて fast-forward)

| リポジトリ | ブランチ | push 前 HEAD | push 後 HEAD | 内容 |
|---|---|---|---|---|
| kagurasumusun/mingwrt | master | `7c35691` | `69043bc` | (1) clang ビルド対応(Makefile.in の `-C` 除去 + `_mingw.h` の `__clang__` プローブ) (2) ヘッダの Clang/libc++ 互換(float.h / stdlib.h / `__small`) (3) coredll+coredll6 の def 完全化(30 名) (4) mingwex/wince の CE 数学系 8 種追加+Makefile 登録 (5) coredll_stubs.c の C17 対応 |
| kagurasumusun/w32api | wip | `51de0ad` | `7192b73` | libce/coredll.def の def 完全化(30 名・mingwrt と同一作業) |
| kagurasumusun/pthread-win32 | master | `06e7608` | `4ae6417` | (1) WinCE のスレッド作成を CreateThread/ExitThread 経路へ(_beginthreadex の WINCE ガード 5 ファイル) (2) GNU interlocked ブロックの x86 限定 (3) `_ptw32.h` の `__declspec` プローブに `__clang__` 追加 |
| kagurasumusun/llvm-project | llvm-wince | `ac4a5ca7d` | (進行中) | `a9c0d082a` CI を Stage 1+lit に削減(死んだ .actions/.github/actions コピー削除) → `696f2a437` wince-sysroot 削除(1841 ファイル)+ LLVM_SUPPORTED_RUNTIMES 登録 revert → `43d0852d6` utils/wince のビルドスクリプト群削除 → 本コミット(文書更新) |
| kagurasumusun/cellvm-build | master | (新規作成) | (進行中) | サブモジュール 4 つ + 社内向け sysroot コード + 適応済みスクリプト + フルパイプライン CI + README |

### 14.3 留意事項

- **並行セッション**: リ-org 実行中に別セッションが llvm-wince にコミットを重ねていた(zlib→libpng→pixman の Stage 5 修正群)。push ごとに `git fetch` → 必要なら rebase → fast-forward push で衝突を回避。**cellvm-build の llvm-project 固定は rebase 後の最終 tip に更新する**。
- **wince-sysroot のベンダリング差分はすべてコンポーネントリポジトリへ反映済み**: push 前の diff 検証で、各コンポーネントの push 後ツリーが旧ベンダーツリーと完全一致(CVS メタデータと README.llvmvendor.md を除く)を確認。
- **cellvm-build の CI がフルパイプラインの唯一の実行者**: llvm-project 側 CI はコンパイラゲートのみ。Stage 5(zlib→…→EasyRPG Player)の CI 反復は cellvm-build 側で継続。
- **認証**: push はすべてユーザー提供 PAT(一時的環境変数経由、記録なし)。

## 15. Phase 2 第 3 弾 (2026-09-01): armasm Path B の残構文を「既存経路」で実装

**方針**: 新しい機構を作らない。`DCxU`・`GBLA`・`SETA`・`IF` 系は
**汎用 AsmParser が既に持つディレクティブと同じ経路**で処理した。
- `addAliasForDirective` は `AsmParser::DirectiveKindMap` にキーを足すだけ
  (`lib/MC/MCParser/AsmParser.cpp` の実装は 1 行)。参照は
  **文の先頭トークンを lower しただけの文字列**で行われる
  (`parseStatement` 内 `DirectiveKindMap.find(IDVal.lower())`)ため、
  ドット無し `IF`/`ELSEIF`/`ELSE`/`ENDIF`/`IFDEF`/`ifndef` がそのまま汎用実装に届く。
  汎用の条件アセンブリは `TheCondState` で**入れ子と未実行分岐のスキップ**を管理し、
  その判定は拡張ディスパッチより**前**にある。よって armasm 側に
  スキップ機構は不要(誤って `.else`/`.endif` を無視ハンドラで潰すと入れ子が壊れる)。
- `DCxU` は汎用 `.byte/.short/...` 相当だが、**ラベル前置形**(`a DCDU 4`)は
  拡張の登録表(`lookupMasmDirective`)にしか登録されないため、拡張ハンドラとして
  実装し `parseDirectiveDataValue`/`parseDirectiveDCF` の薄いラッパにした。
- `GBLA`/`LCLA` は「宣言=0 代入」(= `.set` と同じ `emitAssignment`)、
  `SETA`/`SETL` は `name EQU expr` 用に**フォークが既に持っていた**
  `parseStatement` の代入キーワード分岐を 1 分岐に拡張して
  `parseAssignment(Kind::Set)` に流す。`EQU` は `Kind::Equiv` に変更し、
  `.set` と同じ「多重定義を弾かない」経路から `.equ` 相当の再定義エラーに修正
  (実測: `e EQU 1` + `e EQU 2` → `error: redefinition of 'e'`)。
**意図的に外した**もの: `SETB TRUE/FALSE`(汎用式は TRUE/FALSE を読めず、
通すと**黙って 0 になる**実測があった。よってキーワードから外し、既存のエラーにする)。
`IF :DEF:` も式として通らない(実測 `unknown token in expression`)ので、
同義の `IFDEF name` を別名で提供して回避経路を示す。
**マクロ処理系は診断化**: `IgnoreDirective` で黙って無視していた
`WHILE`/`WEND`/`MACRO`/`MEND`/`GET`/`INCLUDE`/`LTORG` は
`parseDirectiveNeedsMacroPass` が名指しでエラーに変換(無視は誤ったプログラムを
組み立てるため)。
**検証**: ローカルに ARM のみ llvm-mc/llvm-readobj/llvm-objdump/FileCheck を実ビルドし、
`test/MC/ARM/wince-armasm{,-labels,-data,-cond}.s` の RUN 行を全て実走して PASS。
新規テストの byte 期待値は**すべて objdump 実出力から機械生成**(手計算不可)。
CI のゲートリスト(`.github/workflows/main.yml`)にも 2 ファイルを追加済み。

---

## 16. (2026-09-01): 重複実装の監査と既存機構への統合

「既存の LLVM/Clang 機構で済むのに自前実装してしまった箇所」の全数監査
(fork HEAD ⇔ fork 直前の upstream 基底 `b91f74c49964` の全差分対比) の結果、
既存機構の複製になっていた 4 箇所を既存経路へ統合した。lit ゲート (31 件) が
挙動固定を持つため、CI が初回検証者。**マクロ集合・出力バイト列は不変**のはずで、
変更は構造のみ。

1. `[AsmPrinter]` `emitCESEHActionsForRange` (upstream `emitSEHActionsForRange` の
   複製, 非コメント行 25/36 が逐語同一) と CE 版 `emitCESpecificHandlerTable`
   (同 `emitCSpecificHandlerTable` の複製) を削除し、既存メンバに `IsCE` パラメータを
   追加して共用にした。CE 固有部分は次の 3 点だけ:
   (a) 親フレームオフセット = StackSize (x64 は `SEHSetFrameOffset`)
   (b) カウント語に `ce_handlerdata` ラベル (PDATA_EH の handler-data が指す位置)
   (c) エントリ絶対 ADDR32 — 32bit では `useImageRel32 == false` なので既存の
   `create32bitRef` がそのまま絶対を出す。ウォーカのラベル参照を `getLabel`
   (IMGREL 固定) から `create32bitRef` に変えたが、x64 では両者は同一式。
   `llvm::emitCESpecificHandlerTable` は 3 行ラッパー (WinException を構築して
   メンバ呼び出し) となり、`ARMAsmPrinter::emitCEHandlerData` 側は不変。
2. `[lld][COFF]` `isInRange` の CE 分岐から ARMNT と逐語同一だった 3 ケースを削除し、
   ARMNT と `IMAGE_FILE_MACHINE_ARM` で Thumb 範囲チェックを共用。真に CE 固有の
   `IMAGE_REL_ARM_BRANCH24` (A32 B/BL/BLX, PC+8 基準) だけを machine ガード付きで残した。
   ARMNT の挙動は不変 (BRANCH24 は ARMNT では発行されない)。
3. `[MC][AsmLexer]` `%1010` 2 進リテラル: 既存 `LexMotorolaIntegers` (`%01010110`) と
   同一処理の複製をやめ、`%` の 1 箇所に共用した。`&FF` / `n_xxxx` は upstream に
   対応形が無いため armasm 専用フラグ (`LexArmasmIntegers`) のまま。
4. `[clang][Targets]` ARM / x86 の WinCE ターゲットで二重定義されていた共通マクロ
   (`__CEGCC_VERSION__`, `__COREDLL__`, `__MINGW32__`, `WIN32`, `WINNT`, `_UNICODE`,
   `UNICODE`) を `addWinCEDefines` (OSTargets.cpp) の 1 箇所に統合。定義集合は不変
   (wince.c / wince-x86.c の -dM チェックが固定、順序非依存)。
5. 文書: armasm Path B の "Still missing" 一覧を現状に更新
   (MACRO/MEND/WHILE/WEND/GET/INCLUDE/LTORG は名指し診断、SETS/SETB と
   `IF :DEF:` リテラルは IFDEF/IFNDEF が正規の綴り)。

### 16.1 llvm-ml / MasmParser への統一 (F4) — 今回は実施せず、計画を記録

Path B (共有 AsmParser/AsmLexer の armasm 構文 + `ARMCOFFMasmParser`) は
MasmParser (llvm-ml) と「ラベル前置 + `;` コメント + 独自リテラル + EQU」を重複して
実装している。ただし今日削除すると動作中の機能 (armasm lit 4 件) が失われる。
移行の成立条件を実コードで確認済み:

* MasmParser 本体はほぼターゲット非依存 (x86 言及 2 箇所のみ、MASM32 組込みのゲート)。
  ファクトリ `createMCMasmParser` が存在し、llvm-ml にアーチ限定ゲートは無い
  (SafeSEH の x86 限定のみ)。
* だが (a) clang 統合アセンブラ / llvm-mc のパーサ選択は GNU AsmParser 固定で、
  `-masm=armasm` を MasmParser へ振る配管が無い、(b) `&FF`/`%1010`/`n_xxxx` は
  MasmParser のリテラル機構に無い (MASM は `0ABCh`/`0b1101`)、(c) AREA/DCD 等
  armasm 固有ディレクティブは MasmParser 上でも拡張登録が必要、(d) MASM の EQU/IF と
  armasm の意味論差異 (EQU の再定義規則等) の確認が要る。
* 結論: 複数コミット規模の移行プロジェクトであり、本節では実施しない。
  README の "right endgame" 記述のとおり償却する。着手時は clang のパーサ選択点の
  特定から始めること。

---

## 17. (2026-09-02): 重複・競合・衝突の一斉監査と根本修正

「場当たり・モグラ叩き禁止」の方針に基づき、4 リポジトリ横断で
**重複 (duplication) / 競合 (conflict) / 衝突 (collision)** を機械スキャンで
全列挙してから修正した。全てのマクロ名・ヘッダ名・シンボル名・include 綴りを
網羅するスクリプトで検出し、ヒットした候補を 1 件ずつ使用文脈でトリアージした。

### 17.1 衝突 (識別子/マクロ) — スキャンと結果

方法: w32api/include + mingwrt/include + sysroot/include-overlay の全ヘッダから
`#define __X` を 374 名抽出 × libc++/libc++abi/libunwind/compiler-rt(builtins,
include) の `__X` 識別子 9308 名と交差 → 12 候補を全て文脈確認。

| 候補 | 判定 | 根拠 |
|---|---|---|
| `__small` (w32api basetyps.h) | **実害 → 修正** | libc++ `__functional/function.h` のメンバ名。windows.h→ole2/objbase/rpcdce 連鎖で basetyps が読まれると `<functional>` が必ず壊れる (Stage 5 CI 実際に発生)。**w32api 側で `!defined(__clang__)` ガードを追加** — mingwrt `_mingw.h` に既に在った同一衝突の修正と同条件に統一 (前回は _mingw.h だけ直って basetyps が取りこぼされていた = モグラ叩き構造の実例)。in-tree の `__small` 使用はゼロ (grep 確認)。upstream LLVM 22.x/main も `__small` のままのため、**libc++ 側は変えない** (フォーク逸脱を増やさない) |
| `__TEXT` (winnt.h) | 無害 | libcxx/libunwind の使用は文字列リテラル (セクション名) とコメント内のみ。マクロは展開されない |
| `__except` (windef.h) | 無害 | `defined(__SEH_NOOP)` の opt-in 時のみ定義。既定では未定義 |
| `__cdecl`/`__declspec`/`__stdcall` (windef.h) | 無害 | clang の MS 互換ではコンパイラが事前定義し、`#ifndef` ガードが効く。ARM では呼出規約差もない。Stage 3/4 実ビルド緑 |
| `__int8..__int64` (winnt/_mingw/basetyps) | 無害 | clang MS モードではキーワード。マクロ展開先がキーワードと同義 (long long 等) |
| `__in` 等 SAL 群 (specstrings.h) | 潜在 (非発火) | libc++ 側の `__in` 使用は experimental/tzdb のみで CE では tzdb 無効。specstrings を include しない限り発火しない。要観察項目として記録 |
| `__MSVCRT_VERSION__`/`__need_wint_t`/`__WINCE__` | 協調 | libc++ の NT 分岐は `_LIBCPP_WIN32API` 非定義により CE で不活性。`__need_*` は C ヘッダとの正規の連携プロトコル |
| `__attribute__`/`__volatile__` (_mingw.h) | 無害 | 非GNUC/PCC ブランチのみで定義。clang は到達しない |

### 17.2 衝突 (include 綴り/ケース)

Linux ホストの sysroot はケース非同一の `<Windows.h>` を解決できない。
Player 公式 zip 403 ファイルの全 `#include` を sysroot ヘッダ集合と照合し、
不一致は **3 綴り**と確定: `Windows.h` (player.cpp, scene_gamebrowser.cpp —
修正不可の公式ソース)、`Shellapi.h` (player.cpp)、`Shlwapi.h`
(zip 同梱 wincehelper.cpp — オーバーレイが既に include 自体を除去済み)。
対処: (a) include-overlay に `Windows.h`/`Shellapi.h` 転送ヘッダを追加、
(b) `sysroot/gen-include-aliases.py` を新設し Stage 5 でアプリツリーを
走査して全未来ケースを機械検出・生成 (場当たり対応ではなく機構)。

### 17.3 重複 — 検出と処置

| 重複 | 処置 |
|---|---|
| `excpt.h` が mingwrt と w32api の双方に存在 (ガード名も相違: `_EXCPT_H_` vs `EXCPT_H`) | mingwrt 側 (x86 レガシー、`__try1` 系) を削除。windows.h/rpc.h/pkfuncs.h が include するのは w32api の ARM 形 DISPATCHER_CONTEXT 版で、フラット sysroot ではこれが最終勝者だった。単一ソースに統一 |
| `coredll{,4,6}.def` が mingwrt と w32api/libce に二重管理 | 現状 3 ファイルとも byte 一致を確認。mingwrt を正とし、**sysroot ビルドに byte-equality ガードを追加** (ドリフトで即失敗)。audit-coredll.py が実機ダンプとの三重照合を継続 |
| sysroot/include-overlay/intrin.h | 削除。clang リソースの intrin.h が同一スペルで先に解決されるため到達不能な死にコピーだった (sal.h は clang リソースに無いため overlay が正規経路として存続) |
| posix shim (libposix) ∩ coredll_stubs の `raise` | **意図されたレイヤリング** (CRT 既定=失敗スタブ、libposix=協調実装への upgrades。STATUS "Runtime layering" 通り) であり重複不具合ではない。記録のみ |
| cellvm-build workflow Package の `arm-pc-wince-*` ラッパーと bind-cegcc-names.sh の `arm-mingw32ce-*` | 名称集合も消費者も別 (tarball vs CeGCC 互換 Makefile)。重複ではない |
| armasm Path B の MasmParser 重複 (§16.1) | 引き続き記録通り償却 (複数コミット規模。今回の範囲で実施すると動作中の armasm lit 4 件をフルビルドなしに危険に晒すため) |

### 17.4 競合 (Git/CI 運用)

| 競合 | 処置 |
|---|---|
| cellvm-build の workflow トリガーが `branches: [main]` のまま、実 branch は `wince` のみ → **`wince` push で CI が一度も走らない** | トリガーを `wince` に変更 |
| commit `28713096` (wincehelper.cpp オーバーレイ copy 追加) が全 branch から到達不能の dangling (GitHub 保持 ~90 日) | SHA 指定で復旧し `wince` に反映。**同コミットだけでは不十分だったことも判明**: copy された先で zip 側 wincehelper.cpp:147 の `(void*)0` が C++ 型エラー → `(wchar_t*)0` に修正 |
| Stage 2 スモークが C 言語専用 (`int main(void)` のみ) で、windows.h + libc++ のヘッダ衝突を CI が検出できなかった (上記 `__small` を見逃した構造的原因) | Stage 3 に「C++ header-collision smoke」ステップを新設 (`windows.h` + `<functional>` + `<string>` + `<map>` + `Windows.h` エイリアス経由) |

### 17.5 検証状態

* ソースレベル: 上記全判定は fork HEAD の実ファイルと CI ログ実文による。
* ローカル実行検証: gen-include-aliases.py は実 zip + 実ヘッダ集合で実走し
  期待 3 綴りを検出; シェル/YAML/Python は構文検証済み。
* ビルド・リンク・Stage 5 緑化: **push 後の CI が初回検証者** (llvm 側 lit ゲート →
  cellvm-build フルパイプライン)。実機実行は従来通り未検証。
