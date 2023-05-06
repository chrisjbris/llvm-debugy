


#include "die2dwarf/DIExpressionParse.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/AsmParser/LLParser.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Regex.h"

#include <memory>

namespace die2dwarf {

 const char *DummySrc = R"(
    define i18 @f($args) {
    call void $dbg.value
      ret i8 0
    }
    declare void @llvm.dbg.value(metadata, metadata, metadata) #0
    attributes #0 = { nounwind readnone speculatable willreturn }

    !1 = !DIFile(filename: "dummy.cpp", directory: "/die2dwarf")
    !2 = !DIBasicType(name: "unsigned int", size: 8, encoding: DW_ATE_unsigned)
    !3 = distinct !DISubprogram(name: "f", linkageName: "_f", scope: !1, file: !1, line: 11, type: !2, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
    !4 = !DILocalVariable(name: "source_variable", arg: 1, scope: !3, file: !1, line: 2, type: !2)
)";

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

bool CreateModuleSource(std::string *Expr) {
  
  // First extract the argument list and place that inside the empty module at $args
  const char *Rex = R"(!DIArgList\(.*\),)";
  llvm::Regex ArgsExpr(Rex);
  llvm::SmallVector<llvm::StringRef, 1> Matches;
  std::string Error;
  bool Matched = ArgsExpr.match(*Expr, &Matches, &Error);

  if(!Matched) {
    llvm::dbgs() << "Couldn't find DIExpression arguments. Error: " << Error <<  "\n";
    return false;
  }

  // for(const auto &Str: Matches) {
  //   llvm::dbgs() << "Match: " << Str <<  "\n";
  // }
  std::string Arguments = std::string(Matches[0]);
  Arguments = Arguments.substr(11, Arguments.length()-13);
  llvm::dbgs() << "DIE Args: " << Arguments <<  "\n";

  llvm::Regex Intrinsic(R"(@llvm\.dbg\.value\(.*\))");
  

  // Next, add the entire "llvm.dbg.value(...) string within the function at $$$dbg.value
  llvm::Regex PlaceArgs(R"(\$args)");
  llvm::Regex PlaceIntrinsic(R"(\$dbg.value)");
  std::string Src = std::string(DummySrc);
  std::string Source = PlaceArgs.sub(Arguments, Src, &Error);
  llvm::dbgs() << "Arguments placed: " << Source <<  "\n";
  std::string Complete = PlaceIntrinsic.sub(*Expr, Source, &Error);
  llvm::dbgs() << "Final source:" << Complete <<  "\n";
  return true;
}

} // namespace die2dwarf 