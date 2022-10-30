
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

//  const char *DummySrc = R"(
//     define i18 @f(###args###) {
//     ###dbg.val###
//       ret i8 0
//     }
//     declare void @llvm.dbg.value(metadata, metadata, metadata) #0
//     attributes #0 = { nounwind readnone speculatable willreturn }

//     !1 = !DIFile(filename: "dummy.cpp", directory: "/die2dwarf")
//     !2 = !DIBasicType(name: "unsigned int", size: 8, encoding: DW_ATE_unsigned)
//     !3 = distinct !DISubprogram(name: "f", linkageName: "_f", scope: !1, file: !1, line: 11, type: !2, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
//     !###src_var_num### = !DILocalVariable(name: "source_variable", arg: 1, scope: !3, file: !1, line: 2, type: !2)
// )";
namespace die2dwarf {

class DIExpressionParse {
public:
  std::unique_ptr<llvm::Module> GetModuleFromIR(llvm::StringRef DIExpr);

private:

const char *Part0 = R"(
    define i18 @f(
)";

// Immediately after the DIArgList.
const char *Part1 = R"(
) {

)";

// Insert the complete "call @dbg.value... etc here.

// Post call @dbg.declare
// For now the source variable metadata is always !4.
// TODO: support any number and perform a find & replace or similar op t construct

const char *Part2 = R"(
    ret i8 0
    }
    declare void @llvm.dbg.value(metadata, metadata, metadata) #0
    attributes #0 = { nounwind readnone speculatable willreturn }

    !1 = !DIFile(filename: "dummy.cpp", directory: "/die2dwarf")
    !2 = !DIBasicType(name: "unsigned int", size: 8, encoding: DW_ATE_unsigned)
    !3 = distinct !DISubprogram(name: "f", linkageName: "_f", scope: !1, file: !1, line: 11, type: !2, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
    !4 = !DILocalVariable(name: "comp", scope: !13, file: !1, line: 22, type: !12)
)";

   
  bool EmbedDIExprInDummySrc();

  std::string ConstructedSrc;
  // Avoid recreating these.
  llvm::LLVMContext Ctx;
  llvm::SMDiagnostic SMD;
};

}