//===-- TextToDwarf.h ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides functions that convert a text program to bytes that define
// a DWARF program. Also provide some checking to ensure the text and bytes are
// valid.
//
//===----------------------------------------------------------------------===//
#include <stdlib.h>
#include <string>
#include <vector>


std::vector<uint8_t> StringToBytes(std::string &Input);