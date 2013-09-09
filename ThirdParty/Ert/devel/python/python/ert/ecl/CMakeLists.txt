SET(PYTHON_SOURCES
    __init__.py
    ecl.py
    ecl_case.py
    ecl_default.py
    ecl_file.py
    ecl_grav.py
    ecl_grav_calc.py
    ecl_grid.py
    ecl_kw.py
    ecl_queue.py
    ecl_region.py
    ecl_rft.py
    ecl_rft_cell.py
    ecl_smspec_node.py
    ecl_subsidence.py
    ecl_sum.py
    ecl_sum_node.py
    ecl_sum_vector.py
    ecl_util.py
    fortio.py

 )

add_python_package("Python ert.ecl"  ${PYTHON_INSTALL_PREFIX}/ert/ecl  "${PYTHON_SOURCES}" True)


set( ECL_LOCAL_TARGET  ""  CACHE FILE "Name of optional external ecl_local module")

if (EXISTS ${ECL_LOCAL_TARGET})
   if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
      EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
   endif() 

   EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${ECL_LOCAL_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")            
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
   add_python_package( "Python ert.ecl.ecl_local"  ${PYTHON_INSTALL_PREFIX}/ert/ecl "ecl_local.py" True)
endif()
