## Build instructions

ResInsight uses the CMake build system and requires CMake version 2.8 or higher. Moreover, you need version 4.7.3 of Qt or newer, look below for dependency list.

### CMAKE configuration

If you check the button 'Grouped' in the GUI, the CMake variables are grouped by prefix. This makes it easier to see all settings for ResInsight.


| CMake Name   | Description |
|--------------|---------|
| `RESINSIGHT_USE_OPENMP`               | Enable OpenMP multi-core parallel building |
| `RESINSIGHT_PRIVATE_INSTALL`          | Install as an independent bundle including the necessary Qt libraries |
| `RESINSIGHT_BUILD_DOCUMENTATION`      | Use Doxygen to create the HTML based API documentation |
| `RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE`  | Location of Octave tool mkoctfile used to compile Octave plugins |
| `RESINSIGHT_OCTAVE_PLUGIN_QMAKE`      | Location of qmake to find Qt include files and libraries used to compile Octave plugins |
| `RESINSIGHT_OCTAVE_PLUGIN_32_BIT`     | Windows 64-bit: Flag used to control if Octave plugins will be compiled using 32-bit build environment |

#### Build without Octave plugins
It is possible to compile ResInsight without compiling the Octave plugins. This can be done by specifying blank for the Octave CMake variables. The Octave plugin module will not be build, and CMake will show warnings like 'Failed to find mkoctfile'. This will not break the build or compilation of ResInsight.

#### Build instructions Visual Studio
- Open the CMake GUI
- Set the path to the source code
- Set the path to the build directory
- Click "Configure" and select your preferred compiler, "Visual Studio 10" or "Visual Studio 10 Win64"
- Set the build variables and click "Configure" again
- Click "Generate", and one solution file and several project files will be created in the build directory
- Open the solution file in Visual Studio

#### Optional - build instructions Octave plugins 
To be able to compile the Octave plugins, the path to the Octave development tool `mkoctfile` must be provided. In addition, if you compile x64, you must specify the location of 32-bit version of Qt, as the Octave plugins on Windows are currently only supported for x86.
See description for `RESINSIGHT_OCTAVE_PLUGIN_QMAKE` and `RESINSIGHT_OCTAVE_PLUGIN_32_BIT` above.

##### Octave installation
Currently tested and verified version on Windows is Octave 3.6.1. NB! Version 3.6.2 has compile issues using VS2010, this version will not be able to compile the Octave plugins.
 
- Download and install Octave 3.6.1 for VS2010 from [SourceForge](http://sourceforge.net/projects/octave/files/Octave%20Windows%20binaries/Octave%203.6.1%20for%20Windows%20Microsoft%20Visual%20Studio/octave-3.6.1-vs2010-setup-1.exe/download)
- Download a [missing library file](http://dl.dropbox.com/u/45539519/dirent-vs2010.lib), rename to **dirent.lib** and copy it into Octave lib folder, typically **c:/Octave-3.6.1/lib/dirent.lib** See details on [SourceForge](http://sourceforge.net/mailarchive/message.php?msg_id=28933804)


### Linux

An out-of-tree build is typically done with

    mkdir ResInsight/build
    cd ResInsight/build
    cmake ..
    make
    make install

You will find the ResInsight binary under the Install directory in your build directory.

#### Dependencies for Debian based distributions

    sudo apt-get install git cmake build-essential octave octave-headers qt4-dev-tools

If you are running Ubuntu 12.10 or newer, you will need to replace octave-headers with liboctave-dev :

    sudo apt-get install git cmake build-essential octave liboctave-dev qt4-dev-tools

