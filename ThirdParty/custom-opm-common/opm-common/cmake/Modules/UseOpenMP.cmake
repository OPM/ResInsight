# - Use OpenMP features
#
# Synopsis:
#
#	find_openmp (module)
#
# where:
#
#	module            Name of the module to which OpenMP libraries
#	                  etc. should be added, e.g. "opm-core".
#
# Note: Compiler flags are always added globally, to avoid ABI
# incompatibility problems.
#
# It is assumed that the following variables are available
#
#	${module}_QUIET      Verbosity level of the parent's find module
#	${module}_LIBRARIES  List of libraries to which OpenMP will be added
#
# Example:
#	find_openmp (opm-core)
#	remove_dup_deps (opm-core)

include (AddOptions)
include (UseCompVer)
is_compiler_gcc_compatible ()

macro (find_openmp opm)
  # default is that OpenMP is not considered to be there; if we set this
  # to a blank definition, it may be added but it cannot be removed if
  # it propagates to other projects (someone declares it to be part of
  # _CONFIG_VARS)
  set (HAVE_OPENMP)

  # user code can be conservative by setting USE_OPENMP_DEFAULT
  if (NOT DEFINED USE_OPENMP_DEFAULT)
	set (USE_OPENMP_DEFAULT ON)
  endif (NOT DEFINED USE_OPENMP_DEFAULT)
  option (USE_OPENMP "Enable OpenMP for parallelization" ${USE_OPENMP_DEFAULT})
  if (USE_OPENMP)

  # enabling OpenMP is supposedly enough to make the compiler link with
  # the appropriate libraries
  find_package (OpenMP ${${opm}_QUIET})
  list (APPEND ${opm}_LIBRARIES ${OpenMP_LIBRARIES})
  if (OPENMP_FOUND)
	add_options (C ALL_BUILDS "${OpenMP_C_FLAGS}")
	add_options (CXX ALL_BUILDS "${OpenMP_CXX_FLAGS}")
	set (HAVE_OPENMP 1)
  endif (OPENMP_FOUND)

  # threading library (search for this *after* OpenMP
  set (CMAKE_THREAD_PREFER_PTHREAD TRUE)
  find_package (Threads ${${opm}_QUIET})
  if (CMAKE_USE_PTHREADS_INIT)
	list (APPEND ${opm}_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  endif (CMAKE_USE_PTHREADS_INIT)

  else (USE_OPENMP)
	message (STATUS "OpenMP: disabled")

	# if we don't have OpenMP support, then don't show warnings that these
	# pragmas are unknown
	if (CXX_COMPAT_GCC)
	  add_options (ALL_LANGUAGES ALL_BUILDS "-Wno-unknown-pragmas")
	elseif (MSVC)
	  add_options (ALL_LANGUAGES ALL_BUILDS "-wd4068")
	endif()
  endif (USE_OPENMP)
endmacro (find_openmp opm)
