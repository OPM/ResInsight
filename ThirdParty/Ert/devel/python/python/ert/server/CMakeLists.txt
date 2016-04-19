set(PYTHON_SOURCES
    __init__.py
    ertrpcclient.py
    ertrpcserver.py
    simulation_context.py
)

add_python_package("python.ert.server" ${PYTHON_INSTALL_PREFIX}/ert/server "${PYTHON_SOURCES}" True)

