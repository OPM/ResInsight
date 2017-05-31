---
layout: docs
title: Flow Diagnostics Plots
permalink: /docs/flowdiagnosticsplots/
published: true
---
![]({{ site.baseurl }}/images/FlowDiagnosticsPlotsOverview.png)

Flow Diagnostics Plots are managed from the **Project Tree** of the **Plot Main Window** in the folder **Flow Diagnostics Plots**. This folder contains a **Flow Characteristics Plot**, a default **Well Allocation Plot** and a **Stored Plots** folder containing stored **Well Allocation Plots**.

![]({{ site.baseurl }}/images/FlowDiagnosticsPlotsProjectTree.png)

Please refer to [Cell Results-> Flow Diagnostic Results]({{ site.baseurl }}/docs/cellresults#flow-diagnostic-results) for more description of the results and references to more information about the methodology.

## Well Allocation Plots

Well allocation plots show the flow along a specified well, along with either phase distribution or the amount of support from/to other wells. The total phase or allocation is shown in the legend and as a pie chart, while the well flow is shown in a depth value vs flow graph.  

### Branches

Each branch of the well will be assigned a separate **Track**. For normal wells this is based on the branch detection algorithm used for Well Pipe visualization, and will correspond to the pipe visualization with **Branch Detection** *On*. ( See [Well Pipe Geometry]({{ site.baseurl }}/docs/simulationwells#well-pipe-geometry) )
Multi Segment Wells will be displayed according to their branch information, but tiny branches consisting of only one connection are lumped into the main branch to make the visualization more understandable. ( See  [Dummy branches]({{ site.baseurl }}/docs/simulationwells#dummy-branches) )

### Creating Well Allocation Plots

To plot the Well allocation for a well, right-click the well in the **Project Tree** or in the **3D View** and invoke the command **Plot Well Allocation**.

![]({{ site.baseurl }}/images/SimulationWellContextMenu.png)

The command updates the default **Well Allocation Plot** with new values based on the selection and the settings in the active view. This plot can then be copied to the **Stored Plots** folder by the context command **Add Stored Well Allocation Plot**. 

### Options

The **Legend**, **Total Allocation** pie chart, and the **Well Flow/Allocation** can be turned on or off from the toggles in the **Project Tree**. The other options are controlled from the property panel of a Well Allocation Plot:

![]({{ site.baseurl }}/images/WellAllocationProperties.png)

- **Name** -- Auto generated name used as plot title
- **Show Plot Title** -- Toggles whether to show the title in the plot
- **Plot Data** -- Options controlling when and what the plot is based on 
   - **Case** -- The case to plot data from 
   - **Time Step** -- The selected time step
   - **Well** -- The simulation well to plot
- **Options**
   - **Plot Type**
       - **Allocation** -- Plots *Reservoir well flow rates* along with how this well supports/are 
       supported by other wells.  
       ( This option is only available for cases with Flux results available. ) 
       - **Well Flow** -- Plots *Surface Well Flow Rates* together with phase split between Oil, Gas, and Water.
   - **Flow Type** 
       - **Accumulated** -- Plots an approximation of the accumulated flow along the well 
       - **Inflow Rates** -- Plots the rate of flow from the connection into the well
   - **Group Small Contributions** -- Groups small well contributions into a group called **Other**
   - **Threshold** -- Threshold used by the **Group Small Contributions** option.
   
### Depth Settings

The depth value in the plot can be controlled by selecting the **Accumulated Flow**/**Inflow Rates** item in the **Project Tree**. This item represents the Well-Log-like part of the Well Allocation Plot and its properties are shown below:

![]({{ site.baseurl }}/images/WellAllocationWellLogProperties.png)

- **Name** -- The plot name, updated automatically based on the **Flow Type** and well
- **Depth Type**
  - **Pseudo Length**  -- Use the length along the visualized simulation well pipe as depth. 
  In this mode the curves are extended somewhat above zero depth keeping the curve 
  values constant. This is done to make it easier to see the final values of the curves relative to each other.  
  The depth are calculated with **Branch detection** *On* and using the **Interpolated** well pipe geometry.  
  ( See [Well Pipe Geometry]({{ site.baseurl }}/docs/simulationwells#well-pipe-geometry) )
  - **TVD** -- Use True Vertical Depth on the depth-axis. 
  This will produce distorted plots for horizontal or near horizontal wells. 
  - **Connection Number** -- Use the number of connections counted from the top on the depth-axis.
- **Visible Depth Range** -- These options control the depth zoom
  - **Auto Scale** -- Toggles autoscale on/off. The plot is autoscaled when significant changes to its settings are made
  - **Min**, **Max** -- Sets the visible depth range. These are updated when zooming using the mouse wheel etc.
  
### Accessing the Plot Data

The command context command **Show Plot Data** will show a window containing the plot data in ascii format. The content of this window is easy to copy and paste into Excel or other tools for further processing.

It is also possible to save the ascii data to a file directly by using the context command **Export Plot Data to Text File** on the **Accumulated Flow**/**Inflow Rates** item in the **Project Tree**. 

The total accumulation data can also be viewed in ascci format by the command **Show Total Allocation Data**.

## Flow Characteristics Plot

This window displays three different graphs describing the overall behaviour of the reservoir for each timestep from a flow diagnostics point of view. 

The timesteps available are only those already calculated by the flow diagnostics solver. That means timesteps for which flow diagnostic results have been requested either by Cell Results, Well Allocation Plots, or Well Log Extraction Curves. 

![]({{ site.baseurl }}/images/FlowCharacteristicsPlot.png)

- **Lorenz Coefficient** -- This plot displays the Lorenz coefficient for the complete reservoir for each calculated timestep. The time step color is used as a reference for the timestep in the other graphs.
- **Flow Capacity vs Storage Capacity** -- This plot displays one curve for each timestep of the F-phi curve for the reservoir.
- **Sweep Efficiency** -- This plot displays one Sweep Efficiency curve for each calculated timestep.
