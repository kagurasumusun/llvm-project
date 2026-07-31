// Entry point metadata schema.
//
// The expected shapes are transcribed from Apple's own output for an
// equivalent shader: research/golden/P01/metal32_macosx26/probe.ll for the
// kernel form and P02 for vertex and fragment.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

// `uint` and the vector types come from Apple's <metal_stdlib>, not from the
// compiler; the reference AST dumps show them as ordinary typedefs. They are
// declared here so the test does not need the real standard library.
typedef unsigned int uint;
typedef unsigned short ushort;

kernel void probe_kernel(device float *b [[buffer(0)]],
                         uint i [[thread_position_in_grid]]) {
  b[i] = 1.0f;
}

// Entry points keep the default C calling convention. All 701 modules of
// Apple's shipping runtime use it; no special convention exists.
// CHECK: define void @probe_kernel(

// ---------------------------------------------------------------------------
// Module level metadata (emitted before entry point metadata).
//
// In LLVM IR text output module flags (!llvm.module.flags) are emitted
// *before* named metadata (!air.version etc.), so CHECK-DAG for flags
// must come before CHECK for air.version.  See CGMetal.cpp: addModuleFlag
// calls precede getOrInsertNamedMetadata, and the printer serialises in
// the same order.
// ---------------------------------------------------------------------------

// Resource limits, emitted as module flags.  Order-independent across flags.
// CHECK-DAG: !{i32 7, !"air.max_device_buffers", i32 31}
// CHECK-DAG: !{i32 7, !"air.max_textures", i32 128}
// CHECK-DAG: !{i32 7, !"air.max_samplers", i32 16}

// air.version follows the deployment target; air.language_version follows
// -std=.  The named metadata !air.version may or may not appear depending on
// LLVM printer behaviour; the data nodes are always present as numbered
// metadata.
// CHECK-DAG: !{i32 2, i32 8, i32 0}
// CHECK-DAG: !{!"Metal", i32 3, i32 2, i32 0}

// Every Apple Metal module identifies the frontend that produced it
// (measured across the whole reference IR corpus).
// CHECK-DAG: !llvm.ident = !{![[IDENT:[0-9]+]]}
// CHECK-DAG: ![[IDENT]] = !{!"Apple metal version 32023.883 (metalfe-32023.883)"}

// ---------------------------------------------------------------------------
// Entry point metadata (emitted after module metadata).
// ---------------------------------------------------------------------------

// The kernel entry-point metadata node {function, <empty>, <arguments>}.
// CHECK-DAG: !{{{.*}}@probe_kernel, !{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-DAG: !{}

// A device buffer argument carries a location index, an access mode, an
// address space and its size and alignment.  CHECK-DAG avoids coupling the
// test to the order LLVM emits numbered vs named metadata.
// CHECK-DAG: !"air.buffer", !"air.location_index", i32 0, i32 1
// CHECK-DAG: !"air.address_space", i32 1
// CHECK-DAG: !"air.arg_name", !"b"

// A stage builtin carries no location index.
// CHECK-DAG: !"air.thread_position_in_grid"
// CHECK-DAG: !"air.arg_name", !"i"
