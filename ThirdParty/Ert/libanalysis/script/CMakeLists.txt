set (destination ${CMAKE_INSTALL_PREFIX}/bin)

install(PROGRAMS ert_module DESTINATION ${destination})

if (INSTALL_GROUP)
   install(CODE "EXECUTE_PROCESS(COMMAND chgrp ${INSTALL_GROUP} ${destination}/ert_module)")
   install(CODE "EXECUTE_PROCESS(COMMAND chmod g+w ${destination}/ert_module)")
endif()
