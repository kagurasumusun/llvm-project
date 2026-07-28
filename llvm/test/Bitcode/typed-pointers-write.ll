; Verify that -opaque-pointers=0 makes the bitcode writer emit legacy
; (LLVM 16 format) TYPE_CODE_POINTER records with reconstructed pointee
; types, and that the default path still emits opaque pointer records.
;
; RUN: llvm-as -opaque-pointers=0 %s -o - | llvm-bcanalyzer -dump | FileCheck %s --check-prefix=TYPED
; RUN: llvm-as %s -o - | llvm-bcanalyzer -dump | FileCheck %s --check-prefix=OPAQUE
; RUN: llvm-as -opaque-pointers=0 %s -o - | llvm-dis -o - | FileCheck %s --check-prefix=DIS

target datalayout = "e-p:64:64"

%struct.Pair = type { i32, ptr }
%struct.Res = type { ptr addrspace(1) }

@seed = global i32 42
@pair = global %struct.Pair { i32 7, ptr @seed }

declare ptr @handler(i32)

define ptr @get_seed() {
  ret ptr @seed
}

define i32 @main() {
entry:
  %pstruct = alloca %struct.Pair
  %fld = getelementptr %struct.Pair, ptr %pstruct, i32 0, i32 1
  store ptr @seed, ptr %fld
  ret i32 0
}

; ------- typed-pointer emission (-opaque-pointers=0) -------
; Type table is enumerated in first-encounter order; the record operands
; below reference absolute type IDs within this module:
;   T0=i32  T1=i32*  T2=%struct.Pair={i32,i32*}  T3=%struct.Pair*
;   T4=i8  T5=i8*  T6=i8*(i32)  T7=T6*  T8=()->i32*  T9=T8*
;   T10=()->i32  T11=T10*  T14=(i32*)*
;
; TYPED:      <NUMENTRY
; @seed slot = i32*:
; TYPED:      <POINTER{{.*}} op0=0 op1=0/>
; struct name is preserved from the opaque-IR name:
; TYPED:      record string = 'struct.Pair'
; %struct.Pair = { i32, i32* } -- field pointee recovered from @pair's
; initializer and from the store through the GEP in @main:
; TYPED:      <STRUCT_NAMED{{.*}} op0=0 op1=0 op2=1/>
; TYPED:      <POINTER{{.*}} op0=2 op1=0/>
; declared @handler has no return-pointee evidence: i8 fallback:
; TYPED:      <FUNCTION{{.*}} op0=0 op1=5 op2=0/>
; @get_seed's return pointee is i32, recovered from 'ret ptr @seed':
; TYPED:      <FUNCTION{{.*}} op0=0 op1=1/>
; @main:
; TYPED:      <FUNCTION{{.*}} op0=0 op1=0/>
; slot of the GEP into field 1 of %struct.Pair: (i32*)*
; TYPED:      <POINTER{{.*}} op0=1 op1=0/>

; ------- default (opaque) emission is unchanged -------
; OPAQUE-NOT: <POINTER{{.*}} op0={{[0-9]+}} op1=
; OPAQUE:     <UnknownCode25
; OPAQUE:     record string = 'struct.Pair'
; OPAQUE-NOT: <POINTER{{.*}} op0={{[0-9]+}} op1=

; ------- reader upgrades typed bitcode back to opaque pointer IR -------
; DIS: %struct.Pair = type { i32, ptr }
; DIS: @pair = global %struct.Pair { i32 7, ptr @seed }
; DIS: define ptr @get_seed()
; DIS:   ret ptr @seed
