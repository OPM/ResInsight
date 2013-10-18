#ResInsight#

ResInsight is a 3D viewer and post processing tool for reservoir models. It has been co-developed by Statoil and Ceetron AS / Ceetron Solutions AS with the aim to provide a versatile tool for professionals who need to visualize and process reservoir models. The software is copyrighted by Ceetron and Statoil and licensed under GPL 3+. See COPYING for details.

## DEPENDENCIES ##
ResInsight uses the Ensambles/ert library to access eclipse result files, and the two projects collaborates closely. The source code of the approved ert library version is embedded in the ResInsight source code tree, making downloading and building simple.
ResInsight also features an interface to Octave, making it possible to retrieve data from ResInsight, process them using Octave, and write them back into ResInsight for viewing. If you want to build ResInsight with this feature, you need to install Octave.

Octave [http://www.gnu.org/software/octave/](http://www.gnu.org/software/octave/)
Ensembles/ert [https://github.com/Ensembles/ert](https://github.com/Ensembles/ert)

## SUPPORTED PLATFORMS ##
ResInsight is designed cross-platform from the start. Efforts have been made to ensure that code will compile and run on linux and windows platforms, but the tested platforms are currently 64 bit RHE 5, RHE 6 and Windows 7.

There has been attemts to make ResInsight build and run on OSX as well, but the tweaks needed (submitted by Roland Kaufmann) is not yet incorporated. 

## DOCUMENTATION ##
No efforts to provide documentation has been made yet.

## SOURCE CODE ##
'''
git clone git://github.com/OPM/ResInsight.git
'''

## CONTRIBUTION ##
Contributions are very welcome, although it might take some time for the team to accept pull requests that is not in the main line of the projects focus. Please use the dev branch for contributions and pull requests, as it is the branch dedicated to the day to day development. The master branch is used as a branch for distributing the latest stable release.
Release branches that might pop up are dedicated bug fix branches for the release in question.

## BUILDING RESINSIGHT ##
### Linux ###
ResInsight uses the cmake build system and requires cmake version 2.8 or higher. Moreover, you need version 4.7.3 of Qt or newer, look below for dependecy list. An out-of-tree build is typically done with
'''
   mkdir ResInsight/build
   cd ResInsight/build
   cmake ..
   make
   make install
'''
You will find the ResInsight binary under the Install directory in your build directory.

### Windows ###
Open the CMake GUI.
Set the path to the source code: <ResInsight-sourcecode-folder>
Set the path to the build directory: <ResInsight-build-folder>
Click "Configure" and select your preferred compiler, "Visual Studio 10" or "Visual Studio 10 Win64"
Set the build variables and click "Configure" again.
Click "Generate", and a project file will be created in the build directory <ResInsight-build-folder>

## DEPENDENCIES FOR DEBIAN BASED DISTRIBUTIONS ##
'''sudo apt-get install git cmake build-essential octave octave-headers qt4-dev-tools'''

If you are running Ubuntu 12.10 or newer, you will need to replace octave-headers with liboctave-dev in the line above.
