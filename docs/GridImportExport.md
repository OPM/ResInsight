---
layout: docs
prev_section: reservoirviews
next_section: simulationwells
title: Grid Import
permalink: /docs/gridimportexport/
published: true
---


ResInsight supports the following type of Eclipse input data:
- `*.GRID` and `*.EGRID` files along with their `*.INIT` and restart files `*.XNNN` and `*.UNRST`. 
- Grid and Property data from  `*.GRDECL` files.

### Importing Eclipse cases 

#### Eclipse Results
1. Select **File->Import->Import Eclipse Case** and select an `*.EGRID` or `*.GRID` Eclipse file for import.
2. The case is imported, and a view of the case is created

*TIP:* You can select several grid files in one go by multiple selection of files( Ctrl + left mouse button, Shift + left mouse button). 

#### Eclipse ASCII input data
1. Select **File->Import->Import Input Eclipse Case** and select a `*.GRDECL` file.
2. The case is imported, and a view of the case is created
3. Right click the **Input Properties** in the generated **Input Case** and use the context menu to import additional Eclipse Property data files.

#### Handling missing or wrong MAPAXES

The X and Y grid data can be negated in order to make the Grid model appear correctly in ResInsight. This functionality is accessible in the **Property Editor** for all Case types as the toggle buttons **Flip X Axis** and **Flip Y Axis** as shown in the example below.
 
![]({{ site.baseurl }}/images/CaseProperties.png)


## Export of Eclipse Properties as ASCII data
Eclipse Properties can be exported to Eclipse ASCII files by activating the context menu for a **Cell Result**. ![]({{ site.baseurl }}/images/ExportProperty.png) 

The command will export the property set currently loaded and shown in the 3D View to a file with the following format:

    -- Exported from ResInsight
    <keyword>
    <One number per cell separated by spaces>
    /