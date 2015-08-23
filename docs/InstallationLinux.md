---
layout: docs
prev_section: installation
next_section: reservoirviews
title: Installation and Configuration (Linux)
permalink: /docs/installationlinux/
published: true
---

<small>Note: None of the binary distributions includes support for ABAQUS odb files.</small>


### ResInsight installation

1. Download TAR.GZ binary distribution from [https://github.com/OPM/ResInsight/releases](https://github.com/OPM/ResInsight/releases "release section on GitHub")
2. Extract content from TAR file
3. Start ./ResInsight

#### Octave installation (optional)
The precompiled octave support is only tested for RedHat 6 (ResInsight 1.3.2-dev and earlier, was also tested on RedHat 5) and is not expected to work for other configurations, unless you build ResInsight yourself. See [Build Instructions]({{ site.baseurl }}/docs/buildinstructions)

1. Install Octave directly from the package manager in Linux. See the documentation for your particular distribution. 
2. Launch ResInsight, open **Edit->Preferences** and enter the path to the Octave command line interpreter executable, usually just 'octave'.
