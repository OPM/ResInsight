set(PYTHON_SOURCES
    __init__.py
    ecl.py
    ecl_3d_file.py
    ecl_3dkw.py
    ecl_case.py
    ecl_default.py
    ecl_file.py
    ecl_grav.py
    ecl_grav_calc.py
    ecl_grid.py
    ecl_init_file.py
    ecl_kw.py
    ecl_npv.py
    ecl_region.py
    ecl_restart_file.py
    ecl_rft.py
    ecl_rft_cell.py
    ecl_smspec_node.py
    ecl_subsidence.py
    ecl_sum.py
    ecl_sum_keyword_vector.py
    ecl_sum_node.py
    ecl_sum_tstep.py
    ecl_sum_vector.py
    ecl_util.py
    fortio.py
    ecl_sum_keyword_vector.py
    ecl_cmp.py
    ecl_sum_var_type.py
)

if (BUILD_ERT)
   list(APPEND PYTHON_SOURCES ecl_queue.py)
endif()


add_python_package("python.ert.ecl"  ${PYTHON_INSTALL_PREFIX}/ert/ecl "${PYTHON_SOURCES}" True)



set( ECL_LOCAL_TARGET  ""  CACHE FILE "Name of optional external ecl_local module")

if (EXISTS ${ECL_LOCAL_TARGET})
   if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
      EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
   endif()

   EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${ECL_LOCAL_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
   message(STATUS "Linking ${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py -> ${ECL_LOCAL_TARGET}")
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecl_local.py")
   add_python_package( "python.ert.ecl.ecl_local"  ${PYTHON_INSTALL_PREFIX}/ert/ecl "ecl_local.py" True)
endif()

add_subdirectory(faults)
add_subdirectory(rft)
