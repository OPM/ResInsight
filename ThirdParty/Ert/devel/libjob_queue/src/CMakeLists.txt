#configure_file (${CMAKE_CURRENT_SOURCE_DIR}/CMake/include/libjob_queue_build_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/libjob_queue_build_config.h)

set(source_files forward_model.c queue_driver.c job_queue.c local_driver.c rsh_driver.c ext_job.c ext_joblist.c workflow_job.c workflow.c workflow_joblist.c)
set(header_files job_queue.h queue_driver.h local_driver.h rsh_driver.h ext_job.h ext_joblist.h forward_model.h workflow_job.h workflow.h workflow_joblist.h)

if (USE_LSF)
   list( APPEND source_files lsf_driver.c)
   list( APPEND header_files lsf_driver.h)
   
   if (HAVE_LSF_LIBRARY)
      list( APPEND source_files lsb.c)
      list( APPEND header_files lsb.h)
   endif()
endif()

add_library( job_queue SHARED ${source_files} )
set_target_properties( job_queue PROPERTIES VERSION 1.0 SOVERSION 1.0 )
target_link_libraries( job_queue config ert_util )

if (NEED_LIBDL)
   target_link_libraries( job_queue dl )
endif()

if (INSTALL_ERT)
   install(TARGETS job_queue DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/job_queue/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/job_queue)
   endforeach()
endif()
