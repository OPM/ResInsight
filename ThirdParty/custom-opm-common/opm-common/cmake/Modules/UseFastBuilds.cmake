# - Try to build faster depending on the compiler

# faster builds by using a pipe instead of temp files
include (AddOptions)
include (UseCompVer)
is_compiler_gcc_compatible ()

if (CXX_COMPAT_GCC)
	add_options (ALL_LANGUAGES ALL_BUILDS "-pipe")
endif ()

