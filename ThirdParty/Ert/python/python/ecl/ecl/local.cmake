set( ECL_LOCAL_TARGET  ""  CACHE FILE "Name of optional external ecl_local module")

if (EXISTS ${ECL_LOCAL_TARGET})
   if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
      EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
   endif()

   EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${ECL_LOCAL_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
   add_python_package( "Python ecl.ecl.ecl_local"  ${PYTHON_INSTALL_PREFIX}/ecl/ecl "ecl_local.py" True)
endif()
