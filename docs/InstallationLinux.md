---
layout: docs
title: Linux Installation
permalink: /docs/installationlinux/
published: true
---

<small>Note: None of the binary distributions includes support for ABAQUS odb files.</small>


## Install ResInsight

### From downloaded tarball
1. Download TAR.GZ binary distribution from [https://github.com/OPM/ResInsight/releases](https://github.com/OPM/ResInsight/releases "release section on GitHub")
2. Extract content from TAR file
3. Start ./ResInsight

#### Display Menu Icons in GNOME (Optional)
By default, icons are not visible in menus in the GNOME desktop environment. ResInsight has icons for many menu items, and icons can be set visible by issuing the following commands (Tested on RHEL6) :

```
gconftool-2 --type boolean --set /desktop/gnome/interface/buttons_have_icons true
gconftool-2 --type boolean --set /desktop/gnome/interface/menus_have_icons true
```

This fix was taken from reply number 11 in this [thread](https://bbs.archlinux.org/viewtopic.php?id=117414)

### From Binary Packages on Linux 
 Packages for ResInsight are available as part of the distribution by the [Opm project](http://opm-project.org/?page_id=36)

#### Red Hat Enterprise Linux 6 or 7
Login as root and do :

    yum-config-manager --add-repo https://opm-project.org/package/opm.repo
    yum install resinsight
    yum install resinsight-octave

Then you are good, and can start ResInsight by typing: ResInsight

#### Ubuntu Linux 16.04-64bit
On the command line do: 

    sudo apt-get update
    sudo apt-get install software-properties-common
    sudo apt-add-repository ppa:opm/ppa
    sudo apt-get update
    sudo apt-get install resinsight
    sudo apt-get install octave-resinsight

Start ResInsight by typing : `ResInsight`

## Setup Octave Interface (optional)

1. Install Octave directly from the package manager in Linux. See the documentation for your particular distribution. 
2. Launch ResInsight, open **Edit->Preferences** 
3. Enter the path to the Octave command line interpreter executable `octave-cli` (for older version of octave use `octave`)

<div class="note info">
The precompiled octave interface distributed in the tarball is only tested for RedHat 6. <br>
It is <b>not</b> expected to work for other configurations.
(ResInsight 1.3.2-dev and earlier, was also tested on RedHat 5)<br>
<br>
If you need the octave interface to work on a different OS, you need to build ResInsight yourself.<br> 
See <a href="{{ site.baseurl }}/docs/buildinstructions">Build Instructions</a> 
</div>


## Workaround for crash using Virtual Box
Uncheck **Settings->Display->Enable 3D Acceleration**. Disabling this option will cause OpenGL operations to be executed in software, so the the performance of graphics operations in ResInsight will be slower, but will not crash.

Here is a pointer addressing the issue with Virtual Box, this is not testes by us:

[https://superuser.com/questions/541537/how-to-solve-issues-with-shader-model-in-virtualbox](https://superuser.com/questions/541537/how-to-solve-issues-with-shader-model-in-virtualbox)
