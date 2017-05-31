---
layout: docs
title: Grid Import
permalink: /docs/gridimportexport/
published: true
---

## Importing Eclipse cases 
ResInsight supports the following type of Eclipse input data:

- _`*.GRID`_ and _`*.EGRID`_ files along with their _`*.INIT`_ and restart files _`*.XNNN`_ and _`*.UNRST`_. 
- Grid and Property data from  _`*.GRDECL`_ files.

### Eclipse Results
1. Select **File->Import-> ![]({{ site.baseurl }}/images/Case24x24.png) Import Eclipse Case**  and select an _`*.EGRID`_ or _`*.GRID`_ Eclipse file for import.
2. The case is imported, and a view of the case is created

The **Reload Case** command can be used to reload a previously imported case, to make sure it is up to date. This is useful if the grid or result files changes while a ResInsight session is active.

<div class="note">
You can select several grid files in one go by multiple selection of files (Ctrl + left mouse button, Shift + left mouse button). 
</div>


### Eclipse ASCII input data
1. Select **File->Import-> ![]({{ site.baseurl }}/images/EclipseInput24x24.png) Import Input Eclipse Case** and select a _`*.GRDECL`_ file.
2. The case is imported, and a view of the case is created
3. Right-click the **Input Properties** in the generated **Input Case** and use the context menu to import additional Eclipse Property data files.

### Handling missing or wrong MAPAXES

The X and Y grid data can be negated in order to make the Grid model appear correctly in ResInsight. This functionality is accessible in the **Property Editor** for all Eclipse Case types as the toggle buttons **Flip X Axis** and **Flip Y Axis** as shown in the example below.
 
![]({{ site.baseurl }}/images/CaseProperties.png)

## Importing ABAQUS odb cases
When ResInsight is compiled with ABAQUS-odb support, _`*.odb`_ files can be imported by selecting the command:

**File->Import-> ![]({{ site.baseurl }}/images/GeoMechCase24x24.png) Import Geo Mechanical Model** 

See [Build Instructions]({{ site.baseurl }}/docs/buildinstructions) on how to compile ResInsight with odb-support.
