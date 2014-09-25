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
- **Well cell filter** : Display grid cells that has connections to a well. Controlled from the **Simulation Wells** item.

All filters can be turned on or off using the toggle in the **Project Tree** and controlled from their corresponding **Property Editor**.

## Range filters

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

## Property filters

**Property filters** apply to the results of the **Range filters**. Below is a snapshot of the **Property Editor** of the **Property Filter**.
  
![]({{ site.baseurl }}/images/PropertyFilterProperties.png)

This filter filters the cells based on a property value range (Min - Max). Cells in the range are either shown or hidden depending on the **Filter Type** ( *Include* / *Exclude* ). Exclude-filters removes the selected cells from the **View** even if some other filter includes them.

A new property filter can be made by activating the context menu for **Property Filters**. The new property filter is based on the currently viewed cell result by default.
