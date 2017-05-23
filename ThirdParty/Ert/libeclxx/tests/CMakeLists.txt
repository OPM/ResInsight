add_executable(eclxx_kw eclxx_kw.cpp)
target_link_libraries(eclxx_kw eclxx  ert_utilxx)
add_test(eclxx_kw ${EXECUTABLE_OUTPUT_PATH}/eclxx_kw)

add_executable(eclxx_fortio eclxx_fortio.cpp)
target_link_libraries(eclxx_fortio eclxx  ert_utilxx)
add_test(eclxx_fortio ${EXECUTABLE_OUTPUT_PATH}/eclxx_fortio)

add_executable(eclxx_smspec eclxx_smspec.cpp)
target_link_libraries(eclxx_smspec eclxx  ert_utilxx)
add_test(eclxx_smspec ${EXECUTABLE_OUTPUT_PATH}/eclxx_smspec)

add_executable(eclxx_filename eclxx_filename.cpp)
target_link_libraries(eclxx_filename eclxx  ert_utilxx)
add_test(eclxx_filename ${EXECUTABLE_OUTPUT_PATH}/eclxx_filename)

add_executable(eclxx_types eclxx_types.cpp)
target_link_libraries(eclxx_types eclxx  ert_utilxx)
add_test(eclxx_types ${EXECUTABLE_OUTPUT_PATH}/eclxx_types)
