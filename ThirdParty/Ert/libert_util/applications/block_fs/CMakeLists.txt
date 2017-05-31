add_executable( bls bls.c )

target_link_libraries( bls ert_util )

if (USE_RUNPATH)
   add_runpath( bls )
endif()   


set (destination ${CMAKE_INSTALL_PREFIX}/bin)
if (INSTALL_ERT)
   install(TARGETS bls DESTINATION ${destination})
   if (INSTALL_GROUP)
      install(CODE "EXECUTE_PROCESS(COMMAND chgrp ${INSTALL_GROUP} ${destination}/bls)")
      install(CODE "EXECUTE_PROCESS(COMMAND chmod g+w ${destination}/bls)")
   endif()
endif()

