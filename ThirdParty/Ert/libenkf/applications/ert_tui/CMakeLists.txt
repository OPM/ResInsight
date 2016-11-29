include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set( src_list main.c enkf_tui_main.c  enkf_tui_fs.c  enkf_tui_ranking.c  enkf_tui_misc.c  enkf_tui_table.c  
              enkf_tui_run.c enkf_tui_util.c  enkf_tui_init.c  enkf_tui_export.c  enkf_tui_analysis.c enkf_tui_help.c enkf_tui_simple.c enkf_tui_workflow.c)

execute_process(COMMAND date "+%Y-%m-%d %H:%M:%S" OUTPUT_VARIABLE BUILD_TIME )
string(STRIP ${BUILD_TIME} BUILD_TIME)

find_package(Git)
if(GIT_EXECUTABLE)
   execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
                   WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                   OUTPUT_VARIABLE GIT_COMMIT)
   string(STRIP "${GIT_COMMIT}" GIT_COMMIT)
else()
   set(GIT_COMMIT)
endif()

set_source_files_properties( main.c PROPERTIES COMPILE_DEFINITIONS "COMPILE_TIME_STAMP=\"${BUILD_TIME}\";GIT_COMMIT=\"${GIT_COMMIT}\"")

add_executable( ert_tui ${src_list} )
target_link_libraries( ert_tui enkf sched rms ecl config job_queue analysis ert_util )
if (USE_RUNPATH)
   add_runpath( ert_tui )
endif()

set (destination ${CMAKE_INSTALL_PREFIX}/bin)

if (INSTALL_ERT)
   install(TARGETS ert_tui DESTINATION ${destination})
   if (INSTALL_GROUP)
      install(CODE "EXECUTE_PROCESS(COMMAND chgrp ${INSTALL_GROUP} ${destination}/ert_tui)")
      install(CODE "EXECUTE_PROCESS(COMMAND chmod g+w ${destination}/ert_tui)")
   endif()
endif()
