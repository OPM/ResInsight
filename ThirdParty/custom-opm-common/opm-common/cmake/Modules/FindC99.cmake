# - Module that checks for supported C99 features.

# macro to only add option once
include (AddOptions)

# try to use compiler flag -std=c99
set (C_STD99_FLAGS "-std=c99")

# incidently, the C++ test is so simple that it can be used to compile C as well
include (CheckCCompilerFlag)
check_c_compiler_flag (${C_STD99_FLAGS} HAVE_C99)

# add option if we are capable
if (HAVE_C99)
  add_options (C ALL_BUILDS "${C_STD99_FLAGS}")
else (HAVE_C99)
  set (C_STD99_FLAGS)
endif (HAVE_C99)

# handle quiet and required
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (C99
  DEFAULT_MSG
  C_STD99_FLAGS
  )
