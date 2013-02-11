link_directories( ${ERT_BINARY_DIR}/libsched/src )

include_directories( ${libutil_build_path} )
include_directories( ${libutil_src_path} )
include_directories( ${libsched_src_path} )
include_directories( ${libecl_src_path} )

add_executable( sched_summary.x sched_summary.c )

set(program_list sched_summary.x)
foreach(prog ${program_list})
   target_link_libraries( ${prog} sched ecl ert_util )
   #-----------------------------------------------------------------
   if (SCHED_INSTALL_PREFIX)
      set (destination ${SCHED_INSTALL_PREFIX}/bin)
   else()
      set (destination bin)
   endif()

   install(TARGETS ${prog} DESTINATION ${destination})
   if (INSTALL_GROUP)
      install(CODE "EXECUTE_PROCESS(COMMAND chgrp ${INSTALL_GROUP} ${destination}/${prog})")
      install(CODE "EXECUTE_PROCESS(COMMAND chmod g+w ${destination}/${prog})")
   endif()
endforeach()
