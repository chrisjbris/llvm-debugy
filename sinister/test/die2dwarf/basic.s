; RUN: die2dwarf %s | FileCheck %s



@llvm.dbg.value(metadata !DIArgList(i32 %lsr.iv, i32 %multiplicand), metadata !6, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_mul, DW_OP_stack_value))