# Release-only variant of vcpkg's x64-linux triplet, used by CI.
# See x64-windows-release.cmake for the rationale; the only differences here
# are the library linkage and target system.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_BUILD_TYPE release)

# CMake 4.x removed compatibility with cmake_minimum_required(VERSION < 3.5).
# See x64-osx.cmake for the full rationale.  Must be kept in sync with the
# x64-linux triplet this one shadows.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_POLICY_VERSION_MINIMUM=3.5")

set(VCPKG_CMAKE_SYSTEM_NAME Linux)
