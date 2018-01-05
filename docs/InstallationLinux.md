---
layout: docs
title: Linux Installation
permalink: /docs/installationlinux/
published: true
---

<small>Note: None of the binary distributions includes support for ABAQUS odb files.</small>


### ResInsight Installation

1. Download TAR.GZ binary distribution from [https://github.com/OPM/ResInsight/releases](https://github.com/OPM/ResInsight/releases "release section on GitHub")
2. Extract content from TAR file
3. Start ./ResInsight

### Installation from Binary Packages on Linux
 Packages for ResInsight are available as part of the distribution by the [Opm project](http://opm-project.org/?page_id=36)

### Octave Installation (optional)
The precompiled octave support is only tested for RedHat 6 (ResInsight 1.3.2-dev and earlier, was also tested on RedHat 5) and is not expected to work for other configurations, unless you build ResInsight yourself. See [Build Instructions]({{ site.baseurl }}/docs/buildinstructions)

1. Install Octave directly from the package manager in Linux. See the documentation for your particular distribution. 
2. Launch ResInsight, open **Edit->Preferences** 
3. Enter the path to the Octave command line interpreter executable.  
  ( usually just _`octave`_. )

### Display Menu Icons in GNOME
By default, icons are not visible in menus in the GNOME desktop environment. ResInsight has icons for many menu items, and icons can be set visible by issuing the following commands (Tested on RHEL6) :

```
gconftool-2 --type boolean --set /desktop/gnome/interface/buttons_have_icons true
gconftool-2 --type boolean --set /desktop/gnome/interface/menus_have_icons true
```

This fix was taken from reply number 11 in this [thread](https://bbs.archlinux.org/viewtopic.php?id=117414)


