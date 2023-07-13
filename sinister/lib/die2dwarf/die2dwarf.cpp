

#include <iostream>
#include <map>
#include <memory>

//#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "die2dwarf/DIExpressionParse.h"
#include "die2dwarf/ModuleToDWARF.h"

#include "sinister/utils/FileReader.h"

 const char *DummySrc = R"(
    define void @fun(i32 %lsr.iv, i32 %multiplicand) !dbg !5 {
      call void @llvm.dbg.value(metadata !DIArgList(i32 %lsr.iv, i32 %multiplicand), metadata !6, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_mul, DW_OP_stack_value)), !dbg !7
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
    !9 = !{null, !12, !12}
    !10 = !DIBasicType(name: "unsigned int", size: 8, encoding: DW_ATE_unsigned)
    !11 = !DIBasicType(name: "unsigned int", size: 16, encoding: DW_ATE_unsigned)
    !12 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
    !13 = !DIBasicType(name: "unsigned int", size: 64, encoding: DW_ATE_unsigned)
)";

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

 die2dwarf::DIExpressionParse DP;

 // Embed the llvm.dbg.value in a dummy module.
 // SourceCode = std::string(DummySrc);
 if (!die2dwarf::CreateModuleSource(&SourceCode)) {
    llvm::dbgs() << "Couldn't merge string into source";
    return -1;
 }

 std::unique_ptr<llvm::Module> M = DP.GetModuleFromIR(SourceCode);
 die2dwarf::ModuleToDWARF MTD(*M);
 MTD.print();

 return 0;
}
