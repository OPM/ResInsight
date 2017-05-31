---
layout: docs
title: Preferences
permalink: /docs/preferences/
published: true
---

In this section the different settings that controls the default behaviour of ResInsight is described. These settings can be controlled using the **Preference** dialog available from the **Edit -> Preferences** menu.

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
- **Font Size** -- This font size is used for all labels shown in the 3D Views
- **Default Z Scale Factor** -- Default depth scale for grid models

### 3D views
- **Navigation mode** -- Defines how to use the mouse to interact with with the 3D model.
- **Use shaders** -- This option controls the use of OpenGL shaders. Should be left **On**. Available only for testing purposes.
- **Show 3D Information** -- Displays graphical resource usage as text in the 3D view.

### Other

- **SSIHUB Address** -- Optional Url to Statoil internal web service used to import well paths.
- **Show LAS Curve Without TVD Warning** - Turn off the warning displayed when showing LAS curves in TVD mode

## Eclipse - tab

![]({{ site.baseurl }}/images/EclipsePreferences.png)

### Behaviour When Loading Data

- **Compute DEPTH Related Properties** -- If not present, compute DEPTH, DX, DY, DZ, TOP, BOTTOM when loading new cases
- **Load and Show SOIL** -- Control if SOIL is loaded and applied to grid
- **Import Faults/NNCs/Advanced MSW Data** -- Disable import of data for a case to reduce case import time.
- **Fault Include File Absolute Path Prefix** -- Prefix used on Windows if fault files use absolute UNIX paths

## Octave - tab

![]({{ site.baseurl }}/images/OctavePreferences.png)

### Octave

- **Octave executable location** -- Defines the binary file location for Octave. Usually without path on Linux, and including path on Windows.
- **Show text header when executing scripts** -- Enables the default output that octave outputs when started.

### Script files

- **Shared Script Folder(s)** -- Defines the search paths for octave scripts
- **Script Editor** -- The text editor to invoke when editing scripts

## Summary - tab

![]({{ site.baseurl }}/images/SummaryPreferences.png)

- **Create Summary Plots When Importing Eclipse Case** -- Automatically import the summary case and display a plot if a _`*.SMSPEC`_ file exists when importing an Eclipse binary case
- **Default Vector Selection Filter** -- Wildcard text defining the summary vector filter to be applied by default. Default string is _`F*OPT`_

