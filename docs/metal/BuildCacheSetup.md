# ビルドキャッシュ運用メモ (Metal/typed-pointer 作業用)

このリポジトリのビルド検証はsandbox上で行う。環境(コンテナ)が再作成されると
`/tmp` やビルドディレクトリが失われることがあるため、以下の2段構えにする。

## 構成

| 項目 | 場所 | 説明 |
| --- | --- | --- |
| ビルドディレクトリ | `/tmp/llvm-cfg-check` | cmake + Ninja。-j2, Release, Assertions ON, clang 有効 |
| ccache バイナリ | `/home/user/.local/bin/ccache` | ソースから自前ビルドした 4.10.2 |
| ccache キャッシュ | `/home/user/.ccache` | 100MB・zstd 圧縮 (workspace スナップショットに載る場所) |
| ccache 設定 | `/home/user/.config/ccache/ccache.conf` | `cache_dir` 等 |
| ccache ソース | `/tmp/ccache-build` | 再ビルド用 (zstd 静的ライブラリ含む) |

cmake は `CMAKE_C_COMPILER_LAUNCHER` / `CMAKE_CXX_COMPILER_LAUNCHER` に
ccache へのフルパスを設定済み。全コンパイルが自動でキャッシュされる。

## 環境リセット後の復旧手順

1. cmake/ninja が無ければ reinstall:
   `python3 -m pip install --user --break-system-packages cmake ninja`
   (`~/.local/bin` が残っていれば不要)
2. ccache が無ければ再ビルド (~1分):
   - `/tmp/ccache-build` が残っている場合:
     `cd /tmp/ccache-build && cmake --install build-cc`
   - 跡形もない場合 (github codeload は到達可、Debian mirror は遮断):
     ```
     cd /tmp && curl -sLO https://codeload.github.com/facebook/zstd/tar.gz/refs/tags/v1.5.6
     curl -sLO https://codeload.github.com/ccache/ccache/tar.gz/refs/tags/v4.10.2
     tar xzf v1.5.6 && tar xzf v4.10.2 && mv zstd-1.5.6 zstd && mv ccache-4.10.2 ccache
     make -C zstd/lib libzstd.a -j2
     cmake -S ccache -B build-cc -DCMAKE_BUILD_TYPE=Release \
       -DREDIS_STORAGE_BACKEND=OFF -DENABLE_DOCUMENTATION=OFF -DENABLE_TESTING=OFF \
       -DZSTD_INCLUDE_DIR=$PWD/zstd/lib -DZSTD_LIBRARY=$PWD/zstd/lib/libzstd.a \
       -DCMAKE_INSTALL_PREFIX=$HOME/.local
     cmake --build build-cc -j2 && cmake --install build-cc
     ```
3. ビルドディレクトリが消えていたら再 configure:
   ```
   cmake -S /home/user/llvm-project/llvm -B /tmp/llvm-cfg-check -G Ninja \
     -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=clang \
     -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_INCLUDE_TESTS=OFF \
     -DLLVM_ENABLE_ASSERTIONS=ON \
     -DCMAKE_C_COMPILER_LAUNCHER=$HOME/.local/bin/ccache \
     -DCMAKE_CXX_COMPILER_LAUNCHER=$HOME/.local/bin/ccache
   ```
   キャッシュが残っていれば再ビルドはヒットして大幅に速い。

## 検証用クイックコマンド

```
B=/tmp/llvm-cfg-check/bin
# LLVM 側 (typed pointer writer) — 単体オブジェクトで十分な場合
ninja -C /tmp/llvm-cfg-check LLVMBitWriter llvm-as llvm-dis llvm-bcanalyzer
# lit テスト相当 (FileCheck 直叩き)
$B/llvm-as -opaque-pointers=0 llvm/test/Bitcode/typed-pointers-write.ll -o - \
  | $B/llvm-bcanalyzer -dump | $B/FileCheck llvm/test/Bitcode/typed-pointers-write.ll --check-prefix=TYPED
# clang 側
ninja -C /tmp/llvm-cfg-check clang   # フルビルドは時間がかかるので注意
```
