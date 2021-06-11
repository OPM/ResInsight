# - Use PThread features
#
# Synopsis:
#
#	find_threads (module)
#
# where:
#
#	module            Name of the module to which Threads support
#	                  etc. should be added, e.g. "opm-core".
#
# Note: Compiler flags are always added globally, to avoid ABI
# incompatibility problems.
#
# It is assumed that the following variables are available
#
#	${module}_QUIET      Verbosity level of the parent's find module
#	${module}_LIBRARIES  List of libraries to which Thread support will be added
#
# Example:
#	find_threads (opm-core)

include (AddOptions)
include (UseCompVer)
is_compiler_gcc_compatible ()

macro(find_threads opm)
# default is that Threads are added
option(USE_PTHREAD "Use pthreads" ON)

# if USE_PTHREAD is enabled then check and set HAVE_PTHREAD
if( USE_PTHREAD )
  # threading library
  set (CMAKE_THREAD_PREFER_PTHREAD TRUE)
  find_package (Threads ${${opm}_QUIET})
  if (CMAKE_USE_PTHREADS_INIT)
    list (APPEND ${opm}_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
    if (CXX_COMPAT_GCC)
      check_cxx_accepts_flag ("-pthread" HAVE_PTHREAD)
      if(HAVE_PTHREAD)
        add_options (ALL_LANGUAGES ALL_BUILDS "-pthread")
        set(HAVE_PTHREAD "1")
      endif(HAVE_PTHREAD)
    endif (CXX_COMPAT_GCC)
  else(CMAKE_USE_PTHREADS_INIT)
    set(USE_PTHREAD OFF)
  endif (CMAKE_USE_PTHREADS_INIT)
else( USE_PTHREAD )
  # reset HAVE_PTHREAD
  set(HAVE_PTHREAD "")
endif( USE_PTHREAD )

endmacro(find_threads opm)
