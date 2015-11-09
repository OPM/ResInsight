---
layout: docs
title: Windows Installation
permalink: /docs/installation/
published: true
---


<small>Note: None of the binary distributions includes support for ABAQUS odb files.</small>

### ResInsight installation

1. Download ZIP binary distribution from [https://github.com/OPM/ResInsight/releases](https://github.com/OPM/ResInsight/releases "release section on GitHub")
2. Extract content from ZIP file
3. Start ResInsight.exe 

#### Octave installation (optional)
To make ResInsight work with Octave you need to do the following two steps:

1. Install the correct version of Octave
2. Setup ResInsight to find the Octave executable.

##### ResInsight 1.4.0 and later
1. ResInsight 1.4.0 is delivered with support for Octave 4.0.0 which can be downloaded here: [Octave-4.0.0](ftp://ftp.gnu.org/gnu/octave/windows/octave-4.0.0_0-installer.exe)
2. Launch ResInsight.exe, open **Edit->Preferences** and enter the path to the Octave command line interpreter executable, usually 'C:\Your\Path\To\Octave-x.x.x\bin\octave-cli.exe'

##### ResInsight 1.3.2-dev and earlier 
1. Earlier versions of ResInsight for Windows had precompiled support for Octave 3.6.1 for VS2010 and can be downloaded here: [Octave-3.6.1 for VS 2010](https://github.com/OPM/ResInsight/releases/download/1.0.0/octave-3.6.1-vs2010-setup-1.2.exe)
- Download an [additional library file](https://github.com/OPM/ResInsight/releases/download/1.0.0/dirent.lib) and copy it into Octave lib folder, typically **c:/Octave-3.6.1/lib/dirent.lib**
2. Launch ResInsight.exe, open **Edit->Preferences** and enter the path to the Octave command line interpreter executable, usually 'C:\Your\Path\To\Octave-x.x.x\bin\octave-cli.exe'
