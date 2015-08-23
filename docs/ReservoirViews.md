---
layout: docs
prev_section: installationlinux
next_section: gridimportexport
title: Working with 3D Views
permalink: /docs/reservoirviews/
published: true
---

3D Views are the windows displaying the Grid Models. The visualization is controlled by the **Project Tree** item representing the **View** and their subitems. Each item has a set of properties that can be editied in the **Property Editor**.

![]({{ site.baseurl }}/images/3DViewOverview.png)

Several views can be added to the same case by right clicking the case or a view in the case and select **New View**. You can also **Copy** and then **Paste** a view into a Case. All the settings are then copied to the new view.  

Views of Eclipse models and Geomechanical models has a lot in common, but Eclipse views has some features that applies to eclipse simulations only.

## Common view features

### Cell Result &nbsp;![]({{ site.baseurl }}/images/CellResult.png)

The **Cell Result** item defines which Eclipse property the 3D View uses for the main cell color. The property can be chosen in the property panel of the **Cell Result** item. The mapping between cell values and color is defined by the **Legend Definition**  ![]({{ site.baseurl }}/images/Legend.png) along with some appearance settings on the Legend itself. (Number format etc.)

### Range Filters &nbsp;![]({{ site.baseurl }}/images/CellFilter_Range.png) and Property Filters &nbsp;![]({{ site.baseurl }}/images/CellFilter_Values.png) 

In order to see different sets of cells, and cells inside the reservoir, Views uses cell filters.
Please refer to [Cell Filters]({{ site.baseurl }}/docs/filters) to read more about them.

### Info Box

The **Info Box** controls the visibility of the animation progress, the Case description box, and the results histogram displayed in the top right corner of the view.

The **Animation Progress** shows which time step you are viewing. 

![]({{ site.baseurl }}/images/AnimationProgress.png)

The **Info Text** shows general info about the case, the selected results, and some statistics. 

![]({{ site.baseurl }}/images/CaseInfoText.png)

The **Histogram** shows a histogram of the complete time series of the currently loaded **Cell Result** together with:

- The mean value ( a blue line ) 
- P10 and P90 ( red lines )

![]({{ site.baseurl }}/images/HistogramExample.png)

## Eclipse-only features

### Cell Edge Results ![]({{ site.baseurl }}/images/EdgeResult_1.png)

The **Cell Edge Result** visualization mode is one of ResInsight's special features. Its main use is to show the MULT(X, Y, Z) properties at the same time. 
This will show the MULT property values *different from 1.0* along the correct edges of the cells. In effect this highlights the faults and makes it easy to verify all the MULT values in one go.

![]({{ site.baseurl }}/images/CellEdgeExample.png)

ResInsight supports all properties ending with X, Y, Z and X-, Y-, Z-. However, it is only the MULT property that ignores values of 1.0.

When selecting a result variable for cell edge, a second legend shows up in the 3D view showing the variation in values for this second property. Color legend management is available when selecting the **Legend Definition** item belonging to the **Cell Edge Result** item. 

### Separate Fault Result

Default result mapping on faults is using the result specified in **Cell Result**. If a different result mapping is wanted, enable the checkbox and select the result from the result selection dialog in the **Property Editor**. A second legend for the fault result is added to the view.

### Simulation Wells
This item controls the visualization of the Eclipse simulation wells.
Please refer to [Simulation Wells]({{ site.baseurl }}/docs/simulationwells) to read more.

### Faults
Visualization of the faults in the model is controlled by this item. 
Please refer to [Faults]({{ site.baseurl }}/docs/faults) to read more.
