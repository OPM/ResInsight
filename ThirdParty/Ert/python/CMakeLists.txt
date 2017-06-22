install( DIRECTORY cmake DESTINATION share )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
include(init_python)
init_python( 2.7 )


find_python_package(numpy 1.7.1 ${PYTHON_INSTALL_PREFIX})
if (NOT DEFINED PY_numpy)
   message(WARNING "numpy module not found - Python wrappers not enabled")
   set( BUILD_PYTHON OFF PARENT_SCOPE )
   return()
endif()


if (BUILD_TESTS)
   add_subdirectory( tests )
endif()
add_subdirectory( python )


