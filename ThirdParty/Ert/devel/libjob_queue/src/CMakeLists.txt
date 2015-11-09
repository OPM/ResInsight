#configure_file (${CMAKE_CURRENT_SOURCE_DIR}/CMake/include/libjob_queue_build_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/libjob_queue_build_config.h)


set(source_files job_queue_status.c forward_model.c queue_driver.c job_queue.c job_node.c job_list.c local_driver.c rsh_driver.c torque_driver.c ext_job.c ext_joblist.c workflow_job.c workflow.c workflow_joblist.c job_queue_manager.c)
set(header_files job_queue.h queue_driver.h local_driver.h job_node.h job_list.h rsh_driver.h torque_driver.h ext_job.h ext_joblist.h forward_model.h workflow_job.h workflow.h workflow_joblist.h job_queue_manager.h)
set_property(SOURCE rsh_driver.c PROPERTY COMPILE_FLAGS "-Wno-error")

list( APPEND source_files lsf_driver.c)
list( APPEND header_files lsf_driver.h)

if (HAVE_LSF_LIBRARY)
   list( APPEND source_files lsb.c)
   list( APPEND header_files lsb.h)
endif()

add_library( job_queue SHARED ${source_files} )
set_target_properties( job_queue PROPERTIES VERSION 1.0 SOVERSION 1.0 )
target_link_libraries( job_queue config ert_util )
if (USE_RUNPATH)
   add_runpath( job_queue )
endif()

target_link_libraries( job_queue dl )

if (INSTALL_ERT)
   install(TARGETS job_queue DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/job_queue/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/job_queue)
   endforeach()
endif()
