set(PYTHON_SOURCES
    __init__.py
    local_max_running.py
    lsf_max_running.py
    lsf_queue.py
    lsf_request.py
    queue_system_selector.py
    rsh_command.py
    rsh_host_list.py
    rsh_max_running.py
)

add_python_package("python.ert_gui.models.connectors.queue_system"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/connectors/queue_system "${PYTHON_SOURCES}" True)

