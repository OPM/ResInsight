# Overlay of vcpkg's default x64-osx triplet.
#
# Why this exists:
#   CMake on macOS does not always populate CMAKE_OSX_SYSROOT (notably when
#   only Xcode Command Line Tools are installed, no full Xcode).  vcpkg's
#   per-port build then injects "-isysroot ${CMAKE_OSX_SYSROOT}" verbatim,
#   yielding a literal "-isysroot " with no path.  Subsequent flags such as
#   "-g" get consumed as the sysroot argument, and openssl (a transitive
#   dep of arrow) fails with "fatal error: 'sys/types.h' file not found".
#
#   Detecting the SDK path via `xcrun --show-sdk-path` and exporting it as
#   VCPKG_OSX_SYSROOT pins the value vcpkg ships to every child build.
#
# Tracked in #14045.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

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

# CMake 4.x removed compatibility with cmake_minimum_required(VERSION < 3.5).
# Many vcpkg ports still ship CMakeLists with older minimums (libevent,
# protobuf, grpc, several boost shims).  Inject the policy-min so each
# port's vcpkg_cmake_configure call accepts the legacy minimum.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
