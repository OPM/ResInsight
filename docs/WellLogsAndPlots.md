---
layout: docs
title: Well Log Plots
permalink: /docs/welllogsandplots/
published: true
---

![]({{ site.baseurl }}/images/WellLogPlotOverview.png)

ResInsight can display well logs by extracting data from a simulation model along a well trajectory and from imported LAS-files. Extracted simulation data can be exported to LAS-files for further processing. 

## Well Log Plots 

Well log plots can be created in several ways: 

1. Right click the empty area below all the items in the **Project Tree** and select **New Well Log Plot**. A plot is created with one **Track** and an empty **Curve**.
2. Right click a wellpath, either in the **Project Tree** or in the 3D-view, and select either **New Well Log Extraction Curve**. A new plot with a single **Track** and a **Curve** is created. The curve is setup to match your selection (Well trajectory, active case and result) 
3. Right click a LAS-file channel in the **Project Tree** and select **Add to New Plot**. A new plot with a single **Track** and a **Curve** is created. The curve is setup to plot the selected LAS-file channel.

Each **Well Log Plot** can contain several *Tracks*, and each **Track** can contain several **Curves**.

![]({{ site.baseurl }}/images/WellLogPlotWindow.png)

Tracks and Curves can be organized using drag and drop functionality in the **Project Tree**. Tracks can be moved from one plot to another, and you can alter the order in which they appear in the **Plot**. **Curves** can be moved from one **Track** to another.

All the **Tracks** in the same plot always display the same depth range, and share the *True Veritcal Depth (TVD)* or *Measured Depth (MD)* setting. In the property panel of the plot, the exact depth range can be adjusted along with the depth type setting (TVD/MD).

### Depth unit

The depth unit can be set using the **Depth unit** option. Currently ResInsight supports *Meter* and *Feet*. The first curve added to a plot will set the plot unit based on the curve unit. Additional curves added to a plot will be converted to the plot unit if needed.

### Depth zoom and pan

The visible depth range can be panned using the mouse wheel while the mouse pointer hovers over the plot.
Pressing and holding **CTRL** while using the mouse wheel will allow you to zoom in or out depth-wise, towards the mouse position.

## Tracks

Tracks can be created by right clicking a **Well Log Plot** and select **New Track**

![]({{ site.baseurl }}/images/TrackProperties.png)

A track controls the x-axis range of the display, and can be edited from the property panel of the **Track**. 
Logarithmic display is controlled using the **Logarithmic Scale** option.

## Curves

Curves can be created by right clicking a **Track** in the **Project Tree**, or by the commands mentioned above.
There are two types of curves: *Well Log Extraction Curves* and *Well Log LAS Curves*. 

Curve visual appearance is controlled in the **Appearance** section:

- **Color** - Controls the color of the curve
- **Thickness** - Number of pixels used to draw the curve
- **Point style** - Defines the style used to draw the result points of the curve, select *None* to disable drawing of points
- **Line style** - Defines the the style used to draw the curve, select  *None* to disable line drawing

### Well Log Extraction Curves

Ectraction curves acts as an artifical well log curve. Instead of probing the real well, a simulation model is probed instead.


The property panel for a geomechanical model is shown below:

![]({{ site.baseurl }}/images/WellLogExtractionCurveProperties.png)

The first group of options controls all the input needed to setup the data extraction for the curve - Well Path, Case, and result value. The selection of result value is somewhat different between geomechanical cases and Eclipse cases. In addition you can select what timestep to address if the selected property is a dynamic one. 

<div class="note">
Placing keyboard focus in the <b>Time Step</b> drop-downbox will allow you to use the arrow keys to quickly step through the timesteps while watching the changes in the curve. 
</div>

The disply name of a curve is normally generated automatically. The options grouped below **Auto Name** can be used to tailor the length and content of the curve name.

#### Curve extraction calculation

This section describes how the values are extracted from the grid when creating a Well log Extraction Curve.

Ectraction curves are calculated by finding the intersections between a well trajectory and the cell-faces in a particular grid model. Usually there are two intersections at nearly the same spot; the one leaving the previous cell, and the one entering the next one. At each intersection point the measured depth along the trajectory is interpolated from the trajectory data. The result value is retreived from the corresponding cell in different ways depending on the nature of the underlying result. 

For Eclipse results the cell face value is used directly. This is normally the same as the corresponding cell value, but if a **Directional combined results** is used, (See [ Derived Results ]({{ site.baseurl }}/docs/derivedresults) ) it will be that particular face's value.

Abaqus results are interpolated across the intersected cell-face from the result values associated with the nodes of that face. This is also the case for integration point results, as they are directly associated with their corresponding element node in ResInsight. 

### Well Log LAS Curves

LAS-curves shows the values in a particular channel in a LAS-file.

The property panel of a LAS-curve is shown below:

![]({{ site.baseurl }}/images/WellLogLasCurveProperties.png)

<div class="note">
You can also create a LAS-curve by a simple drag-drop operation in the <b>Project Tree</b>: Drag one of the LAS channels and drop it onto a <b>Track</b>. A new curve will be created with the correct setting.
</div>

## LAS-file Support

ResInsight has some support for reading and writing LAS files. In the following two sections this support is described.

### Importing LAS-files
LAS-files can be imported using the command: **File->Import->Import Well Logs from File**.

ResInsight will search for the the well name in the imported LAS-files among your existing **Well Trajectories**.
If a match is found, the LAS-file is placed as a child of that trajectory. If not, a new empty trajectory entry is created with the imported LAS-file under it.

![]({{ site.baseurl }}/images/LasFilesInTree.png)

If the LAS-file does not contain a well name, the file name is used instead. 

### Exporting LAS-files

A set of curves can be exported to LAS files by right clicking the curves, well log track, or well log plots and select **Export To LAS Files ...**. An export dialog is diplayed, allowing the user to configure how to export curve data.

![]({{ site.baseurl }}/images/export_to_las_files.png)

- **Export Folder** - Location of the exported LAS files, one file per unique triplet of well path, case and time step
- **Resample Curve Data** - If enabled, all curves are resampled at the given resample interval before export
- **TVDRKB** - If enabled, TVDRKB for all curves based on the listed well paths are exported. If the difference field is blank, no TVDRKB values are exported.

