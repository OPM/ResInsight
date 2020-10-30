# ~~~
# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

#
#
# AUTO_VCPKG_GIT_TAG - which vcpkg tag to clone if building ourselves
#
# AUTO_VCPKG_ROOT - root where `vcpkg` is (or should be installed)
#   NOTE: it should be fullpath to the place otherwise it will be relative to CMAKE_SOURCE_DIR
#

set(AUTO_VCPKG_GIT_REPOSITORY "https://github.com/Microsoft/vcpkg.git")
if (DEFINED AUTO_VCPKG_GIT_TAG)
    set(USE_AUTO_VCPKG_GIT_TAG "GIT_TAG ${AUTO_VCPKG_GIT_TAG}")
endif ()

function (vcpkg_setroot)
    if (DEFINED AUTO_VCPKG_ROOT)
        return()
    endif ()
    set(AUTO_VCPKG_ROOT "${CMAKE_BINARY_DIR}/vcpkg" CACHE STRING "")
    set(ENV{VCPKG_ROOT} "${AUTO_VCPKG_ROOT}")
    message(STATUS "AutoVcpkg: using vcpkg root ${AUTO_VCPKG_ROOT}")
endfunction()

function (vcpkg_download)
    vcpkg_setroot()
    set(vcpkg_download_contents [===[
cmake_minimum_required(VERSION 3.5)
project(vcpkg-download)

include(ExternalProject)
ExternalProject_Add(vcpkg
            GIT_REPOSITORY @AUTO_VCPKG_GIT_REPOSITORY@
            @USE_AUTO_VCPKG_GIT_TAG@
            GIT_SHALLOW ON
            SOURCE_DIR @AUTO_VCPKG_ROOT@
            PATCH_COMMAND ""
            CONFIGURE_COMMAND  ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            LOG_DOWNLOAD ON
            LOG_CONFIGURE ON
            LOG_INSTALL ON)
    ]===])
    get_filename_component(AUTO_VCPKG_ROOT_FULL ${AUTO_VCPKG_ROOT} ABSOLUTE)
    string(REPLACE "@AUTO_VCPKG_GIT_REPOSITORY@" "${AUTO_VCPKG_GIT_REPOSITORY}" vcpkg_download_contents "${vcpkg_download_contents}")
    string(REPLACE "@USE_AUTO_VCPKG_GIT_TAG@" "${USE_AUTO_VCPKG_GIT_TAG}" vcpkg_download_contents "${vcpkg_download_contents}")
    string(REPLACE "@AUTO_VCPKG_ROOT@" "${AUTO_VCPKG_ROOT_FULL}" vcpkg_download_contents "${vcpkg_download_contents}")
    if(CCACHE_BIN)
        set(CMAKE_COMPILER_WRAPPER "-DCMAKE_CXX_COMPILER_LAUNCHER=${CCACHE_BIN} -DCMAKE_C_COMPILER_LAUNCHER=${CCACHE_BIN}")
    endif()
    file(WRITE "${CMAKE_BINARY_DIR}/vcpkg-download/CMakeLists.txt" "${vcpkg_download_contents}")
    execute_process(COMMAND "${CMAKE_COMMAND}"
            "-H${CMAKE_BINARY_DIR}/vcpkg-download"
            "-B${CMAKE_BINARY_DIR}/vcpkg-download"
            ${CMAKE_COMPILER_WRAPPER}
            ${USE_AUTO_VCPKG_TRIPLET})
    execute_process(COMMAND "${CMAKE_COMMAND}"
            "--build" "${CMAKE_BINARY_DIR}/vcpkg-download")
endfunction ()

function (vcpkg_bootstrap)
    find_program(AUTO_VCPKG_EXECUTABLE
            vcpkg PATHS ${AUTO_VCPKG_ROOT})
    if (NOT AUTO_VCPKG_EXECUTABLE)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/vcpkg-bootstrap.cmake" "${AUTO_VCPKG_ROOT}")
        execute_process(COMMAND ${CMAKE_COMMAND} -P "${AUTO_VCPKG_ROOT}/vcpkg-bootstrap.cmake"
                WORKING_DIRECTORY ${AUTO_VCPKG_ROOT})
    endif ()
endfunction ()

function (vcpkg_configure AUTO_VCPKG_BOOTSTRAP_SKIP)
    if (AUTO_VCPKG_EXECUTABLE AND DEFINED AUTO_VCPKG_ROOT)
        set(CMAKE_TOOLCHAIN_FILE
                "${AUTO_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
        return()
    endif ()

    message(STATUS "AutoVcpkg: searching for vcpkg in ${AUTO_VCPKG_ROOT}")
    find_program(AUTO_VCPKG_EXECUTABLE
            vcpkg vcpkg.exe PATHS ${AUTO_VCPKG_ROOT})
    if (NOT AUTO_VCPKG_EXECUTABLE)
        message(STATUS "AutoVcpkg: vcpkg not found, bootstrapping a new installation")
        if (NOT AUTO_VCPKG_BOOTSTRAP_SKIP)
            vcpkg_download()
            vcpkg_bootstrap()
        endif ()
        # Validate now we have something
        find_program(AUTO_VCPKG_EXECUTABLE
            vcpkg vcpkg.exe PATHS ${AUTO_VCPKG_ROOT})
        if (NOT AUTO_VCPKG_EXECUTABLE)
            message(FATAL_ERROR "AutoVcpkg: Cannot find vcpkg executable")
        endif ()
    endif ()
    mark_as_advanced(AUTO_VCPKG_ROOT)
    mark_as_advanced(AUTO_VCPKG_EXECUTABLE)
    set(CMAKE_TOOLCHAIN_FILE
            "${AUTO_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endfunction ()

function (vcpkg_install)
    cmake_parse_arguments(_vcpkg_install "" "TRIPLET" "" ${ARGN})
    if (NOT ARGN)
        message(STATUS "AutoVcpkg: vcpkg_install() called with no packages to install")
        return()
    endif ()

    set(packages ${ARGN})
    if (NOT _vcpkg_install_TRIPLET)
        if ("${CMAKE_SYSTEM_NAME}" MATCHES "(Windows)")
            if (NOT "${CMAKE_GENERATOR}" MATCHES "(Win32|IA32|x86)")
                set(_vcpkg_install_TRIPLET "x64-windows")
            else ()
                set(_vcpkg_install_TRIPLET "x86-windows")
            endif ()
            #set(USE_AUTO_VCPKG_TRIPLET "-DVCPKG_TARGET_TRIPLET=${_vcpkg_install_TRIPLET}")
            #set(VCPKG_TARGET_TRIPLET ${_vcpkg_install_TRIPLET})
            list(TRANSFORM packages APPEND ":${_vcpkg_install_TRIPLET}")
        endif ()
    endif ()
    string(CONCAT join ${packages})
    string(TOLOWER "${AUTO_VCPKG_GIT_TAG}:${packages}" packages_cache)
    message(STATUS "AutoVcpkg: vcpkg packages: ${packages}")

    vcpkg_setroot()
    if (NOT EXISTS "${AUTO_VCPKG_ROOT}/vcpkg" OR NOT EXISTS "${AUTO_VCPKG_ROOT}/.vcpkg-root")
        vcpkg_configure(off)
    else ()
        vcpkg_configure(on) # skip bootstrap
    endif ()

    message(STATUS "AutoVcpkg: vcpkg_install() called to install: ${join}")
    execute_process (COMMAND "${AUTO_VCPKG_EXECUTABLE}" "install" ${packages})
endfunction ()