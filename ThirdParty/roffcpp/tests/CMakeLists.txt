
# Tests need to be added as executables first
add_executable(roffcpp-tests TokenTests.cpp TokenizerTests.cpp roffcpptestmain.cpp)

target_compile_features(roffcpp-tests PRIVATE cxx_std_17)

target_link_libraries(roffcpp-tests PRIVATE roffcpp gtest)

add_test(NAME roffcpp-tests COMMAND roffcpp-tests)
