---
layout: docs
title: Simulation Wells
permalink: /docs/simulationwells/
published: true
---

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
		  	 
## Well pipes of Multi Segment Wells

### Geometry approximation
The pipe geometry generated for MSW's are based on the topology of the well (branch/segment structure) and the position of the cells being connected. The segment lengths are used as hints to place the branch points at sensible places. Thus the pipe geometry itself is not geometrically correct, but makes the topology of the well easier to see.

### Dummy branches
Often MSW's are modeled using a long stem without connections and a multitude of small branches; one for each connection. ResInsight offsets the the pipe within the cell to clearly show how the topology of the well is defined.

![]({{ site.baseurl }}/images/MSWDummyBranchExample.png)

### Picking reveals Segment/Branch info

Branch and segment info of a MSW-connected-Cell is shown in the **Result Info** window when picking a cell in the 3D View. This can be handy when relating the visualization to the input files.

## Individual Simulation Well options 

Each of the wells can have some individual settings. These options works as specializations of the ones set on the global level (**Simulation Wells** See above) but will *only come into play when they are not ignored by the global settings*.

This is particularly important to notice for the **Show Well Pipe** and **Range Filter** options. They will not have effect if the corresponding global settings in **Simulation Wells** allows them to.
 
The properties of a single well are shown below.

![]({{ site.baseurl }}/images/WellProperties.png)

One option needs further explanation:

- **Pipe Radius Scale** This option is a scale that is added to the "global" scale set in the **Simulation Wells** properties.


