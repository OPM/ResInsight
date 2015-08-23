---
layout: docs
prev_section: faults
next_section: wellpaths
title: Cell Filters
permalink: /docs/filters/
published: true
---

Cell Filters are used to control visibility of the cells in the 3D view. Three types of filters exists:

- **Range filter**     : Define a IJK subset of the model.
- **Property filter**  : Define a value range for a property to control cell visibility.
- **Well cell filter** : Display grid cells that has connections to a well. 

### Well cell filters
Well cell filters are a special type of filters that are controlled from the **Simulation Wells** item only. They are not applicable for Geomechanical cases.

### Common properties for Range and Property Filters

Both filter types can be turned on or off using the toggle in the **Project Tree** and controlled from their corresponding **Property Editor**.

Range Filters and Property filters can either be set to **Include** cells or to **Exclude** them. 

The **Exclude** setting is used to explicitly remove cells from the visualization, regardless of what other filters say. 
The **Include** setting behaves differently for Range filters and Property Filters but marks the cells as visible.
The icon in front of the filters show a + or - sign to indicate the setting ![]({{ site.baseurl }}/images/FilterIncEx.png)

### Range filters

Range filters enables the user to define a set of visible regions in the 3D view based on IJK boxes.
Each *Include* range filter will *add more cells* to the visualization. The view will show the union of all the *Include* range filters.

A new range filter can be added by activating the context menu for the **Range Filters** collection in the **Project Tree**. 

<div class="note">
An I,J or K-slice range filter can be added directly from a Cell in the <b>3D View</b> by rightclicking the cell and using the context menu. 
</div>

Below is a snapshot of the **Property Editor** of the **Range Filter** :

![]({{ site.baseurl }}/images/RangeFilterProperties.png)

 - **Filter Type** : The filter can either make the specified range visible ( *Include* ), or remove the range from the View ( *Exclude* ).
 - **Grid** :  This option selects which of the grids the range is addressing.
 - **Apply to Subgrids** : This option tells ResInsight to use the visibility of the cells in the current grid to control the visibility of the cells in sub-LGR's. If this option is turned off, Sub LGR-cells is not included in this particular Range Filter.  
 
The **Start** and **Width** labels in front of the sliders features a number in parenthesis denoting maximum available value.<br>
The **Start** labels shows the index of the start of the active cells.<br>
The **Width** labels shows the number of active cells from the start of the active cells.

### Property filters

**Property filters** applies to the results of the **Range filters** and limits the visible cells to the ones approved by the filter. For a cell to be visible it must be accepted by all the property filters. 

Below is a snapshot of the **Property Editor** of the **Property Filter**.
  
![]({{ site.baseurl }}/images/PropertyFilterProperties.png)

The filter is based on a property value range (Min - Max). Cells in the range are either shown or hidden depending on the **Filter Type** (*Include*/*Exclude*). Exclude-filters removes the selected cells from the **View** even if some other filter includes them.

A new property filter can be made by activating the context menu for **Property Filters**. The new property filter is based on the currently viewed cell result by default.

The name of the property filter is automatically set to *"propertyname (min .. max)"* as you edit the property filter.
