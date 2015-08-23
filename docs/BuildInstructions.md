---
layout: docs
prev_section: preferences
next_section: octaveinterfacereference
title: Build Instructions
permalink: /docs/buildinstructions/
published: true
---


## CMake configuration

ResInsight uses the CMake build system and requires CMake version 2.8 or higher. In addition, you need Qt version 4 to build ResInsight (4.7.3 or above).

The ResInsight build may be configured in different ways, with optional support for Octave plugins, ABAQUS ODB API, and OpenMP. This is configured through options in CMake.

If you check the button 'Grouped' in the CMake GUI, the CMake variables are grouped by prefix. This makes it easier to see all of the options for ResInsight.

## Build instructions
- Open the CMake GUI
- Set the path to the source code
- Set the path to the build directory
- Click "Configure" and select your preferred compiler
- Set the build options and click "Configure" again (see ResInsight specific options below)
- Click "Generate" to generate the makefiles or solution file and project files in the build directory
- Run the compiler using the generated makefiles or solution file/project files to build ResInsight

### Windows
ResInsight has been verified to build and run on Windows 7/8 using Microsoft Visual Studio 2010. Typical usage on Windows is to follow the build instructions above, and then open the generated solution file in Visual Studio to build the application.

### Linux

ResInsight has been verified to build and run on RedHat Linux 6. Typical usage is to follow the build instructions above to build the makefiles. Then go to the build directory, and run:

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
To be able to compile the Octave plugins, the path to the Octave development tool `mkoctfile` must be provided.

It is possible to build ResInsight without compiling the Octave plugins. This can be done by specifying blank for the Octave CMake options. The Octave plugin module will not be built, and CMake will show warnings like 'Failed to find mkoctfile'. This will not break the build or compilation of ResInsight.

ResInsight has been verified to build and run with Octave versions 3.4.3, 3.8.1, and 4.0.0.

#### Octave related CMake options for ResInsight

| CMake Name   | Description |
|--------------|---------|
| `RESINSIGHT_OCTAVE_PLUGIN_32_BIT`     | Set 32-bit MSVC compiler environment while running mkoctfile |
| `RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE`  | Location of Octave tool mkoctfile used to compile Octave plugins |
| `RESINSIGHT_OCTAVE_PLUGIN_QMAKE`      | Location of Qt version to use when compiling Octave plugins. Must be compatible with Octave runtime. (Use the Qt version embedded in Octave. The qmake executable itself is not used, only the path to the directory.) |

### Optional - ABAQUS ODB API 
ResInsight can be built with support for ABAQUS ODB files. This requires an installation of the ABAQUS ODB API from Simulia on the build computer. The path to the ABAQUS ODB API folder containing header files and library files must be specified. Leaving this option blank gives a build without ODB support. ResInsight has been built and tested with ABAQUS ODB API version 6.14-3 on Windows 7/8 and RedHat Linux 6.

#### ABAQUS ODB API related CMake options for ResInsight

| CMake Name   | Description |
|--------------|---------|
| `RESINSIGHT_ODB_API_DIR`              | Optional path to the ABAQUS ODB API from Simulia |

### Dependencies for Debian based distributions

- sudo apt-get install git cmake build-essential octave octave-headers qt4-dev-tools

If you are running Ubuntu 12.10 or newer, you will need to replace octave-headers with liboctave-dev :

- sudo apt-get install git cmake build-essential octave liboctave-dev qt4-dev-tools
