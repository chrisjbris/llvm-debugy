# RUN: sinister %s | FileCheck %s

# CHECK: Result: 42

DW_OP_const1u(38) DW_OP_const1u(4) DW_OP_plus DW_OP_stack_value
