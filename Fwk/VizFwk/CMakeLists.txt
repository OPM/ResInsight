cmake_minimum_required(VERSION 2.8.12)

project(VizFramework)


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
    option(CEE_USE_QT5 "Use Qt5" OFF)
	add_subdirectory(LibGuiQt)
endif()


if (CEE_STAND_ALONE)

    option(CEE_BUILD_UNIT_TESTS "Build unit tests" ON)
    if (CEE_BUILD_UNIT_TESTS)
        add_subdirectory(Tests)
    endif()


    option(CEE_BUILD_TEST_APPS "Build test apps" ON)
    if (CEE_BUILD_TEST_APPS)
        if (CEE_BUILD_GUI_QT)
            # For now, build the snippet libs here
            add_subdirectory(Tests/SnippetsBasis)

            add_subdirectory(TestApps/Qt/QtMinimal)
            add_subdirectory(TestApps/Qt/QtMultiView)
            add_subdirectory(TestApps/Qt/QtSnippetRunner)
        endif()
    endif()

endif()
