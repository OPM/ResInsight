---
layout: docs
title: Preferences
permalink: /docs/preferences/
published: true
---

In this section the different settings that controls the default behavior of ResInsight is described. These settings can be controlled using the **Preference** dialog available from the **Edit -> Preferences** menu.

![]({{ site.baseurl }}/images/Preferences.png)

The preferences are not stored in the project files, but rather in a platform specific way for each user. 

## General - tab

### Default settings

This group of options controls visual settings that will be used when creating new views.

- **Viewer Background** 
- **Gridlines** -- Controls whether to show the gridlines by default.
- **Mesh Color** 
- **Mesh Color Along Faults**
- **Well Label Color**
- **Font Size** -- This font size is used for all labels shown in the 3D Views.
- **Default Z Scale Factor** -- Default depth scale for grid models.

### 3D Views
- **Navigation mode** -- Defines how to use the mouse to interact with with the 3D model.
- **Use shaders** -- This option controls the use of OpenGL shaders. Should be left **On**. Available only for testing purposes.
- **Show 3D Information** -- Displays graphical resource usage as text in the 3D view.

### Other

- **SSIHUB Address** -- Optional URL to Statoil internal web service used to import well paths.
- **Show LAS Curve Without TVD Warning** - Turn off the warning displayed when showing LAS curves in TVD mode.

## Eclipse - tab

![]({{ site.baseurl }}/images/EclipsePreferences.png)

### Behavior When Loading Data

- **Compute DEPTH Related Properties** -- If not present, compute DEPTH, DX, DY, DZ, TOP, BOTTOM when loading new cases.
- **Load and Show SOIL** -- Control if SOIL is loaded and applied to grid.
- **Import Faults/NNCs/Advanced MSW Data** -- Disable import of data for a case to reduce case import time.
- **Fault Include File Absolute Path Prefix** -- Prefix used on Windows if fault files use absolute UNIX paths.
- **Use Result Index File** -- If enabled ResInsight will try to save a result index file when opening a new case. The file is stored in the same directory as the _`*.EGRID`_ file with filename _`<casename>.RESINSIGHT_IDX`_ If it exists, ResInsight will use this when loading the case, resulting in a significant speedup.
- **Skip Import of Simulation Well Data** -- Disable import of simulation well data for a case to reduce case import time (opposite toggling than the other import commands).

## Octave - tab

![]({{ site.baseurl }}/images/OctavePreferences.png)

### Octave

- **Octave executable location** -- Defines the binary file location for Octave. Usually without path on Linux, and including path on Windows.
- **Show text header when executing scripts** -- Enables the default output that octave outputs when started.

### Script Files

- **Shared Script Folder(s)** -- Defines the search paths for octave scripts
- **Script Editor** -- The text editor to invoke when editing scripts

