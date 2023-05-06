; RUN: die2dwarf %s | FileCheck %s

; The value of the locations does not matter. Purify just translates to a DWARF
; spec program. It will be the job of sinister to set, iterate and check values.

; Note: Must use metadata !4 for the source variable atm as this is hardcoded,
;       for now.

@llvm.dbg.value(metadata !DIArgList(i64 %lsr.iv, i32 %multiplicand), metadata !4, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_mul, DW_OP_stack_value))