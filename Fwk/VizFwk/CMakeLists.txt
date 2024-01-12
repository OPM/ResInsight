cmake_minimum_required(VERSION 3.15)

project(VizFramework)


if (CEE_CEEVIZ_ROOT)
    message(STATUS "CEE_CEEVIZ_ROOT: ${CEE_CEEVIZ_ROOT}")
else()
    set(CEE_CEEVIZ_ROOT ${PROJECT_SOURCE_DIR})
    message(STATUS "Setting CEE_CEEVIZ_ROOT to ${CEE_CEEVIZ_ROOT}")
endif()	

# Determine if we're being run stand-alone or invoked from some other project
set(CEE_STAND_ALONE ON)
if (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    message("VizFramework project being built stand-alone")
else()
    set(CEE_STAND_ALONE OFF)
    message("VizFramework project is invoked from other project")
endif()


if (CEE_STAND_ALONE)
    # Set a default build type (not relevant for WIN32)
    if (NOT CMAKE_BUILD_TYPE)
	    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif()

    include(CMake/Utils/ceeDetermineCompilerFlags.cmake)
endif()


# Allow use of non-threadsafe reference counter in cvf::Object on systems with no atomics support
option(CEE_WORKAROUND_ON_SYSTEMS_WITHOUT_ATOMICS "Allow use of non-threadsafe reference counter on systems with no atomics support" OFF)
if (CEE_WORKAROUND_ON_SYSTEMS_WITHOUT_ATOMICS)
    add_definitions(-DCVF_WORKAROUND_TO_COMPILE_ON_SYSTEMS_WITHOUT_ATOMICS)
endif()


add_subdirectory(LibCore)
add_subdirectory(LibIo)
add_subdirectory(LibGeometry)
add_subdirectory(LibRender)
add_subdirectory(LibViewing)

add_subdirectory(LibRegGrid2D)
add_subdirectory(LibStructGrid)
add_subdirectory(LibFreeType)
add_subdirectory(ThirdParty/FreeType)

add_subdirectory(LibUtilities)


option(CEE_BUILD_GUI_QT "Build GUI library for Qt" ON)
if (CEE_BUILD_GUI_QT)
    option(CEE_USE_QT6 "Use Qt6" OFF)
    option(CEE_USE_QT5 "Use Qt5" OFF)
	add_subdirectory(LibGuiQt)
endif()


if (CEE_STAND_ALONE)
    option(CEE_BUILD_UNIT_TESTS "Build unit tests" ON)
    option(CEE_BUILD_TEST_APPS "Build test apps" ON)
endif()

if (CEE_BUILD_UNIT_TESTS OR CEE_BUILD_TEST_APPS)
    # Add CeeViz's root source dir as a preprocessor directive so unit tests and test apps can determine where to find the resources they want.
    add_definitions(-DCVF_CEEVIZ_ROOT_SOURCE_DIR="${CEE_CEEVIZ_ROOT}")
endif()

if (CEE_BUILD_UNIT_TESTS)
    add_subdirectory(Tests)
endif()

if (CEE_BUILD_TEST_APPS)
    # For now, build the snippet libs here
    add_subdirectory(Tests/SnippetsBasis)

    if (CEE_BUILD_GUI_QT)
        add_subdirectory(TestApps/Qt/QtMinimal)

        if (CEE_USE_QT5)
            add_subdirectory(TestApps/Qt/QtMinimal_GLWidget)
            add_subdirectory(TestApps/Qt/QtMinimal_deprecated)
            add_subdirectory(TestApps/Qt/QtMultiView_deprecated)
        endif()
    endif()

    if (WIN32)
        add_subdirectory(TestApps/Win32/Win32SnippetRunner)
    endif()
endif()
