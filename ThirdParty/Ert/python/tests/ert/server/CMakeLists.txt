set(TEST_SOURCES
    __init__.py
    test_simulation_context.py
    test_rpc_service.py
    test_rpc_storage.py
)

add_python_package("python.tests.ert.server" ${PYTHON_INSTALL_PREFIX}/tests/ert/server "${TEST_SOURCES}" False)

addPythonTest(ert.server.simulation_context tests.ert.server.test_simulation_context.SimulationContextTest)
addPythonTest(ert.server.rpc_service tests.ert.server.test_rpc_service.RPCServiceTest)
addPythonTest(ert.server.rpc_storage tests.ert.server.test_rpc_storage.RPCStorageTest)