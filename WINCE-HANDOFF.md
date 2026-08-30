# Windows CE × LLVM/Clang ツールチェーン — 完全引き継ぎ資料 (HANDOFF)

> 最終更新: 2026-08-30 (Phase 2) / ブランチ: `llvm-wince` / HEAD: `23644c7075`
>
> **NOTE (2026-08-30):** §6.1 was rewritten and §11 added during Phase 2.
> Everything dated before that describes an older tree: the
> `EncodingType::CE` triple emitter in `MCWin64EH.cpp` mentioned there was
> **removed** (`84aec8c1`), `EmitCE()` no longer exists, and ARM SEH is
> implemented through a completely different (per-function) route.  Read
> §11 first, then the rest.
> この文書はセッション引き継ぎ用。後続のエージェントまたは人間が**この文書 alone で**作業を継続できることを目標に書かれている。

---

## 0. 30秒サマリ

- **目的**: `kagurasumusun/llvm-project`(LLVM 22.1.8ベースのフォーク)を、**Windows CE (CE 6.0中心、5.0/4.x/3.0/2.xまで設定で対応) の完全なクロス開発ツールチェーン**にすること。
- **方針**: 独自ランタイムは作らない。**kagurasumusun/mingwrt + kagurasumusun/w32api(CeGCC系)+ pthreads4w を in-tree ベンダーし、それぞれ自身の configure/make で clang を使ってビルドする**。LLVM/Clang/LLD 側の改修は「正規の機構」として in-tree で行う。
- **現状**: アプリ/ドライバ/OAL 開発に必要な一式はほぼ実装済み・検証済み(下記)。OS イメージ(sysgen/makeimg/BIB)のビルド基盤は **AE600 ソース(ユーザー手元)が前提**で、実装計画は確定済み・未実装。
- **直近のブロッカー**: ユーザーの GitHub private リポ `kagurasumusun/wince-source` に**トークンからアクセスできない**(404)。トークンへのリポ追加待ち。ARM32 WinEH (SEH) 実装の参考ソースがそこにある。

---

## 1. リポジトリ/ブランチ/認証の現状

| 項目 | 値 |
|---|---|
| ワークスペース | なし(サンドボックス制約のためクローンを作らず GitHub REST API で作業。2026-08-30 時点) |
| ブランチ | `llvm-wince`(デフォルトブランチ。**作業対象はこれ**) |
| リモート | `origin` = `https://github.com/kagurasumusun/llvm-project.git` |
| HEAD | `23644c7075` (Phase 2 のコミット適用後) |
| 注意 | `arena/wince-wineh-ce` は **diverged (ahead 7 / behind 35)**。SEH を別方式(ターゲット全体 WinEH)で実装した未統合分岐。詳細は §11.3 |
| プッシュ | **全コミット push 済み**(`Everything up-to-date` 状態) |
| upstream tag | `refs/tags/upstream-22.1.8`(差分確認用に fetch 済み) |

### 認証(重要な罠)

- ユーザー提供の fine-grained トークン(`github_pat_11CD5...`)は **`llvm-project` のみ有効**。
  - `llvm-project` への push/blob 作成: ✅
  - `kagurasumusun/mingwrt`, `pthread-win32` への書き込み: ❌ 403(過去に拒否された経緯あり → ユーザーの指示で「他人リポはいじるな」になり、pthread への直接コミットは撤回済み)
  - `kagurasumusun/wince-source`(private): ❌ **404 = トークンの Repository access に含まれていない**
- サンドボックスにはシステム側の GitHub 認証(ボット)もあり、`git push origin` は `llvm-project` に限定して動く。**トークンを URL 埋め込みで push すると bot 認証が優先され 403 になる**ことがある → `llvm-project` 以外への push は REST API 経脈か、ユーザー自身が行う。
- **ユーザーへの依頼中の事項**:
  1. fine-grained トークンに `wince-source` を追加(Contents: Read で可)
  2. `kagurasumusun/mingwrt` にローカルコミット `5ed3cc4` "Build with LLVM/Clang in addition to GCC" を push(clang対応のため。llvm-project 側のベンダーツリーには同変更済みなので、push は整合性のためだけに必要)

---

## 2. ディレクトリマップ(追加・変更したもの)

```
wince-sysroot/                    ← 登録済み LLVM ランタイムプロジェクト本体
├── CMakeLists.txt                ランタイム登録(LLVM_ENABLE_RUNTIMES=wince-sysroot)
├── include-overlay/              sysroot include に最後に重ねるヘッダ(SAL, intrin.h)
├── mingwrt/                      kagurasumusun/mingwrt@7c35691 + clang対応コミット相当
│                                 (include/_mingw.h __declspecプローブに __clang__ 追加、
│                                  Makefile.in の .def 前処理から -C 除去、
│                                  coredll_stubs.c setlocale引数の名前化)
├── w32api/                       kagurasumusun/w32api@51de0ad 無修正
├── pthread-win32/                GerHobbelt/pthread-win32@06e7608(=上流HEAD)+ WinCE修正3点
│                                 (詳細は同dir README.llvmvendor.md)
├── gmon/                         -pg サンプリングプロファイラ (gcrt3.c, libgmon.c)
└── posix/                        execv/execl(p)/system/waitpid/popen/pclose/signal/alarm
    └── sys/wait.h

utils/wince/
├── build-wince-sysroot.sh        ステージ2(sysroot組み立て)本体。検証済み
├── build-wince-runtimes.sh       ステージ3(compiler-rt/libunwind/libcxx)ドライバ
├── audit-coredll.py              dumpbin出力 vs def 突合ツール
├── armasm/armasm-convert.py      armasm→GNU 変換器(フォールバック用。後述)
└── README.md                     ★全仕様・監査表・検証状況の一次資料(必読)

clang/cmake/caches/WinCE.cmake    ステージ1(ホストビルド)キャッシュ
clang/test/Driver/wince.c         ドライバ lit テスト(更新済)
clang/test/Driver/wince-x86.c     i386-mingw32ce テスト
clang/test/Driver/wince-ctors.c?  → 実ファイルは clang/test/CodeGen/ARM/wince-global-ctors.c
clang/test/CodeGen/ARM/wince-*.c  EHABI テーブル / グローバル ctor テスト
clang/test/Parser/ms-pragmas-full.c  MSVCプラグマ実装テスト
clang/test/Sema/ms-extern.c       extern extern 等の MSVC 構文テスト
lld/test/COFF/wince-*.s           lld の CE 挙動テスト(image/ctors/relocs/thunk/exidx)
llvm/test/MC/ARM/wince-*.s        MCレベルのCEテスト
.actions/build-wince-llvm.yml     CI(ユーザーが .github/workflows/main.yml へ移動済み)
```

削除したもの: `wince-crt/`(旧ベスポークCRT)、`.gitmodules`+`third-party/*` サブモジュール(すべて in-tree 化)、`utils/wince/patches/`(不要化)。

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
- SAL: `wince-sysroot/include-overlay/sal.h` を新規作成(`_In_/_Out_` 系 + 旧 `__in` 系の no-op 定義)
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
- lit テスト群はソース上更新済みだが、**lit 実行は未**(ステージ1が必要)

### 未検証/未実施

1. **ステージ1(LLVM/Clang自体のビルド)** — このサンドボックス(2コア/3GB)では不可能。CI(`.actions/build-wince-llvm.yml` → ユーザーが `.github/workflows/main.yml` に移動済み)で実行される想定
2. **ステージ3の実ビルド** — 同上
3. **実機テスト** — 一切未実施。旧 `wince-crt/docs/DEVICE-TESTING.md` は削除済み(必要なら履歴から復元)
4. coredll dump の残チャンク2/3/5/6の `audit-coredll.py` 検収(チャンク0/1/4/7は手動照合済み。2/3/5/6も現在は全部照合済みという記載に更新するのを忘れていた可能性 → **要確認**: 実際には §3.10 の通り全9チャンク照合・欠落30追加が完了している)

---

## 5. 重要な設計判断と理由(後続者が再検討しないよう記録)

1. **ベスポークCRTを廃止した理由**: ユーザー指摘どおり、mingwrt/w32api をそのまま使う方が正しい。wince-crt は削除済み
2. **サブモジュール廃止 → in-tree vendoring**: ユーザー指摘。LLVMの構造ではランタイムが自身のソースを所有(compiler-rt と同型)
3. **`third-party/` ではない場所に置く**: ユーザー指摘で `wince-sysroot/` ランタイムプロジェクト直下に移動
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

- [ ] `utils/wince/README.md` の「COREDLL def completeness」節: 「chunks 0/1/4/7 spot-check」の記述を「全9チャンク照合完了・欠落30追加・残ゼロ」に更新(§3.10 の通り作業は完了済み。README の記述だけ古い箇所が残っている可能性)
- [ ] `.actions/build-wince-llvm.yml` と `.github/workflows/main.yml` の重複解消(ユーザーが main.yml に移動済み。`.actions` 側は残置。ユーザー判断で削除可と伝える)
- [ ] `kagurasumusun/mingwrt` への push リマインド(コミット `5ed3cc4`。現在は llvm-project 内ベンダーに同内容があるためブロッカーではない)
- [ ] `wince-sysroot/posix/` の `signal.c` — VEH(`AddVectoredExceptionHandler`)が CE カーネルに存在するか coredll.def に追記するかの検討(フォルト起因 SIGSEGV 対応のため)
- [ ] errno のスレッド対応(TlsAlloc が解禁されたので実装可能に。ただしランタイム意味論の変更なので要承認)

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
utils/wince/build-wince-sysroot.sh \
  --toolchain <install>/bin --target arm-pc-wince \
  --prefix <install>/wince-sysroot --jobs $(nproc)
```

スモークスタンドイン(本ツールチェーン無しで動作確認するトリック):
zig bundled clang を `/tmp/fake-tc/bin/{clang,llvm-ar,llvm-ranlib,llvm-dlltool}` にラッパーとして置き、`/tmp/wince-defines.h`(CEプリディファイン集)を `-include` する。過去の検証はすべてこの方法。詳細は git 履歴の会話ログではなく、スクリプト自体を見れば分かる構造。

### ステージ3(ランタイム)

```bash
utils/wince/build-wince-runtimes.sh --toolchain <install>/bin \
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
5. ユーザー: 「サブモジュールではなく正規の llvm&clang の一部として」→ in-tree vendoring(third-party は不適切と指摘され wince-sysroot/ へ)
6. ユーザー: 「断線経路を全部確認」「SEH/デストラクタが壊れている」→ 各種修正
7. ユーザー: 「-pg 実装して」「posix 実装して」「armasm 正規実装して」「C17 更新して」→ 実施
8. ユーザー: 「AE600 があるので OS ビルド基盤も自前実装可能では?」→ 実装計画策定(未実装)
9. ユーザー: 「wince-source から参考ソースを取得して WinEH 実装して」→ **トークン権限不足でブロック中**

---

## 10. 関連ドキュメント

- `utils/wince/README.md` — 一次資料(最も詳細)。矛盾があればこちらを優先し本資料を更新
- `wince-sysroot/*/README.llvmvendor.md` — ベンダー証跡
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
2. armasm Path B の完全化。**第 2 弾で主要部分を実装済み(§12)**。
   残りは `%`/`&`/`n_` 数値リテラル、`DCFS`/`DCFD`、`GBLA`/`SETA`、
   `MACRO`/`MEND`、`IF`/`ENDIF`。実用経路は
   `utils/wince/armasm/armasm-convert.py`(Path A)のまま。
3. x86 CE の SEH(B10 で診断するようにしただけ。実装は未)。
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

### 12.2 未実装(Path B)

アラインメント未調整の `DCFU`/`DCFSU`/`DCFDU`/`DCQU`、`EQU` の文字列・
論理演算子、`GBLA`/`SETA`、`MACRO`/`MEND`、`IF`/`ENDIF`。このためドライバは
既定で armasm に切り替えない(第 1 弾の判断を維持)。

### 12.3 追加で気をつけたい点

* `n_xxxx` の判定は「数字の直後に `_` があること」で行っている。基数が
  2〜16 の範囲外なら従来どおりの字句解析に流すので、`0x1F` や `1_000`
  (基数 1 は不正)の挙動は変わらない。
* `&` と `%` は `AsmToken::Amp` / `Percent` としても使われ得るが、
  armasm 時のみ・直後が有効な数字のときだけ数値として読む。
* `DCFS` の単精度変換は `APFloat::convert()` に依存。`DCFD` は
  `AsmParser` が既に double で読むのでそのまま 8 バイト書くだけで正しい。
