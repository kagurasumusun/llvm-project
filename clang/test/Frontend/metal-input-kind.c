// RUN: %clang_cc1 -triple air64-apple-macosx10.15 -x metal -std=metal2.0 -fsyntax-only %s
// RUN: %clang_cc1 -triple air32-apple-ios10.3 -x metal -std=metal1.0 -fsyntax-only %s

// This file intentionally avoids Metal-specific syntax. It verifies that the
// frontend recognizes the Metal input kind, Metal language standards, and AIR
// target triples.
