set(PYTHON_SOURCES
    __init__.py
    ert_server.py
    ert_socket.py
    ert_client.py
    run_context.py
 )

add_python_package("python.ert.server"  ${PYTHON_INSTALL_PREFIX}/ert/server "${PYTHON_SOURCES}" True)

