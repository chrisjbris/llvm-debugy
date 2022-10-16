


#include "purify/DIExpressionParse.h"

#include "llvm/AsmParser/LLParser.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>

std::unique_ptr<llvm::Module>
DIExpressionParse::GetModuleFromIR(llvm::StringRef Src) {

 // llvm::dbgs() << " Source is: \n" << Src << "\n\n";

  std::unique_ptr<llvm::Module> M = llvm::parseAssemblyString(Src, SMD, Ctx);

  if (!M) {
    SMD.print("DebugInfoTest", llvm::errs());
  }
  else {
      llvm::dbgs() << *M;
  }



  return M;
}
