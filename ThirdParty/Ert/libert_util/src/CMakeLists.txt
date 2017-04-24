include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set(source_files
    rng.c
    lookup_table.c
    statistics.c
    mzran.c
    set.c
    hash_node.c
    hash_sll.c
    hash.c
    node_data.c
    node_ctype.c
    util.c
    msg.c
    arg_pack.c
    path_fmt.c
    menu.c
    subst_list.c
    subst_func.c
    vector.c
    parser.c
    stringlist.c
    matrix.c
    buffer.c
    log.c
    template.c
    timer.c
    time_interval.c
    string_util.c
    type_vector_functions.c
    ui_return.c
    ert_version.c
    struct_vector.c
    perm_vector.c
    test_util.c
)

set(header_files
    ssize_t.h
    type_macros.h
    rng.h
    lookup_table.h
    statistics.h
    mzran.h
    set.h
    hash.h
    hash_node.h
    hash_sll.h
    node_data.h
    node_ctype.h
    util.h
    msg.h
    arg_pack.h
    path_fmt.h
    stringlist.h
    menu.h
    subst_list.h
    subst_func.h
    vector.h
    parser.h
    matrix.h
    buffer.h
    log.h
    template.h
    timer.h
    time_interval.h
    string_util.h
    type_vector_functions.h
    ui_return.h
    struct_vector.h
    buffer_string.h
    perm_vector.h
    ert_version.h
    test_util.h
)



# Add compile time variable values to ert_version.c
if (ERT_WINDOWS)
   execute_process(COMMAND cmd.exe /c "echo %date% %time%"  OUTPUT_VARIABLE BUILD_TIME )   
else() # Linux or Darwin
   execute_process(COMMAND date "+%Y-%m-%d %H:%M:%S" OUTPUT_VARIABLE BUILD_TIME )
endif()
string(STRIP ${BUILD_TIME} BUILD_TIME)

find_package(Git)
if(GIT_FOUND)
   execute_process(
     COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
     OUTPUT_VARIABLE GIT_COMMIT
     OUTPUT_STRIP_TRAILING_WHITESPACE
   )

   execute_process(
     COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
     OUTPUT_VARIABLE GIT_COMMIT_SHORT
     OUTPUT_STRIP_TRAILING_WHITESPACE
   )
else()
   set( GIT_COMMIT "unknown (git not found!)")
   set( GIT_COMMIT_SHORT "unknown (git not found!)")
   message( WARNING "Git not found. Build will not contain git revision info." )
endif()

set_source_files_properties( ert_version.c PROPERTIES COMPILE_DEFINITIONS "COMPILE_TIME_STAMP=\"${BUILD_TIME}\";GIT_COMMIT=\"${GIT_COMMIT}\";GIT_COMMIT_SHORT=\"${GIT_COMMIT_SHORT}\";ERT_VERSION_MAJOR=${ERT_VERSION_MAJOR};ERT_VERSION_MINOR=${ERT_VERSION_MINOR};ERT_VERSION_MICRO=\"${ERT_VERSION_MICRO}\"")


if (CMAKE_COMPILER_IS_GNUCC)
   set_property(SOURCE hash.c PROPERTY COMPILE_FLAGS "-Wno-error")
endif()

if (ERT_HAVE_LAPACK)
   list( APPEND source_files matrix_lapack.c matrix_blas.c matrix_stat.c regression.c lars.c stepwise.c)
   list( APPEND header_files matrix_lapack.h matrix_blas.h matrix_stat.h regression.h lars.h stepwise.h)
endif()

if (ERT_HAVE_UNISTD)
   list( APPEND source_files path_stack.c )
   list( APPEND header_files path_stack.h )
endif()

foreach (type int double bool long time_t size_t float)
  set(TYPE ${type} )      
  set(src_target        ${CMAKE_CURRENT_BINARY_DIR}/${type}_vector.c)    

  configure_file( vector_template.c ${src_target})

  list( APPEND source_files ${src_target} )
endforeach( type )

find_program (PING_PATH ping)
if (PING_PATH)
   add_definitions( -DPING_CMD=\"${PING_PATH}\")
endif()

if (ERT_HAVE_THREAD_POOL)
   list( APPEND header_files thread_pool.h )
   list( APPEND source_files thread_pool.c )
endif()


# The block_fs filesystem is so heavily dependant on pthreads that it is not 
# built if de not have pthreads.

if (HAVE_PTHREAD)
   list( APPEND source_files block_fs.c )
   list( APPEND header_files block_fs.h )
endif()

# The test_work_area depends on that opendir() is available.
if (ERT_HAVE_GETUID AND ERT_HAVE_OPENDIR)
  list( APPEND source_files test_work_area.c)
  list( APPEND header_files test_work_area.h)
endif()

if (ERT_BUILD_CXX)
  list( APPEND header_files test_util.hpp )
endif()

add_library( ert_util   ${LIBRARY_TYPE}  ${source_files} )

message(STATUS "Linking with: ${ERT_EXTERNAL_UTIL_LIBS}")
set_target_properties( ert_util PROPERTIES VERSION ${ERT_VERSION_MAJOR}.${ERT_VERSION_MINOR} SOVERSION ${ERT_VERSION_MAJOR} )
if (USE_RUNPATH)
   add_runpath( ert_util )
endif()   
target_link_libraries( ert_util ${ERT_EXTERNAL_UTIL_LIBS} )

#-----------------------------------------------------------------
if (INSTALL_ERT)
   install(TARGETS ert_util DESTINATION ${CMAKE_INSTALL_LIBDIR})

   foreach(header ${header_files})
       install(FILES ../include/ert/util/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/util)
   endforeach()
endif()



