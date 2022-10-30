


#include "die2dwarf/DIExpressionParse.h"

#include "llvm/AsmParser/LLParser.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>

namespace die2dwarf {
std::unique_ptr<llvm::Module>
DIExpressionParse::GetModuleFromIR(llvm::StringRef Src) {

  std::unique_ptr<llvm::Module> M = llvm::parseAssemblyString(Src, SMD, Ctx);

  if (!M) {
    SMD.print("DebugInfoTest", llvm::errs());
  }
  else {
      llvm::dbgs() << *M;
  }

  return M;
}

} // namespace die2dwarf 