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

1. Right-click a well path or a simulation well, either in the **Project Tree** or in the 3D-view.  
   Select **New Well Log Extraction Curve**.   
   A new plot with a single **Track** and a **Curve** is created. The curve is setup to match the 
   selected Well trajectory, active case, and result. 
2. Right-click the empty area below all the items in the **Project Tree**.  
   Select **New Well Log Plot**. 
   A plot is created with one **Track** and an empty **Curve**.
3. Right-click a LAS-file channel in the **Project Tree**. Select **Add to New Plot**. A new plot with a single **Track** and a **Curve**  displaying the selected LAS-file channel is created.

Each **Well Log Plot** can contain several *Tracks*, and each **Track** can contain several **Curves**.

![]({{ site.baseurl }}/images/WellLogPlotWindow.png)

Tracks and Curves can be organized using drag and drop functionality in the **Project Tree**. Tracks can be moved from one plot to another, and you can alter the order in which they appear in the **Plot**. **Curves** can be moved from one **Track** to another.

### Measured Depth (MD), True Vertical Depth (TVD) and Pseudo Length (PL)

All the **Tracks** in the same plot always display the same depth range, and share the *True Vertical Depth (TVD)* or *Measured Depth (MD)* setting. In the property panel of the plot, the exact depth range can be adjusted along with the depth type setting (TVD/MD). 

**Simulation Wells** however, is using a *Pseudo Length* instead of the real *Measured Depth* when the depth type is MD, as the MD information is not available in the restart files. The *Pseudo Length* is a length along the coarsely interpolated visualization pipe, and serves only as a very coarse estimation of an MD-like depth. Pseudo length is measured from the simulation-wells first connection-cell (well head connection) to the reservoir. This is very different from MD, which would start at RKB or at sea level.

### Depth Unit

The depth unit can be set using the **Depth unit** option. Currently ResInsight supports *Meter* and *Feet*. The first curve added to a plot will set the plot unit based on the curve unit. Additional curves added to a plot will be converted to the plot unit if needed.

### Depth Zoom and Pan

The visible depth range can be panned using the mouse wheel while the mouse pointer hovers over the plot.
Pressing and holding **CTRL** while using the mouse wheel will allow you to zoom in or out depth-wise, towards the mouse position.

### Accessing the Plot Data

The command context command **Show Plot Data** will show a window containing the plot data in ascii format. The content of this window is easy to copy and paste into Excel or other tools for further processing.

It is also possible to save the ascii data to a file directly by using the context command **Export Plot Data to Text File** on the plot. 

## Tracks

Tracks can be created by right-clicking a **Well Log Plot** and select **New Track**.

![]({{ site.baseurl }}/images/TrackProperties.png)

A track controls the x-axis range of the display, and can be edited from the property panel of the **Track**. In addition to the range, logarithmic display is controlled using the **Logarithmic Scale**, grid lines can be tweaked with the **Show Grid Lines** option and tick intervals can be controlled manually wih the **Manually Set Tick Intervals**  option.

It is also possible to visualize **Zonation** or **Formation Names** in the track (see [Formations]({{site.baseurl}}/docs/formations) for details) as well as [Well Attributes]({{site.baseurl}}/docs/wellpaths#well-path-attributes) such as casing and liners. The following image shows some of the possibilities, with labels for the formations shown only on the second track, while the first track shows the a full cross section of the well attributes and the second track shows only a radial view.
![]({{ site.baseurl }}/images/WellAttributeBothSides.png)

Finally, the width of each track can be manipulated using the **Track Width** setting. This allows the creation of plots with varying width tracks.
![]({{ site.baseurl }}/images/WellLogTracksWidth.png)

## Curves

Curves can be created by right-clicking a **Track** in the **Project Tree**, or by the commands mentioned above.
There are two types of curves: *Well Log Extraction Curves* and *Well Log LAS Curves*. 

Curve visual appearance is controlled in the **Appearance** section:

- **Color** -- Controls the color of the curve.
- **Thickness** -- Number of pixels used to draw the curve.
- **Point style** -- Defines the style used to draw the result points of the curve, select *None* to disable drawing of points.
- **Line style** -- Defines the the style used to draw the curve, select  *None* to disable line drawing.

### Well Log Extraction Curves

Extraction curves acts as an artificial well log curves. Instead of probing the real well, a simulation model is probed instead.

The property panel for an eclipse model is shown below:

![]({{ site.baseurl }}/images/WellLogExtractionCurveProperties.png)

The first group of options controls all the input needed to setup the data extraction for the curve - Well Path, Case, and result value. The selection of result value is somewhat different between geomechanical cases and Eclipse cases. In addition you can select what time step to address if the selected property is a dynamic one. 

<div class="note">
Placing keyboard focus in the <b>Time Step</b> drop-downbox will allow you to use the arrow keys or the mouse wheel to quickly step through the timesteps while watching the changes in the curve. 
</div>

The display name of a curve is normally generated automatically. The options grouped below **Auto Name** can be used to tailor the length and content of the curve name.

#### Curve Extraction Calculation

This section describes how the values are extracted from the grid when creating a Well Log Extraction Curve.

Extraction curves are calculated by finding the intersections between a well trajectory and the cell-faces in a particular grid model. Usually there are two intersections at nearly the same spot; the one leaving the previous cell, and the one entering the next one. At each intersection point the measured depth along the trajectory is interpolated from the trajectory data. The result value is retrieved from the corresponding cell in different ways depending on the nature of the underlying result. 

For Eclipse results the cell face value is used directly. This is normally the same as the corresponding cell value, but if a **Directional combined results** is used ( See [ Derived Results ]({{ site.baseurl }}/docs/derivedresults) ), it will be that particular face's value.

Abaqus results are interpolated across the intersected cell-face from the result values associated with the nodes of that face. This is also the case for integration point results, as they are directly associated with their corresponding element node in ResInsight. 

#### Change Data Source for Plots and Curves

It is possible to change some data source parameters in one go for either a full plot or several selected curves. To change the parameters for a whole plot use either the **Data Source** group in the Property Editor for the Well Log Plot or corresponding toolbar which is visible when a Well Log Plot or any of its children are selected. Changing parameters in the **Data Source** group for the plot will also change the source for Zonation/Formations and Well Path Attributes in addition to the data source for all **Well Log Extraction Curves** and **Well Log LAS Curves**.

![]({{ site.baseurl }}/images/WellLogPlotDataSource.png)

To change data source for some curves, select the curves you wish to change source for and select **Change Data Source** command form the context menu. The following dialog will appear:

![]({{ site.baseurl }}/images/WellLogExtractionChangeDataSource.png)

In both cases, the following parameters are available to change:
- **Case** -- Applies the selected case to all the curves.
- **Trajectory Type** -- Sets whether to use Simulation Wells or Well Paths as a data source for all curves.
- **Well Name** -- Applies this well path to all the curves.
- **Time Step** -- Applies this time step to all the curves.

Common for the different ways of changing data source is that if a parameter is not shared among all the curves, the drop down list will show "Mixed Cases, "Mixed Trajectory Types", "Mixed Well Paths" or "Mixed Time Steps" to indicate that the curves have different values for that parameter. It is still possible to select a common parameter for them which will then be applied across the curves.

### Well Log RFT Curves

Well Log RFT Curves shows the values in a RFT file. See [RFT Plot]({{ site.baseurl }}/docs/rftplot) for details abuot RFT. A curve in a RFT plot will look identical to a RFT curve in a well log plot, if the depth type of the well log plot is TVD, and the interpolation type of the curve is *Point to Point*.

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
See [Importing Well Log Files]({{site.baseurl}}/docs/wellpaths#importing-well-log-files) for details on LAS file import.

### Exporting LAS-files

A set of curves can be exported to LAS files by right-clicking the curves, well log track, or well log plots and select **Export To LAS Files ...**. An export dialog is displayed, allowing the user to configure how to export curve data.

![]({{ site.baseurl }}/images/export_to_las_files.png)

- **Export Folder** -- Location of the exported LAS files, one file per unique triplet of well path, case and time step.
- **Resample Curve Data** -- If enabled, all curves are resampled at the given resample interval before export.
- **TVDRKB** -- If enabled, TVDRKB for all curves based on the listed well paths are exported. If the difference field is blank, no TVDRKB values are exported.

