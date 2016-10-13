if (HAVE_LSF_LIBRARY)
   add_executable( block_node block_node.c )
   target_link_libraries( block_node job_queue ert_util)
   if (USE_RUNPATH)
      add_runpath( block_node )
   endif()

   install(TARGETS block_node DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()
