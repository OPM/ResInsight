cmake_minimum_required (VERSION 3.12)

include (CheckCSourceCompiles)
project (ResInsight)

set (VIZ_MODULES_FOLDER_NAME Fwk/VizFwk)

cmake_policy(SET CMP0020 NEW)
if (POLICY CMP0077)
    cmake_policy(SET CMP0077 OLD)
endif()

if (UNIX)
    option(RESINSIGHT_PREFER_LEGACY_OPENGL "Link with Legacy OpenGL libraries. This may be necessary in some virtualization environments" ON)
    if (RESINSIGHT_PREFER_LEGACY_OPENGL)
        set(OpenGL_GL_PREFERENCE LEGACY)
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(BUILD_SHARED_LIBS OFF CACHE BOOL "ERT: Build shared libraries")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(VCPKG_AUTO_INSTALL "Automatically download build pre-requisites with VCPKG" OFF)
if (VCPKG_AUTO_INSTALL)
    include (AutoVcpkg)
    set (AUTO_VCPKG_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/vcpkg")
endif()

# Use CMake to enforce C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

################################################################################
# Setup the main platform defines
################################################################################
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-DCVF_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-DCVF_OSX)
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(_HAS_STD_BYTE 0)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS 		"-DCVF_LINUX -pipe -Wextra -Woverloaded-virtual -Wformat -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS_DEBUG 	"-ggdb -g3 -O0 -DDEBUG -D_DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNO_DEBUG")
endif() 


################################################################################
# OpenMP
################################################################################
option (RESINSIGHT_USE_OPENMP "Enable OpenMP parallellization in the code" ON)
if (RESINSIGHT_USE_OPENMP)
    find_package(OpenMP)
    if(OPENMP_FOUND)
        message(STATUS "Enabling OpenMP support")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -DUSE_OPENMP")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    else()
        message(STATUS "Disabling OpenMP support")
    endif()
endif()


################################################################################
# Version number
################################################################################
include (ResInsightVersion.cmake)


################################################################################
# Octave
################################################################################
find_package(Octave)

################################################################################
# Unity Build
################################################################################

# CMAKE_UNITY_BUILD was introduced in CMake 3.16.2
option(RESINSIGHT_ENABLE_UNITY_BUILD "Experimental speedup of compilation using CMake Unity Build" OFF)
mark_as_advanced(FORCE RESINSIGHT_ENABLE_UNITY_BUILD)
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  set(CAF_ENABLE_UNITY_BUILD true)
  set(CVF_ENABLE_UNITY_BUILD true)
endif()

################################################################################
# ERT
################################################################################

# Use of CMAKE_CXX_COMPILER_LAUNCHER is not working with libecl. Disable by temporarily setting
# compiler launcher to nothing
set(TEMP_CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_CXX_COMPILER_LAUNCHER})
set(CMAKE_CXX_COMPILER_LAUNCHER)

if (NOT MSVC)
  # Linux: Optional configuration of externally installed ERT, requires path to libraries and includes
  set(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT "" CACHE PATH "Path to installed ERT libraries")
  set(RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT "" CACHE PATH "Path to installed ERT includes")
endif()

if (RESINSIGHT_ERT_EXTERNAL_LIB_ROOT OR RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT)
  if (NOT(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT AND RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT))
    message(FATAL_ERROR "Both RESINSIGHT_ERT_EXTERNAL_LIB_ROOT and RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT must be defined")
  endif()

    message(FATAL_ERROR "TODO: Building using and external system installed ERT is broken.")

  list(APPEND ERT_INCLUDE_DIRS
    ${RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT}
  )

  list(APPEND ERT_LIBRARIES
    ${RESINSIGHT_ERT_EXTERNAL_LIB_ROOT}/libecl.so
  )
  
else()

    # Disable install of ERT libs and headers, as Ert code is compiled and linked directly
    SET(INSTALL_ERT OFF CACHE BOOL "ERT: Install library")
    SET(ERT_USE_OPENMP ${OPENMP_FOUND} CACHE BOOL "ERT: Compile using OpenMP")

    # Remember original state
    set(ORIGINAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    
    if (MSVC)
      # Force static linking on Windows
      set(BUILD_SHARED_LIBS OFF)
      set(ERT_HAVE_UNISTD OFF) #If anyone has mingw installed
    else ()
      set(RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT "" CACHE STRING "Path to ERT CMakeList.txt (source path)")
       
      # Force dynamic linking on other platforms
      # Copy of libraries into install folder of ResInsight is done a bit further down in this file
      set(BUILD_SHARED_LIBS ON)
    endif()

    if (RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT)
        add_subdirectory(${RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT} ${CMAKE_BINARY_DIR}/ThirdParty/Ert)
    else()
        add_subdirectory(ThirdParty/Ert)
    endif ()

    if (MSVC)
         # libecl : Disable some warnings  
         set_target_properties(ecl PROPERTIES COMPILE_FLAGS "/wd4244 /wd4267 /wd4013 /wd4190 /wd4018 /wd4477 /wd4098 /wd4293 /wd4305 /wd4020 /wd4028 /wd4715")
    else()
        set_target_properties(ecl PROPERTIES COMPILE_FLAGS "-Wno-deprecated -Wno-deprecated-declarations -Wno-clobbered")
    endif()

    list(APPEND THIRD_PARTY_LIBRARIES
        ecl
    )

    set_property(TARGET  
      catch2
      PROPERTY FOLDER "Thirdparty"
    )
    
    # Restore original state
    set(BUILD_SHARED_LIBS ${ORIGINAL_BUILD_SHARED_LIBS})
    
endif(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT OR RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT)

# restore buildcache when libecl has been configured
set(CMAKE_CXX_COMPILER_LAUNCHER ${TEMP_CMAKE_CXX_COMPILER_LAUNCHER})

#####################################

######################################
# Go through required git submodules #
######################################
list(APPEND THIRD_PARTY_SUBMODULES qwt)

###############################################
# Init GIT submodules if they haven't already #
###############################################
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        foreach (SUBMODULE ${THIRD_PARTY_SUBMODULES})
            set (SUBDIR "${PROJECT_SOURCE_DIR}/ThirdParty/${SUBMODULE}")
            message(STATUS "Initializing GIT submodules")
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${SUBDIR}
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE GIT_SUBMOD_RESULT)
            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                message(FATAL_ERROR "git submodule update --init failed for ${SUBMODULE} with ${GIT_SUBMOD_RESULT}, please checkout submodules")
            endif()
        endforeach()
    endif()
endif()


################################################################################
# buildcache
# 
# buildcache will be used if found in path
# It is required to use compiler flag /Zi to be able to use buildcache
# See https://docs.microsoft.com/en-us/cpp/build/reference/z7-zi-zi-debug-information-format?view=vs-2019
# The adjustments of build flags must also be placed after libecl, as the adjustments
# cause compiler issues for libecl
################################################################################
find_program(buildcache_program buildcache)
if(buildcache_program)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${buildcache_program}")
  message( STATUS "Found buildcache from : ${buildcache_program}" )

  if (MSVC)
    string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    if(CMAKE_CXX_FLAGS_DEBUG MATCHES "/Zi")
      message(STATUS "Clearing pdb setting")
      string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
  endif()
else()
    message( STATUS "No buildcache found" )
endif()

################################################################################
# HDF5
################################################################################
if(MSVC)
    set(RESINSIGHT_HDF5_DIR "" CACHE PATH "Windows Only: Optional path to HDF5 libraries on Windows")
endif()

if(MSVC)
    if(NOT ${RESINSIGHT_HDF5_DIR} EQUAL "")
        list(APPEND EXTERNAL_LINK_LIBRARIES
            ${RESINSIGHT_HDF5_DIR}/lib/hdf5.lib
            ${RESINSIGHT_HDF5_DIR}/lib/hdf5_cpp.lib
        )

        set(RESINSIGHT_FOUND_HDF5 1)
        message( STATUS "Using HDF5 from : ${RESINSIGHT_HDF5_DIR}" )
    else()
        message( WARNING "Use of HDF5 is enabled, but RESINSIGHT_HDF5_DIR is empty. Specify RESINSIGHT_HDF5_DIR to be able to use HDF5" )
    endif()
else()
    find_package(HDF5 COMPONENTS CXX)
    if (HDF5_FOUND)
        list(APPEND EXTERNAL_LINK_LIBRARIES
            ${HDF5_LIBRARIES}
        )

        set(RESINSIGHT_FOUND_HDF5 1)
        message( STATUS "Using HDF5 libraries : ${HDF5_LIBRARIES}" )

        option (RESINSIGHT_HDF5_BUNDLE_LIBRARIES "Bundle HDF5 libraries" OFF)
        mark_as_advanced(FORCE RESINSIGHT_HDF5_BUNDLE_LIBRARIES) 
        if (RESINSIGHT_HDF5_BUNDLE_LIBRARIES)
            message( STATUS "Bundling of HDF5 libraries is enabled" )
        endif() # RESINSIGHT_HDF5_BUNDLE_LIBRARIES

    else()
        message( WARNING "Use of HDF5 is enabled, but no HDF5 is found." )
    endif() # HDF5_FOUND
endif() # MSVC


################################################################################
# Opm
################################################################################

if (VCPKG_AUTO_INSTALL)
    vcpkg_install(boost-filesystem)
    vcpkg_install(boost-spirit)
endif()

add_subdirectory(ThirdParty/custom-opm-flowdiagnostics)
add_subdirectory(ThirdParty/custom-opm-flowdiag-app)
add_subdirectory(ThirdParty/custom-opm-common)
add_subdirectory(ThirdParty/custom-opm-common/custom-opm-parser-tests)

list(APPEND OPM_LIBRARIES
    custom-opm-flowdiagnostics
    custom-opm-flowdiag-app
    custom-opm-common
)

set_property(TARGET  
    ${OPM_LIBRARIES}
    PROPERTY FOLDER "Thirdparty/OPM"
)

################################################################################
# NRLib
################################################################################

add_subdirectory(ThirdParty/NRLib)

list(APPEND THIRD_PARTY_LIBRARIES
    NRLib
)

################################################################################
# Qt
################################################################################
find_package(Qt5 COMPONENTS Core Gui OpenGL Network Script ScriptTools Widgets OPTIONAL_COMPONENTS Charts)
if (Qt5Charts_FOUND)
   message(STATUS "QtCharts found: ${Qt5Charts_LIBRARIES}")
   add_definitions(-DUSE_QTCHARTS)
else()
   message(STATUS "QtCharts missing.")
endif()

# Open GL
find_package( OpenGL )

################################################################################
# Qwt
################################################################################

add_subdirectory(ThirdParty/qwt)

if(RESINSIGHT_ENABLE_UNITY_BUILD)
    message("Cmake Unity build is enabled on : qwt")
    set_property(TARGET qwt PROPERTY UNITY_BUILD true)
    message("Cmake Unity build is enabled on : custom-opm-flowdiagnostics")
    set_property(TARGET custom-opm-flowdiagnostics PROPERTY UNITY_BUILD true)
    message("Cmake Unity build is enabled on : custom-opm-common")
    set_property(TARGET custom-opm-common PROPERTY UNITY_BUILD true)
endif()

list(APPEND THIRD_PARTY_LIBRARIES
    qwt
)

################################################################################
# Nightcharts
################################################################################

add_subdirectory(ThirdParty/nightcharts)

list(APPEND THIRD_PARTY_LIBRARIES
    nightcharts
)

################################################################################
# C++ Mathematical Expression Parsing And Evaluation Library 
################################################################################

add_subdirectory(ThirdParty/expressionparser)

list(APPEND THIRD_PARTY_LIBRARIES
    expressionparser
)

################################################################################
# clipper
################################################################################

add_subdirectory(ThirdParty/clipper)

list(APPEND THIRD_PARTY_LIBRARIES
    clipper
)

################################################################################
# Thirdparty libraries are put in ThirdParty solution folder
################################################################################
set_property(TARGET  
    ${THIRD_PARTY_LIBRARIES}
    PROPERTY FOLDER "Thirdparty"
)

################################################################################
# Vizualization Framework
################################################################################

# Allow use of non-threadsafe reference counter in cvf::Object on systems with no atomics support
if (CMAKE_COMPILER_IS_GNUCC)
 
    if (NOT DEFINED HAVE_GCC_SYNC_FUNCTIONS)
        check_c_source_compiles("int main(int argc, char **argv) {
            int a;
            __sync_add_and_fetch(&a, 1);
            __sync_fetch_and_add(&a, 1);
            __sync_sub_and_fetch(&a, 1);
            __sync_fetch_and_sub(&a, 1); }" HAVE_GCC_SYNC_FUNCTIONS)
    endif()
 
    if (HAVE_GCC_SYNC_FUNCTIONS)
        message(STATUS "GCC synchronization functions detected")
    else()
        message(STATUS "GCC synchronization functions NOT detected, fallback to non threadsafe reference counting")
        add_definitions(-DCVF_USE_NON_THREADSAFE_REFERENCE_COUNT)
    endif()
 
endif()

add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibCore)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibGeometry)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibRender)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibViewing)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibGuiQt)

list(APPEND VIZ_FWK_LIBRARIES
    LibGuiQt
    LibViewing
    LibRender
    LibGeometry
    LibCore
)

set_property(TARGET  
    ${VIZ_FWK_LIBRARIES}
    PROPERTY FOLDER "VizFwk"
)


################################################################################
# Application Framework
################################################################################

add_subdirectory(Fwk/AppFwk/cafAnimControl)
add_subdirectory(Fwk/AppFwk/cafViewer)

add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmCore)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmUiCore)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmXml)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel)

add_subdirectory(Fwk/AppFwk/cafCommand)
add_subdirectory(Fwk/AppFwk/cafUserInterface)
add_subdirectory(Fwk/AppFwk/cafPdmCvf)
add_subdirectory(Fwk/AppFwk/CommonCode)
add_subdirectory(Fwk/AppFwk/cafVizExtensions)
option(CAF_CVF_SCRIPTING "" ON)
add_subdirectory(Fwk/AppFwk/cafPdmScripting)
set_property(TARGET cafPdmScripting PROPERTY FOLDER "AppFwk")

add_subdirectory(Fwk/AppFwk/cafCommandFeatures)
set_property(TARGET cafCommandFeatures PROPERTY FOLDER "AppFwk")

add_subdirectory(Fwk/AppFwk/cafTensor)
add_subdirectory(Fwk/AppFwk/cafHexInterpolator)

list(APPEND APP_FWK_LIBRARIES
    cafPdmCore
    cafPdmUiCore
    cafPdmXml
    cafProjectDataModel
    
    cafUserInterface
    cafViewer
    cafAnimControl
    cafCommand
    cafPdmCvf

    cafTensor
    CommonCode
    cafVizExtensions
    cafPdmScripting
)

set_property(TARGET  
    ${APP_FWK_LIBRARIES}
    cafHexInterpolator
    PROPERTY FOLDER "AppFwk"
)

option (RESINSIGHT_INCLUDE_APPFWK_TESTS "Enable AppFwk Tests" OFF)
mark_as_advanced(FORCE RESINSIGHT_INCLUDE_APPFWK_TESTS)

if (RESINSIGHT_INCLUDE_APPFWK_TESTS)
    # Unit Tests
    add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafProjectDataModel_UnitTests)
    set_property(TARGET cafProjectDataModel_UnitTests PROPERTY FOLDER "AppFwkTests")

    add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests)
    set_property(TARGET cafPdmCore_UnitTests PROPERTY FOLDER "AppFwkTests")

    add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)
    set_property(TARGET cafPdmXml_UnitTests PROPERTY FOLDER "AppFwkTests")

    add_subdirectory(Fwk/AppFwk/cafPdmScripting/cafPdmScripting_UnitTests)
    set_property(TARGET cafPdmScripting_UnitTests PROPERTY FOLDER "AppFwkTests")

    # Executables
    add_subdirectory(Fwk/AppFwk/cafTests/cafTestApplication)
    set_property(TARGET cafTestApplication PROPERTY FOLDER "AppFwkTests")
    
    add_subdirectory(Fwk/AppFwk/cafTests/cafTestCvfApplication)
    set_property(TARGET cafTestCvfApplication PROPERTY FOLDER "AppFwkTests")
endif()



################################################################################
# Installation settings
################################################################################

set (RESINSIGHT_INSTALL_FOLDER ".")

# override system install prefix if private installation chosen
option (RESINSIGHT_PRIVATE_INSTALL "Linux only: Install the libecl shared libraries along the executable" ON)
mark_as_advanced(FORCE RESINSIGHT_PRIVATE_INSTALL)

if (RESINSIGHT_PRIVATE_INSTALL)
   
    ################################################################################
    # ERT shared library files
    # Install procedure will copy so-files from ERT into same install folder as ResInsight
    ################################################################################
    
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(ERT_SHARED_LIB_FILES
            ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.so
            ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.so.2
            ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.so.2.4
        )
      
      install(FILES ${ERT_SHARED_LIB_FILES} DESTINATION ${RESINSIGHT_INSTALL_FOLDER} )
    endif()
    
endif (RESINSIGHT_PRIVATE_INSTALL)

if (RESINSIGHT_HDF5_BUNDLE_LIBRARIES)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        # find all file names with text hdf5
        # use file globbing to also copy the symlinks to make sure the dependencies from ResInsight runtime is correct
        foreach (FILE_TO_COPY ${HDF5_LIBRARIES})
            STRING( FIND ${FILE_TO_COPY} "hdf5" POS_IN_STRING ) 
            if(${POS_IN_STRING} GREATER -1)
                file(GLOB FILE_AND_SYMLINKS ${FILE_TO_COPY}*)
                install(FILES ${FILE_AND_SYMLINKS} DESTINATION ${RESINSIGHT_INSTALL_FOLDER} )
            endif()
        endforeach()
    endif()
endif (RESINSIGHT_HDF5_BUNDLE_LIBRARIES)


################################################################################
# Application 
################################################################################
add_subdirectory(ApplicationCode)

if (OCTAVE_MKOCTFILE)
  message (STATUS "Adding OctavePlugin library")
  
  add_subdirectory(OctavePlugin)
else (OCTAVE_MKOCTFILE)
  message (STATUS "Could not find OCTAVE_MKOCTFILE, skipping OctavePlugin library")
endif(OCTAVE_MKOCTFILE)

################################################################################
# Code documentation using Doxygen
################################################################################
option(RESINSIGHT_BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" OFF)
if(RESINSIGHT_BUILD_DOCUMENTATION)
  FIND_PACKAGE(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR 
      "Doxygen is needed to build the documentation. Please install it correctly")
  endif()
  #-- Configure the Template Doxyfile for our specific project
  configure_file(Doxyfile.in 
                 ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  #-- Add a custom target to run Doxygen when ever the project is built
  add_custom_target (Docs 
                    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
                    SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
  # IF you do NOT want the documentation to be generated EVERY time you build the project
  # then leave out the 'ALL' keyword from the above command.
endif(RESINSIGHT_BUILD_DOCUMENTATION)

