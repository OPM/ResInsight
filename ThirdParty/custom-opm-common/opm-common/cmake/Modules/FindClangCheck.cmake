# Find clang-check.
#
# This module defines:
#  CLANGCHECK_PROGRAM, the clang-check executable.
#  CLANGHCECK_FOUND, If false, do not try to use cppcheck.
#
find_program(CLANGCHECK_PROGRAM NAMES clang-check clang-check-3.8)

find_package_handle_standard_args(ClangCheck DEFAULT_MSG CLANGCHECK_PROGRAM)

mark_as_advanced(CLANGCHECK_PROGRAM)
