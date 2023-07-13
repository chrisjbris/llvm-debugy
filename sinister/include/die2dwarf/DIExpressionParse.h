
// Wrap the llvm asm/IR parsing methods and provide any dummy objects so that
// a DIEpression and DIArglist can be parsed and then tranlated to pure DWARF.
// More specifically, the LLVM DWARF extensions can be replaced with DWARF
// spec conformant operators and operands, ready to be passed to sinister
// for interpretation.
#include <memory>

#include "llvm/ADT/StringRef.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/AsmParser/LLParser.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

// Provide a minimal base IR program that can be augmented with the locations
// and DWARF program from the user input. Currently LLVM only supports parsing
// a complete program, so we must construct one, parse it, then extract the
// relevant DWARf components.



namespace die2dwarf {

  bool CreateModuleSource(std::string *Expr);

class DIExpressionParse {
public:
  std::unique_ptr<llvm::Module> GetModuleFromIR(llvm::StringRef DIExpr);

private:



   
  bool EmbedDIExprInDummySrc();

  std::string ConstructedSrc;
  // Avoid recreating these.
  llvm::LLVMContext Ctx;
  llvm::SMDiagnostic SMD;
};

}