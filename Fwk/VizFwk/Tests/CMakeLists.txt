cmake_minimum_required(VERSION 2.8)

# Builds all the unit tests


# Just use the base compile flags for all unit tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_BASE_CXX_FLAGS}")

# And then relax some of the warnings for the unit tests
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-variadic-macros -Wno-long-long -Wno-sign-compare")
endif()

# Setup our platform dependent libraries here, assuming they'll be the same for all the tests
if (UNIX AND NOT APPLE)
	set(CEE_PLATFORM_LIBS  rt pthread)
endif()

add_definitions(-DCVF_UTEST_DEFAULT_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/TestData/")

add_subdirectory(LibCore_UnitTests)
add_subdirectory(LibIo_UnitTests)
add_subdirectory(LibGeometry_UnitTests)
add_subdirectory(LibRender_UnitTests)
add_subdirectory(LibViewing_UnitTests)

add_subdirectory(LibRegGrid2D_UnitTests)
add_subdirectory(LibStructGrid_UnitTests)

add_subdirectory(LibUtilities_UnitTests)

if (CEE_BUILD_GUI_QT)
	add_subdirectory(LibGuiQt_UnitTests)
endif()
