//===-- sinister.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the sinister DWARF interpreter.
//
//===----------------------------------------------------------------------===//
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

#include "llvm/Support/CommandLine.h"
#include "sinister/DwarfExecutor.h"
#include "sinister/utils/FileReader.h"
#include "sinister/sinister.h"
#include "sinister/TextToDwarf.h"
#include <iomanip>
#include <iostream>

static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                                llvm::cl::desc("<source file>"),
                                                llvm::cl::Required);

// For early debugging.
static llvm::cl::opt<bool> PrintInput(
    "printinput",
    llvm::cl::desc(
        "After the input source file has been read, print it to stdout."));
static llvm::cl::alias PrintInputA("p", llvm::cl::desc("Alias for -printinput"),
                                   llvm::cl::aliasopt(PrintInput));

void test();
auto main(int argc, const char **argv) -> int {
  llvm::cl::ParseCommandLineOptions(argc, argv);

  std::string SourceCode;
  if (!FileToString(InputFilename, SourceCode)) {
    std::cout << "Failed to read file: '" << InputFilename << "\n";
    return 1;
  }

  if (InputFilename != "") {
    std::cout << "Input file: '" << InputFilename << "'\n";
    if (PrintInput)
      std::cout << "Contents:\n===\n" << SourceCode << "\n===\n";
  } else {
    std::cout << "Expected an input file.\n";
  }

  std::vector<uint8_t> *DemoProg = new std::vector<uint8_t>();
  {
    // std::string DemoExpression =
    //     "DW_OP_const1u(1) DW_OP_const1u(2) DW_OP_plus DW_OP_stack_value";
    auto Result = parseDwarfExpression(SourceCode);
    if (!Result.has_value()) {
      std::cerr << "Failed parseDwarfExpression(): \n'" << SourceCode << "'\n";
      return -1;
    }
    *DemoProg = std::move(Result.value());
  }

  sinister::DwarfExecutor DE;
  DE.SetProgram(DemoProg);

  // Note - If the expected is not checked before it goes out of scope, then
  // there is an exception. So best to check immediately.
  llvm::Expected<lldb_private::Scalar> Result = DE.EvaluateProgram();
  if (!Result) {
    std::cerr << "Failed EvaluateProgram(): \n";
    for (uint8_t Ch: *DemoProg)
        std::cerr << "0x" << std::setfill('0') << std::setw(2) << std::hex
                  << (unsigned)Ch << " ";
    std::cerr << "\n";
    return -1;
  }

  // TODO: start using the llvm output methods
  std::cout << "Result: " << Result->UInt() << "\n\n";
  return 0;
}
