# Release-only variant of vcpkg's x64-windows triplet, used by CI.
#
# Why this exists:
#   vcpkg builds every port in both debug and release unless VCPKG_BUILD_TYPE
#   says otherwise, and that variable is only read from the triplet file --
#   passing -DVCPKG_BUILD_TYPE=release to the top-level CMake call has no
#   effect on the ports vcpkg builds.  CI only ever links release binaries, so
#   the debug half is pure cost: on a cold cache it is roughly half of an
#   85 minute vcpkg step (grpc, arrow, openssl and protobuf dominate).
#
#   Kept as a separate triplet rather than folded into x64-windows so local
#   developer builds keep their debug dependencies.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_PROVIDED_FORTRAN ON)

set(VCPKG_BUILD_TYPE release)

# CMake 4.x removed compatibility with cmake_minimum_required(VERSION < 3.5).
# See x64-osx.cmake for the full rationale.  Must be kept in sync with the
# x64-windows triplet this one shadows.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
