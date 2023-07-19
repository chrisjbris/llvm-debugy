
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

#include "die2dwarf/ModuleToDWARF.h"

#include <iostream>

namespace die2dwarf {




void ModuleToDWARF::print() {
  if (!Expr) {
    assert(Expr && "Expected non-null Expr.");
    return;
  }

 std::cout << "Printed the DWARFS!\n";

 // Element-wise print the DIExpr, with special cases for DW_OP_LLVM_*.

}

} // namespace die2dwarf