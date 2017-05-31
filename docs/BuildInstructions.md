---
layout: docs
title: Build Instructions
permalink: /docs/buildinstructions/
published: true
---

## Source code
The source code is hosted at [GitHub](https://github.com/opm/resinsight)

## Dependencies and prerequisites

### Windows compiler

Visual Studio 2015 or later is supported.

### GCC compiler

GCC version 4.9 or later is supported. On RedHat Linux 6 you need to install devtoolset-3, and enable it with 
    
    source /opt/rh/devtoolset-3/enable

### Qt

[Qt](http://download.qt.io/archive/qt/) Qt4 version 4.6.2 or later is supported. Qt5 is not supported yet.
On Windows we recommend Qt-4.8.7, while the default installation will do under Linux. 

You will need to patch the Qt sources in order to make them build using Visual Studion 2015 using this : [Qt-patch](https://github.com/appleseedhq/appleseed/wiki/Making-Qt-4.8.7-compile-with-Visual-Studio-2015) 

### CMake
[CMake](https://cmake.org/download/) version 2.8 or later is supported.

## Build instructions
The ResInsight build may be configured in different ways, with optional support for Octave plugins, ABAQUS ODB API, and OpenMP. This is configured using options in CMake.

If you check the button 'Grouped' in the CMake GUI, the CMake variables are grouped by prefix. This makes it easier to see all of the options for ResInsight.

- Open the CMake GUI
- Set the path to the source code
- Set the path to the build directory
- Click **Configure** and select your preferred compiler
- Set the build options and click "Configure" again (see ResInsight specific options below)
- Click **Generate** to generate the makefiles or solution file and project files in the build directory
- Run the compiler using the generated makefiles or solution file/project files to build ResInsight

### Windows
ResInsight has been verified to build and run on Windows 7/8/10 using Microsoft Visual Studio 2015. Typical usage on Windows is to follow the build instructions above, and then open the generated solution file in Visual Studio to build the application.


### Linux

ResInsight has been verified to build and run on RedHat Linux 6, but you need to install the  Typical usage is to follow the build instructions above to build the makefiles. Then go to the build directory, and run:

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

### General CMake options for ResInsight

| CMake Name   | Description |
|--------------|---------|
| `RESINSIGHT_BUILD_DOCUMENTATION`      | Use Doxygen to create the HTML based API documentation |
| `RESINSIGHT_PRIVATE_INSTALL`          | Install as an independent bundle including the necessary Qt libraries |
| `RESINSIGHT_USE_OPENMP`               | Enable OpenMP parallellization in the code |

### Optional - Octave plugins 
To be able to compile the Octave plugins, the path to the Octave development tool _`mkoctfile`_ must be provided.

It is possible to build ResInsight without compiling the Octave plugins. This can be done by specifying blank for the Octave CMake options. The Octave plugin module will not be built, and CMake will show warnings like 'Failed to find mkoctfile'. This will not break the build or compilation of ResInsight.

ResInsight has been verified to build and run with Octave versions 3.4.3, 3.8.1, and 4.0.0.

#### Octave related CMake options for ResInsight

| CMake Name   | Description |
|--------------|---------|
| `RESINSIGHT_OCTAVE_PLUGIN_32_BIT`     | Set 32-bit MSVC compiler environment while running mkoctfile |
| `RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE`  | Location of Octave tool mkoctfile used to compile Octave plugins |
| `RESINSIGHT_OCTAVE_PLUGIN_QMAKE`      | Location of Qt version to use when compiling Octave plugins. Must be compatible with Octave runtime. (Use the Qt version embedded in Octave. The qmake executable itself is not used, only the path to the directory.) |

#### Octave Dependencies for Debian Based Distributions

- sudo apt-get install git cmake build-essential octave octave-headers qt4-dev-tools

If you are running Ubuntu 12.10 or newer, you will need to replace octave-headers with liboctave-dev :

- sudo apt-get install git cmake build-essential octave liboctave-dev qt4-dev-tools

### Optional - ABAQUS ODB API 

ResInsight can be built with support for ABAQUS ODB files. This requires an installation of the ABAQUS ODB API from Simulia on the build computer. The path to the ABAQUS ODB API folder containing header files and library files must be specified. Leaving this option blank gives a build without ODB support. ResInsight has been built and tested with ABAQUS ODB API version 6.14-3 on Windows 7/8 and RedHat Linux 6.

ABAQUS ODB API related CMake options for ResInsight

| CMake Name   | Description |
|--------------|---------|
| `RESINSIGHT_ODB_API_DIR`              | Optional path to the ABAQUS ODB API from Simulia |

