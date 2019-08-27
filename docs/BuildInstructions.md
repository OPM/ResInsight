---
layout: docs
title: Build Instructions
permalink: /docs/buildinstructions/
published: true
---

## Source code
The source code is hosted at [GitHub](https://github.com/opm/resinsight)

In a git enabled shell do: `git clone https://github.com/OPM/ResInsight.git`

## Dependencies and Prerequisites

### Windows Compiler

Visual Studio 2015 or later is supported.

### GCC Compiler

GCC version 4.9 or later is supported. On RedHat Linux 6 you need to install devtoolset-3, and enable it with 
    
    source /opt/rh/devtoolset-3/enable

### Qt4

[Qt](http://download.qt.io/archive/qt/) Qt4 version 4.6.2 or later is supported. On Windows we recommend Qt-4.8.7, while the default installation will do under Linux. 

You will need to patch the Qt sources in order to make them build using Visual Studio 2015 using this : 
[Qt-patch](https://github.com/appleseedhq/appleseed/wiki/Making-Qt-4.8.7-compile-with-Visual-Studio-2015) 

### Qt5 (BETA)
Qt5 is now supported as beta functionality. To use Qt5, specify the cmake variable 

`RESINSIGHT_BUILD_WITH_QT5=TRUE`

and then specify location of Qt5

`Qt5_DIR=d:\Qt\5.11.3\msvc2017_64\lib\cmake\Qt5`


### CMake
[CMake](https://cmake.org/download/) version 2.8 or later is supported.

## Build Instructions
The ResInsight build may be configured in different ways, with optional support for Octave plugins, 
ABAQUS ODB API, HDF5, and OpenMP. This is configured using options in CMake.

If you check the button 'Grouped' in the CMake GUI, the CMake variables are grouped by prefix. 
This makes it easier to see all of the options for ResInsight.

- Open the CMake GUI
- Set the path to the source code
- Set the path to the build directory
- Click **Configure** and select your preferred compiler
- Set the build options and click "Configure" again (see ResInsight specific options below)
- Click **Generate** to generate the makefiles or solution file and project files in the build directory
- Run the compiler using the generated makefiles or solution file/project files to build ResInsight

### Windows
ResInsight has been verified to build and run on Windows 7/8/10 using Microsoft Visual Studio 2015/2017. 
Typical usage on Windows is to follow the build instructions above, and then open the generated 
solution file in Visual Studio to build the application.

### Linux

Typical usage is to follow the build instructions above to build the makefiles. Then go to the build directory, and run:

- make
- make install

To build from the command line without using the CMake GUI:

- mkdir ResInsight_build
- cd ResInsight_build
- ...
- (set CMake options)
- ...
- cmake < path to ResInsight source folder >
- make
- make install

You will find the ResInsight binary under the Install directory in your build directory.

### CMake Options for ResInsight

| CMake Name                                        | Default | Description                                                           |
|---------------------------------------------------|---------|-----------------------------------------------------------------------|
| `RESINSIGHT_BUILD_DOCUMENTATION`                  | OFF     | Use Doxygen to create the HTML based API documentation. Doxygen must be properly installed. |
| `RESINSIGHT_HDF5_DIR`                             | Blank   | Windows Only: Optional path to HDF5 libraries on Windows |
| `RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS`       | OFF     | Include Application Code Unit Tests |
| `RESINSIGHT_ODB_API_DIR`                          | Blank   | Optional path to the ABAQUS ODB API from Simulia. Needed for support of geomechanical models |
| `RESINSIGHT_USE_OPENMP`                           | ON      | Enable OpenMP parallellization in the code |

#### Advanced Options

| CMake Name                                        | Default | Description                              |
|---------------------------------------------------|---------|--------------------------------------------------------|
| `RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_SOURING`     | ON     | Enable Souring features |
| `RESINSIGHT_PRIVATE_INSTALL`                      | ON      | Linux only: Install the libecl shared libraries along the executable |
| `RESINSIGHT_ENABLE_COTIRE`                        | OFF     | Experimental speedup of compilation using cotire |
| `RESINSIGHT_HDF5_BUNDLE_LIBRARIES`                | OFF     | Bundle HDF5 libraries with ResInsight  |

### Optional Libraries and features

#### HDF5

HDF5 is used to read SourSimRL result files. On Windows this is optional, while on Linux the installed HDF5 library will be used if present.

Use an advanced flag RESINSIGHT_HDF5_BUNDLE_LIBRARIES to enable bundling of HDF5 libraries.

Tested with 1.8.18 on windows, and default installation on RedHat 6.

#### Octave

Octave is now detected searching the file system, and will usually find Octave on Linux. If Octave is not detected, the following file path variable must be defined

`OCTAVE_CONFIG_EXECUTABLE : d:\octave\Octave-4.0.0\bin\octave-config.exe`

It is possible to build ResInsight without compiling the Octave plugins. This can be done by specifying blank for 
the Octave CMake options. The Octave plugin module will not be built, and CMake will show warnings like 'Failed to find mkoctfile'. 
This will not break the build or compilation of ResInsight.

ResInsight has been verified to build and run with Octave versions 3.4.3, 3.8.1, and 4.0.0 on RedHat linux, and 4.0.0 on Windows.

##### Octave Dependencies for Debian Based Distributions

- sudo apt-get install git cmake build-essential octave octave-headers qt4-dev-tools

If you are running Ubuntu 12.10 or newer, you will need to replace octave-headers with liboctave-dev :

- sudo apt-get install git cmake build-essential octave liboctave-dev qt4-dev-tools

#### ODB support

ResInsight can be built with support for ABAQUS ODB files. This requires an installation of the ABAQUS ODB API 
from Simulia on the build computer. The path to the ABAQUS ODB API folder containing header files and library 
files must be specified. Leaving this option blank gives a build without ODB support. 
ResInsight has been built and tested with ABAQUS ODB API version 6.14-3 on Windows 7/8/10 and RedHat Linux 6.
