add_executable( well_info_test.x well_info_test.c )

include_directories( ${libecl_src_path} )
include_directories( ${libwell_src_path} )
include_directories( ${libutil_src_path} )
include_directories( ${libutil_build_path} )

set(program_list well_info_test.x )
foreach(prog ${program_list})
   target_link_libraries( ${prog} well ecl util )
   
   #-----------------------------------------------------------------

   if (WELL_INSTALL_PREFIX)
      set (destination ${CMAKE_INSTALL_PREFIX}/${WELL_INSTALL_PREFIX}/bin)
   else()
      set (destination ${CMAKE_INSTALL_PREFIX}/bin)
   endif()

   install(TARGETS ${prog} DESTINATION ${destination})
   if (INSTALL_GROUP)
      install(CODE "EXECUTE_PROCESS(COMMAND chgrp ${INSTALL_GROUP} ${destination}/${prog})")
      install(CODE "EXECUTE_PROCESS(COMMAND chmod g+w ${destination}/${prog})")
   endif()
endforeach()
