add_executable( segment_info segment_info.c )
add_executable( CF_dump well_CF_dump.c )
add_executable( ri_well_test ri_well_test.c )

set(program_list segment_info CF_dump ri_well_test)
foreach(prog ${program_list})
   target_link_libraries( ${prog} ecl_well ecl)
   if (USE_RUNPATH)
      add_runpath( ${prog} )
   endif()   

   
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
