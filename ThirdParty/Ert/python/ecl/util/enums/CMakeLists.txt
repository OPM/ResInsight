set(PYTHON_SOURCES
    __init__.py
    rng_alg_type_enum.py
    rng_init_mode_enum.py
)

add_python_package("python.ecl.util.enums"  ${PYTHON_INSTALL_PREFIX}/ecl/util/enums "${PYTHON_SOURCES}" True)

