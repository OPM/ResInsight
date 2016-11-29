add_library(copmparser SHARED
cparser.cc
cdeck.cc cdeck_keyword.cc cdeck_record.cc cdeck_item.cc
cparse_context.cc
ctable_manager.cc
ctable.cc
ctable_index.cc
ceclipse_grid.cc
cschedule.cc
)

target_link_libraries(copmparser opmparser)
set_target_properties(copmparser PROPERTIES VERSION ${opm-parser_VERSION_MAJOR}.${opm-parser_VERSION_MINOR}
                                            SOVERSION ${opm-parser_VERSION_MAJOR})
install( TARGETS copmparser DESTINATION ${CMAKE_INSTALL_LIBDIR} )
