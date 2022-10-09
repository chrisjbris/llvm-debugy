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

#include "sinister/DwarfExecutor.h"
#include "sinister/sinister.h"
#include "sinister/TextToDwarf.h"


auto main(int argc, const char **argv) -> int {

    std::vector<uint8_t> DemoProg;
    {
        std::string DemoExpression =
            "DW_OP_const1u(1) DW_OP_const1u(2) DW_OP_plus DW_OP_stack_value";
        auto Result = parseDwarfExpression(DemoExpression);
        if (!Result.has_value())
            return -1;
        DemoProg = std::move(Result.value());
    }

    sinister::DwarfExecutor DE;
    DE.SetProgram(&DemoProg);

    // Note - If the expected is not checked before it goes out of scope, then
    // there is an exception. So best to check immediately.
    llvm::Expected<lldb_private::Scalar> Result = DE.EvaluateProgram();
    if(!Result)
        return -1;

    // TODO: start using the llvm output methods
    std::cout << "Result: " << Result->UInt() << "\n\n";

    return 0;
}