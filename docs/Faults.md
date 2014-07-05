---
layout: docs
prev_section: wellpaths
next_section: batchcommands
title: Faults
permalink: /docs/faults/
published: true
---

ResInsight will detect all the cell faces with no geometrically matching neighbours, and display them as *Faults*. 
This means that the are drawn in special ways, and that their visual appearance can be controlled separately from the rest of the grid.

## Fault Names and NNC

### Import of Fault info from `*.DATA`-files
If enabled, ResInsight will also import fault information from the `*.DATA` files and use this information to group the detected faces into named items which is available in the ![]({{ site.baseurl }}/images/draw_style_faults_24x24.png) **Faults** item in the **Project Tree**. The imported faults are ordered in ascending order based on their name.

>
***Note:*** As import of faults can be time consuming, reading of faults can be disabled from **Preferences -> Read fault data**

### Undefined grid faults
All the detected cell faces are compared to the faults imported from the `*.DATA` file in order to group them. If a particular face is *not* found among the fault faces defined in the `*.DATA` file or it's opposite face, the cell face is added to a fault called **Undefined grid faults**. This particular Fault will always be present, even if reading of the `*.DATA` file is disabled.

### Fault color
Each named Fault is given a color on import. This color can be controlled by selecting the fault and edit its  **Fault color** in the **Property Editor.**

### NNC visualization
ResInsight will read Non Neighbor Connections from the Eclipse output file (`*.INIT`), and create explicit visualizations of those witch have a common surface area. These NNC's are then sorted onto the Fault's and their visibility is controlled from the **Property Editor** of the **Faults** Item in the **Project Tree**.

The color of the NNC faces are set to be a bit lighter than their corresponding named fault, and can not be controlled directly.

Currently the only result property that is mapped onto the NNC is the static TRANSXYZ property which displays the transmissibility associated to each face.

### Picking info

When clicking on a cell face that is member of a fault, the fault name is displayed in the **Result Info** window, along with cell, and result property info. 

When clicking on a NNC,  the relevant data for this NNC is displayed.

## Fault visualization options


### Toolbar control
Visualization mode and mesh lines can be controlled from the toolbar.

- ![]({{ site.baseurl }}/images/draw_style_faults_24x24.png) **Faults-Only** visualization mode. 
   <br>When turned on, this option hides all the grid cells, and shows only the fault faces in the reservoir limited by the applied range and property filters. (Unless **Show faults outside filters** are turned on. See below.)
- ![]({{ site.baseurl }}/images/draw_style_surface_24x24.png) Turns faces on and mesh off
- ![]({{ site.baseurl }}/images/draw_style_surface_w_fault_mesh_24x24.png) Turns on all faces, and shows meshlines on faults only.
   <br> This is a useful method to highlight the faults in your reservoir, because the faults stands out with black outlining. 
- ![]({{ site.baseurl }}/images/draw_style_faults_label_24x24.png) Shows labels for faults  

### Common Fault Options
By clicking the ![]({{ site.baseurl }}/images/draw_style_faults_24x24.png) **Faults** item in the **Project Tree**, the following options common to all the faults are displayed: 

 ![]({{ site.baseurl }}/images/FaultProperties.png)
 
#### Fault labels
- **Show labels**: Displays one label per fault with the name defined in the `*.DATA`-file
- **Label color**: Defines the label color
 
#### Fault options
- **Show faults outside filters**: Turning this option on, will display faults outside the filter region, making the fault visualization completely ignore the Range and Property filters in action.
- **Show results on faults**: This toggle controls whether to show the selected result property on the faults or not. This should normally be left on.
- **Show NNCs**: Toggles whether to display the Non Neighbor Connections, or not.

#### Fault Face Visibility
This group of options controls the visibility of the fault faces. Since they work together, and in some cases are overridden by the system, they can be a bit confusing. 

First of all. These options are only available in **Faults-only** visualization mode. ( See *Toolbar Control* above) When not in **Faults-Only** mode, ResInsight overrides the options, and the controls are inactive. 

Secondly: The option you would normally want to adjust is **Dynamic Face Selection** (See below).

- **Show defined faces**: Displays the fault cell faces that are defined on the Eclipse input file (`*.DATA`)
- **Show opposite faces**: Displays the opposite fault cell faces from what is defined on the input file, based on IJK neighbors.
 <br> *These two options should normally be left **On***. <br>They are useful when investigating the exact faults information provided on the `*.DATA` file. 
 <br>If you need to use them, it is normally wise to set the **Dynamic Face Selection** to "Show Both"

##### Dynamic Face Selection

At one particular position on a fault there are usually two cells competing for your attention: The cell closer to you as the viewer, or the one further from you. When showing results, this becomes important because these two cell faces have different result property values, and thus color. 

This option controls which of the two cell faces you actually can see: The one behind the fault, or the one in front of the fault. There is also an option of showing both, which will give you an undefined mixture, making it hard to be certain what you see.  

This means that ResInsight turns on or off the faces based on your view position and this option to make sure that you always see the faces (and thus the result property) you request.