

#include <iostream>
#include <map>
#include <memory>

#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "die2dwarf/DIExpressionParse.h"

#include "sinister/utils/FileReader.h"

static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                                llvm::cl::desc("<source file>"),
                                                llvm::cl::Required);

auto main(int argc, const char **argv) -> int {
  llvm::cl::ParseCommandLineOptions(argc, argv);

  std::string SourceCode;
  if (!FileToString(InputFilename, SourceCode)) {
    std::cout << "Failed to read file: '" << InputFilename << "\n";
    return 1;
  }


//const char 

 const char *MinimalSrc = R"(
      call void @llvm.dbg.value(metadata i16 %b, metadata !9, metadata !DIExpression())
)";

 die2dwarf::DIExpressionParse DP;

 // Embed the llvm.dbg.value in a dummy module.
 if(!die2dwarf::CreateModuleSource(&SourceCode)) {
  llvm::dbgs() << "Couldn't merge string into source";
  return -1;
 }

 std::unique_ptr<llvm::Module> M = DP.GetModuleFromIR(SourceCode);

 return 0;
}
