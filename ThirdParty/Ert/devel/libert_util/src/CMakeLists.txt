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
    thread_pool.c
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
    version.c
    struct_vector.c
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
    thread_pool.h
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
)

set( test_source test_util.c )
set( test_headers test_util.h )


# Add compile time variable values to version.c
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

set_source_files_properties( version.c PROPERTIES COMPILE_DEFINITIONS "COMPILE_TIME_STAMP=\"${BUILD_TIME}\";GIT_COMMIT=\"${GIT_COMMIT}\";GIT_COMMIT_SHORT=\"${GIT_COMMIT_SHORT}\";ERT_VERSION_MAJOR=${ERT_VERSION_MAJOR};ERT_VERSION_MINOR=${ERT_VERSION_MINOR};ERT_VERSION_MICRO=\"${ERT_VERSION_MICRO}\"")


if (CMAKE_COMPILER_IS_GNUCC)
   set_property(SOURCE hash.c PROPERTY COMPILE_FLAGS "-Wno-error")
endif()


if (WITH_LATEX)
   if (HAVE_FORK)
      add_definitions( -DWITH_LATEX )
      add_definitions( -DLATEX_CMD=\"${LATEX_PATH}\")

      list( APPEND source_files latex.c )
      list( APPEND header_files latex.h )
   else()
      message( STATUS "Disabling latex support due to missing fork() call")
   endif()
endif()

if (WITH_LAPACK)
   list( APPEND source_files matrix_lapack.c matrix_blas.c matrix_stat.c regression.c lars.c stepwise.c)
   list( APPEND header_files matrix_lapack.h matrix_blas.h matrix_stat.h regression.h lars.h stepwise.h)
endif()

if (UNISTD_HEADER)
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

# The block_fs filesystem is so heavily dependant on pthreads that it is not 
# built if de not have pthreads.
if (ERT_BUILD_BLOCK_FS)
  list( APPEND source_files block_fs.c )
  list( APPEND header_files block_fs.h )

  list( APPEND header_files thread_pool_posix.h )
endif()

# The test_work_area depends on that opendir() is available.
if (HAVE_GETUID AND HAVE_OPENDIR)
  list( APPEND source_files test_work_area.c)
  list( APPEND header_files test_work_area.h )
endif()

if (ERT_BUILD_CXX)
  list( APPEND header_files test_util.hpp )
endif()

add_library( ert_util   ${LIBRARY_TYPE}  ${source_files} )
if (BUILD_TESTS)
   add_library( test_util  ${LIBRARY_TYPE}  ${test_source} )
   target_link_libraries( test_util ert_util )
endif()

set_target_properties( ert_util PROPERTIES VERSION 1.0 SOVERSION 1.0 )
if (USE_RUNPATH)
   add_runpath( ert_util )
endif()   


if (WITH_PTHREAD)
  target_link_libraries( ert_util ${PTHREAD_LIBRARY} )
  include_directories( ${PTHREAD_HEADER} )
endif()   

if (WITH_ZLIB)
  target_link_libraries( ert_util ${ZLIB_LIBRARY} )
  include_directories( ${ZLIB_HEADER} )
endif()   

if ( ERT_WINDOWS )
  target_link_libraries( ert_util ${SHWLAPI_LIBRARY} )
endif()

if (WITH_LAPACK)
   target_link_libraries( ert_util ${LAPACK_LIBRARY} )
   if (NEED_BLAS)
     target_link_libraries( ert_util ${BLAS_LIBRARY} )
   endif()
endif()

if (NEED_LIBM)
   target_link_libraries( ert_util m )
endif()
if (NEED_LIBDL)
   target_link_libraries( ert_util dl )
endif()

#-----------------------------------------------------------------
if (INSTALL_ERT)
   install(TARGETS ert_util DESTINATION ${CMAKE_INSTALL_LIBDIR})

   foreach(header ${header_files})
       install(FILES ../include/ert/util/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/util)
   endforeach()
endif()



