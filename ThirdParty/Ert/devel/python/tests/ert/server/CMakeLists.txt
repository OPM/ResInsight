set(TEST_SOURCES
    __init__.py
    test_server.py
    test_socket.py
)

add_python_package("python.tests.ert.server" ${PYTHON_INSTALL_PREFIX}/tests/ert/server "${TEST_SOURCES}" False)

addPythonTest(ert.server.socket tests.ert.server.test_socket.SocketTest LABELS StatoilData)
addPythonTest(ert.server.server tests.ert.server.test_server.ServerTest LABELS StatoilData)