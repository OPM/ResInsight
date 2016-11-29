---
layout: docs
title: Result Inspection
permalink: /docs/resultinspection/
published: true
---
![]({{ site.baseurl }}/images/ResultInspectionOverview.png)

The results mapped on the 3D model can be inspected in detail by left clicking the interesting cells in the 3D view. 
The selected cells will be highlighted and text information extracted from the intersection point will be displayed in the docking window **Result Info**.

{% comment %}  ![]({{ site.baseurl }}/images/ResultInfoWithSelectedCell.png) {% endcomment %}

If a dynamic result is active, the result values of the selected cells for all time steps are displayed in the docking window **Result Plot** as one curve for each cell. 

Additional curves can be added to the plot if CTRL-key is pressed during picking. The different cells are highlighted in different colors, and the corresponding curve is colored using the same color.

{% comment %} ![]({{ site.baseurl }}/images/ResultPlotWithSelectedCell.png) {% endcomment %}

To clear the cell-selection, left-click outside the visible geometry.

<div class="note">
Visibility of the docking widows can be controlled from the <b>Windows</b> menu.
</div>

## Result Info information
Clicking on different type of geometry will display slightly different information as described in the following tables:

### Eclipse model

Geometry      | Description
--------------|------------
Reservoir cell| Displays grid cell result value, cell face, grid index and IJK coordinates for the cell. The intersection point coordinates is also displayed. Additional result details are listed in the section **-- Grid cell result details --**
Fault face    | Displays the same info as for a *Reservoir cell*. In addition the section **-- Fault result details --** containing Fault Name and Fault Face information.
Fault face with NNC | Displays the same info as *Fault face*, except the Non Neighbor Connections (NNC) result value is displayed instead of grid cell value. Information added in section **-- NNC details --** is geometry information of the two cells connected by the NNC.
Formation names| Displays name of formation the cell is part of

### Geomechanical model

When clicking in the 3D scene, the selected geometry will be an element. 

Name                   | Description
-----------------------|------------
Closest result         | Closest node ID and result value
Element                | Element ID and IJK coordinate for the element
Intersection point     | Location of left-click intersection of the geometry
Element result details | Lists all integration point IDs and results with associated node IDs and node coordinates
Formation names        | Displays name of formation the cell is part of
