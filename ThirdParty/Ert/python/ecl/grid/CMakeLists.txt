set(PYTHON_SOURCES
    __init__.py
    cell.py
    ecl_grid.py
    ecl_region.py
    ecl_grid_generator.py
)

add_python_package("python.ecl.grid" ${PYTHON_INSTALL_PREFIX}/ecl/grid "${PYTHON_SOURCES}" True)

add_subdirectory(faults)
