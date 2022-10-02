from inspect import getsourcefile
import os
import re
import subprocess

import lit
import lit.formats
import lit.llvm
import lit.util

from lit.llvm.subst import FindTool
from lit.llvm.subst import ToolSubst

# name: The name of this test suite.
config.name = 'SINISTER'
config.suffixes = ['.c', '.cpp', '.test', '.s']

# The root path where tests are located. This property is necessary for test
# discovery when using the 'check-all' target.
config.test_source_root = os.path.dirname(__file__)
