---
layout: docs
title: Result Inspection
permalink: /docs/resultinspection/
published: true
---
![]({{ site.baseurl }}/images/ResultInspectionOverview.png)

The results mapped on the 3D model can be inspected in detail by left clicking cells in the 3D view. 
The selected cells will be highlighted, text information displayed in the **Result Info** docking window, and the time-history values plotted in the **Result Plot**, if available.

<div class="note">
Visibility of the docking widows can be controlled from the <b>Windows</b> menu.
</div>

## Result Info information

Clicking cells will display slightly different information text based on the case type as described in the following tables:

### Eclipse model

Geometry      | Description
--------------|------------
Reservoir cell| Displays grid cell result value, cell face, grid index and IJK coordinates for the cell. The intersection point coordinates is also displayed. Additional result details are listed in the section **-- Grid cell result details --**
Fault face    | Displays the same info as for a *Reservoir cell*. In addition the section **-- Fault result details --** containing Fault Name and Fault Face information.
Fault face with NNC | Displays the same info as *Fault face*, except the Non Neighbor Connections (NNC) result value is displayed instead of grid cell value. Information added in section **-- NNC details --** is geometry information of the two cells connected by the NNC.
Formation names| Displays name of formation the cell is part of

### Geomechanical model

Name                   | Description
-----------------------|------------
Closest result         | Closest node ID and result value
Element                | Element ID and IJK coordinate for the element
Intersection point     | Location of left-click intersection of the geometry
Element result details | Lists all integration point IDs and results with associated node IDs and node coordinates
Formation names        | Displays name of formation the cell is part of

## Result Plot

If a dynamic none-Flow Diagnostics result is active, the result values of the selected cells for all time steps are displayed in the docking window **Result Plot** as one curve for each cell. 

Additional curves can be added to the plot if CTRL-key is pressed during picking. The different cells are highlighted in different colors, and the corresponding curve is colored using the same color.

To clear the cell-selection, left-click outside the visible geometry.

### Adding the curves to a Summary plot

The time history curves of the selected cells can be added to a Summary Plot by right-clicking in the **Result Plot** or in the 3D View.

![]({{ site.baseurl }}/images/ResultPlotToSummaryPlotCommand.png)

A dialog will appear to prompt you to select an existion plot, or to create a new one.

![]({{ site.baseurl }}/images/ResultPlotToSummaryPlotDialog.png)

