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

#include "lldb/Expression/DWARFExpression.h"

#include "lldb/Core/Value.h"
#include "lldb/Core/dwarf.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-forward.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/CodeGen/ScheduleDAGInstrs.h"
#include "llvm/IR/DerivedTypes.h"

#include "sinister/DwarfExecutor.h"
#include <vector>

namespace sinister {

bool DwarfExecutor::SetProgram(std::vector<uint8_t> *Data) {
  if (Data == nullptr)
    return false;

  if (Data->empty())
    return false;

  Program.reset(std::move(Data));
  return true;
}

llvm::Expected<lldb_private::Scalar> DwarfExecutor::EvaluateProgram(
    lldb::ModuleSP module_sp = {}, DWARFUnit *unit = nullptr,
    lldb_private::ExecutionContext *exe_ctx = nullptr) {

  if (Program->empty() || (*Program).empty()) {
    const lldb_private::Status::ValueType V = 24;
    ErrStatus.SetError(V, lldb::ErrorType::eErrorTypeGeneric);
    ErrorMsg = "Unexpected empty program.";
    ErrStatus.SetErrorString(ErrorMsg.c_str());
    return ErrStatus.ToError();
  }

  if (!lldb_private::DWARFExpression::Evaluate(
          exe_ctx, /*reg_ctx*/ nullptr, module_sp, Extractor, unit,
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