//===-- FileReader.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines functions to read files.
//
//===----------------------------------------------------------------------===//

#include "sinister/utils/FileReader.h"
#include <string>
#include <fstream>
#include <streambuf>

bool FileToString(std::string FileName, std::string &Dest) {
    std::ifstream stream(FileName);
    if (!stream.good())
        return false;

    Dest = "";
    stream.seekg(0, std::ios::end);
    Dest.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);

    Dest.assign((std::istreambuf_iterator<char>(stream)),
                std::istreambuf_iterator<char>());
    return true;
}
