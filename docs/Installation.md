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

### Octave installation (optional)
1. Download [Octave-4.0.0](ftp://ftp.gnu.org/gnu/octave/windows/octave-4.0.0_0-installer.exe) and install it. 
2. Launch ResInsight.exe, open **Edit->Preferences**. 
3. On the **Octave** tab, enter the path to the Octave command line interpreter executable.  
   ( Usually _`C:\Your\Path\To\Octave-x.x.x\bin\octave-cli.exe`_ )

<div class="note info">
A binary package of ResInsight will normally <b>not</b> work with other Octave versions than the one it is compiled with. 
</div>

<div class="note info">
You <b>have</b> to point to the <b>cli</b> binary in the windows octave installation. The <code>octave.exe</code> will not work as it is launching the octave GUI.
</div>
