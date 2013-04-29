add_python_package("Python ert.ecl"  ${PYTHON_INSTALL_PREFIX}/ert/ecl  "ecl_case.py;ecl_default.py;ecl_file.py;ecl_grav_calc.py;ecl_grav.py;ecl_grid.py;ecl_kw.py;ecl.py;ecl_queue.py;ecl_region.py;ecl_rft.py;ecl_subsidence.py;ecl_sum.py;ecl_util.py;fortio.py;__init__.py;libecl.py" True)

if (EXISTS "ecl_local.py")
   add_python_file( "Python ert.ecl.ecl_local"  ${PYTHON_INSTALL_PREFIX}/ert/ecl "ecl_local.py" True)
endif()
