---
layout: docs
title: Faults
permalink: /docs/faults/
published: true
---
![]({{ site.baseurl }}/images/FaultsIllustration.png)

This section describes how Faults are detected and visualized. NNC's are a part of the Faults visualization and are thus also mentioned in this section. 

## Fault Detection

ResInsight always scans the grids for geometrical faults when they are loaded. When two opposite cell faces of I, J, K neighbor cells does not match geometrically, they are tagged. 

All the tagged cell faces are then compared to the faults possibly imported from the _`*.DATA`_ file in order to group them. If a particular face is *not* found among the fault faces defined in the _`*.DATA`_ file (or their opposite faces), the cell face is added to one of two predefined faults: 

- **Undefined grid faults** 
- **Undefined grid faults With Inactive** 
 
The first fault is used if both the neighbor cells are active. If one or both of the neighbor cells are inactive, the second fault is used. 

These particular Faults will always be present, even when reading of fault information from the _`*.DATA`_ file is disabled.

### Information from `*.DATA`-files

#### Fault Information
If enabled in **Preferences**, ResInsight will import fault information from the _`*.DATA`_ files and use this information to group the cell faces into named items. The imported faults are ordered in ascending order based on their name.

<div class="note info">
The <b>DATA</b> file is parsed for the <b>FAULT</b> keyword while respecting any <b>INCLUDE</b> and <b>PATH</b> keywords.<br>
As import of faults can be time consuming, reading of faults can be disabled from <b>Preferences->Import faults</b>
</div>

#### NNC Data
If enabled in **Preferences**, ResInsight will read Non Neighbor Connections from the Eclipse output file (_`*.INIT`_), and create explicit visualizations of those. 
The NNC's are sorted onto the Fault's and their visibility is controlled along with them.

## Fault Visualization Options

### Fault Visibility
Faults can be hidden and shown in several ways. 

- Checking or unchecking the checkbox in front of the fault will show or hide the fault. 
- Visibility for multiple faults can be controlled at the same time by selecting multiple faults and use the context menu: **On**, **Off** and **Toggle**. 
- Right-clicking a Fault in the 3D View will enable a context menu with a command to hide the fault.

### Fault Color
Each named Fault is given a color on import. This color can be controlled by selecting the fault and edit its  **Fault color** in the **Property Editor.**

### Separate Fault Result
The default result mapping used on faults are to use the same as specified in **Cell Result**. If a different result mapping is wanted, enable the checkbox at **Separate Fault Result** and select the result from the result selection dialog in the **Property Editor**. A second legend for the fault result is then added to the view.

### Toolbar Control
Visualization mode and mesh lines can be controlled from the toolbar.

- ![]({{ site.baseurl }}/images/draw_style_faults_24x24.png) **Faults-Only** visualization mode. 
   <br>When turned on, this option hides all the grid cells, and shows only the fault faces in the reservoir limited by the applied range and property filters. (Unless **Show faults outside filters** are turned on. See below.)
- ![]({{ site.baseurl }}/images/draw_style_surface_24x24.png) Turns faces on and mesh off
- ![]({{ site.baseurl }}/images/draw_style_surface_w_fault_mesh_24x24.png) Turns on all faces, and shows mesh lines on faults only.
   <br> This is a useful method to highlight the faults in your reservoir, because the faults stands out with black outlining. 
- ![]({{ site.baseurl }}/images/draw_style_faults_label_24x24.png) Shows labels for faults  

### Faults Properties
By clicking the ![]({{ site.baseurl }}/images/draw_style_faults_24x24.png) **Faults** item in the **Project Tree**, the following options common to all the faults are displayed: 

 ![]({{ site.baseurl }}/images/FaultProperties.png)
 
##### Fault Labels
- **Show labels** -- Displays one label per fault with the name defined in the _`*.DATA`_ file
- **Label color** -- Defines the label color
 
##### Fault Options
- **Show faults outside filters** -- Turning this option on, will display faults outside the filter region, making the fault visualization completely ignore the Range and Property filters in action.

##### Fault Face Visibility
This group of options controls the visibility of the fault faces. Since they work together, and in some cases are overridden by the system, they can be a bit confusing. 

First of all, these options are only available in **Faults-only** visualization mode ( See [Toolbar Control](#toolbar-control) ). When not in **Faults-Only** mode, ResInsight overrides the options, and the controls are inactive. 

Secondly, the option you would normally want to adjust is **Dynamic Face Selection** ( See below ).

- **Show defined faces** -- Displays the fault cell faces that are defined on the Eclipse input file (_`*.DATA`_)
- **Show opposite faces** -- Displays the opposite fault cell faces from what is defined on the input file, based on IJK neighbors.  
  *These two options should normally be left **On**. They are useful when investigating the exact faults information provided on the `*.DATA` file. If you need to use them, it is normally wise to set the **Dynamic Face Selection** to "Show Both".*
- **Dynamic Face Selection** -- At one particular position on a fault there are usually two cells competing for your attention: The cell closer to you as the viewer, or the one further from you. When showing results, this becomes important because these two cell faces have different result property values, and thus color.  
  This option controls which of the two cell faces you actually can see: The one behind the fault, or the one in front of the fault. There is also an option of showing both, which will give you an undefined mixture, making it hard to be certain what you see.  
  This means that ResInsight turns on or off the faces based on your view position and this option to make sure that you always see the faces (and thus the result property) you request.

##### NNC Visibility
 
- **Show NNCs** -- Toggles whether to display the Non Neighbor Connections, or not.
- **Hide NNC geometry if no NNC result is available** -- Automatically hides NNC geometry if no NNC results are available

<div class="note info">
The color of the NNC faces are set to be a bit lighter than their corresponding named fault, and can not be controlled directly.
</div>

## Fault Export

Faults can be exported to separate files in the _`*grdecl`_ file format. This is useful for example if you need a list of the geometrically detected faults that has not been covered by entries in the eclipse FAULTS keyword.  

To export some faults, select the faults you want to export in the **Project Tree**, right-click them and select the command **Export Faults ...** from the context menu.

 ![]({{ site.baseurl }}/images/ExportFaultsMenu.png)

You are then prompted to select a destination folder. Each Fault is exported to a file named _`Faults_<fault name>_<case name>.grdecl`_ and stored in the selected folder. 

The fault name of **Undefined Grid Faults** is simplified to _`UNDEF`_, while **Undefined Grid Faults With Inactive** is simplified to _`UNDEF_IA`_. All other faults keep their original name.


