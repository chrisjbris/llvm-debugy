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
#include <vector>

#include "sinister/DwarfExecutor.h"
#include "sinister/sinister.h"




auto main(int argc, const char **argv) -> int {

//  {const1-byte, 1, const1-byte, 2, add, stack_value}
std::vector<uint8_t> *DemoProg = new std::vector<uint8_t>({0x08, 0x0a, 0x08, 0x02, 0x22, 0x9f});

    sinister::DwarfExecutor DE;
    DE.SetProgram(DemoProg);

    // Note - If the expected is not checked before it goes out of scope, then
    // there is an exception. So best to check immediately.
    llvm::Expected<lldb_private::Scalar> Result = DE.EvaluateProgram();
    if(!Result)
        return -1;

    // TODO: start using the llvm output methods
    std::cout << "Result: " << Result->UInt() << "\n\n";

    return 0;
}