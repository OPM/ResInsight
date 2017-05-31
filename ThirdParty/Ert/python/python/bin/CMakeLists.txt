if (BUILD_ERT)
   set(application_list
        ert_tcp_server
        ertshell
        ert_upgrade_fs107
   )
   foreach(prog ${application_list} )
      file(COPY ${prog} DESTINATION ${EXECUTABLE_OUTPUT_PATH})             
   endforeach()
endif()



if (INSTALL_ERT)
   set (destination ${CMAKE_INSTALL_PREFIX}/bin)
   foreach(prog ${application_list} )
      install(PROGRAMS ${prog} DESTINATION ${destination})
      if (INSTALL_GROUP)
         install(CODE "EXECUTE_PROCESS(COMMAND chgrp ${INSTALL_GROUP} ${destination}/${prog})")
         install(CODE "EXECUTE_PROCESS(COMMAND chmod g+w ${destination}/${prog})")
      endif()
   endforeach()
endif()

