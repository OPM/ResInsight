install( DIRECTORY cmake DESTINATION share )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
include(init_python)
init_python( 2.7 )


find_python_package(numpy 1.7.1 ${PYTHON_INSTALL_PREFIX})
if (NOT DEFINED PY_numpy)
   message(WARNING "numpy module not found - Python wrappers not enabled. Install with: \"pip install numpy\"")
   set( ENABLE_PYTHON OFF PARENT_SCOPE )
   return()
endif()

find_python_package(cwrap 1 ${PYTHON_INSTALL_PREFIX})
if (NOT DEFINED PY_cwrap)
   message(WARNING "cwrap module not found Python wrappers not enabled. Install with: \"pip install cwrap\"")
   set( ENABLE_PYTHON OFF PARENT_SCOPE )
   return()
endif()

configure_file(test_env.py.in   ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}/test_env.py )

add_subdirectory( ecl )

if (INSTALL_ERT_LEGACY)
   add_subdirectory( ert )
endif()

if (BUILD_TESTS)
   add_subdirectory( tests )
endif()

