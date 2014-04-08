---
layout: docs
prev_section: gettingstarted
next_section: reservoirviews
title: Installation and configuration 
permalink: /docs/installation/
---

## Installation and configuration 

### Windows

1. Download ZIP binary distribution from [https://github.com/OPM/ResInsight/releases](https://github.com/OPM/ResInsight/releases "release section on GitHub")
2. Extract content from ZIP file
3. (OPTIONAL) Launch ResInsight.exe, open **Edit ->Preferences** and define location of Octave, usually 'ResInsightRoot/octave/bin/octave.exe'

#### Optional - Octave installation
Currently tested and verified version on Windows is Octave 3.6.1. NB! Version 3.6.2 has compile issues using VS2010, this version will not be able to compile the Octave plugins.
 
- Download and install Octave 3.6.1 for VS2010 from [SourceForge](http://sourceforge.net/projects/octave/files/Octave%20Windows%20binaries/Octave%203.6.1%20for%20Windows%20Microsoft%20Visual%20Studio/octave-3.6.1-vs2010-setup-1.exe/download)
- Download a [missing library file](https://github.com/OPM/ResInsight/releases/download/1.0.0/dirent.lib) and copy it into Octave lib folder, typically **c:/Octave-3.6.1/lib/dirent.lib** See details on [SourceForge](http://sourceforge.net/mailarchive/message.php?msg_id=28933804)


## Linux

1. Download TAR.GZ binary distribution from [https://github.com/OPM/ResInsight/releases](https://github.com/OPM/ResInsight/releases "release section on GitHub")
2. Extract content from TAR file
3. (OPTIONAL) Launch ResInsight, open **Edit -> Preferences** and define location of Octave in the field **Octave**, usually 'ResInsightRoot/octave/bin/octave-cli'
 


