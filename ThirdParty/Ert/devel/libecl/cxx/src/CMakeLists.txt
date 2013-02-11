set( source_files fortio.cxx ecl_kw.cxx )
set( header_files fortio.hpp ecl_kw.hpp )

add_library( eclxx_static STATIC ${source_files})
add_library( eclxx_shared SHARED ${source_files})

set_target_properties( eclxx_static PROPERTIES OUTPUT_NAME eclxx )
set_target_properties( eclxx_shared PROPERTIES OUTPUT_NAME eclxx )

target_link_libraries( eclxx_static ecl_static )
target_link_libraries( eclxx_shared ecl_shared )

if (ECL_INSTALL_PREFIX)
   install(TARGETS eclxx_static eclxx_shared DESTINATION ${ECL_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${ECL_INSTALL_PREFIX}/include)
else()
   #
   install(TARGETS eclxx_static DESTINATION ${CMAKE_INSTALL_LIBDIR})
   install(FILES ${header_files} DESTINATION include)
endif()        
