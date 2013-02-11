set(source_files rng.c lookup_table.c statistics.c mzran.c set.c hash_node.c hash_sll.c hash.c node_data.c node_ctype.c util.c thread_pool.c msg.c arg_pack.c path_fmt.c menu.c subst_list.c subst_func.c vector.c parser.c stringlist.c matrix.c buffer.c log.c template.c timer.c test_util.c)


set(header_files ssize_t.h type_macros.h rng.h lookup_table.h statistics.h mzran.h set.h hash.h hash_node.h hash_sll.h node_data.h node_ctype.h util.h thread_pool.h msg.h arg_pack.h path_fmt.h  stringlist.h menu.h subst_list.h subst_func.h vector.h parser.h matrix.h buffer.h log.h template.h timer.h test_util.h)

if (WITH_LATEX)
   add_definitions( -DWITH_LATEX )
   add_definitions( -DLATEX_CMD=\"${LATEX_PATH}\")

   list( APPEND source_files latex.c )
   list( APPEND header_files latex.h )
endif()

if (WITH_LAPACK)
   list( APPEND source_files matrix_lapack.c matrix_blas.c regression.c lars.c stepwise.c)
   list( APPEND header_files matrix_lapack.h matrix_blas.h regression.h lars.h stepwise.h)
endif()

foreach (type int double bool long time_t size_t float)
  set(TYPE ${type} )      
  set(src_target        ${CMAKE_CURRENT_BINARY_DIR}/${type}_vector.c)    

  configure_file( vector_template.c ${src_target})

  list( APPEND source_files ${src_target} )
endforeach( type )


#if (WITH_ZLIB)
#   list( APPEND source_files util_zlib.c )
#   list( APPEND header_files util_zlib.h )
#endif()

# The block_fs filesystem is so heavily dependant on pthreads that it is not 
# built if de not have pthreads.
if (WITH_PTHREAD)
  list( APPEND source_files block_fs.c )
  list( APPEND header_files block_fs.h )

  list( APPEND header_files thread_pool_posix.h )
endif()

add_library( ert_util  ${LIBRARY_TYPE}  ${source_files} )
set_target_properties( ert_util PROPERTIES VERSION 1.0 SOVERSION 1.0 )

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

#-----------------------------------------------------------------
install(TARGETS ert_util DESTINATION ${CMAKE_INSTALL_LIBDIR})
foreach(header ${header_files})
    install(FILES ../include/ert/util/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/util)
endforeach()




