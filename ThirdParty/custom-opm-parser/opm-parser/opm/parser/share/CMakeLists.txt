install( DIRECTORY keywords DESTINATION ${CMAKE_INSTALL_PREFIX}/share/opm/parser/eclipse)

#-----------------------------------------------------------------
# The install() command above will recursively install all the files and
# directories below the current directory. This will potentially include
# editor backup files and other garbage. 
#
# An alternative which queries git for files to install is:
#
# execute_process( COMMAND git ls-tree --name-only -r HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE KEYWORD_LIST_STRING)
# string(REPLACE "\n" ";" KEYWORD_LIST ${KEYWORD_LIST_STRING})
# 
# foreach( keyword ${KEYWORD_LIST} )
#     install( FILES ${keyword} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/opm/parser/eclipse RENAME ${keyword})
# endforeach()
# 
# The disadvantage with this is approach is that:
#
# 1. You must be building from a proper git clone, and can not build
#     from e.g. a .tar.gz distribution
#
# 2. The list of files is configured during the cmake configure
#    process, and will typically be out of sync if you have added more
#    keywords.
#-----------------------------------------------------------------
