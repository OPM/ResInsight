# ##############################################################################
# std::stacktrace availability
#
# libstdc++ has shipped <stacktrace> and defined __cpp_lib_stacktrace since GCC
# 13, but the implementation lives in a separate library (libstdc++exp).  Some
# distributions ship the headers without that library -- Red Hat's gcc-toolset
# is one -- so the header-only feature test in RiaMainTools.cpp and
# RiaOpenTelemetryManager.h reports the feature as present and the link then
# fails with undefined references to std::__stacktrace_impl::_S_current.
#
# Probe by actually linking: first with no extra library (libc++, and any
# libstdc++ that folds the implementation in), then with stdc++exp, from
# RESINSIGHT_GCC_STDCPP_EXP_PATH when a custom GCC build is used.  If neither
# links, define RIA_HAS_STD_STACKTRACE=0 for the whole project so the crash
# handler compiles out its stack-trace capture and logs the signal only.
#
# Sets RESINSIGHT_STACKTRACE_LIBRARIES for the link line (empty when the
# feature needs no extra library, or is unavailable).
# ##############################################################################

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

set(RESINSIGHT_GCC_STDCPP_EXP_PATH
    ""
    CACHE PATH "Path to search for stdc++exp"
)

set(RESINSIGHT_STACKTRACE_LIBRARIES "")

set(_ri_stacktrace_source
    "#include <stacktrace>
int main()
{
    return static_cast<int>( std::stacktrace::current().size() );
}"
)

cmake_push_check_state(RESET)
check_cxx_source_compiles(
  "${_ri_stacktrace_source}" RESINSIGHT_STACKTRACE_NO_EXTRA_LIBRARY
)

if(NOT RESINSIGHT_STACKTRACE_NO_EXTRA_LIBRARY)
  # Default to the plain library name and let the compiler driver resolve it
  # from its own library directories, which find_library() does not search.
  set(_ri_stdcpp_exp stdc++exp)
  if(RESINSIGHT_GCC_STDCPP_EXP_PATH)
    find_library(
      STDCPP_EXP_LIBRARY
      NAMES stdc++exp
      HINTS ${RESINSIGHT_GCC_STDCPP_EXP_PATH}
    )
    if(STDCPP_EXP_LIBRARY)
      set(_ri_stdcpp_exp ${STDCPP_EXP_LIBRARY})
    endif()
  endif()

  set(CMAKE_REQUIRED_LIBRARIES ${_ri_stdcpp_exp})
  check_cxx_source_compiles(
    "${_ri_stacktrace_source}" RESINSIGHT_STACKTRACE_NEEDS_STDCPP_EXP
  )
  if(RESINSIGHT_STACKTRACE_NEEDS_STDCPP_EXP)
    set(RESINSIGHT_STACKTRACE_LIBRARIES ${_ri_stdcpp_exp})
  endif()
endif()
cmake_pop_check_state()

if(RESINSIGHT_STACKTRACE_NO_EXTRA_LIBRARY)
  message(STATUS "std::stacktrace: available, no extra library required")
elseif(RESINSIGHT_STACKTRACE_NEEDS_STDCPP_EXP)
  message(
    STATUS "std::stacktrace: available, linking ${RESINSIGHT_STACKTRACE_LIBRARIES}"
  )
else()
  message(
    STATUS
      "std::stacktrace: not usable with this toolchain (no stdc++exp), crash handler will log signals without stack traces"
  )
  add_compile_definitions(RIA_HAS_STD_STACKTRACE=0)
endif()
