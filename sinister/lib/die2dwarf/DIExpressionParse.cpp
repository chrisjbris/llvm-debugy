


#include "die2dwarf/DIExpressionParse.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/AsmParser/LLParser.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Regex.h"

#include <map>
#include <memory>

namespace die2dwarf {

 const char *DummySrc = R"(
    define void @fun($args) !dbg !5 {
      call void $dbg.value , !dbg !7
      ret void
    }
    declare void @llvm.dbg.value(metadata, metadata, metadata) #0
    attributes #0 = { nounwind readnone speculatable willreturn }

    !llvm.dbg.cu = !{!0}
    !llvm.module.flags = !{!1, !2}

    !0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang version 14.0.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
    !1 = !{i32 7, !"Dwarf Version", i32 4}
    !2 = !{i32 2, !"Debug Info Version", i32 3}
    !3 = !DIFile(filename: "dummy.cpp", directory: "/die2dwarf")
    !4 = !DIBasicType(name: "unsigned int", size: 8, encoding: DW_ATE_unsigned)
    !5 = distinct !DISubprogram(name: "f", linkageName: "_f", scope: !3, file: !3, line: 11, type: !8, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
    !6 = !DILocalVariable(name: "source_variable", arg: 1, scope: !5, file: !3, line: 2, type: !10)
    !7 = !DILocation(line: 0, scope: !5)
    !8 = !DISubroutineType(types: !9)
    !9 = $subroutine_types
    !10 = !DIBasicType(name: "unsigned int", size: 8, encoding: DW_ATE_unsigned)
    !11 = !DIBasicType(name: "unsigned int", size: 16, encoding: DW_ATE_unsigned)
    !12 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
    !13 = !DIBasicType(name: "unsigned int", size: 64, encoding: DW_ATE_unsigned)
)";

std::unique_ptr<llvm::Module>
DIExpressionParse::GetModuleFromIR(llvm::StringRef Src) {

  std::unique_ptr<llvm::Module> M = llvm::parseAssemblyString(Src, SMD, Ctx);

  if (!M)
    SMD.print("DebugInfoTest", llvm::errs());
  else
      llvm::dbgs() << *M;

  return M;
}

void GetSubroutineTypes(std::string *ArgList, llvm::SmallVectorImpl<llvm::StringRef> *Types) {
    // Match all the "iX" or "iX*" and create the types list for the subroutine

const char *ArgTypes = R"((i[0-9]+[\*]*)*)";
llvm::Regex RexArgTypes(ArgTypes);
std::string Error;

  if (!RexArgTypes.match(*ArgList, Types, &Error)) {
      llvm::dbgs() << "Couldn't match the argument list type declarations in '" << *ArgList << "'. Error: " << Error
                   << "\n";
  }
}



bool CreateModuleSource(std::string *Expr) {
  
  std::string Error;
  llvm::dbgs() << "Test contents: " << *Expr <<  "\n";

  // Extract the dbg.value statement from the test so it can be inserted into
  // the dummy module.
  const char *DbgValue = R"(@llvm\.dbg\.value\(.*\))";
  llvm::Regex RexDbgValue(DbgValue);
  llvm::SmallVector<llvm::StringRef, 1> M;

  if (!RexDbgValue.match(*Expr, &M, &Error)) {
      llvm::dbgs() << "Couldn't find dbg.value(). Error: " << Error
                   << "\n";
      return false;
  }
  auto DbgValueCall = std::string(M[0]);
  llvm::dbgs() << "Test intrinsic call: " << DbgValueCall << "\n";

  // Extract the argument list and place that inside the empty module at $args
  const char *ArgList = R"(!DIArgList\(.*\),)";
  llvm::Regex RexArgList(ArgList);
  llvm::SmallVector<llvm::StringRef, 1> Matches;

  if(!RexArgList.match(*Expr, &Matches, &Error)) {
    llvm::dbgs() << "Couldn't find DIExpression arguments. Error: " << Error <<  "\n";
    return false;
  }

  std::string Arguments = std::string(Matches[0]);
  Arguments = Arguments.substr(11, Arguments.length()-13);
  llvm::dbgs() << "DIE Args: " << Arguments <<  "\n";

  llvm::SmallVector<llvm::StringRef, 1> Types;
  GetSubroutineTypes(&Arguments, &Types);



  // Create the argument types as

  // Next, add the entire "llvm.dbg.value(...) string within the function at
  // $dbg.value
  llvm::Regex PlaceArgs(R"(\$args)");
  llvm::Regex PlaceIntrinsic(R"(\$dbg.value)");
  llvm::Regex SubRoutineTypeInsertion(R"(\$subroutine_types)");
  std::string SubRoutineTypeMetadata = "!{null";   
  std::map<std::string, std::string> TypeIndexes {{"i8","!10"}, {"i16","!11"}, {"i32","!12"}, {"i64","!13"}};
  for (const auto &T : Types) {
    SubRoutineTypeMetadata = SubRoutineTypeMetadata + ", " + TypeIndexes[std::string(T)];

  }

  SubRoutineTypeMetadata += "}";
  std::string Src = std::string(DummySrc);
  std::string TypedSrc = SubRoutineTypeInsertion.sub(SubRoutineTypeMetadata, DummySrc, &Error);
  std::string Source = PlaceArgs.sub(Arguments, TypedSrc, &Error);
  std::string Complete = PlaceIntrinsic.sub(DbgValueCall, Source, &Error);
  *Expr = Complete;
  return true;
}

} // namespace die2dwarf 