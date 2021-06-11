# - Remove duplicate library declarations
#
# Synopsis:
#
#	remove_duplicate_libraries (module)
#
# where
#	module         Name of the module whose libraries should be pruned

# Copyright (C) 2013 Uni Research AS
# This file is licensed under the GNU General Public License v3.0

# libraries should always be trimmed from the beginning, so that also
# missing functions in those later in the list will be resolved
macro (remove_duplicate_libraries module)
  if (DEFINED ${module}_LIBRARIES)
	list (REVERSE ${module}_LIBRARIES)
	list (REMOVE_DUPLICATES ${module}_LIBRARIES)
	list (REVERSE ${module}_LIBRARIES)
  endif (DEFINED ${module}_LIBRARIES)
endmacro (remove_duplicate_libraries module)

# headers can be trimmed from the end, since adding a directory to
# the list is an idempotent action
macro (remove_duplicate_var module suffix)
  if (DEFINED ${module}_${suffix})
	list (REMOVE_DUPLICATES ${module}_${suffix})
  endif (DEFINED ${module}_${suffix})
endmacro (remove_duplicate_var module suffix)

# fix up both headers and libraries, in case two dependencies have
# included the same second-level library independently
macro (remove_dup_deps module)
  remove_duplicate_var (${module} INCLUDE_DIRS)
  remove_duplicate_var (${module} LINKER_FLAGS)
  remove_duplicate_var (${module} CONFIG_VARS)
  remove_duplicate_libraries (${module})
endmacro (remove_dup_deps module)
