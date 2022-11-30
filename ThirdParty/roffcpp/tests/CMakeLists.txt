
# Tests need to be added as executables first
add_executable(roffcpp-tests TokenTests.cpp TokenizerTests.cpp ReaderTests.cpp ../../gtest/gtest-all.cc roffcpptestmain.cpp)


CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/RoffTestDataDirectory.hpp.cmake
                ${CMAKE_BINARY_DIR}/Generated/RoffTestDataDirectory.hpp
)

target_include_directories(roffcpp-tests PUBLIC ../../ ${CMAKE_BINARY_DIR}/Generated)

target_compile_features(roffcpp-tests PRIVATE cxx_std_17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(roffcpp-tests PRIVATE roffcpp Threads::Threads)

add_test(NAME roffcpp-tests COMMAND roffcpp-tests)
