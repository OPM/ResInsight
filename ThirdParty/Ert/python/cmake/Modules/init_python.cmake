# This macro will initialize the current cmake session for Python. The
# macro starts by looking for the Python interpreter of correct
# version. When a Python interepreter of the correct version has been
# located the macro will continue to set variables, load other cmake
# modules and generate scripts to be used in the remaining part of the
# cmake process. 
#
# Variables which will be set:
# ----------------------------
#
# PYTHON_INSTALL_PREFIX: All python packages will be located in
#        ${GLOBAL_PREFIX}/${PYTHON_INSTALL_PREFIX} - this applies both
#        when searching for dependencies and when installing.
#
# CTEST_PYTHONPATH: Normal ':' separated path variables which is
#        passed to the test runner. Should contain the PYTHONPATH to
#        all third party packages which are not in the default search
#        path. The CTEST_PYTHONPATH variable will be updated by the
#        python_package( ) function when searching for third party
#        packages.
#
#
# New functions/macros which will be available:
# ---------------------------------------------
#
# add_python_package( ): This function will copy python source files
#        to the build directory, 'compile' them and set up installation.
#
#
# add_python_test( ): Set up a test based on invoking a Python test
#        class with a small python executable front end.
#
# find_python_package( ): Will search for a python package.
#
#
# New scripts generated:
# ----------------------
#
#
# cmake_pyc: Small script which will run in-place Python compilation
#        of a directory tree recursively.
#
# cmake_pyc_file: Small script which will compile one python file.
#
# ctest_run_python: Small script which will invoke one Python test class.
#
# All the generated scripts will be located in ${PROJECT_BINARY_DIR}/bin.
#
#
# Downstream projects should use this as:
#
# include( init_python )
# init_python( 2.7 )
# ...

macro(init_python target_version)

   FIND_PACKAGE(PythonInterp)
   if (NOT DEFINED PYTHON_EXECUTABLE)
      message(WARNING "Python interpreter not found - Python wrappers not enabled")
      set( BUILD_PYTHON OFF PARENT_SCOPE )
      return()
   endif()

   if (NOT "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}" STREQUAL "${target_version}")
      message(WARNING "Need Python version ${target_version}, found version: ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} - Python wrappers not enabled")
      set( BUILD_PYTHON OFF PARENT_SCOPE )
      return()
   endif()

   if (EXISTS "/etc/debian_version")
      set( PYTHON_PACKAGE_PATH "dist-packages")
   else()
      set( PYTHON_PACKAGE_PATH "site-packages")
   endif()

   set(PYTHON_INSTALL_PREFIX  "lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/${PYTHON_PACKAGE_PATH}"  CACHE STRING "Subdirectory to install Python modules in")
   set(CTEST_PYTHONPATH ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX})
   configure_python_env( )
   include(add_python_test)
   include(find_python_package) 
   include(add_python_package)
endmacro()



# The function configure_python_env( ) will generate three small
# Python scripts which will be located in ${PROJECT_BINARY_DIR}/bin
# and will be used when 'compiling' and testing Python code. The
# function will be called from the init_python() macro.

function( configure_python_env )

FILE(WRITE "${PROJECT_BINARY_DIR}/bin/ctest_run_python"
"import sys
import os
from unittest import TextTestRunner


def runTestCase(tests, verbosity=0):
    test_result = TextTestRunner(verbosity=verbosity).run(tests)

    if len(test_result.errors) or len(test_result.failures):
        test_result.printErrors()
        sys.exit(1)


def update_path():
    for path in os.environ['CTEST_PYTHONPATH'].split(':'):
        sys.path.insert(0 , path)
        
    
if __name__ == '__main__':
    update_path( )
    from ecl.test import ErtTestRunner

    for test_class in sys.argv[1:]:
        tests = ErtTestRunner.getTestsFromTestClass(test_class)
        
        # Set verbosity to 2 to see which test method in a class that fails.
        runTestCase(tests, verbosity=0)
")    

#-----------------------------------------------------------------

FILE(WRITE "${PROJECT_BINARY_DIR}/bin/cmake_pyc"
"
import py_compile
import os
import os.path
import sys
import shutil


src_file = sys.argv[1]
target_file = sys.argv[2]

(target_path , tail) = os.path.split( target_file )
if not os.path.exists( target_path ):
   try:
      os.makedirs( target_path )
   except:
      # When running make with multiple processes there might be a
      # race to create this directory.
      pass

shutil.copyfile( src_file , target_file )
shutil.copystat( src_file , target_file )
try:
    py_compile.compile( target_file , doraise = True)
except Exception as error:
    sys.exit('py_compile(%s) failed:%s' % (target_file , error))
")

#-----------------------------------------------------------------

FILE(WRITE "${PROJECT_BINARY_DIR}/bin/cmake_pyc_file"
"
import py_compile
import os
import sys
import os.path

# Small 'python compiler' used in the build system for ert. 

for file in sys.argv[1:]:
    try:
        py_compile.compile( file , doraise = True )
    except Exception as error:
        sys.exit('py_compile(%s) failed:%s' % (file , error))
")
        


endfunction()