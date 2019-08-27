# Compiling and Installing **_libecl_** on Windows

## Prerequisits:
* Python 2.7 or 3.x https://www.python.org/ or https://anaconda.org/
* Microsoft Visual Studio  https://visualstudio.microsoft.com/downloads/
* Local copy of **_libecl_** 

## Instructions:
1. Download or clone the **_libecl_** Github repository to your local disk.

2. Python 2.7 or 3.x installation
   - Download a python instalation or a python environment solution such as Anaconda.

3.  Download and install Microsoft Visual Studio . At a minimum **_libecl_** requires the VS Studio packages for cmake, msbuild, c and c++ compilers (CL.exe).

4. Open a MSVC command prompt such as _x64 Native Tools Command Prompt for VS 2017_ from your start menu. In the open prompt, navigate to the **_libecl_** source directory you created in step 1. Use the Python package manager **pip** to install **_libecl_** requirements via `pip install -r requirements.txt`. If Python is not accessible from the prompt it may be necessary to add the Python environment location to your system path variable `PATH`. 
    
5. Execute the build commands with the desired CMAKE parameters from `README.md`. The cmake generator can be _`NMake Makefiles`_ , _`Ninja`_ or an appropriate version of _`MSVC`_. For the availble options type `cmake -G` in the MSVC command prompt.  

   An example build and install is provided below where %VARIABLE% are user defined directory paths:
~~~~
    cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX=%INSTALLPATH% -DBUILD_SHARED_LIBS="ON"  -DENABLE_PYTHON="ON"    -DCMAKE_BUILD_TYPE="Release" %SOURCEPATH%
    cmake --build %BUILDPATH% --config Release --target install
~~~~
6. For **_libecl_** to be accessible in Python the `%INSTALLPATH%\lib\pythonX.Y\site-package` and Python subdirectories must be added to the `PATH` and `PYTHONPATH` variables. Where `pythonx.y` is the current Python version _e.g._ (`python2.7`, `python3.6` _etc._) .

8. Open a Python interactive session and run `import ecl` to check that the install and paths are now set.