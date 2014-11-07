add_executable( ert_module_test ert_module_test.c )
target_link_libraries( ert_module_test analysis ert_util )

if (USE_RUNPATH)
   add_runpath( ert_module_test )
endif()

set (destination ${CMAKE_INSTALL_PREFIX}/bin)

install(TARGETS ert_module_test DESTINATION ${destination})

if (INSTALL_GROUP)
   install(CODE "EXECUTE_PROCESS(COMMAND chgrp ${INSTALL_GROUP} ${destination}/ert_module_test)")
   install(CODE "EXECUTE_PROCESS(COMMAND chmod g+w ${destination}/ert_module_test)")
endif()

if (BUILD_TESTS)
   ert_module_name( VAR_RML  rml_enkf  ${LIBRARY_OUTPUT_PATH} )
   add_test( analysis_module_test_RML ${EXECUTABLE_OUTPUT_PATH}/ert_module_test ${VAR_RML})
endif()
