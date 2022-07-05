# We make this a cmake module so it can be used from opm-simulators' CMakeLists.txt also
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "
import site, sys
try:
    sys.stdout.write(site.getsitepackages()[-1])
except e:
    sys.stdout.write('')" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES_PATH)
  # -------------------------------------------------------------------------
  # 1: Wrap C++ functionality in Python
if (PYTHON_SITE_PACKAGES_PATH MATCHES ".*/dist-packages/?" AND
    CMAKE_INSTALL_PREFIX MATCHES "^/usr.*")
  # dist-packages is only used if we install below /usr and python's site packages
  # path matches dist-packages
  set(PYTHON_PACKAGE_PATH "dist-packages")
else()
  set(PYTHON_PACKAGE_PATH "site-packages")
endif()
if(PYTHON_VERSION_MAJOR)
  set(PY_MAJOR ${PYTHON_VERSION_MAJOR})
else()
  set(PY_MAJOR ${Python3_VERSION_MAJOR})
endif()
if(PYTHON_VERSION_MINOR)
  set(PY_MINOR ${PYTHON_VERSION_MINOR})
else()
  set(PY_MINOR ${Python3_VERSION_MINOR})
endif()
set(PYTHON_INSTALL_PREFIX "lib/python${PY_MAJOR}.${PY_MINOR}/${PYTHON_PACKAGE_PATH}" CACHE STRING "Subdirectory to install Python modules in")
