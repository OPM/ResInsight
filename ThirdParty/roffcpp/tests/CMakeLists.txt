
# Tests need to be added as executables first
add_executable(roffcpp-tests TokenTests.cpp TokenizerTests.cpp roffcpptestmain.cpp)

target_compile_features(roffcpp-tests PRIVATE cxx_std_17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(roffcpp-tests PRIVATE roffcpp gtest Threads::Threads)

add_test(NAME roffcpp-tests COMMAND roffcpp-tests)
