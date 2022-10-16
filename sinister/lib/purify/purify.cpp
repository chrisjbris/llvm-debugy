

#include <iostream>
#include <map>
#include <memory>

#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "purify/DIExpressionParse.h"
#include "purify/purify.h"

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


 const char *MinimalSrc = R"(
      call void @llvm.dbg.value(metadata i16 %b, metadata !9, metadata !DIExpression())
)";

 DIExpressionParse DP;

 std::unique_ptr<llvm::Module> M = DP.GetModuleFromIR(SourceCode);

 return 0;
}
