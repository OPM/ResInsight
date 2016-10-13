add_executable(eclxx_kw eclxx_kw.cpp)
target_link_libraries(eclxx_kw eclxx test_util ert_utilxx)
add_test(eclxx_kw ${EXECUTABLE_OUTPUT_PATH}/eclxx_kw)

add_executable(eclxx_fortio eclxx_fortio.cpp)
target_link_libraries(eclxx_fortio eclxx test_util ert_utilxx)
add_test(eclxx_fortio ${EXECUTABLE_OUTPUT_PATH}/eclxx_fortio)

add_executable(eclxx_smspec eclxx_smspec.cpp)
target_link_libraries(eclxx_smspec eclxx test_util ert_utilxx)
add_test(eclxx_smspec ${EXECUTABLE_OUTPUT_PATH}/eclxx_smspec)
