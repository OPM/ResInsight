---
layout: docs
prev_section: contact
next_section: casegroupsandstatistics
title: Working with 3D Views
permalink: /docs/reservoirviews/
published: true
---

3D Views are the windows displaying the Grid Models. The visualization is controlled by the **Project Tree** item representing the **View** and their subitems. Each item has a set of proerties that can be editied in the **Property View**.

![]({{ site.baseurl }}/images/3DViewOverview.png)

Below is a description of the most important View settings and their properties.

### Cell Result ![]({{ site.baseurl }}/images/CellResult.png)

The **Cell Result** item defines which Eclipse property the 3D View uses for the main cell color. The property can be chosen in the property panel of the **Cell Result** item. The mapping between cell values and color is defined by the **Legend Definition**  ![]({{ site.baseurl }}/images/Legend.png) along with some appearance settings on the Legend itself. (Number format etc.)

### TRANSXYZ
Normally the Cell Result setting gives one cell color based on the legend and the selected Result Property. This is *not* the case for the special static TRANXYZ property. This property gives each face the correct color based on the TRANS value that is associated with that particular face. 
The Positive I-face of the cell gets the cell TRANX value, while the J-face gets the TRANY-value etc. The negative faces, however, get the value from the neighbor cell in that direction. The negative I-face gets the TRANX value of the IJK-neighbor in negative I direction, and so on for the J- and K-faces.

### Cell Edge Results ![]({{ site.baseurl }}/images/EdgeResult_1.png)

The **Cell Edge Result** visualization mode is one of ResInsight's special features. Its main use is to show the MULT(X, Y, Z) properties at the same time. 
This will show the MULT property values *different from 1.0* along the correct edges of the cells. In effect this highlights the faults and makes it easy to verify all the MULT values in one go.

![]({{ site.baseurl }}/images/CellEdgeExample.png)

ResInsight supports all properties ending with X, Y, Z and X-, Y-, Z-. However, it is only the MULT property that ignores values of 1.0.

When selecting a result variable for cell edge, a second legend shows up in the 3D view showing the variation in values for this second property. Color legend management is available when selecting the **Legend Definition** item belonging to the **Cell Edge Result** item. 

### Info Box

The **Info Box** controls the visibility of the animation progress, the Case description box, and the results histogram.

The **Results Histogram** shows a histogram of the complete time series of the currently loaded **Cell Result** together with:

- The mean value ( a blue line ) 
- P10 and P90 ( red lines )

![]({{ site.baseurl }}/images/HistogramExample.png)


### Cell Filters
Cell Filters are used to control visibility of the cells in the 3D view. Three types of filters exists:

- **Range filter**     : Define a IJK subset of the model.
- **Property filter**  : Define a value range for a property to control cell visibility.
- **Well cell filter** : Display grid cells that has connections to a well. Controlled from the **Simulation Wells** item.

All filters can be turned on or off using the toggle in the **Project Tree** and controlled from their corresponding **Property Editor**.

#### Range filters

Using range filters enables the user to define a set of IJK visible regions in the 3D view.
A new range filter can be added by activating the context menu for the **Range Filters** collection in the **Project Tree**.

*TIP:* An I,J or K-slice range filter can be added directly from a Cell in the **3D View** by rightclicking the cell and using the context menu. 

Below is a snapshot of the **Property Editor** of the **Range Filter** :

![]({{ site.baseurl }}/images/RangeFilterProperties.png)

 - **Filter Type** : The filter can either make the specified range visible ( *Include* ), or remove the range from the View ( *Exclude* ).
 - **Grid** :  This option selects which of the grids the range is addressing.
 - **Apply to Subgrids** : This option tells ResInsight to use the visibility of the cells in the current grid to control the visibility of the cells in sub-LGR's. If this option is turned off, Sub LGR-cells is not included in this particular Range Filter.  
 
The **Start** and **Width** labels in front of the sliders features a number in parenthesis denoting maximum available value.<br>
The **Start** labels shows the index of the start of the active cells.<br>
The **Width** labels shows the number of active cells from the start of the active cells.

#### Property filters

**Property filters** apply to the results of the **Range filters**. Below is a snapshot of the **Property Editor** of the **Property Filter**.
  
![]({{ site.baseurl }}/images/PropertyFilterProperties.png)

This filter filters the cells based on a property value range (Min - Max). Cells in the range are either shown or hidden depending on the **Filter Type** ( *Include* / *Exclude* ). Exclude-filters removes the selected cells from the **View** even if some other filter includes them.

A new property filter can be made by activating the context menu for **Property Filters**. The new property filter is based on the currently viewed cell result by default.

### Simulation Wells

This item controls the overall settings for how wells in the Eclipse simulation are visualized.
The wells are shown in two ways:

1. A pipe through all cells with well connections
2. By adding the well cells to the set of visible cells 

The latter is handled internally as a special range filter, and adds cells to the set of range filtered cells.

The Property Editor of the **Simulation Wells** item is shown below: 

![]({{ site.baseurl }}/images/SimulationWellsProperties.png)



- **Add cells to range filter** This option controls how the well cells 
    (cells with connections to wells) are added to the set of range filtered cells.
  - *All On* will add the cells from all wells disregarding the individual settings on the well.
  - *All Off* will prevent any well cells to be added. 
  - *Individually* will respect the individual settings for each well, and add the cells from the wells with this option set on. 
-  **Use Well Fence** and 
-  **Well Fence direction** Controls whether to add extensions of the well cells in the I, J or K direction to the set of range filtered cells
- **Well head** These options control the appearance and position of the well labels and and symbols of the top of the well
- **Global Well Pipe Visibility** Controls if and when to show the pipe representation of the wells. The options are:
   - *All On* will show the pipes from all wells disregarding the individual settings on the well.
   - *All Off* will hide all simulation well pipes. 
		- *Individual* Will respect the individual settings for each well, and only show the well pipes from the wells with this option set on. See below.
		- *Visible Cells Filtered* This option will only show the pipes of wells that are connected to visible cells. That means the combined result of **Range Filters**, **Property Filters** and any **Well Range Filters**.
		*NOTE* : All Wells with **Well Range Filter** turned on will always be visible with this option selected. 
- **Pipe Radius Scale** Scaling the pipe radius by the average max cell size.
- **Geometry based Branch detection** Applies only to ordinary wells (not MSW) 
  and will detect that parts of a well really is a branch. Those well parts will 
  be visualized as a branch starting at the well head instead of at the previous connected cell.	 
		  	 
##### Well pipes of Multi Segment Wells

###### Geometry approximation
The pipe geometry generated for MSW's are based on the topology of the well (branch/segment structure) and the position of the cells being connected. The segment lengths are used as hints to place the branch points at sensible places. Thus the pipe geometry itself is not geometrically correct, but makes the topology of the well easier to see.

###### Dummy branches
Often MSW's are modeled using a long stem without connections and a multitude of small branches; one for each connection. ResInsight offsets the the pipe within the cell to clearly show how the topology of the well is defined.

![]({{ site.baseurl }}/images/MSWDummyBranchExample.png)

###### Picking reveals Segment/Branch info

Branch and segment info of a MSW-connected-Cell is shown in the **Result Info** window when picking a cell in the 3D View. This can be handy when relating the visualization to the input files.

### Individual Simulation Well options 

Each of the wells can have some individual settings. These options works as specializations of the ones set on the global level (**Simulation Wells** See above) but will *only come into play when they are not ignored by the global settings*.

This is particularly important to notice for the **Show Well Pipe** and **Range Filter** options. They will not have effect if the corresponding global settings in **Simulation Wells** allows them to.
 
The properties of a single well are shown below.

![]({{ site.baseurl }}/images/WellProperties.png)

One option needs further explanation:

- **Pipe Radius Scale** This option is a scale that is added to the "global" scale set in the **Simulation Wells** properties.
