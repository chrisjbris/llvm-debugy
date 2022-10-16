# RUN: purify %s | FileCheck %s

# The value of the locations does not matter. Purify just translates to a DWARF
# spec program. It will be the job of sinister to set, iterate and chec values.

@llvm.dbg.value(metadata !DIArgList(i64 %lsr.iv, i32 %multiplicand), metadata !2, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_mul, DW_OP_stack_value))