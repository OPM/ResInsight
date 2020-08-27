# Find CppCheck.
#
# This module defines:
#  CPPCHECK_PROGRAM, the cppcheck executable.
#  CPPCHECK_FOUND, If false, do not try to use cppcheck.
#
find_program(CPPCHECK_PROGRAM NAMES cppcheck)

find_package_handle_standard_args(CppCheck DEFAULT_MSG CPPCHECK_PROGRAM)

mark_as_advanced(CPPCHECK_PROGRAM)
