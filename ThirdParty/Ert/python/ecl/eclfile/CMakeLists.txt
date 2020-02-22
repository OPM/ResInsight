set(PYTHON_SOURCES
    __init__.py
    ecl_3d_file.py
    ecl_file.py
    ecl_file_view.py
    ecl_init_file.py
    ecl_restart_file.py
    ecl_kw.py
    fortio.py
    ecl_3dkw.py

)

add_python_package("python.ecl.eclfile" ${PYTHON_INSTALL_PREFIX}/ecl/eclfile "${PYTHON_SOURCES}" True)
