//===-- DbgValToModule.h ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file the class that embeds a dbg.value in a dummy program so that it
// can be read as LLVM assembly and later translated to pure DWARF.
//===----------------------------------------------------------------------===//

 // To use parseAssemblyString() to read the DIArgList and DIExpression into
 // their in-memory representations, a valid LLVM IR program is required. The
 // following is the basis for that 'dummy' program'.
 // Chris: I don't think the type of the source variable matters.
 // Variables to replaces:
 // ###args### - The DIArgList
 // ##src_var_num### - The metadata number of the source variable
 const char *DummySrc = R"(
    define i18 @f(###args###) {
    ###dbg.val###
      ret i18 0
    }
    declare void @llvm.dbg.value(metadata, metadata, metadata) #0
    attributes #0 = { nounwind readnone speculatable willreturn }

    !1 = !DIFile(filename: "dummy.cpp", directory: "/die2dwarf")
    !2 = !DIBasicType(name: "unsigned int", size: 8, encoding: DW_ATE_unsigned)
    !3 = distinct !DISubprogram(name: "f", linkageName: "_f", scope: !1, file: !1, line: 11, type: !2, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
    !###src_var_num### = !DILocalVariable(name: "source_variable", arg: 1, scope: !3, file: !1, line: 2, type: !2)
)";

class DbgValToModule { 

    
};