#configure_file (${CMAKE_CURRENT_SOURCE_DIR}/CMake/include/libjob_queue_build_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/libjob_queue_build_config.h)

set(source_files forward_model.c queue_driver.c job_queue.c lsf_driver.c local_driver.c rsh_driver.c ext_job.c ext_joblist.c ext_cmd.c)
set(header_files job_queue.h queue_driver.h lsf_driver.h local_driver.h rsh_driver.h ext_job.h ext_joblist.h forward_model.h ext_cmd.h)

add_library( job_queue ${LIBRARY_TYPE} ${source_files} )
set_target_properties( job_queue PROPERTIES VERSION 1.0 SOVERSION 1.0 )

target_link_libraries( job_queue config ert_util )
if (HAVE_LSF)
   target_link_libraries( job_queue bat ${LSF_LIBRARY} nsl)
endif()                                                   

if (NEED_LIBDL)
   target_link_libraries( job_queue dl )
endif()


install(TARGETS job_queue DESTINATION ${CMAKE_INSTALL_LIBDIR})
foreach(header ${header_files})
    install(FILES ../include/ert/job_queue/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/job_queue)
endforeach()
