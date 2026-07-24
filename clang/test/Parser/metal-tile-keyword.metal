// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

tile void tile_keyword_entry() {}
[[tile]] void tile_attr_entry() {}
