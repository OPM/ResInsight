add_python_target(python_ecl ${PYTHON_INSTALL_PREFIX}/ert/ecl "ecl_case;ecl_default;ecl_file;ecl_grav_calc;ecl_grav;ecl_grid;ecl_kw;ecl;ecl_queue;ecl_region;ecl_rft;ecl_subsidence;ecl_sum;ecl_util;fortio;__init__;libecl")

if (EXISTS "ecl_local.py")
   add_python_target( python_ecl_local ${PYTHON_INSTALL_PREFIX}/ert/ecl "ecl_local")
endif()
