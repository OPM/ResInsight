# Release-only variant of the x64-osx overlay triplet, used by CI.
#
# Why this exists:
#   vcpkg builds every port in both debug and release unless VCPKG_BUILD_TYPE
#   says otherwise, and that variable is only read from the triplet file --
#   passing -DVCPKG_BUILD_TYPE=release to the top-level CMake call has no
#   effect on the ports vcpkg builds.  CI only ever links release binaries, so
#   the debug half is pure cost.
#
#   Kept as a separate triplet rather than folded into x64-osx so local
#   developer builds keep their debug dependencies.  Must be kept in sync with
#   x64-osx.cmake, which it shadows; see that file for the sysroot and
#   policy-minimum rationale.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_BUILD_TYPE release)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES x86_64)

if(NOT DEFINED VCPKG_OSX_SYSROOT OR VCPKG_OSX_SYSROOT STREQUAL "")
  execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE _xcrun_sdk_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _xcrun_result
  )
  if(_xcrun_result EQUAL 0 AND _xcrun_sdk_path)
    set(VCPKG_OSX_SYSROOT "${_xcrun_sdk_path}")
  endif()
endif()

list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
