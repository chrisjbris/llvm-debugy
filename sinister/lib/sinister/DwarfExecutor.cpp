//===-- DWARFExecutor.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines functions that can be used for parallel code generation.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/ArrayRef.h"
#include "llvm/CodeGen/ScheduleDAGInstrs.h"
#include "llvm/IR/DerivedTypes.h"


#include "lldb/Core/Value.h"
#include "lldb/Core/dwarf.h"
#include "lldb/Expression/DWARFExpression.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-forward.h"
#include "lldb/Utility/DataExtractor.h"

#include "sinister/DwarfExecutor.h"

#include <cstdint>
#include <vector>

namespace sinister {

bool DwarfExecutor::SetProgram(std::vector<uint8_t> *Data) {
  if (Data == nullptr)
    return false;

  if (Data->empty())
    return false;

  Program.reset(Data);
  return true;
}


// Based on /ldb/unittests/Expression/DWARFExpressionTest.cpp.
llvm::Expected<lldb_private::Scalar> DwarfExecutor::EvaluateProgram(
    lldb::ModuleSP module_sp, DWARFUnit *unit,
    lldb_private::ExecutionContext *exe_ctx) {

  if (Program->empty() || (*Program).empty()) {
    const lldb_private::Status::ValueType V = 24;
    ErrStatus.SetError(V, lldb::ErrorType::eErrorTypeGeneric); //TODO: correct params so this error is useful
    ErrorMsg = "Unexpected empty program.";
    ErrStatus.SetErrorString(ErrorMsg.c_str());
    return ErrStatus.ToError();
  }

  // TODO: check these are the settings for DWARF
  Extractor.reset();
  Extractor = std::make_unique<lldb_private::DataExtractor>(
      Program->data(), Program->size(), lldb::eByteOrderLittle,
      /*address_size_bytes*/ 4);

  if (!lldb_private::DWARFExpression::Evaluate(
          exe_ctx, /*reg_ctx*/ nullptr, module_sp, *Extractor, unit,
          lldb::eRegisterKindLLDB,
          /*initial_value_ptr*/ nullptr,
          /*object_address_ptr*/ nullptr, Result, &ErrStatus))
    return ErrStatus.ToError();

  switch (Result.GetValueType()) {
  case lldb_private::Value::ValueType::Scalar:
    return Result.GetScalar();
  case lldb_private::Value::ValueType::LoadAddress:
    return LLDB_INVALID_ADDRESS;
  case lldb_private::Value::ValueType::HostAddress: {
    // Convert small buffers to scalars to simplify the tests.
    lldb_private::DataBufferHeap &buf = Result.GetBuffer();
    if (buf.GetByteSize() <= 8) {
      uint64_t val = 0;
      memcpy(&val, buf.GetBytes(), buf.GetByteSize());
      return lldb_private::Scalar(
          llvm::APInt(buf.GetByteSize() * 8, val, false));
    }
  }
    LLVM_FALLTHROUGH;
  default:
    return ErrStatus.ToError();
  }
}

} // namespace sinister