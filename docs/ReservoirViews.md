---
layout: docs
title: 3D Views
permalink: /docs/reservoirviews/
published: true
---
![]({{ site.baseurl }}/images/3DViewOverview.png)

3D Views are the windows displaying the Grid Models. The visualization is controlled by the **Project Tree** item representing the **View** and their sub items.

![]({{ site.baseurl }}/images/ViewTree.png)

Each item has a set of properties that can be edited in the **Property Editor**.

Several views can be added to the same case by right-clicking the case or a view and select **New View**. You can also **Copy** and then **Paste** a view into a Case. All the settings are then copied to the new view.  

Views of Eclipse models and Geomechanical models has a lot in common, but Eclipse views has some features that applies to Eclipse simulations only.

## Common view features

### View properties

Grid appearance can be controlled from the **Property Editor** when a view is selected. This includes background color and z scaling. In addition, cell visibility controls of inactive and invalid cells.

![]({{ site.baseurl }}/images/ViewProperties.png) 

Visibility of the grid box with labels displaying the coordinates for the reservoir can also be controlled using **Show Grid Box**. 

### Cell Result &nbsp;![]({{ site.baseurl }}/images/CellResult.png)

The **Cell Result** item defines which Eclipse or Geomechanical property the 3D View uses for the main cell color. The property can be chosen in the property panel of the **Cell Result** item. The mapping between cell values and color is defined by the **Legend Definition**  ![]({{ site.baseurl }}/images/Legend.png) along with some appearance settings on the Legend itself. (Number format etc.)

Please refer to [Result Color Legend]({{ site.baseurl }}/docs/resultcolorlegend) for details.

### Range Filters &nbsp;![]({{ site.baseurl }}/images/CellFilter_Range.png) and Property Filters &nbsp;![]({{ site.baseurl }}/images/CellFilter_Values.png) 

In order to see different sets of cells, and cells inside the reservoir, Views use cell filters. Please refer to [Cell Filters]({{ site.baseurl }}/docs/filters) to read more about them.

### Info Box

The **Info Box** controls the visibility of the animation progress, the Case description box, and the results histogram displayed in the top right corner of the view.

The **Animation Progress** shows which time step you are viewing. 

![]({{ site.baseurl }}/images/AnimationProgress.png)

The **Info Text** shows general info about the case, the selected results, and some statistics. **Mobile Volume Weighted Mean** is the mean of the current **Cell Property** weighted by the **Mobile pore volume**. Mobile pore volume is defined in every cell as: MULTPV _*_ PORV(1-SWCR). If MULTPV data is not present, it is ignored in the equation. The same applies to SWCR data.

![]({{ site.baseurl }}/images/infoBox.png)

The **Histogram** shows a histogram of the complete time series of the currently loaded **Cell Result** together with:

- The mean value ( a blue line ) 
- P10 and P90 ( red lines )

![]({{ site.baseurl }}/images/HistogramExample.png)

#### Statistics Options

- **Statistics Time Range** -- controls whether all time steps or only the current time step are included when statistics is computed. Flow Diagnostic results can only use the current time step option.  
- **Statistics Cell Range** -- controls if visible cells or all active cells is included when statistics is computed.

<div class="note">
The <b>Info Box</b> settings can be activated by clicking on the Info Text in the 3D view.
</div>

### Grids &nbsp;![]({{ site.baseurl }}/images/draw_style_meshlines_24x24.png)

![]({{ site.baseurl }}/images/ViewTreeGrids.png)

The **Grids** node contains child nodes for **Main Grid**, **LGRs** and **Temporary LGRs** where each grid's visibilty can be controlled. The **LGRs** node contains all LGRs loaded from file, while **Temporary LGRs** contains all temporary LGRs (see [Completions LGR]({{site.baseurl}}/docs/completionslgr)).
Toggling **Grids** off will hide all grids cell geometry. This option is used to display geometry for faults and intersections. This feature is also available as a toolbutton called *Hide grid cells*.

### Intersections &nbsp;![]({{ site.baseurl }}/images/CrossSections16x16.png)
Intersections are used to cut the geometry and show result values mapped onto this geometry.
Please refer to [Intersections]({{ site.baseurl }}/docs/intersections) for details.

### Activate items by clicking
Selected overlay items in the 3D view can activate a corresponding item in the **Property Editor**. This is implemented for Info box and result color legends. Please note that this feature is activated by clicking inside the texture/colored part of the legend. 


## Eclipse-only features

### Cell Edge Results ![]({{ site.baseurl }}/images/EdgeResult_1.png)

![]({{ site.baseurl }}/images/CellEdgeExample.png)

The **Cell Edge Result** visualization mode is one of ResInsight's special features. Its main use is to show the MULT(X, Y, Z) properties at the same time. This will show the MULT property values *different from 1.0* along the correct edges of the cells. In effect this highlights the faults and makes it easy to verify all the MULT values in one go.

ResInsight supports all properties ending with X, Y, Z and X-, Y-, Z-. However, it is only the MULT property that ignores values of 1.0.

When selecting a result variable for cell edge, a second legend shows up in the 3D view showing the variation in values for this second property. Color legend management is available when selecting the **Legend Definition** item belonging to the **Cell Edge Result** item. 

Select **Custom Edge Result** to specify one cell result to be mapped onto all cell edges. This way two cell results can easily be compared and visualized in the same view. The **Custom Edge Result** can either be a static or dynamic result.

### Separate Fault Result

Default result mapping on faults is using the result specified in **Cell Result**. If a different result mapping is wanted, enable the checkbox and select the result from the result selection dialog in the **Property Editor**. A second legend for the fault result is added to the view.

### Simulation Wells
This item controls the visualization of the Eclipse simulation wells.
Please refer to [Simulation Wells]({{ site.baseurl }}/docs/simulationwells) to read more.

### Well Connection Factors
This item controls the visualization of well connection factors.
See [Visualization and Inspection of Well Connection Factors]({{site.baseurl}}/docs/completions/#visualization-and-inspection-of-well-connection-factors) for details.

### Fractures
This item controls the visualization of fractures.
See [Fractures]({{site.baseurl}}/docs/completions/#fractures) for details.

### Faults
Visualization of the faults in the model is controlled by this item. 
Please refer to [Faults]({{ site.baseurl }}/docs/faults) to read more.
