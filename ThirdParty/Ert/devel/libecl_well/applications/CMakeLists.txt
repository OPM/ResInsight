add_executable( well_info_test well_info_test.c )
add_executable( segment_info segment_info.c )

set(program_list well_info_test segment_info )
foreach(prog ${program_list})
   target_link_libraries( ${prog} ecl_well ecl)
   
   #-----------------------------------------------------------------

   if (WELL_INSTALL_PREFIX)
      set (destination ${CMAKE_INSTALL_PREFIX}/${WELL_INSTALL_PREFIX}/bin)
   else()
      set (destination ${CMAKE_INSTALL_PREFIX}/bin)
   endif()

   if (INSTALL_ERT)
      install(TARGETS ${prog} DESTINATION ${destination})
      if (INSTALL_GROUP)
         install(CODE "EXECUTE_PROCESS(COMMAND chgrp ${INSTALL_GROUP} ${destination}/${prog})")
         install(CODE "EXECUTE_PROCESS(COMMAND chmod g+w ${destination}/${prog})")
      endif()
   endif()
endforeach()
