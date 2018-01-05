---
layout: docs
title: Cell Filters
permalink: /docs/filters/
published: true
---
![]({{ site.baseurl }}/images/FiltersOverview.png)

Cell Filters are used to control visibility of the cells in the 3D view. Two types of filters exists:

- **Range Filter** -- Extracts an IJK subset of the model.
- **Property Filter** -- Extracts cells with a property value matching a value range.

<div class="note">
The visibilities of cells connection to wells, and fences based on these cells can be controlled from <b> <a href="{{ site.baseurl }}/docs/simulationwells">Simulation Wells</a> </b>.<br> 
<small><i>(Not applicable for Geomechanical cases)</i></small>
</div>

## Common Properties for Range and Property Filters

Both filter types can be turned on or off using the toggle in the **Project Tree** and controlled from their corresponding **Property Editor**.

![]({{ site.baseurl }}/images/FiltersInTreeView.png)

Range Filters and Property filters can either be set to **Include** cells or to **Exclude** them. 

The **Exclude** setting is used to explicitly remove cells from the visualization, regardless of what other filters say. 
The **Include** setting behaves differently for Range filters and Property Filters but marks the cells as visible.
The icon in front of the filters show a + or - sign to indicate the setting ![]({{ site.baseurl }}/images/FilterIncEx.png)


## Range filters

Range filters enables the user to define a set of visible regions in the 3D view based on IJK boxes.
Each *Include* range filter will *add more cells* to the visualization. The view will show the union of all the *Include* range filters.

A new range filter can be added by activating the context menu for the **Range Filters** collection in the **Project Tree**. 

<div class="note">
An I,J or K-slice range filter can be added directly from a Cell in the <b>3D View</b> by right-clicking the cell and using the context menu. 
</div>

Below is a snapshot of the **Property Editor** of the **Range Filter** :

![]({{ site.baseurl }}/images/RangeFilterProperties.png)

 - **Filter Type** -- The filter can either make the specified range visible ( *Include* ), or remove the range from the View ( *Exclude* ).
 - **Grid** --  This option selects which of the grids the range is addressing.
 - **Apply to Subgrids** -- This option tells ResInsight to use the visibility of the cells in the current grid to control the visibility of the cells in sub-LGR's. If this option is turned off, Sub LGR-cells is not included in this particular Range Filter.  
 
The **Start** and **Width** labels in front of the sliders features a number in parenthesis denoting maximum available value.<br>
The **Start** labels shows the index of the start of the active cells.<br>
The **Width** labels shows the number of active cells from the start of the active cells.

## Property Filters

**Property Filters** applies to the results of the **Range Filters** and limits the visible cells to the ones approved by the filter. For a cell to be visible it must be accepted by all the property filters. 

A new property filter can be made by activating the context menu on **Property Filters** or by right-clicking inside a 3D view. The new property filter is based on the currently viewed cell result by default. 

The name of the property filter is automatically set to *"propertyname (min .. max)"* as you edit the property filter.

<div class="note">
The context command <b>Apply As Cell Result</b> on a property filter, sets the Cell Color Result to the same values as the selected property filter.
</div>

Below is a snapshot of the **Property Editor** of the **Property Filter**.
  
![]({{ site.baseurl }}/images/PropertyFilterProperties.png)

### Property Value Range
The filter is based on a property value range (Min - Max). Cells in the range are either shown or hidden depending on the **Filter Type** (*Include*/*Exclude*). Exclude-filters removes the selected cells from the **View** even if some other filter includes them.

#### Range Behavior for Flow Diagnostic Results
Normally the available range in the sliders is the max and min of all the values in all the time steps. For Flow Diagnostics results, however, the available range is based on the current time step. 

We still need to keep the range somewhat fixed while moving from time step to time step, so in order to do so ResInsight tries to keep the intentions of your range settings, as the available range changes. If either the max or min value is set to the limit, ResInsight will keep that setting at the limit even when the limit changes. If you set a specific value for the max or the min, that setting will keep its value, even if it happens to end up outside the available range at a time step.   

### Category Selection
If the property is representing integer values, well tracer names or [ formation names ]({{ site.baseurl }}/docs/formations), the property filter displays a list of available categories used to filter cells. The separate values can then be toggled on or off using the list in the Property Editor.

![]({{ site.baseurl }}/images/PropertyFilterWithCategories.png)

If it is more convenient to filter the values using a value range, toggle the **Category Selection** option off.

