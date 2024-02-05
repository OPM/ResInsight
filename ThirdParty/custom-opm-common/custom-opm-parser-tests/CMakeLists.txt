
project ( opm-parser-tests )

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)

set(THREADS_PREFER_PTHREAD_FLAG ON)

set( PROJECT_FILES opm-parser-BasicTest.cpp )

# add the executable 
add_executable (${PROJECT_NAME} ${PROJECT_FILES} )

# Languages and global compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/OpmTestDataDirectory.h.cmake
                ${CMAKE_BINARY_DIR}/Generated/OpmTestDataDirectory.h
)

target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/Generated )

target_link_libraries ( ${PROJECT_NAME}
    custom-opm-common
	GTest::gtest_main
)

# Add dependency of Shlwapi.lib for Windows platforms
if (MSVC)
  target_link_libraries(${PROJECT_NAME} Shlwapi)
endif()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})