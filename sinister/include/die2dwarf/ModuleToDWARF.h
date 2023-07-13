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

// Given a synthetic module created with a string dbg.value, enable printing
// of the DIExpression in pure DWARF instead of LLVM extended-DWARF.

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

#include <iostream>

 namespace die2dwarf {

class ModuleToDWARF { 
public:
  ModuleToDWARF(llvm::Module &Module): M(Module) {
    // Find the dbg.value & point to the expression.
    std::cout << "\n\n\n Synthetic module:\n";
    
    auto Fun = Module.getNamedValue("fun");
    assert(Fun && "Could not find function in module.");
    auto F = cast<llvm::Function>(Fun);
    auto I = &F->front().front();
    auto *DbgVal = cast<llvm::DbgValueInst>(I);
    Expr = DbgVal->getExpression();
  }

    void print();

private:
    llvm::Module &M;
    llvm::DIExpression *Expr;
  };

 } // namespace die2dwarf