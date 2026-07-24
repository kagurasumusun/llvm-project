// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

tile void tile_keyword_entry() {}
[[tile]] void tile_attr_entry() {}

// CHECK-DAG: !air.tile = !{![[TILE0:[0-9]+]], ![[TILE1:[0-9]+]]}
// CHECK-DAG: ![[TILE0]] = !{ptr @{{.*}}tile_keyword_entry{{.*}}, !{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-DAG: ![[TILE1]] = !{ptr @{{.*}}tile_attr_entry{{.*}}, !{{[0-9]+}}, !{{[0-9]+}}}
