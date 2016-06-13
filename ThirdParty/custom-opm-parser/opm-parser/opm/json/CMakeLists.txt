set( json_source JsonObject.cpp )
set( json_headers JsonObject.hpp )
if (NOT HAVE_CJSON)
    list(APPEND json_source cjson/cJSON.c)
    set( CJSON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
    set(CJSON_LIBRARY "")
endif()
add_library(opmjson ${json_source})
target_link_libraries( opmjson ${CJSON_LIBRARY} ${Boost_LIBRARIES} )
set_target_properties(opmjson PROPERTIES VERSION ${opm-parser_VERSION_MAJOR}.${opm-parser_VERSION_MINOR}
                                         SOVERSION ${opm-parser_VERSION_MAJOR})

install( TARGETS opmjson DESTINATION ${CMAKE_INSTALL_LIBDIR} )
foreach ( header ${json_headers} ) 
     install( FILES ${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/opm/json RENAME ${header})
endforeach()

add_subdirectory( tests )
