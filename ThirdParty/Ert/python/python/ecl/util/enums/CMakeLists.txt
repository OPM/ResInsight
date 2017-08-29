set(PYTHON_SOURCES
    __init__.py
    rng_alg_type_enum.py
    rng_init_mode_enum.py
    ui_return_status_enum.py
    llsq_result_enum.py
    message_level_enum.py
)

add_python_package("python.ecl.util.enums"  ${PYTHON_INSTALL_PREFIX}/ecl/util/enums "${PYTHON_SOURCES}" True)

