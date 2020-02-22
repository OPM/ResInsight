set(PYTHON_SOURCES
    __init__.py
)

add_python_package("python.ecl.util"  ${PYTHON_INSTALL_PREFIX}/ecl/util "${PYTHON_SOURCES}" True)

add_subdirectory(enums)
add_subdirectory(util)
add_subdirectory(test)
add_subdirectory(geometry)
