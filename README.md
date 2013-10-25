# ResInsight

ResInsight is an open source, cross-platform 3D visualization and post processing tool for reservoir models and simulations. The system also constitutes a framework for further development and support for new data sources and visualization methods, e.g. additional solvers, seismic data, CSEM, geomechanics, and more. 

The user interface is tailored for efficient interpretation of reservoir simulation data with specialized visualizations of properties, faults and wells. It enables easy handling of a large number of realizations and calculation of statistics. To be highly responsive, ResInsight exploits multi-core CPUs and GPUs. Integration with GNU Octave enables powerful and flexible result manipulation and computations. Derived results can be returned to ResInsight for further handling and visualization. Eventually, derived and computed properties can be directly exported to Eclipse input formats for further simulation cycles and parameter studies.

The main input data is *.GRID and *.EGRID files along with their *.INIT and restart files *.XNNN and *.UNRST. ResInsight also supports selected parts of Eclipse input files and can read grid information and corresponding cell property data sets.

ResInsight has been co-developed by Statoil ASA, Ceetron Solutions AS, and Ceetron AS with the aim to provide a versatile tool for professionals who need to visualize and process reservoir models. The software is copyrighted by Ceetron and Statoil and licensed under GPL 3+. See COPYING for details.

### Dependencies
ResInsight uses the Ensambles/ert library to access Eclipse result files, and the two projects collaborates closely. The source code of the approved ert library version is embedded in the ResInsight source code tree, making downloading and building simple.
ResInsight also features an interface to Octave, making it possible to retrieve data from ResInsight, process them using Octave, and write them back into ResInsight for viewing. If you want to build ResInsight with this feature, you need to install Octave.

Octave : [http://www.gnu.org/software/octave/](http://www.gnu.org/software/octave/)

Ensembles/ert : [https://github.com/Ensembles/ert](https://github.com/Ensembles/ert)

### Supported Platforms
ResInsight is designed cross-platform from the start. Efforts have been made to ensure that code will compile and run on linux and windows platforms, but the tested platforms are currently 64 bit RHE 5, RHE 6 and Windows 7.

There has been attemts to make ResInsight build and run on OSX as well, but the tweaks needed (submitted by Roland Kaufmann) is not yet incorporated. 

### Documentation

An online [ Users Guide ](Documentation/UsersGuide/UsersGuide.md) with some reference content is 
[ here ](Documentation/UsersGuide/UsersGuide.md)

### Source Code

    git clone git://github.com/OPM/ResInsight.git

### Contribution
Contributions are very welcome, although it might take some time for the team to accept pull requests that is not in the main line of the projects focus. 

Please use the dev branch for contributions and pull requests, as it is the branch dedicated to the day to day development. 

The master branch is supposed to be stable, and is updated when we want to publish a new stable release.

Release branches that might pop up are dedicated bug fix branches for the release in question.

### Building Resinsight
#### Linux ###
ResInsight uses the CMake build system and requires CMake version 2.8 or higher. Moreover, you need version 4.7.3 of Qt or newer, look below for dependecy list. An out-of-tree build is typically done with

    mkdir ResInsight/build
    cd ResInsight/build
    cmake ..
    make
    make install

You will find the ResInsight binary under the Install directory in your build directory.

#### Windows ###
Open the CMake GUI.
Set the path to the source code: <ResInsight-sourcecode-folder>
Set the path to the build directory: <ResInsight-build-folder>
Click "Configure" and select your preferred compiler, "Visual Studio 10" or "Visual Studio 10 Win64"
Set the build variables and click "Configure" again.
Click "Generate", and a project file will be created in the build directory <ResInsight-build-folder>

### Dependencies for Debian based distributions

    sudo apt-get install git cmake build-essential octave octave-headers qt4-dev-tools

If you are running Ubuntu 12.10 or newer, you will need to replace octave-headers with liboctave-dev :

    sudo apt-get install git cmake build-essential octave liboctave-dev qt4-dev-tools

