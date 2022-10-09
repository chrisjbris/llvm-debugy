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
#include <optional>
#include <string>
#include <vector>

/// Parse the DWARF expression string and encode the result. Returns nullopt
/// and prints errors to stderr if there are any problems.
std::optional<std::vector<uint8_t>>
parseDwarfExpression(std::string const &Input);
