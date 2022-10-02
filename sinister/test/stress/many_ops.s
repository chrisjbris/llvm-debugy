# RUN: sinister %s --print-result # | FileCheck %s


# Insert a very long DWARF program here. It is intended to stress the functions
# that convert text programs to byte vectors and the interpreter.

# CHECK: Result Value: 42
