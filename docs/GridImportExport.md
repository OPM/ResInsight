---
layout: docs
title: Grid Import and Property Export
permalink: /docs/gridimportexport/
published: true
---



### Importing Eclipse cases 
ResInsight supports the following type of Eclipse input data:

- `*.GRID` and `*.EGRID` files along with their `*.INIT` and restart files `*.XNNN` and `*.UNRST`. 
- Grid and Property data from  `*.GRDECL` files.

#### Eclipse Results
1. Select **File->Import-> ![]({{ site.baseurl }}/images/Case24x24.png) Import Eclipse Case**  and select an `*.EGRID` or `*.GRID` Eclipse file for import.
2. The case is imported, and a view of the case is created

<div class="note">
You can select several grid files in one go by multiple selection of files (Ctrl + left mouse button, Shift + left mouse button). 
</div>

#### Eclipse ASCII input data
1. Select **File->Import-> ![]({{ site.baseurl }}/images/EclipseInput24x24.png)Import Input Eclipse Case** and select a `*.GRDECL` file.
2. The case is imported, and a view of the case is created
3. Right click the **Input Properties** in the generated **Input Case** and use the context menu to import additional Eclipse Property data files.

#### Handling missing or wrong MAPAXES

The X and Y grid data can be negated in order to make the Grid model appear correctly in ResInsight. This functionality is accessible in the **Property Editor** for all Eclipse Case types as the toggle buttons **Flip X Axis** and **Flip Y Axis** as shown in the example below.
 
![]({{ site.baseurl }}/images/CaseProperties.png)

### Importing ABAQUS odb cases
When ResInsight is compiled with ABAQUS-odb support, `*.odb` files can be imported by selecting the command:

**File->Import-> ![]({{ site.baseurl }}/images/GeoMechCase24x24.png) Import Geo Mechanical Model** 

See [Build Instructions]({{ site.baseurl }}/docs/buildinstructions) on how to compile ResInsight with odb-support.


### Export of Eclipse Properties as ASCII data

Eclipse Properties can be exported to Eclipse ASCII files by activating the context 
menu for a **Cell Result**. ![]({{ site.baseurl }}/images/ExportProperty.png) 

The command will export the property set currently loaded and shown in the 3D View to a file with the following format:

    -- Exported from ResInsight
    <keyword>
    <One number per cell separated by spaces>
    /
