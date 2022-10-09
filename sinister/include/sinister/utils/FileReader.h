

//===-- FileReader.h ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines functions that can be used to read files.
//
//===----------------------------------------------------------------------===//

#include <string>

bool FileToString(std::string FileName, std::string &Dest);

