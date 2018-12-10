set(PYTHON_SOURCES
    __init__.py
    fault_block.py
    fault_block_layer.py
    fault_collection.py
    fault.py
    fault_line.py
    fault_segments.py
    layer.py
)

add_python_package("python.ecl.grid.faults"  ${PYTHON_INSTALL_PREFIX}/ecl/grid/faults "${PYTHON_SOURCES}" True)
