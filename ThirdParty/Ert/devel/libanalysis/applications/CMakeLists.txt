add_executable( ert_module_test ert_module_test.c )
target_link_libraries( ert_module_test analysis ert_util )

if (USE_RUNPATH)
   add_runpath( ert_module_test )
endif()

install(TARGETS ert_module_test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
