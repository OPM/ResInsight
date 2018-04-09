set(PYTHON_SOURCES
    __init__.py
    ecl_npv.py
    ecl_smspec_node.py
    ecl_sum.py
    ecl_sum_keyword_vector.py
    ecl_sum_node.py
    ecl_sum_tstep.py
    ecl_sum_vector.py
    ecl_cmp.py
    ecl_sum_var_type.py
)

add_python_package("python.ecl.summary"  ${PYTHON_INSTALL_PREFIX}/ecl/summary "${PYTHON_SOURCES}" True)
