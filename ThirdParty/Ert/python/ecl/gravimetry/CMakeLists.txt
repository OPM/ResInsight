set(PYTHON_SOURCES
    __init__.py
    ecl_grav.py
    ecl_grav_calc.py
    ecl_subsidence.py

)

add_python_package("python.ecl.gravimetry"  ${PYTHON_INSTALL_PREFIX}/ecl/gravimetry "${PYTHON_SOURCES}" True)
