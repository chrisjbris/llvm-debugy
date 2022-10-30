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
 namespace purify {

class DbgValToModule { 
public:
    bool ParseDbgVal(StringRef DvgVal);

private:
    llvm::Module *M;

};

 } // namespace purify