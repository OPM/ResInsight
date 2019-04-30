---
layout: docs
title: Getting Started
permalink: /docs/gettingstarted/
published: true
---

### User Interface Overview 

ResInsight has two main windows. One for 3D related visualization, and one for 2D graphs and plots. These ares shown in the images below. 

##### 3D Main Window
![ResInsight User Interface]({{ site.baseurl }}/images/ResInsightUIMediumSize.png)

##### Plot Main Window
![ResInsight User Interface]({{ site.baseurl }}/images/ResInsightMainPlotMediumSize.png)

#### Switching Between the Two Main Windows

The two main windows has a toolbar button each, that directly opens and raises the other window.
![3D Main Window]({{ site.baseurl }}/images/3DWindow24x24.png)
![Plot Main Window]({{ site.baseurl }}/images/PlotWindow24x24.png)

Each of the windows can also be closed freely, but if both are closed, ResInsight exits.

#### Docking Windows

Each of the main windows has a central area and several docking windows surrounding it. The different docking 
windows can be managed from the **Windows** menu or directly using the local menu bar of the docking window.

- **Project Tree** -- contains all application objects in a tree structure.
- **Property Editor** -- displays all properties for the selected object in the **Project Tree**
- **Process Monitor** -- displays output from Octave when executing Octave scripts
- **Result Info** -- displays info for the selected object in the 3D scene
- **Result Plot** -- displays curves based on result values for the selected cells in the 3D scene
- **Messages** -- displays occasional info and warnings related to operations executed.

Result Info and Result Plot is described in detail in [ Result Inspection ]({{ site.baseurl }}/docs/resultinspection)

<div class="note">
<h5>Use several Project Trees and Property Editors</h5>
If you want to pin the property editor for a certain object, you can add 
a new Project Tree and Property Editor by using the command <b>Windows->New Project and Property View</b>.
</div>

### Toolbars 

A selected subset of actions are presented as controls in the toolbar. The different sections in the toolbar can be dragged and positioned anywhere as small floating toolbars. Management of the toolbar is done by right-clicking on the toolbar and then manipulating the displayed menu.

#### Managing 3D Views and Plot Windows 

In the main area of the application, several 3D views or plot windows can be open at the same time. One of them will be active and the active view can be either maximized to use the whole main area, or restored so that you can see the open windows.

Standard window management for applying normal and maximized state is available in the upper right corner.

![Restore Down]({{ site.baseurl }}/images/RestoreDown.PNG)

Commands to arrange the windows in the standard ways are available from the **Windows** menu

- **Tile Windows** -- distribute all open view windows to fill available view widget space
  - The order of the tiled windows are determined by the window positions and the type of view at the time of running the tile command. The leftmost window are tiled first, then the next leftmost and so on. Master views are tiled before slave views.
- **Cascade Windows** -- organize all open view windows slightly offset on top of each other
- **Close All Windows** -- close all open view windows

When **Tile Windows** is activated, the windows will remain tiled until a view window is manually resized or another window arranging is selected.

#### Editing 3D Views and Plot Windows Content

Most of the settings and features of ResInsight is accessible through the **Project Tree** and the **Property Editor**. Selecting an item in the **Project Tree** activates the corresponding Window, and shows the item properties in the **Property Editor** available for editing. 

Toggling a checkbox next to an item in the **Project Tree** will toggle visibility in the window. Toggling a checkbox for a collection of items will affect the visibility for all items in the collection. ![]({{ site.baseurl }}/images/TreeViewToggle.png)

Context menu commands are also available to do special operations on a selected set of items.

How to interact and manipulate the 3D model is described in [Model Navigation]({{ site.baseurl }}/docs/modelnavigation)


### Cases and Their Types

A *Case* in ResInsight means a Grid model with a particular set of results or property data. There are three different types of Eclipse cases and one type of Geomechanical cases.

#### Eclipse Cases
There are three different Eclipse Case types: 

##### Result Case ![]({{ site.baseurl }}/images/Case24x24.png) 
This is a Case based on the results of an Eclipse simulation, read from a grid file together with static and restart data. Multiple Cases can be selected and read from a folder.

##### Input Case ![]({{ site.baseurl }}/images/EclipseInput24x24.png) 
This Case type is based on a _`*.GRDECL`_ file, or a part of an Eclipse *Input* file. This Case type supports loading single ASCII files defining Eclipse Cell Properties, and also to export modified property sets to ASCII files.
Each of the Eclipse properties are listed as separate entities in the **Project Tree**, and can be renamed and exported.
See [ Grid Import and Property Export ]({{ site.baseurl }}/docs/gridimportexport)

#####  Statistics Case ![]({{ site.baseurl }}/images/Histogram24x24.png)
This is a Case type that belongs to a *Grid Case Group* and makes statistical calculations based on the source cases in the Grid Case Group. See [ Grid Case Groups and Statistics ]({{ site.baseurl }}/docs/casegroupsandstatistics).

##### Summary Case ![]({{ site.baseurl }}/images/SummaryCase24x24.png)

This is the case type listed in the Plot Main Window, and represents an _`*.SMSPEC`_ file. These Cases are available for Summary Plotting. See [ Summary Plots ]({{ site.baseurl }}/docs/summaryplots).
 
#### Geomechanical cases ![]({{ site.baseurl }}/images/GeoMechCase24x24.png)

There are only one type of geomechanical cases, namely the ABAQUS-odb case type. The geomechanical cases are sorted into its own folder in the project tree named **Geomechanical Models** ![]({{ site.baseurl }}/images/GeoMechCases24x24.png) as opposed to the **Grid Models** folder where the Eclipse cases and **Grid Case Groups** resides.

#### Grid Case Groups ![]({{ site.baseurl }}/images/GridCaseGroup24x24.png) 

A **Grid Case Group** is a group of Eclipse **Result Cases** with identical grids, but generally different active cells, initial values and results. These cases are called *Source Cases*. The purpose of a Grid Case group is to make it easy to calculate statistics across the source cases both for static and dynamic Eclipse Properties. See [ Grid Case Groups and Statistics ]({{ site.baseurl }}/docs/casegroupsandstatistics).


### The Project File and the Cache Directory

ResInsight stores all the views and settings in a Project File with the extension: _`*.rsp`_.
This file only contains *references* to the real data files, and does not in any way copy the data itself. Data files generated by ResInsight are also referenced from the Project File.

Statistics calculations, octave generated property sets, and SSI-hub imported well paths are saved to a folder named _`<ProjectFileName>_cache`_ in the same directory as the project file. If you need to move your project, make sure you move this folder along. If you do not, the calculations or well path import needs to be done again.

<div class="note">
The <code>*.rsp</code> file is an XML file, and can be edited by any text editor.  
</div>

