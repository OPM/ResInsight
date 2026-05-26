# Overlay of vcpkg's default arm64-osx triplet.  See x64-osx.cmake for the
# rationale (sysroot detection); the only difference here is the target
# architecture.  Tracked in #14045.

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)

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

# CMake 4.x removed compatibility with cmake_minimum_required(VERSION < 3.5).
# See x64-osx.cmake for the same workaround.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
